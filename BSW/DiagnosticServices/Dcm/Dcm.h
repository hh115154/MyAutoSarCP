/**
 * @file    Dcm.h
 * @brief   AUTOSAR CP R25-11 — Diagnostic Communication Manager
 * @details Implements AUTOSAR_CP_SWS_DiagnosticCommunicationManager (R25-11)
 *          Functional Cluster: Diagnostic Services
 *
 * AUTOSAR Document: AUTOSAR_CP_SWS_DiagnosticCommunicationManager
 * Version        : R25-11
 */
#ifndef DCM_H
#define DCM_H

#include "Std_Types.h"
#include "ComStack_Types.h"

/* ================================================================
 * Type Definitions
 * ================================================================ */
typedef uint8  Dcm_StatusType;
typedef uint8  Dcm_SesCtrlType;   /**< UDS session type */
typedef uint8  Dcm_SecLevelType;  /**< UDS security level */
typedef uint8  Dcm_ProtocolType;

typedef enum {
    DCM_E_OK              = 0x00u,
    DCM_E_PENDING         = 0x0Au,
    DCM_E_ABORT           = 0x0Bu,
    DCM_E_FORCE_RCRRP     = 0x0Cu
} Dcm_ReturnReadMemType;

/* UDS Session Control Types — SWS_Dcm_01217 */
#define DCM_DEFAULT_SESSION        ((Dcm_SesCtrlType)0x01u)
#define DCM_PROGRAMMING_SESSION    ((Dcm_SesCtrlType)0x02u)
#define DCM_EXTENDED_DIAG_SESSION  ((Dcm_SesCtrlType)0x03u)

/* UDS Security Access Levels */
#define DCM_SEC_LEV_LOCKED     ((Dcm_SecLevelType)0x00u)
#define DCM_SEC_LEV_LEVEL1     ((Dcm_SecLevelType)0x01u)
#define DCM_SEC_LEV_LEVEL2     ((Dcm_SecLevelType)0x02u)

/* ================================================================
 * API  [SWS_Dcm_01061 .. SWS_Dcm_01155]
 * ================================================================ */
void           Dcm_Init(const void* ConfigPtr);
void           Dcm_DeInit(void);

/** Main function — handles UDS state machine and timeouts */
void           Dcm_MainFunction(void);

/** Get current diagnostic session [SWS_Dcm_01080] */
Std_ReturnType Dcm_GetSesCtrlType(Dcm_SesCtrlType* SesCtrlType);

/** Get current security level [SWS_Dcm_01081] */
Std_ReturnType Dcm_GetSecurityLevel(Dcm_SecLevelType* SecLevel);

/** Get active protocol [SWS_Dcm_01082] */
Std_ReturnType Dcm_GetActiveProtocol(Dcm_ProtocolType* ActiveProtocol);

/** Reset to default session [SWS_Dcm_01083] */
Std_ReturnType Dcm_ResetToDefaultSession(void);

/** Trigger transmission of response-pending [SWS_Dcm_00241] */
Std_ReturnType Dcm_TriggerOnEvent(uint8 RoeEventId);

/** Called by ComM on mode change */
void           Dcm_ComM_NoComModeEntered(uint8 NetworkId);
void           Dcm_ComM_SilentComModeEntered(uint8 NetworkId);
void           Dcm_ComM_FullComModeEntered(uint8 NetworkId);

void           Dcm_GetVersionInfo(Std_VersionInfoType* versioninfo);

#endif /* DCM_H */
