/**
 * @file test_swc_engine.cpp
 * @brief Unit tests for SwcEngine (AUTOSAR CP R25-11)
 */
#include <gtest/gtest.h>
extern "C" {
#include "Rte.h"
#include "SwcEngine.h"
}

class SwcEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        Rte_Start();
        SwcEngine_Init();
    }
    void TearDown() override { Rte_Stop(); }
};

TEST_F(SwcEngineTest, InitStateIsInit) {
    /* After Init, state machine moves to OFF in first cycle */
    const SwcEngine_DataType* d = SwcEngine_GetData();
    /* Init sets to ENGINE_STATE_INIT */
    EXPECT_EQ(ENGINE_STATE_INIT, d->state);
}

TEST_F(SwcEngineTest, InitSpeedIsZero) {
    EXPECT_FLOAT_EQ(0.0f, SwcEngine_GetData()->speedRpm);
}

TEST_F(SwcEngineTest, InitTempIs25) {
    EXPECT_FLOAT_EQ(25.0f, SwcEngine_GetData()->tempCelsius);
}

TEST_F(SwcEngineTest, InitNoFault) {
    EXPECT_EQ(0u, SwcEngine_GetData()->faultActive);
}

TEST_F(SwcEngineTest, RunTimeStartsAtZero) {
    EXPECT_EQ(0u, SwcEngine_GetData()->runTimeSeconds);
}

TEST_F(SwcEngineTest, Main100msTransitionsToOff) {
    SwcEngine_MainFunction_100ms(); /* INIT -> OFF */
    EXPECT_EQ(ENGINE_STATE_OFF, SwcEngine_GetData()->state);
}

TEST_F(SwcEngineTest, SetModeStartsEngine) {
    SwcEngine_MainFunction_100ms(); /* -> OFF */
    Rte_Write_BatterySWC_BatteryVoltage(12.6f); /* ensure good battery */
    /* Force OFF -> CRANKING via SetMode */
    Rte_StatusType r = SwcEngine_SetMode(0x01u);
    EXPECT_EQ(RTE_E_OK, r);
    EXPECT_EQ(ENGINE_STATE_CRANKING, SwcEngine_GetData()->state);
}

TEST_F(SwcEngineTest, SetModeInvalidInWrongState) {
    /* Can't start from INIT state */
    EXPECT_EQ(RTE_E_INVALID, SwcEngine_SetMode(0x01u));
}

TEST_F(SwcEngineTest, GetDataNotNull) {
    EXPECT_NE(nullptr, SwcEngine_GetData());
}

TEST_F(SwcEngineTest, WriteToRteAfterInit) {
    /* After init, RTE speed port should be 0 */
    EXPECT_FLOAT_EQ(0.0f, Rte_Irv_EngineSpeed);
}
