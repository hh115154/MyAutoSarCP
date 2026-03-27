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

/* ================================================================
 * HMI Input Interface — 由 HMI 下发的目标值（0 = 使用内部仿真）
 * ================================================================ */

/** @brief  HMI 下发的目标信号（车速/转速/转角）
 *          值 < 0 表示未设定，使用内部仿真状态机 */
typedef struct {
    float32 hmi_speed_kmh;          /**< 目标车速 km/h  (-1 = 不覆盖) */
    float32 hmi_rpm;                /**< 目标转速 RPM   (-1 = 不覆盖) */
    float32 hmi_steering_deg;       /**< 目标转角 deg   (-1 = 不覆盖) */
    uint8   hmi_brake;              /**< 制动踏板 0/1   (0xFF = 不覆盖) */
    uint8   hmi_door;               /**< 车门状态 bitmask (0xFF = 不覆盖) */
    float32 hmi_fuel_pct;           /**< 燃油液位 %     (-1 = 不覆盖) */
    uint8   hmi_valid;              /**< 1=有效 HMI 输入，0=无 */
} SwcEngine_HmiInput_t;

/** @brief  由外部（HMI 命令接收模块）调用，设置 HMI 输入目标值 */
void SwcEngine_SetHmiInput(const SwcEngine_HmiInput_t* input);

/** @brief  获取当前 HMI 输入值（供 SomeIpProvider 打包使用） */
const SwcEngine_HmiInput_t* SwcEngine_GetHmiInput(void);

#endif /* SWC_ENGINE_H */
