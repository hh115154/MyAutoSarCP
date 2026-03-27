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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/* ----------------------------------------------------------------
 * 监控服务 socket（全局，Init 中创建，DeInit 中关闭）
 * ---------------------------------------------------------------- */
static int s_monitor_sock = -1;

/* ----------------------------------------------------------------
 * 内部常量
 * ---------------------------------------------------------------- */
/** CP(MCU)侧绑定的本地端口（不与 AP 冲突） */
#define CP_LOCAL_EVT_PORT    40501u
#define CP_LOCAL_SD_PORT     40490u

/** AP(SOC)侧监听端口（Provider 发往这里） */
#define AP_REMOTE_EVT_PORT   30501u
#define AP_REMOTE_SD_PORT    30490u

/** 监控服务(HMI)专用端口 — CP 额外复制一份到此，不干扰 AP 接收 */
#define MONITOR_REMOTE_EVT_PORT  30502u

#define SOAD_GROUP_EVT  SOAD_SOCONGROUP_VEHICLE_SVC
#define SOAD_GROUP_SD   SOAD_SOCONGROUP_SD

/* ----------------------------------------------------------------
 * 内部状态
 * ---------------------------------------------------------------- */
static uint16_t s_sessionId  = 1u;
static uint8_t  s_e2eCounter = 0u;
static uint8_t  s_sdSent     = 0u;
static uint32_t s_txCount    = 0u;  /* 发送帧计数，用于日志节流 */

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
    printf("[MCU_LOG] {\"level\":\"INFO\",\"module\":\"SomeIpProvider\","
           "\"event\":\"SD_OFFER_SENT\",\"svc_id\":\"0x1001\","
           "\"instance\":\"0x01\",\"ttl\":3}\n");
    fflush(stdout);
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

    /* 创建监控服务专用 UDP socket（发往 127.0.0.1:30502）*/
    s_monitor_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_monitor_sock >= 0) {
        printf("[MCU_LOG] {\"level\":\"INFO\",\"module\":\"SomeIpProvider\","
               "\"event\":\"MONITOR_SOCK_READY\",\"mirror_port\":%u}\n",
               MONITOR_REMOTE_EVT_PORT);
        fflush(stdout);
    }

    printf("[MCU_LOG] {\"level\":\"INFO\",\"module\":\"SomeIpProvider\","
           "\"event\":\"INIT_OK\",\"svc_id\":\"0x1001\","
           "\"ap_port\":%u,\"period_ms\":10}\n",
           AP_REMOTE_EVT_PORT);
    printf("[MCU_LOG] {\"level\":\"INFO\",\"module\":\"SomeIpProvider\","
           "\"event\":\"PUBLISHING\",\"dest\":\"127.0.0.1\",\"port\":%u}\n",
           AP_REMOTE_EVT_PORT);
    fflush(stdout);
}

/* ================================================================
 * SomeIpProvider_MainFunction_10ms
 * ================================================================ */
