/**
 * @file    SomeIpProvider.c
 * @brief   AUTOSAR CP — SOME/IP VehicleSignalService Provider 实现
 *
 * 规范：IPC-ARCH-001 §2 / AUTOSAR_PRS_SOMEIPProtocol
 *
 * 工作流程（10ms 周期）：
 *   1. 从 RTE 读取 SwcEngine + SwcBattery 数据
 *   2. 填充 VehicleSignalPayload_t（含 E2E CRC8 + Counter）
 *   3. 拼接 SOME/IP Header（Service 0x1001, Event 0x8001, Notification）
 *   4. SoAd_IfTransmit → UDP → SOC:30501
 *
 * 注：本机仿真中 MCU 侧绑定 127.0.0.1:40501，发往 SOC 127.0.0.1:30501
 *    （避免与 AP 进程绑定同一端口冲突）
 */

#include "SomeIpProvider.h"
#include "SoAd.h"
#include "SwcEngine.h"
#include "SwcBattery.h"

/* 共享协议头（来自 ipc_demo/someip_common/someip_proto.h 的副本路径）*/
#include "someip_proto.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* ----------------------------------------------------------------
 * 内部常量
 * ---------------------------------------------------------------- */
/** CP(MCU)侧绑定的本地端口（不与 AP 冲突） */
#define CP_LOCAL_EVT_PORT    40501u
#define CP_LOCAL_SD_PORT     40490u

/** AP(SOC)侧监听端口（Provider 发往这里） */
#define AP_REMOTE_EVT_PORT   30501u
#define AP_REMOTE_SD_PORT    30490u

#define SOAD_GROUP_EVT  SOAD_SOCONGROUP_VEHICLE_SVC
#define SOAD_GROUP_SD   SOAD_SOCONGROUP_SD

/* ----------------------------------------------------------------
 * 内部状态
 * ---------------------------------------------------------------- */
static uint16_t s_sessionId  = 1u;
static uint8_t  s_e2eCounter = 0u;
static uint8_t  s_sdSent     = 0u;

/* ----------------------------------------------------------------
 * 内部：发送 SOME/IP SD OfferService
 * ---------------------------------------------------------------- */
static void send_offer_service(void)
{
    /*
     * 简化版 SOME/IP SD：Service Entry Only
     * 完整 SD Header: SOME/IP Header(16B) + SD Header(4B) + Length(4B) + Entry(16B)
     * 这里发送一个最小可识别的 SD Offer 帧（SOC 侧只做简单校验）
     */

    /* --- SOME/IP SD Header（在 SOME/IP 标准头的 Payload 中） --- */
    /* SD Magic Header */
    uint8_t sd_flags    = 0xC0u;  /* reboot flag + unicast flag */

    /* SD Entry: OfferService (type=0x01) */
    SomeIpSdEntry_t entry;
    memset(&entry, 0, sizeof(entry));
    entry.type        = 0x01u;         /* OfferService */
    entry.service_id  = SOMEIP_HTONS(SVC_ID_VEHICLE_SIGNAL);
    entry.instance_id = SOMEIP_HTONS(VEHICLE_SIGNAL_INSTANCE);
    entry.major_ver   = 0x01u;
    entry.ttl[0]      = 0x00u;
    entry.ttl[1]      = 0x00u;
    entry.ttl[2]      = 0x03u;        /* TTL = 3s */
    entry.minor_ver   = SOMEIP_HTONL(0x00000000u);

    /* SOME/IP SD Payload layout:
     *   [0]     flags (1B)
     *   [1..3]  reserved (3B)
     *   [4..7]  entries array length = 16 (4B big-endian)
     *   [8..23] SD Entry (16B)
     *   [24..27] options array length = 0 (4B)
     */
    uint8_t sd_payload[28];
    memset(sd_payload, 0, sizeof(sd_payload));
    sd_payload[0] = sd_flags;
    /* entries length = 16 */
    sd_payload[4] = 0x00u; sd_payload[5] = 0x00u;
    sd_payload[6] = 0x00u; sd_payload[7] = 0x10u;
    memcpy(&sd_payload[8], &entry, sizeof(entry));
    /* options length = 0 */

    /* 构造 SOME/IP Header */
    SomeIpHeader_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.service_id   = SOMEIP_HTONS(0xFFFFu);  /* SD service */
    hdr.method_id    = SOMEIP_HTONS(0x8100u);  /* SD method  */
    hdr.length       = SOMEIP_HTONL((uint32_t)(8u + sizeof(sd_payload)));
    hdr.client_id    = SOMEIP_HTONS(0x0000u);
    {
        uint16_t sid = s_sessionId++;
        hdr.session_id = SOMEIP_HTONS(sid);
    }
    hdr.proto_ver    = SOMEIP_PROTO_VER;
    hdr.iface_ver    = 0x01u;
    hdr.msg_type     = SOMEIP_MSG_NOTIFICATION;
    hdr.return_code  = SOMEIP_RC_OK;

    /* 拼包 */
    uint8_t buf[16 + sizeof(sd_payload)];
    memcpy(buf, &hdr, 16u);
    memcpy(buf + 16u, sd_payload, sizeof(sd_payload));

    SoAd_IfTransmit(SOAD_GROUP_SD, buf, (uint16)(16u + sizeof(sd_payload)));
    printf("[SOMEIP_CP] SD OfferService sent (Service=0x1001, Instance=0x01)\n");
}

