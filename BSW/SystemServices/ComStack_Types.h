/**
 * @file    ComStack_Types.h
 * @brief   AUTOSAR CP R25-11 — Communication Stack Types
 * @details AUTOSAR_CP_SWS_COMStackTypes (R25-11)
 */
#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

#include "Std_Types.h"

typedef uint16 PduIdType;       /**< PDU identifier */
typedef uint32 PduLengthType;   /**< PDU length */

/** PDU information block — SWS_COMTYPE_00011 */
typedef struct {
    uint8*        SduDataPtr;  /**< Pointer to payload data */
    uint8*        MetaDataPtr; /**< Pointer to meta data (optional) */
    PduLengthType SduLength;   /**< Data length in bytes */
} PduInfoType;

typedef enum {
    BUFREQ_OK       = 0x00u,
    BUFREQ_E_NOT_OK = 0x01u,
    BUFREQ_E_BUSY   = 0x02u,
    BUFREQ_E_OVFL   = 0x03u
} BufReq_ReturnType;

typedef enum {
    TP_DATACONF    = 0x00u,
    TP_DATARETRY   = 0x01u,
    TP_CONFPENDING = 0x02u
} TpDataStateType;

typedef uint8  NetworkHandleType;
typedef uint8  EcucPartitionIdType;

#endif /* COMSTACK_TYPES_H */
