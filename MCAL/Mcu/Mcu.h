/**
 * @file    Mcu.h
 * @brief   AUTOSAR CP R25-11 — MCU Driver
 * @details Implements AUTOSAR_CP_SWS_MCUDriver (R25-11)
 *          Functional Cluster: MCAL
 *
 * AUTOSAR Document: AUTOSAR_CP_SWS_MCUDriver
 * Version        : R25-11
 */
#ifndef MCU_H
#define MCU_H

#include "Std_Types.h"

/* ================================================================
 * Type Definitions
 * ================================================================ */
typedef uint8  Mcu_ClockType;          /**< Clock setting ID */
typedef uint8  Mcu_ResetType;          /**< Reset type */
typedef uint32 Mcu_RawResetType;       /**< Raw reset register value */
typedef uint8  Mcu_ModeType;           /**< MCU power mode */
typedef uint8  Mcu_RamSectionType;     /**< RAM section ID */

typedef enum {
    MCU_PLL_LOCKED    = 0x00u,   /**< PLL is locked */
    MCU_PLL_UNLOCKED  = 0x01u,   /**< PLL is unlocked */
    MCU_PLL_STATUS_UNDEFINED = 0x02u
} Mcu_PllStatusType;

typedef enum {
    MCU_POWER_ON_RESET       = 0x00u,
    MCU_WATCHDOG_RESET       = 0x01u,
    MCU_SW_RESET             = 0x02u,
    MCU_RESET_UNDEFINED      = 0x03u
} Mcu_ResetReasonType;

typedef enum {
    MCU_RAM_VALID   = 0x00u,
    MCU_RAM_INVALID = 0x01u
} Mcu_RamStateType;

/* ================================================================
 * API
 * ================================================================ */
void              Mcu_Init(const void* ConfigPtr);
Std_ReturnType    Mcu_InitRamSection(Mcu_RamSectionType RamSection);
Std_ReturnType    Mcu_InitClock(Mcu_ClockType ClockSetting);
Std_ReturnType    Mcu_DistributePllClock(void);
Mcu_PllStatusType Mcu_GetPllStatus(void);
Mcu_ResetReasonType Mcu_GetResetReason(void);
Mcu_RawResetType  Mcu_GetResetRawValue(void);
void              Mcu_PerformReset(void);
void              Mcu_SetMode(Mcu_ModeType McuMode);
void              Mcu_GetVersionInfo(Std_VersionInfoType* versioninfo);
Mcu_RamStateType  Mcu_GetRamState(void);

#endif /* MCU_H */
