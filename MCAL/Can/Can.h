/**
 * @file    Can.h
 * @brief   AUTOSAR CP R25-11 — CAN Driver
 * @details Implements AUTOSAR_CP_SWS_CANDriver (R25-11)
 *          Functional Cluster: MCAL / Communication Hardware Abstraction
 *
 * AUTOSAR Document: AUTOSAR_CP_SWS_CANDriver
 * Version        : R25-11
 */
#ifndef CAN_H
#define CAN_H

#include "Std_Types.h"
#include "ComStack_Types.h"

/* ================================================================
 * Type Definitions  [SWS_Can_00413]
 * ================================================================ */
typedef uint8   Can_HwHandleType;    /**< Hardware object handle */
typedef uint32  Can_IdType;          /**< CAN frame ID (11-bit or 29-bit) */
typedef uint8   Can_HwUnitType;      /**< CAN controller ID */
typedef uint8   Can_ErrorStateType;  /**< Error state of CAN controller */

/** Return type for Can_Write — SWS_Can_00212 */
typedef enum {
    CAN_OK       = 0x00u,   /**< Write accepted */
    CAN_NOT_OK   = 0x01u,   /**< Write failed */
    CAN_BUSY     = 0x02u    /**< Tx object busy */
} Can_ReturnType;

/** CAN controller state — SWS_Can_00101 */
typedef enum {
    CAN_CS_UNINIT  = 0x00u,
    CAN_CS_STARTED = 0x01u,
    CAN_CS_STOPPED = 0x02u,
    CAN_CS_SLEEP   = 0x03u
} Can_ControllerStateType;

/** PDU for CAN transmission — SWS_Can_00158 */
typedef struct {
    Can_IdType    id;           /**< CAN ID */
    PduIdType     swPduHandle;  /**< SW PDU handle */
    uint8         length;       /**< DLC (0..8 for classical, 0..64 FD) */
    const uint8*  sdu;          /**< Pointer to payload */
} Can_PduType;

/** Hardware object info for reception indication — SWS_Can_00725 */
typedef struct {
    Can_IdType       CanId;
    Can_HwHandleType Hoh;
    uint8            ControllerId;
} Can_HwType;

/* ================================================================
 * Controller IDs
 * ================================================================ */
#define CAN_CONTROLLER_0    ((Can_HwUnitType)0x00u)
#define CAN_CONTROLLER_1    ((Can_HwUnitType)0x01u)

/* ================================================================
 * API — SWS_Can_00223 .. SWS_Can_00395
 * ================================================================ */

/**
 * @brief   Initialize the CAN module [SWS_Can_00223]
 */
void Can_Init(const void* Config);

/**
 * @brief   De-initialize CAN module [SWS_Can_00363]
 */
void Can_DeInit(void);

/**
 * @brief   Set CAN controller mode [SWS_Can_00261]
 * @return  E_OK / E_NOT_OK
 */
Std_ReturnType Can_SetControllerMode(Can_HwUnitType Controller, Can_ControllerStateType Transition);

/**
 * @brief   Get CAN controller mode [SWS_Can_00229]
 */
Std_ReturnType Can_GetControllerMode(Can_HwUnitType Controller, Can_ControllerStateType* ControllerModePtr);

/**
 * @brief   Transmit CAN PDU [SWS_Can_00212]
 */
Can_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType* PduInfo);

/**
 * @brief   Disable CAN controller interrupts [SWS_Can_00202]
 */
void Can_DisableControllerInterrupts(Can_HwUnitType Controller);

/**
 * @brief   Enable CAN controller interrupts [SWS_Can_00203]
 */
void Can_EnableControllerInterrupts(Can_HwUnitType Controller);

/**
 * @brief   Check if wake-up occurred [SWS_Can_00360]
 */
Std_ReturnType Can_CheckWakeup(Can_HwUnitType Controller);

/**
 * @brief   Get CAN controller error state [SWS_Can_00453]
 */
Std_ReturnType Can_GetControllerErrorState(Can_HwUnitType ControllerId, Can_ErrorStateType* ErrorStatePtr);

/**
 * @brief   Get Rx error counter [SWS_Can_00511]
 */
Std_ReturnType Can_GetControllerRxErrorCounter(Can_HwUnitType ControllerId, uint8* RxErrorCounterPtr);

/**
 * @brief   Get Tx error counter [SWS_Can_00516]
 */
Std_ReturnType Can_GetControllerTxErrorCounter(Can_HwUnitType ControllerId, uint8* TxErrorCounterPtr);

/**
 * @brief   Main function for polling-mode CAN Tx [SWS_Can_00225]
 */
void Can_MainFunction_Write(void);

/**
 * @brief   Main function for polling-mode CAN Rx [SWS_Can_00226]
 */
void Can_MainFunction_Read(void);

/**
 * @brief   Return CAN driver version info [SWS_Can_00224]
 */
void Can_GetVersionInfo(Std_VersionInfoType* versioninfo);

#endif /* CAN_H */
