/**
 * @file    Det.h
 * @brief   AUTOSAR CP R25-11 — Default Error Tracer
 * @details Implements AUTOSAR_CP_SWS_DefaultErrorTracer (R25-11)
 *          Functional Cluster: System Services
 *
 * AUTOSAR Document: AUTOSAR_CP_SWS_DefaultErrorTracer
 * Version        : R25-11
 */
#ifndef DET_H
#define DET_H

#include "Std_Types.h"

/* ================================================================
 * DET — Module/API IDs (project-assigned)
 * ================================================================ */
#define DET_MODULE_ID_DIO       ((uint16)0x012Du)
#define DET_MODULE_ID_CAN       ((uint16)0x0050u)
#define DET_MODULE_ID_COMM      ((uint16)0x00B4u)
#define DET_MODULE_ID_NVM       ((uint16)0x0050u)
#define DET_MODULE_ID_DCM       ((uint16)0x0035u)
#define DET_MODULE_ID_DEM       ((uint16)0x0036u)
#define DET_MODULE_ID_ECUM      ((uint16)0x0010u)

/* ================================================================
 * Error IDs (common, SWS_Det_00009)
 * ================================================================ */
#define DET_E_PARAM_POINTER     ((uint8)0x01u)  /**< NULL pointer passed */
#define DET_E_PARAM_VALUE       ((uint8)0x02u)  /**< Invalid parameter value */
#define DET_E_UNINIT            ((uint8)0x03u)  /**< Module not initialized */
#define DET_E_ALREADY_INIT      ((uint8)0x04u)  /**< Double initialization */
#define DET_E_TIMEOUT           ((uint8)0x05u)  /**< Operation timed out */

/* ================================================================
 * API  [SWS_Det_00008 .. SWS_Det_00012]
 * ================================================================ */

/**
 * @brief   Initialize DET module [SWS_Det_00008]
 */
void Det_Init(const void* ConfigPtr);

/**
 * @brief   Start DET module (enable error reporting) [SWS_Det_00012]
 */
void Det_Start(void);

/**
 * @brief   Report a development error [SWS_Det_00009]
 * @param   ModuleId     Module that detected the error
 * @param   InstanceId   Instance index within the module
 * @param   ApiId        API service that detected the error
 * @param   ErrorId      Error code
 * @return  E_OK always (or propagated to DET hook)
 */
Std_ReturnType Det_ReportError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);

/**
 * @brief   Report a runtime error [SWS_Det_00162] (AUTOSAR R20-11+)
 */
Std_ReturnType Det_ReportRuntimeError(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 ErrorId);

/**
 * @brief   Report a transient fault [SWS_Det_00161] (AUTOSAR R20-11+)
 */
Std_ReturnType Det_ReportTransientFault(uint16 ModuleId, uint8 InstanceId, uint8 ApiId, uint8 FaultId);

/**
 * @brief   Get version info [SWS_Det_00011]
 */
void Det_GetVersionInfo(Std_VersionInfoType* versioninfo);

#endif /* DET_H */
