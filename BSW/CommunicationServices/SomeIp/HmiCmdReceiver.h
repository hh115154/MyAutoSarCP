/**
 * @file    HmiCmdReceiver.h
 * @brief   AUTOSAR CP — HMI 命令接收模块
 *
 * 功能：
 *   - 监听 UDP 30503，接收来自 monitor_server 转发的 HMI 下行指令
 *   - 指令格式：简单 JSON 文本（monitor_server 不改 CP 就无需 SOME/IP 封装）
 *     {"speed_kmh":60.0,"rpm":2000.0,"steering_deg":15.0,
 *      "brake":0,"door":0,"fuel_pct":75.0}
 *   - 解析后调用 SwcEngine_SetHmiInput() 写入
 *   - 独立后台线程，不阻塞 AUTOSAR 主循环
 *
 * 端口规划：
 *   UDP 30503  ← monitor_server 转发 HMI 下行指令
 */

#ifndef HMI_CMD_RECEIVER_H
#define HMI_CMD_RECEIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief 初始化并启动 HMI 命令接收线程（监听 UDP:30503）*/
void HmiCmdReceiver_Init(void);

/** @brief 停止接收线程 */
void HmiCmdReceiver_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* HMI_CMD_RECEIVER_H */
