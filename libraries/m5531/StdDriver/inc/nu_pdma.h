/**************************************************************************//**
 * @file     nu_pdma.h
 * @version  V1.00
 * @brief    PDMA driver header file
 *
 * @copyright SPDX-License-Identifier: Apache-2.0
 * @copyright Copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#ifndef __NU_PDMA_H__
#define __NU_PDMA_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup PDMA_Driver PDMA Driver
  @{
*/

/** @addtogroup PDMA_EXPORTED_CONSTANTS PDMA Exported Constants
  @{
*/

#define PDMA_CH_MAX    16UL   /*!< Specify Maximum Channels of PDMA  \hideinitializer */

/*---------------------------------------------------------------------------------------------------------*/
/*  Operation Mode Constant Definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define PDMA_OP_STOP        0x00000000UL            /*!<DMA Stop Mode  \hideinitializer */
#define PDMA_OP_BASIC       0x00000001UL            /*!<DMA Basic Mode  \hideinitializer */
#define PDMA_OP_SCATTER     0x00000002UL            /*!<DMA Scatter-gather Mode  \hideinitializer */

/*---------------------------------------------------------------------------------------------------------*/
/*  Data Width Constant Definitions                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define PDMA_WIDTH_8        0x00000000UL            /*!<DMA Transfer Width 8-bit  \hideinitializer */
#define PDMA_WIDTH_16       0x00001000UL            /*!<DMA Transfer Width 16-bit  \hideinitializer */
#define PDMA_WIDTH_32       0x00002000UL            /*!<DMA Transfer Width 32-bit  \hideinitializer */

/*---------------------------------------------------------------------------------------------------------*/
/*  Address Attribute Constant Definitions                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define PDMA_SAR_INC        0x00000000UL            /*!<DMA SAR increment  \hideinitializer */
#define PDMA_SAR_FIX        0x00000300UL            /*!<DMA SAR fix address  \hideinitializer */
#define PDMA_DAR_INC        0x00000000UL            /*!<DMA DAR increment  \hideinitializer */
#define PDMA_DAR_FIX        0x00000C00UL            /*!<DMA DAR fix address  \hideinitializer */

/*---------------------------------------------------------------------------------------------------------*/
/*  Burst Mode Constant Definitions                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define PDMA_REQ_SINGLE     0x00000004UL            /*!<DMA Single Request  \hideinitializer */
#define PDMA_REQ_BURST      0x00000000UL            /*!<DMA Burst Request  \hideinitializer */

#define PDMA_BURST_128      0x00000000UL            /*!<DMA Burst 128 Transfers  \hideinitializer */
#define PDMA_BURST_64       0x00000010UL            /*!<DMA Burst 64 Transfers  \hideinitializer */
#define PDMA_BURST_32       0x00000020UL            /*!<DMA Burst 32 Transfers  \hideinitializer */
#define PDMA_BURST_16       0x00000030UL            /*!<DMA Burst 16 Transfers  \hideinitializer */
#define PDMA_BURST_8        0x00000040UL            /*!<DMA Burst 8 Transfers  \hideinitializer */
#define PDMA_BURST_4        0x00000050UL            /*!<DMA Burst 4 Transfers  \hideinitializer */
#define PDMA_BURST_2        0x00000060UL            /*!<DMA Burst 2 Transfers  \hideinitializer */
#define PDMA_BURST_1        0x00000070UL            /*!<DMA Burst 1 Transfers  \hideinitializer */

/*---------------------------------------------------------------------------------------------------------*/
/*  Table Interrupt Disable Constant Definitions                                                           */
/*---------------------------------------------------------------------------------------------------------*/
#define PDMA_TBINTDIS_ENABLE  (0x0UL<<PDMA_DSCT_CTL_TBINTDIS_Pos)  /*!<DMA Table Interrupt Enabled   \hideinitializer */
#define PDMA_TBINTDIS_DISABLE (0x1UL<<PDMA_DSCT_CTL_TBINTDIS_Pos)  /*!<DMA Table Interrupt Disabled  \hideinitializer */

