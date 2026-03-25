/**
 * @file test_swc_battery.cpp
 * @brief Unit tests for SwcBattery (AUTOSAR CP R25-11)
 */
#include <gtest/gtest.h>
extern "C" {
#include "Rte.h"
#include "SwcBattery.h"
}

class SwcBatteryTest : public ::testing::Test {
protected:
    void SetUp() override {
        Rte_Start();
        SwcBattery_Init();
    }
    void TearDown() override { Rte_Stop(); }
};

TEST_F(SwcBatteryTest, InitStateIsNormal) {
    EXPECT_EQ(BATTERY_STATE_NORMAL, SwcBattery_GetData()->state);
}

TEST_F(SwcBatteryTest, InitVoltageIs12p6) {
    EXPECT_FLOAT_EQ(12.6f, SwcBattery_GetData()->voltage);
}

TEST_F(SwcBatteryTest, InitCurrentIsZero) {
    EXPECT_FLOAT_EQ(0.0f, SwcBattery_GetData()->current);
}

TEST_F(SwcBatteryTest, InitEngineRunningIsOff) {
    EXPECT_EQ(0u, SwcBattery_GetData()->engineRunning);
}

TEST_F(SwcBatteryTest, GetDataNotNull) {
    EXPECT_NE(nullptr, SwcBattery_GetData());
}

TEST_F(SwcBatteryTest, VoltagePublishedToRte) {
    /* After Init, RTE voltage IRV should be updated */
    SwcBattery_MainFunction_10ms();
    BatteryVoltageType v = 0.0f;
    Rte_Read_EngineSWC_BatteryVoltage(&v);
    /* Voltage should be in a sane range */
    EXPECT_GT(v, 10.0f);
    EXPECT_LT(v, 18.0f);
}

TEST_F(SwcBatteryTest, StateNormalWhenVoltageOk) {
    /* Voltage 12.6 V -> NORMAL */
    SwcBattery_MainFunction_10ms();
    SwcBattery_MainFunction_100ms();
    EXPECT_EQ(BATTERY_STATE_NORMAL, SwcBattery_GetData()->state);
}

TEST_F(SwcBatteryTest, EngineRunningAffectsCurrent) {
    /* Simulate engine running */
    Rte_Write_EngineSWC_EngineRunning(1u);
    SwcBattery_MainFunction_10ms();
    /* When engine running, current should be negative (charging) */
    EXPECT_LT(SwcBattery_GetData()->current, 0.0f);
}

TEST_F(SwcBatteryTest, EngineOffPositiveCurrent) {
    Rte_Write_EngineSWC_EngineRunning(0u);
    SwcBattery_MainFunction_10ms();
    EXPECT_GT(SwcBattery_GetData()->current, 0.0f);
}
