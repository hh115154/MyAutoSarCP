/**
 * @file    Dem.c  —  Host-build stub
 */
#include "Dem.h"

void Dem_PreInit(const void* cfg)  { (void)cfg; }
void Dem_Init(const void* cfg)     { (void)cfg; }
void Dem_Shutdown(void)            {}
void Dem_MainFunction(void)        {}

Std_ReturnType Dem_SetEventStatus(Dem_EventIdType id, Dem_EventStatusType st) {
    (void)id; (void)st;
    return E_OK;
}

Std_ReturnType Dem_GetEventStatus(Dem_EventIdType id, Dem_EventStatusType* st) {
    (void)id;
    if (!st) return E_NOT_OK;
    *st = DEM_EVENT_STATUS_PASSED;
    return E_OK;
}

Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType id, Dem_DTCFormatType fmt, Dem_DTCType* dtc) {
    (void)id; (void)fmt;
    if (!dtc) return E_NOT_OK;
    *dtc = 0u;
    return E_OK;
}

Std_ReturnType Dem_ClearDTC(Dem_DTCType dtc, Dem_DTCFormatType fmt, Dem_DTCOriginType orig) {
    (void)dtc; (void)fmt; (void)orig;
    return E_OK;
}

Dem_ReturnSetFilterType Dem_SetDTCFilter(Dem_DTCStatusMaskType m, Dem_DTCFormatType f,
    Dem_DTCOriginType o, Dem_FilterWithSeverityType ws, Dem_DTCSeverityType sv) {
    (void)m; (void)f; (void)o; (void)ws; (void)sv;
    return 0u;
}

Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredDTC(Dem_DTCType* dtc, Dem_DTCStatusMaskType* st) {
    (void)dtc; (void)st;
    return 0u;
}

Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleIdType id, uint8 state) {
    (void)id; (void)state;
    return E_OK;
}

Std_ReturnType Dem_EnableDTCSetting(Dem_DTCType d, Dem_DTCFormatType f, Dem_DTCOriginType o) {
    (void)d; (void)f; (void)o;
    return E_OK;
}

Std_ReturnType Dem_DisableDTCSetting(Dem_DTCType d, Dem_DTCFormatType f, Dem_DTCOriginType o) {
    (void)d; (void)f; (void)o;
    return E_OK;
}

void Dem_GetVersionInfo(Std_VersionInfoType* v) { (void)v; }
