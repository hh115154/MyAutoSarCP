/**
 * @file    SwcEngine.c
 * @brief   AUTOSAR CP R25-11 — Engine Control SWC Implementation
 */
#include "SwcEngine.h"
#include <string.h>

/* ================================================================
 * Private Data
 * ================================================================ */
static SwcEngine_DataType s_EngineData = {
    .state        = ENGINE_STATE_INIT,
    .speedRpm     = 0.0f,
    .tempCelsius  = 25.0f,
    .runTimeSeconds = 0u,
    .faultActive  = 0u
};

/* Simulated sensor input (host build: no real HW) */
static float32 s_SimulatedSpeed = 0.0f;
static float32 s_SimulatedTemp  = 25.0f;

/* HMI 下发的目标值（由 SomeIpCmdReceiver 写入，原子操作安全）*/
static SwcEngine_HmiInput_t s_HmiInput = {
    .hmi_speed_kmh    = -1.0f,
    .hmi_rpm          = -1.0f,
    .hmi_steering_deg = -1.0f,
    .hmi_brake        = 0xFFu,
    .hmi_door         = 0xFFu,
    .hmi_fuel_pct     = -1.0f,
    .hmi_valid        = 0u,
};

/* ================================================================
 * Private helpers
 * ================================================================ */
static void Engine_UpdateStateMachine(void)
{
    BatteryVoltageType vbat = 0.0f;
    Rte_Read_EngineSWC_BatteryVoltage(&vbat);

    switch (s_EngineData.state) {
        case ENGINE_STATE_INIT:
            s_EngineData.state = ENGINE_STATE_OFF;
            break;

        case ENGINE_STATE_OFF:
            /* Start cranking when battery OK */
            if (vbat >= 10.5f) {
                s_EngineData.state = ENGINE_STATE_CRANKING;
                s_SimulatedSpeed   = 400.0f;
            }
            break;

        case ENGINE_STATE_CRANKING:
            if (s_EngineData.speedRpm > 350.0f) {
                s_EngineData.state = ENGINE_STATE_RUNNING;
            }
            break;

        case ENGINE_STATE_RUNNING:
            s_EngineData.runTimeSeconds++;
            /* Simulate gentle warm-up */
            if (s_SimulatedTemp < 90.0f) {
                s_SimulatedTemp += 0.5f;
            }
            s_SimulatedSpeed = 800.0f + (float32)(s_EngineData.runTimeSeconds % 200u);

            /* Fault: overtemperature > 120 °C */
            if (s_EngineData.tempCelsius > 120.0f) {
                s_EngineData.faultActive = 1u;
                Rte_Call_EngineSWC_ReportDiagEvent(0x0004u, /* ENGINE_OVERTEMP */ 1u);
                s_EngineData.state = ENGINE_STATE_STOPPING;
            }
            break;

        case ENGINE_STATE_STOPPING:
            s_SimulatedSpeed = (s_SimulatedSpeed > 50.0f) ? (s_SimulatedSpeed - 100.0f) : 0.0f;
            if (s_SimulatedSpeed <= 0.0f) {
                s_SimulatedSpeed   = 0.0f;
                s_EngineData.state = ENGINE_STATE_OFF;
            }
            break;

        case ENGINE_STATE_IDLE:
            s_SimulatedSpeed = 750.0f;
            break;

        default:
            break;
    }
}

/* ================================================================
 * Public Runnable: Init
 * ================================================================ */
void SwcEngine_Init(void)
{
    s_EngineData.state          = ENGINE_STATE_INIT;
    s_EngineData.speedRpm       = 0.0f;
    s_EngineData.tempCelsius    = 25.0f;
    s_EngineData.runTimeSeconds = 0u;
    s_EngineData.faultActive    = 0u;
    s_SimulatedSpeed            = 0.0f;
    s_SimulatedTemp             = 25.0f;

    memset(&s_HmiInput, 0, sizeof(s_HmiInput));
    s_HmiInput.hmi_speed_kmh    = -1.0f;
    s_HmiInput.hmi_rpm          = -1.0f;
    s_HmiInput.hmi_steering_deg = -1.0f;
    s_HmiInput.hmi_brake        = 0xFFu;
    s_HmiInput.hmi_door         = 0xFFu;
    s_HmiInput.hmi_fuel_pct     = -1.0f;
    s_HmiInput.hmi_valid        = 0u;

    Rte_Write_EngineSWC_EngineSpeed(0.0f);
    Rte_Write_EngineSWC_EngineTemperature(25.0f);
    Rte_Write_EngineSWC_EngineRunning(0u);
}

/* ================================================================
 * Public Runnable: 10 ms — read sensors, write ports
 * ================================================================ */
void SwcEngine_MainFunction_10ms(void)
{
    /* HMI 有效输入时直接覆盖仿真值，否则使用内部状态机值 */
    if (s_HmiInput.hmi_valid) {
        if (s_HmiInput.hmi_rpm >= 0.0f) {
            s_SimulatedSpeed        = s_HmiInput.hmi_rpm;  /* rpm 直接设 */
        }
        /* speed_kmh 由 SomeIpProvider 打包时用 hmi 值，不影响 rpm 状态机 */
    }

    s_EngineData.speedRpm    = s_SimulatedSpeed;
    s_EngineData.tempCelsius = s_SimulatedTemp;

    Rte_Write_EngineSWC_EngineSpeed(s_EngineData.speedRpm);
    Rte_Write_EngineSWC_EngineTemperature(s_EngineData.tempCelsius);
    Rte_Write_EngineSWC_EngineRunning(
        (s_EngineData.state == ENGINE_STATE_RUNNING) ? 1u : 0u);
}

/* ================================================================
 * Public Runnable: 100 ms — state machine
 * ================================================================ */
void SwcEngine_MainFunction_100ms(void)
{
    Engine_UpdateStateMachine();
}

/* ================================================================
 * Client-Server: SetMode
 * ================================================================ */
Rte_StatusType SwcEngine_SetMode(uint8 requestedMode)
{
    if (requestedMode == 0x01u && s_EngineData.state == ENGINE_STATE_OFF) {
        s_EngineData.state = ENGINE_STATE_CRANKING;
        s_SimulatedSpeed   = 400.0f;
        return RTE_E_OK;
    }
    if (requestedMode == 0x00u && s_EngineData.state == ENGINE_STATE_RUNNING) {
        s_EngineData.state = ENGINE_STATE_STOPPING;
        return RTE_E_OK;
    }
    return RTE_E_INVALID;
}

/* ================================================================
 * Accessor (for unit tests / DID read)
 * ================================================================ */
const SwcEngine_DataType* SwcEngine_GetData(void)
{
    return &s_EngineData;
}

/* ================================================================
 * HMI Input — 由 SomeIpCmdReceiver 在收到 HMI_COMMAND 帧时调用
 * ================================================================ */
void SwcEngine_SetHmiInput(const SwcEngine_HmiInput_t* input)
{
    if (!input) return;
    s_HmiInput = *input;
    /* 如果 HMI 设置了有效转速，立即强制进入 RUNNING 状态 */
    if (input->hmi_valid && input->hmi_rpm >= 0.0f) {
        if (s_EngineData.state == ENGINE_STATE_OFF ||
            s_EngineData.state == ENGINE_STATE_INIT) {
            s_EngineData.state = ENGINE_STATE_RUNNING;
        }
    }
}

const SwcEngine_HmiInput_t* SwcEngine_GetHmiInput(void)
{
    return &s_HmiInput;
}