/* ================================================================
 * SomeIpProvider_Init
 * ================================================================ */
void SomeIpProvider_Init(void)
{
    SoAd_Init();

    /* 打开事件通知 Socket（UDP，发往 SOC:30501） */
    SoAd_SoConConfigType evtCfg;
    evtCfg.localAddr  = "127.0.0.1";
    evtCfg.localPort  = CP_LOCAL_EVT_PORT;
    evtCfg.remoteAddr = "127.0.0.1";
    evtCfg.remotePort = AP_REMOTE_EVT_PORT;
    evtCfg.isTcp      = 0u;
    evtCfg.groupId    = SOAD_GROUP_EVT;
    SoAd_OpenSoConGroup(SOAD_GROUP_EVT, &evtCfg);

    /* 打开 SD Socket（UDP，发往 SOC:30490） */
    SoAd_SoConConfigType sdCfg;
    sdCfg.localAddr  = "127.0.0.1";
    sdCfg.localPort  = CP_LOCAL_SD_PORT;
    sdCfg.remoteAddr = "127.0.0.1";
    sdCfg.remotePort = AP_REMOTE_SD_PORT;
    sdCfg.isTcp      = 0u;
    sdCfg.groupId    = SOAD_GROUP_SD;
    SoAd_OpenSoConGroup(SOAD_GROUP_SD, &sdCfg);

    /* 发送 SD OfferService */
    send_offer_service();
    s_sdSent = 1u;

    printf("[SOMEIP_CP] VehicleSignalService Provider initialized\n");
    printf("[SOMEIP_CP] Publishing to 127.0.0.1:%u every 10ms\n", AP_REMOTE_EVT_PORT);
}

/* ================================================================
 * SomeIpProvider_MainFunction_10ms
 * ================================================================ */
void SomeIpProvider_MainFunction_10ms(void)
{
    const SwcEngine_DataType*  eng  = SwcEngine_GetData();
    const SwcBattery_DataType* batt = SwcBattery_GetData();

    /* ---- 填充 Payload ---- */
    VehicleSignalPayload_t payload;
    memset(&payload, 0, sizeof(payload));
    payload.vehicle_speed_kmh   = eng->speedRpm * 0.05f;   /* 粗略换算 */
    payload.engine_rpm          = eng->speedRpm;
    payload.brake_pedal         = (eng->state == ENGINE_STATE_STOPPING) ? 1u : 0u;
    payload.steering_angle_deg  = 0.0f;                    /* 暂无转向 SWC */
    payload.door_status         = 0x00u;                   /* 暂无门控 SWC */
    payload.fuel_level_pct      = batt->voltage * 6.25f;   /* 模拟燃油 */
    payload.e2e_counter         = s_e2eCounter++;

    /* E2E CRC8（不含 crc 字节本身） */
    payload.e2e_crc = e2e_crc8((const uint8_t*)&payload,
                                sizeof(payload) - 2u);  /* 最后 2 字节=crc+counter */

    /* ---- 构造 SOME/IP Header ---- */
    SomeIpHeader_t hdr;
    memset(&hdr, 0, sizeof(hdr));
    hdr.service_id  = SOMEIP_HTONS(SVC_ID_VEHICLE_SIGNAL);  /* 0x1001 */
    hdr.method_id   = SOMEIP_HTONS(VEHICLE_SIGNAL_EVT_ID);  /* 0x8001 */
    hdr.length      = SOMEIP_HTONL((uint32_t)(8u + sizeof(payload)));
    hdr.client_id   = SOMEIP_HTONS(0x0000u);
    {
        uint16_t sid = s_sessionId++;
        hdr.session_id = SOMEIP_HTONS(sid);
    }
    hdr.proto_ver   = SOMEIP_PROTO_VER;
    hdr.iface_ver   = 0x01u;
    hdr.msg_type    = SOMEIP_MSG_NOTIFICATION;
    hdr.return_code = SOMEIP_RC_OK;

    /* ---- 拼包发送 ---- */
    uint8_t buf[16u + sizeof(payload)];
    memcpy(buf, &hdr, 16u);
    memcpy(buf + 16u, &payload, sizeof(payload));

    Std_ReturnType ret = SoAd_IfTransmit(SOAD_GROUP_EVT,
                                          buf,
                                          (uint16)(16u + sizeof(payload)));
    (void)ret;
}

/* ================================================================
 * SomeIpProvider_DeInit
 * ================================================================ */
void SomeIpProvider_DeInit(void)
{
    SoAd_DeInit();
    printf("[SOMEIP_CP] Provider deinitialized\n");
}
