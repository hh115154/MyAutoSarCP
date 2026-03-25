/**
 * @file    SwcBattery.c
 * @brief   AUTOSAR CP R25-11 — Battery Monitor SWC Implementation
 */
#include "SwcBattery.h"

/* ================================================================
 * Private Data
 * ================================================================ */
static SwcBattery_DataType s_BattData = {
    .state         = BATTERY_STATE_NORMAL,
    .voltage       = 12.6f,
    .current       = 0.0f,
    .engineRunning = 0u
};

/* Simulated voltage drift */
static float32 s_SimVoltage = 12.6f;
static uint32  s_Cycle      = 0u;

/* ================================================================
 * Private helpers
 * ================================================================ */
static void Battery_EvaluateState(void)
{
    if (s_BattData.voltage > 16.0f) {
        if (s_BattData.state != BATTERY_STATE_OVERVOLTAGE) {
            s_BattData.state = BATTERY_STATE_OVERVOLTAGE;
            Rte_Call_EngineSWC_ReportDiagEvent(0x0002u, 1u); /* BATTERY_OVERVOLTAGE */
        }
    } else if (s_BattData.voltage < 9.0f) {
        if (s_BattData.state != BATTERY_STATE_CRITICAL) {
            s_BattData.state = BATTERY_STATE_CRITICAL;
            Rte_Call_EngineSWC_ReportDiagEvent(0x0001u, 1u); /* BATTERY_UNDERVOLTAGE */
        }
    } else if (s_BattData.voltage < 10.5f) {
        if (s_BattData.state != BATTERY_STATE_LOW) {
            s_BattData.state = BATTERY_STATE_LOW;
        }
    } else {
        if (s_BattData.state != BATTERY_STATE_NORMAL) {
            /* Recover: clear events */
            Rte_Call_EngineSWC_ReportDiagEvent(0x0001u, 0u);
            Rte_Call_EngineSWC_ReportDiagEvent(0x0002u, 0u);
        }
        s_BattData.state = BATTERY_STATE_NORMAL;
    }
}

/* ================================================================
 * Runnable: Init
 * ================================================================ */
void SwcBattery_Init(void)
{
    s_BattData.state         = BATTERY_STATE_NORMAL;
    s_BattData.voltage       = 12.6f;
    s_BattData.current       = 0.0f;
    s_BattData.engineRunning = 0u;
    s_SimVoltage             = 12.6f;
    s_Cycle                  = 0u;

    Rte_Write_BatterySWC_BatteryVoltage(12.6f);
    Rte_Write_BatterySWC_BatteryCurrent(0.0f);
}

/* ================================================================
 * Runnable: 10 ms — read ADC, publish voltage/current
 * ================================================================ */
void SwcBattery_MainFunction_10ms(void)
{
    /* Simulate slow discharge while engine is off, charge while running */
    Rte_Read_BatterySWC_EngineRunning(&s_BattData.engineRunning);

    s_Cycle++;
    if (s_BattData.engineRunning) {
        /* Engine running: slight charge */
        s_SimVoltage = 13.8f + 0.002f * (float32)(s_Cycle % 50u);
        s_BattData.current = -8.0f; /* Alternator charging */
    } else {
        /* Engine off: slight discharge */
        if (s_SimVoltage > 11.0f) {
            s_SimVoltage -= 0.001f;
        }
        s_BattData.current = 0.5f; /* Quiescent draw */
    }

    s_BattData.voltage = s_SimVoltage;

    Rte_Write_BatterySWC_BatteryVoltage(s_BattData.voltage);
    Rte_Write_BatterySWC_BatteryCurrent(s_BattData.current);
}

/* ================================================================
 * Runnable: 100 ms — state machine, NvM persistence
 * ================================================================ */
void SwcBattery_MainFunction_100ms(void)
{
    Battery_EvaluateState();

    /* Persist battery data via RTE NvM mirror port */
    Rte_Write_BatterySWC_NvMBatteryData(&s_BattData);
}

/* ================================================================
 * Accessor
 * ================================================================ */
const SwcBattery_DataType* SwcBattery_GetData(void)
{
    return &s_BattData;
}
