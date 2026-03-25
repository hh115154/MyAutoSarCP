/**
 * @file    Std_Types.h
 * @brief   AUTOSAR CP R25-11 — Standard Type Definitions
 * @details AUTOSAR_CP_SWS_StandardTypes (R25-11)
 */
#ifndef STD_TYPES_H
#define STD_TYPES_H

#include <stdint.h>
#include <stddef.h>

/* ================================================================
 * Primitive Types  [SWS_Std_00005]
 * ================================================================ */
typedef uint8_t   uint8;
typedef uint16_t  uint16;
typedef uint32_t  uint32;
typedef uint64_t  uint64;
typedef int8_t    sint8;
typedef int16_t   sint16;
typedef int32_t   sint32;
typedef int64_t   sint64;
typedef float     float32;
typedef double    float64;

/* ================================================================
 * Return Type  [SWS_Std_00006]
 * ================================================================ */
typedef uint8 Std_ReturnType;

#define E_OK        ((Std_ReturnType)0x00u)
#define E_NOT_OK    ((Std_ReturnType)0x01u)

/* ================================================================
 * High/Low  [SWS_Std_00007]
 * ================================================================ */
#define STD_HIGH    ((uint8)0x01u)
#define STD_LOW     ((uint8)0x00u)
#define STD_ACTIVE  ((uint8)0x01u)
#define STD_IDLE    ((uint8)0x00u)
#define STD_ON      ((uint8)0x01u)
#define STD_OFF     ((uint8)0x00u)

/* ================================================================
 * Version Info  [SWS_Std_00015]
 * ================================================================ */
typedef struct {
    uint16 vendorID;
    uint16 moduleID;
    uint8  sw_major_version;
    uint8  sw_minor_version;
    uint8  sw_patch_version;
} Std_VersionInfoType;

/* ================================================================
 * NULL_PTR  [SWS_Std_00031]
 * ================================================================ */
#ifndef NULL_PTR
#  define NULL_PTR  ((void*)0)
#endif

#endif /* STD_TYPES_H */