void SomeIpProvider_MainFunction_10ms(void)
{
    const SwcEngine_DataType*    eng   = SwcEngine_GetData();
    const SwcBattery_DataType*   batt  = SwcBattery_GetData();
    const SwcEngine_HmiInput_t*  hmi   = SwcEngine_GetHmiInput();

    /* ---- 填充 Payload ---- */
    VehicleSignalPayload_t payload;
    memset(&payload, 0, sizeof(payload));

    /* 优先使用 HMI 输入，否则用内部仿真值 */
    payload.vehicle_speed_kmh  = (hmi->hmi_valid && hmi->hmi_speed_kmh >= 0.0f)
                                 ? hmi->hmi_speed_kmh
                                 : eng->speedRpm * 0.05f;

    payload.engine_rpm         = (hmi->hmi_valid && hmi->hmi_rpm >= 0.0f)
                                 ? hmi->hmi_rpm
                                 : eng->speedRpm;

    payload.brake_pedal        = (hmi->hmi_valid && hmi->hmi_brake != 0xFFu)
                                 ? hmi->hmi_brake
                                 : (eng->state == ENGINE_STATE_STOPPING) ? 1u : 0u;

    /* 转角 HMI 覆盖：哨兵值 -1.0f 表示不设定，其他值（含负值）直接使用 */
    payload.steering_angle_deg = (hmi->hmi_valid && hmi->hmi_steering_deg != -1.0f)
                                 ? hmi->hmi_steering_deg
                                 : 0.0f;

    payload.door_status        = (hmi->hmi_valid && hmi->hmi_door != 0xFFu)
                                 ? hmi->hmi_door
                                 : 0x00u;

    payload.fuel_level_pct     = (hmi->hmi_valid && hmi->hmi_fuel_pct >= 0.0f)
                                 ? hmi->hmi_fuel_pct
                                 : batt->voltage * 6.25f;

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
    s_txCount++;

    /* 每 10 帧（约 100ms）输出一条发布日志，清晰记录每次 SOME/IP 信号发布 */
    if (s_txCount % 10u == 1u) {
        printf("[MCU_LOG] {\"level\":\"INFO\",\"module\":\"SomeIpProvider\","
               "\"event\":\"SOMEIP_TX_PUBLISH\","
               "\"dest\":\"127.0.0.1:30501\",\"proto\":\"SOME/IP\","
               "\"svc_id\":\"0x1001\",\"evt_id\":\"0x8001\","
               "\"session\":%u,\"e2e_cnt\":%u,\"e2e_crc\":%u,"
               "\"speed\":%.2f,\"rpm\":%.1f,\"steer\":%.2f,"
               "\"brake\":%d,\"door\":%d,\"fuel\":%.2f,"
               "\"hmi_valid\":%d,\"tx_total\":%u}\n",
               (unsigned)(s_sessionId - 1u),
               (unsigned)payload.e2e_counter,
               (unsigned)payload.e2e_crc,
               payload.vehicle_speed_kmh,
               payload.engine_rpm,
               payload.steering_angle_deg,
               (int)payload.brake_pedal,
               (int)payload.door_status,
               payload.fuel_level_pct,
               (int)hmi->hmi_valid,
               (unsigned)s_txCount);
        fflush(stdout);
    }

    /* 每 100 帧（约 1s）额外输出一条统计摘要（保留原有 TX_FRAME）*/
    if (s_txCount % 100u == 1u) {
        printf("[MCU_LOG] {\"level\":\"INFO\",\"module\":\"SomeIpProvider\","
               "\"event\":\"TX_FRAME\","
               "\"session\":%u,\"e2e_cnt\":%u,\"e2e_crc\":%u,"
               "\"speed\":%.1f,\"rpm\":%.0f,\"steer\":%.1f,"
               "\"brake\":%d,\"door\":%d,\"fuel\":%.1f,"
               "\"hmi_valid\":%d,\"tx_total\":%u}\n",
               (unsigned)(s_sessionId - 1u),
               (unsigned)payload.e2e_counter,
               (unsigned)payload.e2e_crc,
               payload.vehicle_speed_kmh,
               payload.engine_rpm,
               payload.steering_angle_deg,
               (int)payload.brake_pedal,
               (int)payload.door_status,
               payload.fuel_level_pct,
               (int)hmi->hmi_valid,
               (unsigned)s_txCount);
        fflush(stdout);
    }
    (void)ret;

    /* 同时镜像发送一份到监控服务端口（30502），不影响 AP 接收 */
    if (s_monitor_sock >= 0) {
        struct sockaddr_in dst;
        dst.sin_family      = AF_INET;
        dst.sin_port        = htons(MONITOR_REMOTE_EVT_PORT);
        dst.sin_addr.s_addr = inet_addr("127.0.0.1");
        sendto(s_monitor_sock, buf, 16u + sizeof(payload), 0,
               (struct sockaddr*)&dst, sizeof(dst));
    }
}

/* ================================================================
 * SomeIpProvider_DeInit
 * ================================================================ */
void SomeIpProvider_DeInit(void)
{
    SoAd_DeInit();
    if (s_monitor_sock >= 0) {
        close(s_monitor_sock);
        s_monitor_sock = -1;
    }
    printf("[MCU_LOG] {\"level\":\"INFO\",\"module\":\"SomeIpProvider\","
           "\"event\":\"DEINIT\",\"tx_total\":%u}\n",
           (unsigned)s_txCount);
    fflush(stdout);
}