/*---------------------------------------------------------------------------------------------------------*/
/*  Peripheral Transfer Mode Constant Definitions                                                          */
/*---------------------------------------------------------------------------------------------------------*/
#define PDMA_MEM                0UL /*!<DMA Connect to Memory \hideinitializer */
#define PDMA_UART0_TX           4UL /*!<DMA Connect to UART0_TX \hideinitializer */
#define PDMA_UART0_RX           5UL /*!<DMA Connect to UART0_RX \hideinitializer */
#define PDMA_UART1_TX           6UL /*!<DMA Connect to UART1_TX \hideinitializer */
#define PDMA_UART1_RX           7UL /*!<DMA Connect to UART1_RX \hideinitializer */
#define PDMA_UART2_TX           8UL /*!<DMA Connect to UART2_TX \hideinitializer */
#define PDMA_UART2_RX           9UL /*!<DMA Connect to UART2_RX \hideinitializer */
#define PDMA_UART3_TX          10UL /*!<DMA Connect to UART3_TX \hideinitializer */
#define PDMA_UART3_RX          11UL /*!<DMA Connect to UART3_RX \hideinitializer */
#define PDMA_UART4_TX          12UL /*!<DMA Connect to UART4_TX \hideinitializer */
#define PDMA_UART4_RX          13UL /*!<DMA Connect to UART4_RX \hideinitializer */
#define PDMA_UART5_TX          14UL /*!<DMA Connect to UART5_TX \hideinitializer */
#define PDMA_UART5_RX          15UL /*!<DMA Connect to UART5_RX \hideinitializer */
#define PDMA_UART6_TX          16UL /*!<DMA Connect to UART6_TX \hideinitializer */
#define PDMA_UART6_RX          17UL /*!<DMA Connect to UART6_RX \hideinitializer */
#define PDMA_UART7_TX          18UL /*!<DMA Connect to UART7_TX \hideinitializer */
#define PDMA_UART7_RX          19UL /*!<DMA Connect to UART7_RX \hideinitializer */
#define PDMA_UART8_TX          20UL /*!<DMA Connect to UART8_TX \hideinitializer */
#define PDMA_UART8_RX          21UL /*!<DMA Connect to UART8_RX \hideinitializer */
#define PDMA_UART9_TX          22UL /*!<DMA Connect to UART9_TX \hideinitializer */
#define PDMA_UART9_RX          23UL /*!<DMA Connect to UART9_RX \hideinitializer */
#define PDMA_USCI0_TX          24UL /*!<DMA Connect to USCI0_TX \hideinitializer */
#define PDMA_USCI0_RX          25UL /*!<DMA Connect to USCI0_RX \hideinitializer */
#define PDMA_QSPI0_TX          26UL /*!<DMA Connect to QSPI0_TX \hideinitializer */
#define PDMA_QSPI0_RX          27UL /*!<DMA Connect to QSPI0_RX \hideinitializer */
#define PDMA_QSPI1_TX          28UL /*!<DMA Connect to QSPI1_TX \hideinitializer */
#define PDMA_QSPI1_RX          29UL /*!<DMA Connect to QSPI1_RX \hideinitializer */
#define PDMA_SPI0_TX           30UL /*!<DMA Connect to SPI0_TX \hideinitializer */
#define PDMA_SPI0_RX           31UL /*!<DMA Connect to SPI0_RX \hideinitializer */
#define PDMA_SPI1_TX           32UL /*!<DMA Connect to SPI1_TX \hideinitializer */
#define PDMA_SPI1_RX           33UL /*!<DMA Connect to SPI1_RX \hideinitializer */
#define PDMA_SPI2_TX           34UL /*!<DMA Connect to SPI2_TX \hideinitializer */
#define PDMA_SPI2_RX           35UL /*!<DMA Connect to SPI2_RX \hideinitializer */
#define PDMA_SPI3_TX           36UL /*!<DMA Connect to SPI3_TX \hideinitializer */
#define PDMA_SPI3_RX           37UL /*!<DMA Connect to SPI3_RX \hideinitializer */
#define PDMA_ACMP0             38UL /*!<DMA Connect to ACMP0 \hideinitializer */
#define PDMA_ACMP1             39UL /*!<DMA Connect to ACMP1 \hideinitializer */
#define PDMA_ACMP2             40UL /*!<DMA Connect to ACMP2 \hideinitializer */
#define PDMA_ACMP3             41UL /*!<DMA Connect to ACMP3 \hideinitializer */
#define PDMA_EPWM0_P1_RX       42UL /*!<DMA Connect to EPWM0_P1_RX \hideinitializer */
#define PDMA_EPWM0_P2_RX       43UL /*!<DMA Connect to EPWM0_P2_RX \hideinitializer */
#define PDMA_EPWM0_P3_RX       44UL /*!<DMA Connect to EPWM0_P3_RX \hideinitializer */
#define PDMA_EPWM1_P1_RX       45UL /*!<DMA Connect to EPWM1_P1_RX \hideinitializer */
#define PDMA_EPWM1_P2_RX       46UL /*!<DMA Connect to EPWM1_P2_RX \hideinitializer */
#define PDMA_EPWM1_P3_RX       47UL /*!<DMA Connect to EPWM1_P3_RX \hideinitializer */
#define PDMA_I2C0_TX           48UL /*!<DMA Connect to I2C0_TX \hideinitializer */
#define PDMA_I2C0_RX           49UL /*!<DMA Connect to I2C0_RX \hideinitializer */
#define PDMA_I2C1_TX           50UL /*!<DMA Connect to I2C1_TX \hideinitializer */
#define PDMA_I2C1_RX           51UL /*!<DMA Connect to I2C1_RX \hideinitializer */
#define PDMA_I2C2_TX           52UL /*!<DMA Connect to I2C2_TX \hideinitializer */
#define PDMA_I2C2_RX           53UL /*!<DMA Connect to I2C2_RX \hideinitializer */
#define PDMA_I2C3_TX           54UL /*!<DMA Connect to I2C3_TX \hideinitializer */
#define PDMA_I2C3_RX           55UL /*!<DMA Connect to I2C3_RX \hideinitializer */
#define PDMA_I2S0_TX           56UL /*!<DMA Connect to I2S0_TX \hideinitializer */
#define PDMA_I2S0_RX           57UL /*!<DMA Connect to I2S0_RX \hideinitializer */
#define PDMA_I2S1_TX           58UL /*!<DMA Connect to I2S1_TX \hideinitializer */
#define PDMA_I2S1_RX           59UL /*!<DMA Connect to I2S1_RX \hideinitializer */
#define PDMA_TMR0              60UL /*!<DMA Connect to TMR0 \hideinitializer */
#define PDMA_TMR1              61UL /*!<DMA Connect to TMR1 \hideinitializer */
#define PDMA_TMR2              62UL /*!<DMA Connect to TMR2 \hideinitializer */
#define PDMA_TMR3              63UL /*!<DMA Connect to TMR3 \hideinitializer */
#define PDMA_EADC0_RX          64UL /*!<DMA Connect to EADC0_RX \hideinitializer */
#define PDMA_EADC1_RX          65UL /*!<DMA Connect to EADC1_RX \hideinitializer */
#define PDMA_DAC0_TX           66UL /*!<DMA Connect to DAC0_TX \hideinitializer */
#define PDMA_DAC1_TX           67UL /*!<DMA Connect to DAC1_TX \hideinitializer */
#define PDMA_EPWM0_CH0_TX      68UL /*!<DMA Connect to EPWM0_CH0_TX \hideinitializer */
#define PDMA_EPWM0_CH1_TX      69UL /*!<DMA Connect to EPWM0_CH1_TX \hideinitializer */
#define PDMA_EPWM0_CH2_TX      70UL /*!<DMA Connect to EPWM0_CH2_TX \hideinitializer */
#define PDMA_EPWM0_CH3_TX      71UL /*!<DMA Connect to EPWM0_CH3_TX \hideinitializer */
#define PDMA_EPWM0_CH4_TX      72UL /*!<DMA Connect to EPWM0_CH4_TX \hideinitializer */
#define PDMA_EPWM0_CH5_TX      73UL /*!<DMA Connect to EPWM0_CH5_TX \hideinitializer */
#define PDMA_EPWM1_CH0_TX      74UL /*!<DMA Connect to EPWM1_CH0_TX \hideinitializer */
#define PDMA_EPWM1_CH1_TX      75UL /*!<DMA Connect to EPWM1_CH1_TX \hideinitializer */
#define PDMA_EPWM1_CH2_TX      76UL /*!<DMA Connect to EPWM1_CH2_TX \hideinitializer */
#define PDMA_EPWM1_CH3_TX      77UL /*!<DMA Connect to EPWM1_CH3_TX \hideinitializer */
#define PDMA_EPWM1_CH4_TX      78UL /*!<DMA Connect to EPWM1_CH4_TX \hideinitializer */
#define PDMA_EPWM1_CH5_TX      79UL /*!<DMA Connect to EPWM1_CH5_TX \hideinitializer */
#define PDMA_EINT0             80UL /*!<DMA Connect to EINT0 \hideinitializer */
#define PDMA_EINT1             81UL /*!<DMA Connect to EINT1 \hideinitializer */
#define PDMA_EINT2             82UL /*!<DMA Connect to EINT2 \hideinitializer */
#define PDMA_EINT3             83UL /*!<DMA Connect to EINT3 \hideinitializer */
#define PDMA_EINT4             84UL /*!<DMA Connect to EINT4 \hideinitializer */
#define PDMA_EINT5             85UL /*!<DMA Connect to EINT5 \hideinitializer */
#define PDMA_EINT6             86UL /*!<DMA Connect to EINT6 \hideinitializer */
#define PDMA_EINT7             87UL /*!<DMA Connect to EINT7 \hideinitializer */
#define PDMA_PSIO_TX           90UL /*!<DMA Connect to PSIO_TX \hideinitializer */
#define PDMA_PSIO_RX           91UL /*!<DMA Connect to PSIO_RX \hideinitializer */
#define PDMA_I3C0_TX           92UL /*!<DMA Connect to I3C0_TX \hideinitializer */
#define PDMA_I3C0_RX           93UL /*!<DMA Connect to I3C0_RX \hideinitializer */
/*---------------------------------------------------------------------------------------------------------*/
/*  Interrupt Type Constant Definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define PDMA_INT_TRANS_DONE 0x00000000UL            /*!<Transfer Done Interrupt  \hideinitializer */
#define PDMA_INT_TEMPTY     0x00000001UL            /*!<Table Empty Interrupt  \hideinitializer */
#define PDMA_INT_TIMEOUT    0x00000002UL            /*!<Timeout Interrupt \hideinitializer */
#define PDMA_INT_ALIGN      0x00000003UL            /*!<Transfer Alignment Interrupt  \hideinitializer */


