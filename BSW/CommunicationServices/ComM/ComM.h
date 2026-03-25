/**
 * @file    ComM.h
 * @brief   AUTOSAR CP R25-11 — Communication Manager
 * @details Implements AUTOSAR_CP_SWS_CommunicationManager (R25-11)
 *          Functional Cluster: Communication Services
 *
 * AUTOSAR Document: AUTOSAR_CP_SWS_CommunicationManager
 * Version        : R25-11
 */
#ifndef COMM_H
#define COMM_H

#include "Std_Types.h"
#include "ComStack_Types.h"

/* ================================================================
 * Type Definitions
 * ================================================================ */
typedef uint8 ComM_UserHandleType;     /**< User handle */
typedef uint8 ComM_ModeType;           /**< Communication mode */
typedef uint8 ComM_InitiatorType;

typedef enum {
    COMM_NO_COMMUNICATION     = 0x00u,  /**< No bus communication */
    COMM_SILENT_COMMUNICATION = 0x01u,  /**< Receive only */
    COMM_FULL_COMMUNICATION   = 0x02u   /**< Full bidirectional */
} ComM_ModeEnum;

typedef enum {
    COMM_INHIBIT_WAKE_UP  = 0x01u,
    COMM_PREVENT_WAKE_UP  = 0x02u
} ComM_InhibitionStatusType;

/* Channel IDs (project-specific, generated) */
#define COMM_CHANNEL_CAN0      ((NetworkHandleType)0x00u)
#define COMM_CHANNEL_CAN1      ((NetworkHandleType)0x01u)
#define COMM_CHANNEL_ETH0      ((NetworkHandleType)0x02u)

/* User IDs */
#define COMM_USER_ENGINE_SWC   ((ComM_UserHandleType)0x00u)
#define COMM_USER_BATTERY_SWC  ((ComM_UserHandleType)0x01u)

/* ================================================================
 * API  [SWS_ComM_00793 .. SWS_ComM_00822]
 * ================================================================ */
void           ComM_Init(const void* ConfigPtr);
void           ComM_DeInit(void);
Std_ReturnType ComM_RequestComMode(ComM_UserHandleType User, ComM_ModeType ComMode);
Std_ReturnType ComM_GetRequestedComMode(ComM_UserHandleType User, ComM_ModeType* ComModePtr);
Std_ReturnType ComM_GetCurrentComMode(ComM_UserHandleType User, ComM_ModeType* ComModePtr);
Std_ReturnType ComM_PreventWakeUp(NetworkHandleType Channel, boolean Status);
Std_ReturnType ComM_LimitChannelToNoComMode(NetworkHandleType Channel, boolean Status);
Std_ReturnType ComM_LimitECUToNoComMode(boolean Status);
Std_ReturnType ComM_ReadInhibitCounter(uint16* CounterValue);
Std_ReturnType ComM_ResetInhibitCounter(void);
Std_ReturnType ComM_SetECUGroupClassification(ComM_InhibitionStatusType Status);
void           ComM_GetVersionInfo(Std_VersionInfoType* versioninfo);
void           ComM_MainFunction(NetworkHandleType Channel);

#endif /* COMM_H */
