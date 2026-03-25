/**
 * @file    WdgM.h
 * @brief   AUTOSAR CP R25-11 — Watchdog Manager
 * @details Implements AUTOSAR_CP_SWS_WatchdogManager (R25-11)
 *          Functional Cluster: System Services
 *
 * AUTOSAR Document: AUTOSAR_CP_SWS_WatchdogManager
 * Version        : R25-11
 */
#ifndef WDGM_H
#define WDGM_H

#include "Std_Types.h"

/* ================================================================
 * Type Definitions
 * ================================================================ */
typedef uint8  WdgM_SupervisedEntityIdType;  /**< Supervised entity ID */
typedef uint8  WdgM_CheckpointIdType;        /**< Checkpoint ID */
typedef uint8  WdgM_ModeType;                /**< Watchdog manager mode */

typedef enum {
    WDGM_LOCAL_STATUS_OK        = 0x00u,
    WDGM_LOCAL_STATUS_FAILED    = 0x01u,
    WDGM_LOCAL_STATUS_EXPIRED   = 0x02u,
    WDGM_LOCAL_STATUS_DEACTIVATED = 0x03u
} WdgM_LocalStatusType;

typedef enum {
    WDGM_GLOBAL_STATUS_OK      = 0x00u,
    WDGM_GLOBAL_STATUS_FAILED  = 0x01u,
    WDGM_GLOBAL_STATUS_EXPIRED = 0x02u,
    WDGM_GLOBAL_STATUS_STOPPED = 0x03u,
    WDGM_GLOBAL_STATUS_DEACTIVATED = 0x04u
} WdgM_GlobalStatusType;

/* Supervised entity & checkpoint IDs (project-specific) */
#define WDGM_SE_TASK_10MS       ((WdgM_SupervisedEntityIdType)0x00u)
#define WDGM_SE_TASK_100MS      ((WdgM_SupervisedEntityIdType)0x01u)
#define WDGM_SE_COMM            ((WdgM_SupervisedEntityIdType)0x02u)

#define WDGM_CP_ALIVE           ((WdgM_CheckpointIdType)0x00u)
#define WDGM_CP_START           ((WdgM_CheckpointIdType)0x01u)
#define WDGM_CP_END             ((WdgM_CheckpointIdType)0x02u)

/* ================================================================
 * API  [SWS_WdgM_00077 .. SWS_WdgM_00169]
 * ================================================================ */
void             WdgM_Init(const void* ConfigPtr);
void             WdgM_DeInit(void);
Std_ReturnType   WdgM_SetMode(WdgM_ModeType Mode, uint16 CallerID);
Std_ReturnType   WdgM_GetMode(WdgM_ModeType* Mode);
Std_ReturnType   WdgM_CheckpointReached(WdgM_SupervisedEntityIdType SEID, WdgM_CheckpointIdType CheckpointID);
Std_ReturnType   WdgM_GetLocalStatus(WdgM_SupervisedEntityIdType SEID, WdgM_LocalStatusType* Status);
Std_ReturnType   WdgM_GetGlobalStatus(WdgM_GlobalStatusType* Status);
void             WdgM_PerformReset(void);
uint8            WdgM_GetFirstExpiredSEID(WdgM_SupervisedEntityIdType* SEID);
void             WdgM_MainFunction(void);
void             WdgM_GetVersionInfo(Std_VersionInfoType* versioninfo);

#endif /* WDGM_H */
