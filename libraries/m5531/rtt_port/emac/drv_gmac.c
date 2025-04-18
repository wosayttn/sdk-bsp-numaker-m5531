/**************************************************************************//**
*
* @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date            Author           Notes
* 2021-07-23      Wayne            First version
*
******************************************************************************/

#include "rtconfig.h"

#if defined(BSP_USING_EMAC)

#include <rtthread.h>
#include <rtdevice.h>
#include "lwipopts.h"
#include <netif/ethernetif.h>
#include <netif/etharp.h>
#include <lwip/sys.h>
#include <lwip/icmp.h>
#include <lwip/pbuf.h>
#include "NuMicro.h"
#include <string.h>
#include "synopGMAC_Host.h"

//#define DBG_ENABLE
#undef  DBG_ENABLE
#define DBG_LEVEL  LOG_LVL_ASSERT
#define DBG_SECTION_NAME  "drv_gmac"
#define DBG_COLOR
#include <rtdbg.h>

//#define NU_GMAC_DEBUG
#if defined(NU_GMAC_DEBUG)
    #define NU_GMAC_TRACE         rt_kprintf
#else
    #define NU_GMAC_TRACE(...)
#endif

enum
{
    GMAC_START = -1,
    GMAC0_IDX,
    GMAC_CNT
};

struct nu_gmac
{
    struct eth_device   eth;
    char               *name;
    uint32_t            base;
    IRQn_Type           irqn;
    rt_timer_t          link_timer;
    rt_uint8_t          mac_addr[8];
    synopGMACNetworkAdapter *adapter;
};
typedef struct nu_gmac *nu_gmac_t;

static struct nu_gmac nu_gmac_arr[] =
{
    {
        .name            =  "e0",
        .base            =  EMAC0_BASE,
        .irqn            =  EMAC0_IRQn,
    },
};

void nu_gmac_pkt_dump(const char *msg, const struct pbuf *p)
{
    rt_uint32_t i;
    rt_uint8_t *ptr = p->payload;

    NU_GMAC_TRACE("%s %d byte\n", msg, p->tot_len);

    for (i = 0; i < p->tot_len; i++)
    {
        if ((i % 8) == 0)
        {
            NU_GMAC_TRACE("  ");
        }
        if ((i % 16) == 0)
        {
            NU_GMAC_TRACE("\r\n");
        }
        NU_GMAC_TRACE("%02x ", *ptr);
        ptr++;
    }
    NU_GMAC_TRACE("\n\n");
}

static int nu_gmac_mdio_read(void *adapter, int addr, int reg)
{
    synopGMACdevice *gmacdev = ((synopGMACNetworkAdapter *)adapter)->m_gmacdev;
    u16 data;
    synopGMAC_read_phy_reg(gmacdev->MacBase, addr, reg, &data);
    return data;
}

static void nu_gmac_mdio_write(void *adapter, int addr, int reg, int data)
{
    synopGMACdevice *gmacdev = ((synopGMACNetworkAdapter *)adapter)->m_gmacdev;
    synopGMAC_write_phy_reg(gmacdev->MacBase, addr, reg, data);
}

static s32 synopGMAC_check_phy_init(synopGMACNetworkAdapter *adapter)
{
    struct ethtool_cmd cmd;
    synopGMACdevice *gmacdev = adapter->m_gmacdev;

    if (!mii_link_ok(&adapter->m_mii))
    {
        gmacdev->DuplexMode = FULLDUPLEX;
        gmacdev->Speed      = SPEED100;
        return 0;
    }
    else
    {
        mii_ethtool_gset(&adapter->m_mii, &cmd);
        gmacdev->DuplexMode = (cmd.duplex == DUPLEX_FULL)  ? FULLDUPLEX : HALFDUPLEX ;
        if (cmd.speed == SPEED_1000)
            gmacdev->Speed = SPEED1000;
        else if (cmd.speed == SPEED_100)
            gmacdev->Speed = SPEED100;
        else
            gmacdev->Speed = SPEED10;
    }

    return gmacdev->Speed | (gmacdev->DuplexMode << 4);
}