/** @} end of group PDMA_EXPORTED_CONSTANTS */

/** @addtogroup PDMA_EXPORTED_FUNCTIONS PDMA Exported Functions
  @{
*/

/**
 * @brief       Get PDMA Interrupt Status
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 *
 * @details     This macro gets the interrupt status.
 * \hideinitializer
 */
#define PDMA_GET_INT_STATUS(pdma) ((uint32_t)(pdma->INTSTS))

/**
 * @brief       Get Transfer Done Interrupt Status
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 *
 * @details     Get the transfer done Interrupt status.
 * \hideinitializer
 */
#define PDMA_GET_TD_STS(pdma) ((uint32_t)(pdma->TDSTS))

/**
 * @brief       Clear Transfer Done Interrupt Status
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 * @param[in]   u32Mask     The channel mask
 *
 * @details     Clear the transfer done Interrupt status.
 * \hideinitializer
 */
#define PDMA_CLR_TD_FLAG(pdma,u32Mask) ((uint32_t)(pdma->TDSTS = (u32Mask)))

/**
 * @brief       Get Target Abort Interrupt Status
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 *
 * @details     Get the target abort Interrupt status.
 * \hideinitializer
 */
#define PDMA_GET_ABORT_STS(pdma) ((uint32_t)(pdma->ABTSTS))

