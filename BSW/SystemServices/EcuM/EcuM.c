/**
 * @file    EcuM.c  —  Host-build stub
 */
#include "EcuM.h"
#include <stdio.h>

static EcuM_StateType s_State = ECUM_STATE_STARTUP;

void EcuM_Init(void) {
    s_State = ECUM_STATE_STARTUP;
    printf("[EcuM ] Init (pre-OS)\n");
}

void EcuM_MainFunction(void) {
    /* state machine tick — nop on host */
}

Std_ReturnType EcuM_RequestRUN(EcuM_UserType user) {
    (void)user;
    s_State = ECUM_STATE_APP_RUN;
    printf("[EcuM ] State -> APP_RUN\n");
    return E_OK;
}

Std_ReturnType EcuM_ReleaseRUN(EcuM_UserType user) {
    (void)user;
    s_State = ECUM_STATE_APP_POST_RUN;
    return E_OK;
}

Std_ReturnType EcuM_RequestPOST_RUN(EcuM_UserType user) {
    (void)user;
    return E_OK;
}

Std_ReturnType EcuM_ReleasePOST_RUN(EcuM_UserType user) {
    (void)user;
    return E_OK;
}

Std_ReturnType EcuM_SelectShutdownTarget(EcuM_ShutdownTargetType t, uint8 m) {
    (void)t; (void)m;
    return E_OK;
}

Std_ReturnType EcuM_GetState(EcuM_StateType* state) {
    if (!state) return E_NOT_OK;
    *state = s_State;
    return E_OK;
}

void EcuM_SetWakeupEvent(EcuM_WakeupSourceType sources) { (void)sources; }
void EcuM_ValidateWakeupEvent(EcuM_WakeupSourceType sources) { (void)sources; }

void EcuM_Shutdown(void) {
    s_State = ECUM_STATE_GO_OFF_ONE;
    printf("[EcuM ] Shutdown sequence\n");
}

void EcuM_GetVersionInfo(Std_VersionInfoType* v) {
    if (!v) return;
    v->vendorID = 0x0000u;
    v->moduleID = 0x0010u;
    v->sw_major_version = 25u;
    v->sw_minor_version = 11u;
    v->sw_patch_version = 0u;
}
