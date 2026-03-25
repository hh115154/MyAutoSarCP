/**
 * @file    EcuM.h
 * @brief   AUTOSAR CP R25-11 — ECU State Manager
 * @details Implements AUTOSAR_CP_SWS_ECUStateManager (R25-11)
 *          Functional Cluster: System Services
 *
 * AUTOSAR Document: AUTOSAR_CP_SWS_ECUStateManager
 * Version        : R25-11
 */
#ifndef ECUM_H
#define ECUM_H

#include "Std_Types.h"

/* ================================================================
 * ECU States  [SWS_EcuM_04016]
 * ================================================================ */
typedef uint8 EcuM_StateType;

#define ECUM_STATE_OFF            ((EcuM_StateType)0x80u)
#define ECUM_STATE_STARTUP        ((EcuM_StateType)0x10u)
#define ECUM_STATE_UP             ((EcuM_StateType)0x20u)
#define ECUM_STATE_APP_RUN        ((EcuM_StateType)0x22u)
#define ECUM_STATE_APP_POST_RUN   ((EcuM_StateType)0x24u)
#define ECUM_STATE_PREP_SHUTDOWN  ((EcuM_StateType)0x44u)
#define ECUM_STATE_GO_SLEEP       ((EcuM_StateType)0x49u)
#define ECUM_STATE_GO_OFF_ONE     ((EcuM_StateType)0x4Cu)
#define ECUM_STATE_GO_OFF_TWO     ((EcuM_StateType)0x4Eu)
#define ECUM_STATE_SLEEP          ((EcuM_StateType)0x50u)
#define ECUM_STATE_RESET          ((EcuM_StateType)0x90u)

/* ================================================================
 * Shutdown Target  [SWS_EcuM_04116]
 * ================================================================ */
typedef uint8 EcuM_ShutdownTargetType;

#define ECUM_SHUTDOWN_TARGET_SLEEP  ((EcuM_ShutdownTargetType)0x00u)
#define ECUM_SHUTDOWN_TARGET_RESET  ((EcuM_ShutdownTargetType)0x01u)
#define ECUM_SHUTDOWN_TARGET_OFF    ((EcuM_ShutdownTargetType)0x02u)

typedef uint8 EcuM_WakeupSourceType;
typedef uint8 EcuM_UserType;
typedef uint8 EcuM_BootTargetType;

/* ================================================================
 * API  [SWS_EcuM_04001 .. SWS_EcuM_04100]
 * ================================================================ */

/** Callout: BSW Init block 0 (before OS start) */
void EcuM_Init(void);

/** Called by OS task to handle the state machine */
void EcuM_MainFunction(void);

/** Request RUN state [SWS_EcuM_04062] */
Std_ReturnType EcuM_RequestRUN(EcuM_UserType user);

/** Release RUN state [SWS_EcuM_04063] */
Std_ReturnType EcuM_ReleaseRUN(EcuM_UserType user);

/** Request POST_RUN state [SWS_EcuM_04064] */
Std_ReturnType EcuM_RequestPOST_RUN(EcuM_UserType user);

/** Release POST_RUN state [SWS_EcuM_04065] */
Std_ReturnType EcuM_ReleasePOST_RUN(EcuM_UserType user);

/** Select shutdown target [SWS_EcuM_04069] */
Std_ReturnType EcuM_SelectShutdownTarget(EcuM_ShutdownTargetType shutdownTarget, uint8 sleepMode);

/** Get the current ECU state [SWS_EcuM_04075] */
Std_ReturnType EcuM_GetState(EcuM_StateType* state);

/** Indicate detected wakeup source [SWS_EcuM_04093] */
void EcuM_SetWakeupEvent(EcuM_WakeupSourceType sources);

/** Validate wakeup event [SWS_EcuM_04094] */
void EcuM_ValidateWakeupEvent(EcuM_WakeupSourceType sources);

/** Called by OS shutdown hook */
void EcuM_Shutdown(void);

/** Get version info */
void EcuM_GetVersionInfo(Std_VersionInfoType* versioninfo);

#endif /* ECUM_H */
