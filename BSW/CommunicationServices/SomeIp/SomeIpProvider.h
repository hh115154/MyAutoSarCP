/**
 * @file    SomeIpProvider.h
 * @brief   AUTOSAR CP — SOME/IP VehicleSignalService Provider 接口
 *
 * 职责：
 *   - 周期性打包 VehicleSignalPayload_t（来自 SwcEngine + SwcBattery RTE 端口）
 *   - 封装 SOME/IP 16B 头 + Payload，通过 SoAd 发送到 SOC（UDP 30501）
 *   - 实现 E2E Profile 2 CRC8 保护（计数器单调递增）
 *   - 实现简化 SOME/IP SD OfferService（UDP 广播 30490）
 *
 * 对应 IPC-ARCH-001 §2.4  Service ID = 0x1001
 */

#ifndef SOMEIP_PROVIDER_H
#define SOMEIP_PROVIDER_H

#include "Std_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 SOME/IP Provider
 *        - 调用 SoAd_Init() + SoAd_OpenSoConGroup()
 *        - 发送首次 SOME/IP SD OfferService
 */
void SomeIpProvider_Init(void);

/**
 * @brief 10 ms 主函数——周期性发布 VehicleSignalService Event
 *        需在 TASK_10MS 中调用
 */
void SomeIpProvider_MainFunction_10ms(void);

/**
 * @brief 反初始化（SOME/IP SD StopOfferService + SoAd_DeInit）
 */
void SomeIpProvider_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* SOMEIP_PROVIDER_H */