/**
 * @brief       Clear Target Abort Interrupt Status
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 * @param[in]   u32Mask     The channel mask
 *
 * @details     Clear the target abort Interrupt status.
 * \hideinitializer
 */
#define PDMA_CLR_ABORT_FLAG(pdma,u32Mask) ((uint32_t)(pdma->ABTSTS = (u32Mask)))

/**
 * @brief       Get Alignment Interrupt Status
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 *
 * @details     Get Alignment Interrupt status.
 * \hideinitializer
 */
#define PDMA_GET_ALIGN_STS(pdma) ((uint32_t)(pdma->ALIGN))

/**
 * @brief       Clear Alignment Interrupt Status
  *
 * @param[in]   pdma        The pointer of the specified PDMA module
 * @param[in]   u32Mask     The channel mask
 *
 * @details     Clear the Alignment Interrupt status.
 * \hideinitializer
 */
#define PDMA_CLR_ALIGN_FLAG(pdma,u32Mask) ((uint32_t)(pdma->ALIGN = (u32Mask)))

/**
 * @brief       Clear Timeout Interrupt Status
  *
 * @param[in]   pdma      The pointer of the specified PDMA module
 * @param[in]   u32Ch     The selected channel
 *
 * @details     Clear the selected channel timeout interrupt status.
 * \hideinitializer
 */