static void eth_rx_push(nu_gmac_t psNuGMAC)
{
    synopGMACNetworkAdapter *adapter = psNuGMAC->adapter;
    synopGMACdevice *gmacdev = (synopGMACdevice *) adapter->m_gmacdev;
    struct netif *netif = psNuGMAC->eth.netif;

    while (1)
    {
        PKT_FRAME_T *psPktFrame = NULL;
        struct pbuf *pbuf = RT_NULL;
        err_t ret;

        s32 s32PktLen = synop_handle_received_data(gmacdev, &psPktFrame);
        if (s32PktLen <= 0)
        {
            break;
        }

        /* Allocate a pbuf chain of pbufs from the pool. */
        if ((pbuf = pbuf_alloc(PBUF_RAW, s32PktLen, PBUF_POOL)) != NULL)
        {
#if defined(RT_USING_CACHE)
            rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, (void *)psPktFrame, s32PktLen);
#endif
            pbuf_take(pbuf, psPktFrame, s32PktLen);
        }
        else
        {
            rt_kprintf("[%s] drop the packet %08x\n", psNuGMAC->name, psPktFrame);
        }

        /* Free or drop descriptor. */
        synopGMAC_set_rx_qptr(gmacdev,
                              (u32)psPktFrame,
                              PKT_FRAME_BUF_SIZE,
                              0);

        if ((ret = netif->input(pbuf, netif)) != ERR_OK)
        {
            rt_kprintf("[%s] input error %08x err_t:%08x\n", psNuGMAC->name, pbuf, ret);
            pbuf_free(pbuf);
            break;
        }

    } //while(1)

}

