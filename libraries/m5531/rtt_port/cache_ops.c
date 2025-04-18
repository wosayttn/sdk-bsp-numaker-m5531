/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-10-29     Wayne        the first version
 */
#include "rtconfig.h"
#include <rthw.h>
#include "NuMicro.h"

#if defined(RT_USING_CACHE)

/*
 * CPU interfaces
 */
void rt_hw_cpu_icache_enable(void)
{
    SCB_EnableICache();
}

void rt_hw_cpu_icache_disable(void)
{
    SCB_DisableICache();
}

rt_base_t rt_hw_cpu_icache_status(void)
{
    return 0;
}

void rt_hw_cpu_icache_ops(int ops, void *addr, int size)
{
    if (ops & RT_HW_CACHE_INVALIDATE)
    {
        SCB_InvalidateICache_by_Addr(addr, size);
    }
}

void rt_hw_cpu_dcache_enable(void)
{
    SCB_EnableDCache();
}

void rt_hw_cpu_dcache_disable(void)
{
    SCB_DisableDCache();
}

rt_base_t rt_hw_cpu_dcache_status(void)
{
    return 0;
}

void rt_hw_cpu_dcache_ops(int ops, void *addr, int size)
{

    if ((ops & RT_HW_CACHE_FLUSH_INVALIDATE) == RT_HW_CACHE_FLUSH_INVALIDATE)
    {
        SCB_CleanInvalidateDCache_by_Addr(addr, size);
    }
    else if (ops & RT_HW_CACHE_FLUSH)
    {
        SCB_CleanDCache_by_Addr(addr, size);
    }
    else if (ops & RT_HW_CACHE_INVALIDATE)
    {
        SCB_InvalidateDCache_by_Addr(addr, size);
    }
    else
    {
        RT_ASSERT(0);
    }

}

#endif
