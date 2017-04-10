#ifndef PHI_LIB_PHI_LIB_H_
#define PHI_LIB_PHI_LIB_H_

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ch.h"
#include "chprintf.h"
#include "hal.h"
#include "phi_conf.h"

#if defined(STM32F303xC) || defined(STM32F072xB)
#define STM32_REG_UNIQUE_ID 0x1FFFF7AC
#elif defined(STM32F407xx)
#define STM32_REG_UNIQUE_ID 0x1FFF7A10
#else
#error ?
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PHI_ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define PHI_DEV_ID(x1, x2, x3, x4) ((x1 << 24) | (x2 << 16) | (x3 << 8) | x4)

#define PHI_HW_SW_VER(is_bootloader, hw_ver, sw_ver) ((is_bootloader << 31) | ((hw_ver & 0x7f) << 24) | (sw_ver & 0xffffff))
#define PHI_HW_SW_VER_GET_IS_BOOTLOADER(hw_sw_ver) (hw_sw_ver & 0x80000000)
#define PHI_HW_SW_VER_GET_SW(hw_sw_ver) (hw_sw_ver & 0xffffff)
#define PHI_HW_SW_VER_GET_HW(hw_sw_ver) ((hw_sw_ver >> 24) & 0x7f)

#define phi_is_aligned(ptr, align) (((uintptr_t)(const void *)(ptr)) % (align) == 0)

static inline long phi_lib_map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline long phi_min(long a, long b)
{
    return (a < b) ? a : b;
}

static inline long phi_max(long a, long b)
{
    return (a > b) ? a : b;
}


#define phi_crc16(buf, len)  phi_crc16_incremental(0xFFFFU, buf, len)
uint16_t phi_crc16_incremental(uint16_t ret, const void * buf, size_t len);
uint32_t phi_crc32(const void * buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* PHI_LIB_PHI_LIB_H_ */