static void nu_gmac_isr(int irqno, void *param)
{
    nu_gmac_t psNuGMAC = (nu_gmac_t)param;

    synopGMACNetworkAdapter *adapter = psNuGMAC->adapter;
    synopGMACdevice *gmacdev = (synopGMACdevice *)adapter->m_gmacdev;

    u32 interrupt, dma_status_reg;
    s32 status;
    u32 u32GmacIntSts;

    // Check GMAC interrupt
    u32GmacIntSts = synopGMACReadReg(gmacdev->MacBase, GmacInterruptStatus);
    if (u32GmacIntSts & GmacTSIntSts)
    {
        gmacdev->synopGMACNetStats.ts_int = 1;
        status = synopGMACReadReg(gmacdev->MacBase, GmacTSStatus);
        if (!(status & (1 << 1)))
        {
            NU_GMAC_TRACE("TS alarm flag not set?\n");
        }
        else
        {
            NU_GMAC_TRACE("TS alarm!\n");
        }
    }
    synopGMACWriteReg(gmacdev->MacBase, GmacInterruptStatus, u32GmacIntSts);


    dma_status_reg = synopGMACReadReg(gmacdev->DmaBase, DmaStatus);
    if (dma_status_reg == 0)
    {
        NU_GMAC_TRACE("dma_status ==0\n");
        return;
    }

    if (dma_status_reg & GmacPmtIntr)
    {
        NU_GMAC_TRACE("%s:: Interrupt due to PMT module", psNuGMAC->name);
        synopGMAC_powerup_mac(gmacdev);
    }

    if (dma_status_reg & GmacLineIntfIntr)
    {
        NU_GMAC_TRACE("%s: GMAC status reg is %08x mask is %08x\n",
                      psNuGMAC->name,
                      synopGMACReadReg(gmacdev->MacBase, GmacInterruptStatus),
                      synopGMACReadReg(gmacdev->MacBase, GmacInterruptMask));

        if (synopGMACReadReg(gmacdev->MacBase, GmacInterruptStatus) & GmacRgmiiIntSts)
        {
            NU_GMAC_TRACE("%s: GMAC RGMII status is %08x\n",
                          psNuGMAC->name,
                          synopGMACReadReg(gmacdev->MacBase, GmacRgmiiCtrlSts));
            synopGMACReadReg(gmacdev->MacBase, GmacRgmiiCtrlSts);
        }
    }

    /* DMA status */
    interrupt = synopGMAC_get_interrupt_type(gmacdev);
    if (interrupt & synopGMACDmaError)
    {
        NU_GMAC_TRACE("%s: 0x%08x@%08x\n", psNuGMAC->name, interrupt, dma_status_reg);
        NU_GMAC_TRACE("%s::Fatal Bus Error Interrupt Seen\n", psNuGMAC->name);
        synopGMAC_disable_dma_tx(gmacdev);
        synopGMAC_disable_dma_rx(gmacdev);

        synopGMAC_take_desc_ownership_tx(gmacdev);
        synopGMAC_take_desc_ownership_rx(gmacdev);

        synopGMAC_init_tx_rx_desc_queue(gmacdev);

        synopGMAC_reset(gmacdev);

        synopGMAC_set_mac_addr(gmacdev, GmacAddr0High, GmacAddr0Low, &psNuGMAC->mac_addr[0]);
        synopGMAC_dma_bus_mode_init(gmacdev, DmaBurstLength32 | DmaDescriptorSkip0/*DmaDescriptorSkip2*/ | DmaDescriptor8Words);
        synopGMAC_dma_control_init(gmacdev, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
        synopGMAC_init_rx_desc_base(gmacdev);
        synopGMAC_init_tx_desc_base(gmacdev);
        synopGMAC_mac_init(gmacdev);
        synopGMAC_enable_dma_rx(gmacdev);
        synopGMAC_enable_dma_tx(gmacdev);
    }

    if ((interrupt & synopGMACDmaRxNormal) ||
            (interrupt & synopGMACDmaRxAbnormal))
    {
        eth_rx_push(psNuGMAC);

        if (interrupt & synopGMACDmaRxAbnormal)
        {
            NU_GMAC_TRACE("%s: 0x%08x@%08x\n", psNuGMAC->name, interrupt, dma_status_reg);
            NU_GMAC_TRACE("%s::Abnormal Rx Interrupt Seen %08x\n", psNuGMAC->name, dma_status_reg);
            if (gmacdev->GMAC_Power_down == 0)
            {
                gmacdev->synopGMACNetStats.rx_over_errors++;
                synopGMACWriteReg(gmacdev->DmaBase, DmaStatus, DmaIntRxNoBuffer);
                synopGMAC_resume_dma_rx(gmacdev);
            }
        }
    }

    if (interrupt & synopGMACDmaRxStopped)
    {
        NU_GMAC_TRACE("%s: 0x%08x@%08x\n", psNuGMAC->name, interrupt, dma_status_reg);
        NU_GMAC_TRACE("%s::Receiver stopped seeing Rx interrupts\n", psNuGMAC->name); //Receiver gone in to stopped state
        if (gmacdev->GMAC_Power_down == 0)   // If Mac is not in powerdown
        {
            gmacdev->synopGMACNetStats.rx_over_errors++;
            synopGMAC_enable_dma_rx(gmacdev);
        }
    }

    //if (interrupt & synopGMACDmaTxNormal)
    //{
    //NU_GMAC_TRACE("%s::Finished Normal Transmission\n", psNuGMAC->name);
    //synop_handle_transmit_over(gmacdev);//Do whatever you want after the transmission is over
    //}

    if (interrupt & synopGMACDmaTxAbnormal)
    {
        NU_GMAC_TRACE("%s: 0x%08x@%08x\n", psNuGMAC->name, interrupt, dma_status_reg);
        NU_GMAC_TRACE("%s::Abnormal Tx Interrupt Seen\n", psNuGMAC->name);
        //if (gmacdev->GMAC_Power_down == 0)   // If Mac is not in powerdown
        //{
        //synop_handle_transmit_over(gmacdev);
        //}
    }

    if (interrupt & synopGMACDmaTxStopped)
    {
        NU_GMAC_TRACE("%s: 0x%08x@%08x\n", psNuGMAC->name, interrupt, dma_status_reg);
        NU_GMAC_TRACE("%s::Transmitter stopped sending the packets\n", psNuGMAC->name);
        if (gmacdev->GMAC_Power_down == 0)    // If Mac is not in powerdown
        {
            synopGMAC_disable_dma_tx(gmacdev);
            synopGMAC_take_desc_ownership_tx(gmacdev);
            synopGMAC_enable_dma_tx(gmacdev);
            NU_GMAC_TRACE("%s::Transmission Resumed\n", psNuGMAC->name);
        }
    }
}

void EMAC0_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    nu_gmac_isr(EMAC0_IRQn, &nu_gmac_arr[GMAC0_IDX]);

    /* leave interrupt */
    rt_interrupt_leave();
}

