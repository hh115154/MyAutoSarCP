/**
 * @file    Det.c
 * @brief   AUTOSAR CP R25-11 — Default Error Tracer 实现（Host-build stub）
 *
 * 规范：AUTOSAR_CP_SWS_DefaultErrorTracer
 * Host-build 实现：将所有 DET 错误打印到 stdout，供调试使用
 */

#include "Det.h"
#include <stdio.h>

static uint8 s_detInitialized = 0u;

void Det_Init(const void* ConfigPtr)
{
    (void)ConfigPtr;
    s_detInitialized = 1u;
}

void Det_Start(void)
{
    s_detInitialized = 1u;
}

Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId,
                                uint8 ApiId,    uint8 ErrorId)
{
    printf("[DET  ] ERROR: ModuleId=0x%04X  InstanceId=0x%02X  "
           "ApiId=0x%02X  ErrorId=0x%02X\n",
           ModuleId, InstanceId, ApiId, ErrorId);
    return E_OK;
}

Std_ReturnType Det_ReportRuntimeError(uint16 ModuleId, uint8 InstanceId,
                                       uint8 ApiId,    uint8 ErrorId)
{
    printf("[DET  ] RUNTIME: ModuleId=0x%04X  InstanceId=0x%02X  "
           "ApiId=0x%02X  ErrorId=0x%02X\n",
           ModuleId, InstanceId, ApiId, ErrorId);
    return E_OK;
}

Std_ReturnType Det_ReportTransientFault(uint16 ModuleId, uint8 InstanceId,
                                         uint8 ApiId,    uint8 FaultId)
{
    printf("[DET  ] TRANSIENT FAULT: ModuleId=0x%04X  InstanceId=0x%02X  "
           "ApiId=0x%02X  FaultId=0x%02X\n",
           ModuleId, InstanceId, ApiId, FaultId);
    return E_OK;
}

void Det_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
    if (versioninfo == NULL) return;
    versioninfo->vendorID         = 0x0000u;
    versioninfo->moduleID         = 0x000Fu;  /* DET Module ID */
    versioninfo->sw_major_version = 25u;
    versioninfo->sw_minor_version = 11u;
    versioninfo->sw_patch_version = 0u;
}
