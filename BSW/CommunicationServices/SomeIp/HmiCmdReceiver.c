/**
 * @file    HmiCmdReceiver.c
 * @brief   HMI 命令接收模块实现
 *
 * 监听 UDP:30503，解析 JSON 文本，写入 SwcEngine HmiInput。
 * JSON 极简解析（不依赖外部库），只提取数字字段。
 *
 * 指令示例（monitor_server → CP）：
 *   {"speed_kmh":60.0,"rpm":2000.0,"steering_deg":15.0,
 *    "brake":0,"door":3,"fuel_pct":75.5}
 *
 * 字段说明：
 *   speed_kmh    — 车速 km/h        (float, 0~300,  -1=不设定)
 *   rpm          — 发动机转速 RPM    (float, 0~8000, -1=不设定)
 *   steering_deg — 方向盘转角 deg    (float, -540~540, -999=不设定)
 *   brake        — 制动踏板 0/1      (int,   -1=不设定)
 *   door         — 车门 bitmask       (int,   -1=不设定)
 *   fuel_pct     — 燃油液位 %        (float, 0~100,  -1=不设定)
 */

#include "HmiCmdReceiver.h"
#include "SwcEngine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define HMI_CMD_PORT  30503u
#define BUF_SIZE      512u

/* ─── 内部状态 ────────────────────────────────────────── */
static pthread_t s_thread;
static volatile int s_running = 0;
static int s_sock = -1;

/* ─── 极简 JSON 数字提取 ──────────────────────────────── */
/** 从 JSON 字符串中提取 key 对应的 float 值，找不到返回default_val */
static float json_get_float(const char* json, const char* key, float default_val)
{
    /* 搜索 "key": 模式 */
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char* p = strstr(json, pattern);
    if (!p) return default_val;

    /* 跳过 key" 后的空白和冒号 */
    p += strlen(pattern);
    while (*p == ' ' || *p == '\t' || *p == ':' || *p == ' ') p++;
    if (!*p) return default_val;

    return (float)strtod(p, NULL);
}

static int json_get_int(const char* json, const char* key, int default_val)
{
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char* p = strstr(json, pattern);
    if (!p) return default_val;

    p += strlen(pattern);
    while (*p == ' ' || *p == '\t' || *p == ':' || *p == ' ') p++;
    if (!*p) return default_val;

    return (int)strtol(p, NULL, 10);
}

/* ─── 接收线程 ────────────────────────────────────────── */
static void* hmi_recv_thread(void* arg)
{
    (void)arg;
    char buf[BUF_SIZE];

    printf("[HmiCmd] Listening on UDP 127.0.0.1:%u\n", (unsigned)HMI_CMD_PORT);

    while (s_running) {
        struct sockaddr_in src;
        socklen_t src_len = sizeof(src);
        ssize_t n = recvfrom(s_sock, buf, BUF_SIZE - 1u, 0,
                             (struct sockaddr*)&src, &src_len);
        if (n <= 0) continue;
        buf[n] = '\0';

        /* 解析 JSON */
        SwcEngine_HmiInput_t input;
        input.hmi_valid        = 1u;
        input.hmi_speed_kmh    = json_get_float(buf, "speed_kmh",    -1.0f);
        input.hmi_rpm          = json_get_float(buf, "rpm",          -1.0f);
        input.hmi_steering_deg = json_get_float(buf, "steering_deg", -1.0f);

        int brake = json_get_int(buf, "brake", -1);
        input.hmi_brake = (brake < 0) ? 0xFFu : (uint8_t)brake;

        int door = json_get_int(buf, "door", -1);
        input.hmi_door  = (door  < 0) ? 0xFFu : (uint8_t)door;

        input.hmi_fuel_pct = json_get_float(buf, "fuel_pct", -1.0f);

        SwcEngine_SetHmiInput(&input);

        printf("[HmiCmd] RX: speed=%.1f rpm=%.0f steer=%.1f brake=%d door=%d fuel=%.1f%%\n",
               input.hmi_speed_kmh, input.hmi_rpm, input.hmi_steering_deg,
               (int)input.hmi_brake, (int)input.hmi_door, input.hmi_fuel_pct);
    }

    return NULL;
}

/* ─── 公开接口 ────────────────────────────────────────── */
void HmiCmdReceiver_Init(void)
{
    s_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_sock < 0) {
        perror("[HmiCmd] socket");
        return;
    }

    int reuse = 1;
    setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(HMI_CMD_PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(s_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[HmiCmd] bind");
        close(s_sock);
        s_sock = -1;
        return;
    }

    /* 设置接收超时，使线程可以检测 s_running */
    struct timeval tv = { .tv_sec = 0, .tv_usec = 200000 };  /* 200ms */
    setsockopt(s_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    s_running = 1;
    pthread_create(&s_thread, NULL, hmi_recv_thread, NULL);
    printf("[HmiCmd] HMI command receiver started on UDP:%u\n", (unsigned)HMI_CMD_PORT);
}

void HmiCmdReceiver_DeInit(void)
{
    s_running = 0;
    if (s_sock >= 0) {
        close(s_sock);
        s_sock = -1;
    }
    pthread_join(s_thread, NULL);
    printf("[HmiCmd] HMI command receiver stopped\n");
}