static void nu_phy_monitor(void *pvData)
{
    nu_gmac_t psNuGMAC = (nu_gmac_t)pvData;
    synopGMACNetworkAdapter *adapter = psNuGMAC->adapter;
    synopGMACdevice         *gmacdev = adapter->m_gmacdev;

    LOG_D("Create %s link monitor timer.", psNuGMAC->name);
    while (1)
    {
        if (!mii_link_ok(&adapter->m_mii))
        {
            if (gmacdev->LinkState)
            {
                eth_device_linkchange(&psNuGMAC->eth, RT_FALSE);
                LOG_I("%s: No Link", psNuGMAC->name);
            }
            gmacdev->DuplexMode = 0;
            gmacdev->Speed = 0;
            gmacdev->LoopBackMode = 0;
            gmacdev->LinkState = 0;
        }
        else
        {
            s32 data = synopGMAC_check_phy_init(adapter);
            if (gmacdev->LinkState != data)
            {
                synopGMAC_mac_init(gmacdev);
                LOG_I("Link is up in %s mode", (gmacdev->DuplexMode == FULLDUPLEX) ? "FULL DUPLEX" : "HALF DUPLEX");
                if (gmacdev->Speed == SPEED1000)
                {
                    LOG_I("Link is with 1000M Speed");
                    synopGMAC_set_mode(gmacdev, 0);
                }
                if (gmacdev->Speed == SPEED100)
                {
                    LOG_I("Link is with 100M Speed");
                    synopGMAC_set_mode(gmacdev, 1);
                }
                if (gmacdev->Speed == SPEED10)
                {
                    LOG_I("Link is with 10M Speed");
                    synopGMAC_set_mode(gmacdev, 2);
                }

                gmacdev->LinkState = data;
                eth_device_linkchange(&psNuGMAC->eth, RT_TRUE);
            }
        }

        //NU_GMAC_TRACE("%s: Interrupt enable: %08x, status:%08x\n", psNuGMAC->name, synopGMAC_get_ie(gmacdev), synopGMACReadReg(gmacdev->DmaBase, DmaStatus));
        //NU_GMAC_TRACE("%s: op:%08x\n", psNuGMAC->name, synopGMACReadReg(gmacdev->DmaBase, DmaControl));
        //NU_GMAC_TRACE("%s: debug:%08x\n", psNuGMAC->name, synopGMACReadReg(gmacdev->MacBase, GmacDebug));

        rt_thread_mdelay(1000);

    } //  while (1)

}

#if defined(RT_USING_CACHE)
    static NVT_NONCACHEABLE DmaDesc s_TXDescBufPool[TRANSMIT_DESC_SIZE];
    static NVT_NONCACHEABLE DmaDesc s_RXDescBufPool[RECEIVE_DESC_SIZE];
#endif

