/**************************************************************************//**
*
* @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date            Author       Notes
* 2022-4-1        Wayne        First version
*
******************************************************************************/

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <fal.h>
#include "drv_fmc.h"

/* ===================== Flash device Configuration ========================= */
#if defined(FAL_PART_HAS_TABLE_CFG)

#if defined(BSP_USING_FMC)

#define FAL_FLASH_DEV_TABLE         \
{                                   \
    &g_falFMC_AP,    \
    &g_falFMC_LD,    \
}
#else
#define FAL_FLASH_DEV_TABLE         \
{                                   \
}
#endif

#define FAL_PART_TABLE                                                     \
{                                                                          \
    {FAL_PART_MAGIC_WORD, "ldrom",     "FMC_LD",  0x0,    (8*1024),   0},  \
    {FAL_PART_MAGIC_WORD, "aprom",     "FMC_AP",  0x0,    (2*1024*1024), 0},  \
}

#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
