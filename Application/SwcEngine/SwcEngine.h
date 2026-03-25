/**
 * @file    SwcEngine.h
 * @brief   AUTOSAR CP R25-11 — Engine Control Software Component
 * @details Atomic Software Component (A-SWC) modeling engine control.
 *          Communicates via RTE Sender-Receiver and Client-Server ports.
 *
 * Component Type : Atomic SWC (Application Layer)
 * AUTOSAR Version: R25-11
 */
#ifndef SWC_ENGINE_H
#define SWC_ENGINE_H

#include "Rte.h"

/* ================================================================
 * Engine State Machine
 * ================================================================ */
typedef enum {
    ENGINE_STATE_INIT     = 0x00u,
    ENGINE_STATE_CRANKING = 0x01u,
    ENGINE_STATE_RUNNING  = 0x02u,
    ENGINE_STATE_IDLE     = 0x03u,
    ENGINE_STATE_STOPPING = 0x04u,
    ENGINE_STATE_OFF      = 0x05u
} SwcEngine_StateType;

typedef struct {
    SwcEngine_StateType state;
    EngineSpeedType     speedRpm;
    EngineTemperatureType tempCelsius;
    uint32              runTimeSeconds;
    uint8               faultActive;
} SwcEngine_DataType;

/* ================================================================
 * Runnable Entry Points (mapped to OS tasks via RTE)
 * ================================================================ */

/** @brief  Init runnable — called once on startup */
void SwcEngine_Init(void);

/** @brief  10 ms cyclic runnable — speed/temp processing */
void SwcEngine_MainFunction_10ms(void);

/** @brief  100 ms cyclic runnable — state machine, diagnostics */
void SwcEngine_MainFunction_100ms(void);

/** @brief  Server runnable — called by other SWCs to request mode */
Rte_StatusType SwcEngine_SetMode(uint8 requestedMode);

/** @brief  Get current engine data (for testing / DID read) */
const SwcEngine_DataType* SwcEngine_GetData(void);

#endif /* SWC_ENGINE_H */