static void nu_memmgr_init(GMAC_MEMMGR_T *psMemMgr)
{
    psMemMgr->u32TxDescSize = TRANSMIT_DESC_SIZE;
    psMemMgr->u32RxDescSize = RECEIVE_DESC_SIZE;

#if defined(RT_USING_CACHE)

    psMemMgr->psTXDescs   = (DmaDesc *)&s_TXDescBufPool[0];
    psMemMgr->psRXDescs   = (DmaDesc *)&s_RXDescBufPool[0];

    psMemMgr->psTXFrames = (PKT_FRAME_T *) rt_malloc_align(sizeof(PKT_FRAME_T) * psMemMgr->u32TxDescSize, RT_ALIGN_SIZE);
    RT_ASSERT(psMemMgr->psTXFrames);
    LOG_D("[%s] First TXFrameAddr= %08x", __func__, psMemMgr->psTXFrames);

    psMemMgr->psRXFrames = (PKT_FRAME_T *) rt_malloc_align(sizeof(PKT_FRAME_T) * psMemMgr->u32RxDescSize, RT_ALIGN_SIZE);
    RT_ASSERT(psMemMgr->psRXFrames);
    LOG_D("[%s] First RXFrameAddr= %08x", __func__, psMemMgr->psRXFrames);

#else

    psMemMgr->psTXDescs = (DmaDesc *) rt_malloc_align(sizeof(DmaDesc) * psMemMgr->u32TxDescSize, RT_ALIGN_SIZE);
    RT_ASSERT(psMemMgr->psTXDescs);
    LOG_D("[%s] First TXDescAddr= %08x", __func__, psMemMgr->psTXDescs);

    psMemMgr->psRXDescs = (DmaDesc *) rt_malloc_align(sizeof(DmaDesc) * psMemMgr->u32RxDescSize, RT_ALIGN_SIZE);
    RT_ASSERT(psMemMgr->psRXDescs);
    LOG_D("[%s] First RXDescAddr= %08x", __func__, psMemMgr->psRXDescs);

    psMemMgr->psTXFrames = (PKT_FRAME_T *) rt_malloc_align(sizeof(PKT_FRAME_T) * psMemMgr->u32TxDescSize, RT_ALIGN_SIZE);
    RT_ASSERT(psMemMgr->psTXFrames);
    LOG_D("[%s] First TXFrameAddr= %08x", __func__, psMemMgr->psTXFrames);

    psMemMgr->psRXFrames = (PKT_FRAME_T *) rt_malloc_align(sizeof(PKT_FRAME_T) * psMemMgr->u32RxDescSize, RT_ALIGN_SIZE);
    RT_ASSERT(psMemMgr->psRXFrames);
    LOG_D("[%s] First RXFrameAddr= %08x", __func__, psMemMgr->psRXFrames);
#endif

}

static void nu_mii_init(synopGMACNetworkAdapter *adapter)
{
    /* MII setup */
    adapter->m_mii.phy_id_mask   = 0x1F;
    adapter->m_mii.reg_num_mask  = 0x1F;
    adapter->m_mii.adapter       = (void *)adapter;
    adapter->m_mii.mdio_read     = nu_gmac_mdio_read;
    adapter->m_mii.mdio_write    = nu_gmac_mdio_write;
    adapter->m_mii.phy_id        = adapter->m_gmacdev->PhyBase;
    adapter->m_mii.supports_gmii = mii_check_gmii_support(&adapter->m_mii);
}

static void nu_phy_link_monitor(nu_gmac_t psNuGMAC)
{
    rt_thread_t threadCtx = rt_thread_create(psNuGMAC->name,
                            nu_phy_monitor,
                            psNuGMAC,
                            1024,
                            25,
                            5);

    if (threadCtx != RT_NULL)
        rt_thread_startup(threadCtx);
}

