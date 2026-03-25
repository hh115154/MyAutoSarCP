/**
 * @file    Dem.h
 * @brief   AUTOSAR CP R25-11 — Diagnostic Event Manager
 * @details Implements AUTOSAR_CP_SWS_DiagnosticEventManager (R25-11)
 *          Functional Cluster: Diagnostic Services
 *
 * AUTOSAR Document: AUTOSAR_CP_SWS_DiagnosticEventManager
 * Version        : R25-11
 */
#ifndef DEM_H
#define DEM_H

#include "Std_Types.h"

/* ================================================================
 * Type Definitions
 * ================================================================ */
typedef uint16 Dem_EventIdType;         /**< DTC event ID */
typedef uint8  Dem_EventStatusType;     /**< Event status */
typedef uint8  Dem_OperationCycleIdType;
typedef uint8  Dem_DTCFormatType;
typedef uint32 Dem_DTCType;
typedef uint8  Dem_DTCOriginType;
typedef uint8  Dem_DTCStatusMaskType;
typedef uint8  Dem_FilterWithSeverityType;
typedef uint8  Dem_DTCSeverityType;
typedef uint8  Dem_ReturnClearDTCType;
typedef uint8  Dem_ReturnSetFilterType;
typedef uint8  Dem_ReturnGetNextFilteredDTCType;
typedef uint8  Dem_ReturnGetStatusOfDTCType;

/* Event Status Values */
#define DEM_EVENT_STATUS_PASSED           ((Dem_EventStatusType)0x00u)
#define DEM_EVENT_STATUS_FAILED           ((Dem_EventStatusType)0x01u)
#define DEM_EVENT_STATUS_PREPASSED        ((Dem_EventStatusType)0x02u)
#define DEM_EVENT_STATUS_PREFAILED        ((Dem_EventStatusType)0x03u)

/* DTC Format */
#define DEM_DTC_FORMAT_OBD     ((Dem_DTCFormatType)0x00u)
#define DEM_DTC_FORMAT_UDS     ((Dem_DTCFormatType)0x01u)
#define DEM_DTC_FORMAT_J1939   ((Dem_DTCFormatType)0x02u)

/* DTC Origin */
#define DEM_DTC_ORIGIN_PRIMARY_MEMORY   ((Dem_DTCOriginType)0x01u)
#define DEM_DTC_ORIGIN_MIRROR_MEMORY    ((Dem_DTCOriginType)0x02u)
#define DEM_DTC_ORIGIN_PERMANENT_MEMORY ((Dem_DTCOriginType)0x03u)

/* Operation Cycle IDs (project-specific) */
#define DEM_OPCYCLE_IGNITION  ((Dem_OperationCycleIdType)0x00u)
#define DEM_OPCYCLE_OBD       ((Dem_OperationCycleIdType)0x01u)

/* Event IDs (project-specific) */
#define DEM_EVENT_BATTERY_UNDERVOLTAGE   ((Dem_EventIdType)0x0001u)
#define DEM_EVENT_BATTERY_OVERVOLTAGE    ((Dem_EventIdType)0x0002u)
#define DEM_EVENT_CAN_BUS_OFF            ((Dem_EventIdType)0x0003u)
#define DEM_EVENT_ENGINE_OVERTEMP        ((Dem_EventIdType)0x0004u)
#define DEM_EVENT_NVM_WRITE_FAILED       ((Dem_EventIdType)0x0005u)

/* ================================================================
 * API  [SWS_Dem_00179 .. SWS_Dem_00588]
 * ================================================================ */
void Dem_PreInit(const void* ConfigPtr);
void Dem_Init(const void* ConfigPtr);
void Dem_Shutdown(void);

/** Report event status from SWC (via RTE) or BSW [SWS_Dem_00179] */
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType EventId, Dem_EventStatusType EventStatus);

/** Get current event status [SWS_Dem_00180] */
Std_ReturnType Dem_GetEventStatus(Dem_EventIdType EventId, Dem_EventStatusType* EventStatusByte);

/** Get DTC for an event [SWS_Dem_00269] */
Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType EventId, Dem_DTCFormatType DTCFormat, Dem_DTCType* DTCOfEvent);

/** Clear DTC group [SWS_Dem_00186] */
Std_ReturnType Dem_ClearDTC(Dem_DTCType DTC, Dem_DTCFormatType DTCFormat, Dem_DTCOriginType DTCOrigin);

/** Set DTC filter for iterating [SWS_Dem_00187] */
Dem_ReturnSetFilterType Dem_SetDTCFilter(Dem_DTCStatusMaskType DTCStatusMask,
                                          Dem_DTCFormatType DTCFormat,
                                          Dem_DTCOriginType DTCOrigin,
                                          Dem_FilterWithSeverityType FilterWithSeverity,
                                          Dem_DTCSeverityType DTCSeverityMask);

/** Get next DTC from filter [SWS_Dem_00198] */
Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredDTC(Dem_DTCType* DTC, Dem_DTCStatusMaskType* DTCStatus);

/** Start/restart operation cycle [SWS_Dem_00194] */
Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleIdType OperationCycleId, uint8 CycleState);

/** Enable/disable event [SWS_Dem_00195] */
Std_ReturnType Dem_EnableDTCSetting(Dem_DTCType DTC, Dem_DTCFormatType DTCFormat, Dem_DTCOriginType DTCOrigin);
Std_ReturnType Dem_DisableDTCSetting(Dem_DTCType DTC, Dem_DTCFormatType DTCFormat, Dem_DTCOriginType DTCOrigin);

/** Cyclic processing */
void Dem_MainFunction(void);

void Dem_GetVersionInfo(Std_VersionInfoType* versioninfo);

#endif /* DEM_H */
