/**
 * @file    SwcBattery.h
 * @brief   AUTOSAR CP R25-11 — Battery Monitor SWC
 * @details Atomic Software Component measuring battery voltage/current
 *          and triggering diagnostic events.
 *
 * Component Type : Atomic SWC (Application Layer)
 * AUTOSAR Version: R25-11
 */
#ifndef SWC_BATTERY_H
#define SWC_BATTERY_H

#include "Rte.h"

typedef enum {
    BATTERY_STATE_NORMAL     = 0x00u,
    BATTERY_STATE_LOW        = 0x01u,  /**< < 10.5 V */
    BATTERY_STATE_CRITICAL   = 0x02u,  /**< < 9.0 V  */
    BATTERY_STATE_OVERVOLTAGE = 0x03u  /**< > 16.0 V */
} SwcBattery_StateType;

typedef struct {
    SwcBattery_StateType state;
    BatteryVoltageType   voltage;
    float32              current;
    uint8                engineRunning;
} SwcBattery_DataType;

/* Runnable Entry Points */
void SwcBattery_Init(void);
void SwcBattery_MainFunction_10ms(void);
void SwcBattery_MainFunction_100ms(void);

const SwcBattery_DataType* SwcBattery_GetData(void);

#endif /* SWC_BATTERY_H */