static rt_err_t nu_gmac_init(rt_device_t device)
{
    rt_err_t ret;
    s32 status = 0;
    char szTmp[32];
    int count;

    nu_gmac_t psNuGMAC = (nu_gmac_t)device;
    RT_ASSERT(psNuGMAC);

    synopGMACNetworkAdapter *adapter = psNuGMAC->adapter;
    synopGMACdevice *gmacdev = (synopGMACdevice *)adapter->m_gmacdev;
    GMAC_MEMMGR_T *psgmacmemmgr = (GMAC_MEMMGR_T *)adapter->m_gmacmemmgr;

    RT_ASSERT(gmacdev);
    RT_ASSERT(psgmacmemmgr);

    LOG_D("[%s] Init %s", __func__, psNuGMAC->name);

    synopGMAC_attach(gmacdev, ((uint32_t)psNuGMAC->base + MACBASE), ((uint32_t)psNuGMAC->base + DMABASE), DEFAULT_PHY_BASE, &psNuGMAC->mac_addr[0]);
    nu_mii_init(adapter);

    /* Reset to make RGMII/RMII setting take affect. */
    synopGMAC_reset(gmacdev);
    synopGMAC_read_version(gmacdev);
    LOG_I("%s: HW version is %08x", psNuGMAC->name, gmacdev->Version);

    /*Check for Phy initialization*/
    synopGMAC_set_mdc_clk_div(gmacdev, GmiiCsrClk2);
    gmacdev->ClockDivMdc = synopGMAC_get_mdc_clk_div(gmacdev);
    status = synopGMAC_check_phy_init(adapter);

    /*Set up the tx and rx descriptor queue/ring*/
    LOG_D("tx desc_queue");
    synopGMAC_setup_tx_desc_queue(gmacdev, &psgmacmemmgr->psTXDescs[0], TRANSMIT_DESC_SIZE, RINGMODE);
    synopGMAC_init_tx_desc_base(gmacdev);
    LOG_D("DmaTxBaseAddr = %08x", synopGMACReadReg(gmacdev->DmaBase, DmaTxBaseAddr));

    LOG_D("rx desc_queue");
    synopGMAC_setup_rx_desc_queue(gmacdev, &psgmacmemmgr->psRXDescs[0], RECEIVE_DESC_SIZE, RINGMODE);
    synopGMAC_init_rx_desc_base(gmacdev);
    LOG_D("DmaRxBaseAddr = %08x", synopGMACReadReg(gmacdev->DmaBase, DmaRxBaseAddr));

    /*Initialize the dma interface*/
    synopGMAC_dma_bus_mode_init(gmacdev, DmaBurstLength32 | DmaDescriptorSkip0/*DmaDescriptorSkip2*/ | DmaDescriptor8Words);
    synopGMAC_dma_control_init(gmacdev, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);

    /*Initialize the mac interface*/
    gmacdev->Speed = SPEED100;
    gmacdev->DuplexMode = FULLDUPLEX;
    synopGMAC_mac_init(gmacdev);

    synopGMAC_pause_control(gmacdev); // This enables the pause control in Full duplex mode of operation
    //synopGMAC_jumbo_frame_enable(gmacdev); // enable jumbo frame

#if defined(RT_LWIP_USING_HW_CHECKSUM)
    /*IPC Checksum offloading is enabled for this driver. Should only be used if Full Ip checksumm offload engine is configured in the hardware*/
    synopGMAC_enable_rx_chksum_offload(gmacdev);    //Enable the offload engine in the receive path
    synopGMAC_rx_tcpip_chksum_drop_enable(gmacdev); // This is default configuration, DMA drops the packets if error in encapsulated ethernet payload
#endif

    /* Set all RX frame buffers. */
    count = 0;
    do
    {
        LOG_D("[%s] Set %d pkt frame buffer address - 0x%08x, size=%d", __func__, count, (u32)(&psgmacmemmgr->psRXFrames[count]), PKT_FRAME_BUF_SIZE);
        status = synopGMAC_set_rx_qptr(gmacdev, (u32)(&psgmacmemmgr->psRXFrames[count]), PKT_FRAME_BUF_SIZE, 0);
        if (status < 0)
        {
            LOG_E("status < 0!!");
            break;
        }
        count++;
    }
    while (count < RECEIVE_DESC_SIZE);

    synopGMAC_clear_interrupt(gmacdev);

    synopGMAC_disable_interrupt_all(gmacdev);
    synopGMAC_enable_interrupt(gmacdev, DmaIntEnable);
    LOG_D("%s: Interrupt enable: %08x", psNuGMAC->name, synopGMAC_get_ie(gmacdev));

    synopGMAC_enable_dma_rx(gmacdev);
    synopGMAC_enable_dma_tx(gmacdev);

    synopGMAC_set_mac_addr(gmacdev, GmacAddr0High, GmacAddr0Low, &psNuGMAC->mac_addr[0]);

    synopGMAC_set_mode(gmacdev, 0);
    nu_phy_link_monitor(psNuGMAC);

    /* Enable NVIC interrupt. */
    LOG_D("[%s] Install %s ISR.", __func__, psNuGMAC->name);
    NVIC_EnableIRQ(psNuGMAC->irqn);

    LOG_D("[%s] Init %s done", __func__, psNuGMAC->name);

    return RT_EOK;
}