#define PDMA_CLR_TMOUT_FLAG(pdma,u32Ch) ((uint32_t)(pdma->INTSTS = (1 << ((u32Ch) + 8))))

/**
 * @brief       Check Channel Status
  *
 * @param[in]   pdma      The pointer of the specified PDMA module
 * @param[in]   u32Ch     The selected channel
 *
 * @retval      0 Idle state
 * @retval      1 Busy state
 *
 * @details     Check the selected channel is busy or not.
 * \hideinitializer
 */
#define PDMA_IS_CH_BUSY(pdma,u32Ch) ((uint32_t)(pdma->TRGSTS & (1 << (u32Ch)))? 1 : 0)

/**
 * @brief       Set Source Address
  *
 * @param[in]   pdma      The pointer of the specified PDMA module
 * @param[in]   u32Ch     The selected channel
 * @param[in]   u32Addr   The selected address
 *
 * @details     This macro set the selected channel source address.
 * \hideinitializer
 */
#define PDMA_SET_SRC_ADDR(pdma,u32Ch, u32Addr) ((uint32_t)(pdma->DSCT[(u32Ch)].SA = (u32Addr)))

/**
 * @brief       Set Destination Address
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 * @param[in]   u32Ch     The selected channel
 * @param[in]   u32Addr   The selected address
 *
 * @details     This macro set the selected channel destination address.
 * \hideinitializer
 */
#define PDMA_SET_DST_ADDR(pdma,u32Ch, u32Addr) ((uint32_t)(pdma->DSCT[(u32Ch)].DA = (u32Addr)))

/**
 * @brief       Set Transfer Count
  *
 * @param[in]   pdma           The pointer of the specified PDMA module
 * @param[in]   u32Ch          The selected channel
 * @param[in]   u32TransCount  Transfer Count
 *
 * @details     This macro set the selected channel transfer count.
 * \hideinitializer
 */
