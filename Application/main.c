/**
 * @file    main.c
 * @brief   AUTOSAR CP R25-11 — ECU Main Entry Point
 * @details Simulates the AUTOSAR CP startup sequence on host (PC).
 *          On real target, StartOS() never returns — tasks are scheduled by OS.
 *
 * AUTOSAR Startup Sequence:
 *   Phase 0: EcuM_Init() — before OS
 *   Phase 1: StartOS()
 *   Phase 2: TASK_INIT runnable
 *   Phase 3: Cyclic tasks (10ms, 100ms, Background)
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "Std_Types.h"
#include "EcuM.h"
#include "Rte.h"
#include "SwcEngine.h"
#include "SwcBattery.h"
#include "SomeIpProvider.h"  /* SOME/IP VehicleSignalService Provider */

/* ================================================================
 * Host-build simulation: cycle counter
 * ================================================================ */
static volatile int g_Running = 1;

static void signal_handler(int sig) {
    (void)sig;
    g_Running = 0;
}

/* ================================================================
 * TASK_INIT — executes once after OS start
 * ================================================================ */
static void Task_Init(void)
{
    printf("[EcuM ] Phase-0 init complete\n");
    printf("[RTE  ] Starting RTE...\n");
    Rte_Start();

    printf("[SWC  ] SwcEngine_Init\n");
    SwcEngine_Init();
    printf("[SWC  ] SwcBattery_Init\n");
    SwcBattery_Init();

    printf("[SOMEIP] SomeIpProvider_Init\n");
    SomeIpProvider_Init();

    printf("[EcuM ] Requesting RUN state (user=0)\n");
    EcuM_RequestRUN(0u);
}

/* ================================================================
 * TASK_10MS — 10 ms cyclic
 * ================================================================ */
static void Task_10ms(void)
{
    SwcEngine_MainFunction_10ms();
    SwcBattery_MainFunction_10ms();
    SomeIpProvider_MainFunction_10ms();  /* 发布 VehicleSignalService Event */
}

/* ================================================================
 * TASK_100MS — 100 ms cyclic
 * ================================================================ */
static void Task_100ms(void)
{
    SwcEngine_MainFunction_100ms();
    SwcBattery_MainFunction_100ms();
}

/* ================================================================
 * Main (host simulation — real target uses StartOS)
 * ================================================================ */
int main(void)
{
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    printf("==============================================\n");
    printf("  MyAutoSarCP  —  AUTOSAR CP R25-11\n");
    printf("  ECU Startup + SOME/IP Provider Simulation\n");
    printf("  VehicleSignalService → UDP 127.0.0.1:30501\n");
    printf("==============================================\n\n");

    /* ---- Phase-0: pre-OS init ---- */
    EcuM_Init();

    /* ---- Phase-1: OS start (simulated) ---- */
    printf("[OS   ] StartOS(OSDEFAULTAPPMODE)\n");
    Task_Init();
    printf("\n[OS   ] Entering cyclic task loop (Ctrl+C to stop)\n\n");

    /* ---- Phase-2: Cyclic simulation ---- */
    uint32 tick       = 0u;
    uint32 tick_100ms = 0u;

    while (g_Running) {
        Task_10ms();

        if (++tick_100ms >= 10u) {
            tick_100ms = 0u;
            Task_100ms();

            /* Print status every second (10 * 100ms) */
            if (tick % 10u == 0u) {
                const SwcEngine_DataType*  eng  = SwcEngine_GetData();
                const SwcBattery_DataType* batt = SwcBattery_GetData();

                printf("[%4us] Engine: state=%d  speed=%.0f rpm  temp=%.1f°C  "
                       "| Battery: %.2f V  %.1f A  state=%d\n",
                       tick / 10u,
                       (int)eng->state,  eng->speedRpm,  eng->tempCelsius,
                       batt->voltage,    batt->current,  (int)batt->state);
            }
            tick++;
        }

        /* Simulate 10 ms tick */
        usleep(10000);
    }

    /* ---- Shutdown ---- */
    printf("\n[EcuM ] Shutdown requested\n");
    SomeIpProvider_DeInit();
    Rte_Stop();
    EcuM_Shutdown();
    printf("[EcuM ] ECU off.\n");

    return EXIT_SUCCESS;
}