static rt_err_t nu_gmac_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

static rt_err_t nu_gmac_close(rt_device_t dev)
{
    return RT_EOK;
}

static rt_size_t nu_gmac_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_size_t nu_gmac_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_err_t nu_gmac_control(rt_device_t device, int cmd, void *args)
{
    nu_gmac_t psNuGMAC = (nu_gmac_t)device;
    RT_ASSERT(device);

    switch (cmd)
    {
    case NIOCTL_GADDR:
        if (args) rt_memcpy(args, &psNuGMAC->mac_addr[0], 6);
        else return -RT_ERROR;
        break;

    default :
        break;
    }

    return RT_EOK;
}

rt_err_t nu_gmac_tx(rt_device_t device, struct pbuf *p)
{
    rt_err_t ret = -RT_ERROR;

    nu_gmac_t psNuGMAC = (nu_gmac_t)device;
    synopGMACNetworkAdapter *adapter = (synopGMACNetworkAdapter *) psNuGMAC->adapter;
    synopGMACdevice *gmacdev = (synopGMACdevice *) adapter->m_gmacdev;
    GMAC_MEMMGR_T *psgmacmemmgr = (GMAC_MEMMGR_T *)adapter->m_gmacmemmgr;

    u32 index = gmacdev->TxNext;
    u8 *pu8PktData = (u8 *)((u32)&psgmacmemmgr->psTXFrames[index]);
    struct pbuf *q;
    rt_uint32_t offset = 0;
    u32 offload_needed;
    s32 status;

    LOG_D("%s: Transmitting data(%08x-%d) start.", psNuGMAC->name, (u32)pu8PktData, p->tot_len);

    /* Copy to TX data buffer. */
    for (q = p; q != NULL; q = q->next)
    {
        rt_uint8_t *ptr = q->payload;
        rt_uint32_t len = q->len;
        memcpy(&pu8PktData[offset], ptr, len);
        offset += len;
    }
#if defined(RT_USING_CACHE)
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)pu8PktData, offset);
#endif

#if defined(RT_LWIP_USING_HW_CHECKSUM)
    offload_needed = 1;
#else
    offload_needed = 0;
#endif

    status = synopGMAC_xmit_frames(gmacdev, (u8 *)pu8PktData, offset, offload_needed, 0);
    if (status != 0)
    {
        LOG_E("%s No More Free Tx skb", __func__);
        ret = -RT_ERROR;
        goto exit_nu_gmac_tx;
    }

    synop_handle_transmit_over(gmacdev);

    LOG_D("%s: Transmitting data(%08x-%d) done.", psNuGMAC->name, (u32)pu8PktData, p->tot_len);

    ret = RT_EOK;

exit_nu_gmac_tx:

    return ret;
}

static void nu_gmac_assign_macaddr(nu_gmac_t psNuGMAC)
{
    static rt_uint32_t value = 0x94539452;
    uint32_t uid[4] = {0};

    /* Read UID from FMC for unique MAC address. */
    void nu_read_uid(uint32_t *id);
    nu_read_uid((uint32_t *)&uid);

    value = value ^ uid[0] ^ uid[1] ^ uid[2] ^ uid[3];

    LOG_I("UID: %08X-%08X-%08X-%08X, value: %08x", uid[0], uid[1], uid[2], uid[3], value);

    /* Assign MAC address */
    psNuGMAC->mac_addr[0] = 0x82;
    psNuGMAC->mac_addr[1] = 0x06;
    psNuGMAC->mac_addr[2] = 0x21;
    psNuGMAC->mac_addr[3] = (value >> 16) & 0xff;
    psNuGMAC->mac_addr[4] = (value >> 8) & 0xff;
    psNuGMAC->mac_addr[5] = (value) & 0xff;

    LOG_I("MAC address: %02X:%02X:%02X:%02X:%02X:%02X", \
          psNuGMAC->mac_addr[0], \
          psNuGMAC->mac_addr[1], \
          psNuGMAC->mac_addr[2], \
          psNuGMAC->mac_addr[3], \
          psNuGMAC->mac_addr[4], \
          psNuGMAC->mac_addr[5]);
    value++;
}