#define PDMA_SET_TRANS_CNT(pdma,u32Ch, u32TransCount) ((uint32_t)(pdma->DSCT[(u32Ch)].CTL=(pdma->DSCT[(u32Ch)].CTL&~PDMA_DSCT_CTL_TXCNT_Msk)|(((u32TransCount)-1) << PDMA_DSCT_CTL_TXCNT_Pos)))

/**
 * @brief       Set Scatter-gather descriptor Address
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 * @param[in]   u32Ch     The selected channel
 * @param[in]   u32Addr   The descriptor address
 *
 * @details     This macro set the selected channel scatter-gather descriptor address.
 * \hideinitializer
 */
#define PDMA_SET_SCATTER_DESC(pdma,u32Ch, u32Addr) ((uint32_t)(pdma->DSCT[(u32Ch)].NEXT = u32Addr))

/**
 * @brief       Stop the channel
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 * @param[in]   u32Ch     The selected channel
 *
 * @details     This macro stop the selected channel.
 * \hideinitializer
 */
#define PDMA_STOP(pdma,u32Ch) ((uint32_t)(pdma->PAUSE = (1 << (u32Ch))))

/**
 * @brief       Pause the channel
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 * @param[in]   u32Ch     The selected channel
 *
 * @details     This macro pause the selected channel.
 * \hideinitializer
 */
#define PDMA_PAUSE(pdma,u32Ch) ((uint32_t)(pdma->PAUSE = (1 << (u32Ch))))

/**
 * @brief       Reset the channel
 *
 * @param[in]   pdma      The pointer of the specified PDMA module
 * @param[in]   u32Ch     The selected channel
 *
 * @details     This macro reset the selected channel.
 * \hideinitializer
 */
#define PDMA_RESET(pdma,u32Ch) ((uint32_t)(pdma->CHRST = (1 << (u32Ch))))

/*---------------------------------------------------------------------------------------------------------*/
/* Define PDMA functions prototype                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
void PDMA_Open(PDMA_T *pdma, uint32_t u32Mask);
void PDMA_Close(PDMA_T *pdma);
void PDMA_SetTransferCnt(PDMA_T *pdma, uint32_t u32Ch, uint32_t u32Width, uint32_t u32TransCount);
void PDMA_SetTransferAddr(PDMA_T *pdma, uint32_t u32Ch, uint32_t u32SrcAddr, uint32_t u32SrcCtrl, uint32_t u32DstAddr, uint32_t u32DstCtrl);
void PDMA_SetTransferMode(PDMA_T *pdma, uint32_t u32Ch, uint32_t u32Peripheral, uint32_t u32ScatterEn, uint32_t u32DescAddr);
void PDMA_SetBurstType(PDMA_T *pdma, uint32_t u32Ch, uint32_t u32BurstType, uint32_t u32BurstSize);
void PDMA_EnableTimeout(PDMA_T *pdma, uint32_t u32Mask);
void PDMA_DisableTimeout(PDMA_T *pdma, uint32_t u32Mask);
void PDMA_SetTimeOut(PDMA_T *pdma, uint32_t u32Ch, uint32_t u32OnOff, uint32_t u32TimeOutCnt);
void PDMA_Trigger(PDMA_T *pdma, uint32_t u32Ch);
void PDMA_EnableInt(PDMA_T *pdma, uint32_t u32Ch, uint32_t u32Mask);
void PDMA_DisableInt(PDMA_T *pdma, uint32_t u32Ch, uint32_t u32Mask);

/** @} end of group PDMA_EXPORTED_FUNCTIONS */
/** @} end of group PDMA_Driver */
/** @} end of group Standard_Driver */

#ifdef __cplusplus
}
#endif

#endif /* __NU_PDMA_H__ */
