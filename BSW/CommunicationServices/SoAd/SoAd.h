/**
 * @file    SoAd.h
 * @brief   AUTOSAR CP R25-11 — Socket Adaptor (SoAd) 接口
 *
 * 规范依据：AUTOSAR_SWS_SocketAdaptor（SWS_SoAd）
 * 职责：
 *   - 将 AUTOSAR PDU Router 的 PDU 映射到 BSD socket（TCP/UDP）
 *   - 管理 Socket Connection Group（SCG）生命周期
 *   - 向上层（SOME/IP TP / DoIP）提供统一 Transmit / Receive 接口
 *
 * 本实现为 Host-build 仿真，底层使用 POSIX socket。
 */

#ifndef SOAD_H
#define SOAD_H

#include "Std_Types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * SoAd 模块状态
 * ================================================================ */
typedef enum {
    SOAD_STATE_UNINIT  = 0x00u, /**< 未初始化          */
    SOAD_STATE_INIT    = 0x01u  /**< 已初始化，可使用   */
} SoAd_StateType;

/* ================================================================
 * Socket Connection Group ID（与 AUTOSAR 配置对应）
 * ================================================================ */
typedef uint8_t SoAd_SoConGroupIdType;

#define SOAD_SOCONGROUP_VEHICLE_SVC  0x01u  /**< VehicleSignalService UDP TX */
#define SOAD_SOCONGROUP_SD           0x02u  /**< SOME/IP SD UDP Multicast    */

/* ================================================================
 * SoAd 配置结构（Host-build 简化版）
 * ================================================================ */
typedef struct {
    const char* localAddr;      /**< 绑定本地 IP（"127.0.0.1"） */
    uint16_t    localPort;      /**< 绑定本地端口              */
    const char* remoteAddr;     /**< 远端 IP（"127.0.0.1"）    */
    uint16_t    remotePort;     /**< 远端端口                  */
    uint8_t     isTcp;          /**< 0=UDP, 1=TCP               */
    SoAd_SoConGroupIdType groupId;
} SoAd_SoConConfigType;

/* ================================================================
 * API
 * ================================================================ */

/**
 * @brief 模块初始化
 */
void SoAd_Init(void);

/**
 * @brief 打开 Socket Connection
 * @return E_OK / E_NOT_OK
 */
Std_ReturnType SoAd_OpenSoConGroup(SoAd_SoConGroupIdType groupId,
                                   const SoAd_SoConConfigType* cfg);

/**
 * @brief 发送 PDU（UDP 单播）
 * @param groupId  目标 Socket Connection Group
 * @param data     数据指针
 * @param len      数据长度（字节）
 * @return E_OK / E_NOT_OK
 */
Std_ReturnType SoAd_IfTransmit(SoAd_SoConGroupIdType groupId,
                                const uint8* data, uint16 len);

/**
 * @brief 非阻塞接收（返回实际接收字节数，0 = 无数据）
 */
uint16 SoAd_IfReceive(SoAd_SoConGroupIdType groupId,
                      uint8* buf, uint16 maxLen);

/**
 * @brief 关闭 Socket Connection
 */
void SoAd_CloseSoConGroup(SoAd_SoConGroupIdType groupId);

/**
 * @brief 模块去初始化
 */
void SoAd_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* SOAD_H */