int32_t nu_gmac_adapter_init(nu_gmac_t psNuGMAC)
{
    synopGMACNetworkAdapter *adapter;

    RT_ASSERT(psNuGMAC);

    /* Allocate net adapter */
    adapter = (synopGMACNetworkAdapter *)rt_malloc_align(sizeof(synopGMACNetworkAdapter), RT_ALIGN_SIZE);
    RT_ASSERT(adapter);
    rt_memset((void *)adapter, 0, sizeof(synopGMACNetworkAdapter));

    /* Allocate device */
    adapter->m_gmacdev = (synopGMACdevice *) rt_malloc_align(sizeof(synopGMACdevice), RT_ALIGN_SIZE);
    RT_ASSERT(adapter->m_gmacdev);
    rt_memset((char *)adapter->m_gmacdev, 0, sizeof(synopGMACdevice));

    /* Allocate memory management */
    adapter->m_gmacmemmgr = (GMAC_MEMMGR_T *) rt_malloc_align(sizeof(GMAC_MEMMGR_T), RT_ALIGN_SIZE);
    RT_ASSERT(adapter->m_gmacmemmgr);
    nu_memmgr_init(adapter->m_gmacmemmgr);

    /* Store adapter to priv */
    psNuGMAC->adapter = adapter;

    return 0;
}

int rt_hw_gmac_init(void)
{
    int i;
    rt_err_t ret = RT_EOK;

    for (i = (GMAC_START + 1); i < GMAC_CNT; i++)
    {
        nu_gmac_t psNuGMAC = (nu_gmac_t)&nu_gmac_arr[i];

        /* Register member functions */
        psNuGMAC->eth.parent.type       = RT_Device_Class_NetIf;
        psNuGMAC->eth.parent.init       = nu_gmac_init;
        psNuGMAC->eth.parent.open       = nu_gmac_open;
        psNuGMAC->eth.parent.close      = nu_gmac_close;
        psNuGMAC->eth.parent.read       = nu_gmac_read;
        psNuGMAC->eth.parent.write      = nu_gmac_write;
        psNuGMAC->eth.parent.control    = nu_gmac_control;
        psNuGMAC->eth.parent.user_data  = psNuGMAC;
        psNuGMAC->eth.eth_rx            = RT_NULL;
        psNuGMAC->eth.eth_tx            = nu_gmac_tx;

        /* Set MAC address */
        nu_gmac_assign_macaddr(psNuGMAC);

        /* Initial GMAC adapter */
        nu_gmac_adapter_init(psNuGMAC);

        /* Register eth device */
        ret = eth_device_init(&psNuGMAC->eth, psNuGMAC->name);
        RT_ASSERT(ret == RT_EOK);
    }

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_gmac_init);

#if 1
/*
    Remeber src += lwipiperf_SRCS in components\net\lwip\lwip-*\SConscript
*/
#include "lwip/apps/lwiperf.h"

static void
lwiperf_report(void *arg, enum lwiperf_report_type report_type,
               const ip_addr_t *local_addr, u16_t local_port, const ip_addr_t *remote_addr, u16_t remote_port,
               u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(local_addr);
    LWIP_UNUSED_ARG(local_port);

    rt_kprintf("IPERF report: type=%d, remote: %s:%d, total bytes: %"U32_F", duration in ms: %"U32_F", kbits/s: %"U32_F"\n",
               (int)report_type, ipaddr_ntoa(remote_addr), (int)remote_port, bytes_transferred, ms_duration, bandwidth_kbitpsec);
}

int lwiperf_example_init(void)
{
    lwiperf_start_tcp_server_default(lwiperf_report, NULL);

    return 0;
}
MSH_CMD_EXPORT(lwiperf_example_init, start lwip tcp server);
INIT_APP_EXPORT(lwiperf_example_init);
#endif

#endif /* if defined(BSP_USING_GMAC) */
