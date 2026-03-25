/**
 * @file test_rte.cpp
 * @brief Unit tests for RTE stub (AUTOSAR CP R25-11)
 */
#include <gtest/gtest.h>
extern "C" {
#include "Rte.h"
}

class RteTest : public ::testing::Test {
protected:
    void SetUp() override { Rte_Start(); }
    void TearDown() override { Rte_Stop(); }
};

TEST_F(RteTest, StartReturnsOk) {
    EXPECT_EQ(E_OK, Rte_Start());
}

TEST_F(RteTest, StopReturnsOk) {
    EXPECT_EQ(E_OK, Rte_Stop());
}

TEST_F(RteTest, WriteReadEngineSpeed) {
    EXPECT_EQ(RTE_E_OK, Rte_Write_EngineSWC_EngineSpeed(1500.0f));
    EXPECT_FLOAT_EQ(1500.0f, Rte_Irv_EngineSpeed);
}

TEST_F(RteTest, WriteReadBatteryVoltage) {
    EXPECT_EQ(RTE_E_OK, Rte_Write_BatterySWC_BatteryVoltage(13.5f));
    BatteryVoltageType v = 0.0f;
    EXPECT_EQ(RTE_E_OK, Rte_Read_EngineSWC_BatteryVoltage(&v));
    EXPECT_FLOAT_EQ(13.5f, v);
}

TEST_F(RteTest, ReadNullPointerReturnsInvalid) {
    EXPECT_EQ(RTE_E_INVALID, Rte_Read_EngineSWC_BatteryVoltage(nullptr));
}

TEST_F(RteTest, NvMBatteryDataWriteRead) {
    const uint8_t data[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    EXPECT_EQ(RTE_E_OK, Rte_Write_BatterySWC_NvMBatteryData(data));
    uint8_t out[16] = {};
    EXPECT_EQ(RTE_E_OK, Rte_Read_BatterySWC_NvMBatteryData(out));
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(data[i], out[i]);
    }
}

TEST_F(RteTest, EngineRunningDefaultIsZero) {
    EngineRunningType r = 0xFFu;
    EXPECT_EQ(RTE_E_OK, Rte_Read_BatterySWC_EngineRunning(&r));
    EXPECT_EQ(0u, r);
}

TEST_F(RteTest, WriteEngineRunningThenRead) {
    EXPECT_EQ(RTE_E_OK, Rte_Write_EngineSWC_EngineRunning(1u));
    EngineRunningType r = 0u;
    EXPECT_EQ(RTE_E_OK, Rte_Read_BatterySWC_EngineRunning(&r));
    EXPECT_EQ(1u, r);
}

TEST_F(RteTest, NvMWriteNullReturnsInvalid) {
    EXPECT_EQ(RTE_E_INVALID, Rte_Write_BatterySWC_NvMBatteryData(nullptr));
    EXPECT_EQ(RTE_E_INVALID, Rte_Read_BatterySWC_NvMBatteryData(nullptr));
}

TEST_F(RteTest, IrvDefaultValues) {
    Rte_Start();
    EXPECT_FLOAT_EQ(0.0f,  Rte_Irv_EngineSpeed);
    EXPECT_FLOAT_EQ(25.0f, Rte_Irv_EngineTemp);
    EXPECT_FLOAT_EQ(12.0f, Rte_Irv_BatteryVoltage);
}
