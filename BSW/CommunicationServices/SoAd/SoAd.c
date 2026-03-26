/**
 * @file    SoAd.c
 * @brief   AUTOSAR CP — Socket Adaptor 实现（Host-build POSIX 仿真）
 *
 * 规范：AUTOSAR_SWS_SocketAdaptor
 * 实现策略：
 *   - 每个 SoConGroup 对应一个 POSIX UDP socket fd
 *   - 非阻塞模式（O_NONBLOCK），供 MainFunction 轮询
 *   - 最多支持 SOAD_MAX_SOCON 个并发连接组
 */

#include "SoAd.h"
#include "Det.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/* ----------------------------------------------------------------
 * 内部常量
 * ---------------------------------------------------------------- */
#define SOAD_MAX_SOCON      4u
#define SOAD_MODULE_ID      56u   /* AUTOSAR Module ID: SoAd */
#define SOAD_INSTANCE_ID    0u

/* ----------------------------------------------------------------
 * 内部数据结构
 * ---------------------------------------------------------------- */
typedef struct {
    int                   fd;          /* socket 文件描述符，-1=未使用 */
    SoAd_SoConGroupIdType groupId;
    struct sockaddr_in    remoteAddr;  /* 目标地址（发送用）           */
    uint8_t               inUse;
} SoAd_InternalSocket_t;

static SoAd_InternalSocket_t s_sockets[SOAD_MAX_SOCON];
static SoAd_StateType         s_state = SOAD_STATE_UNINIT;

/* ----------------------------------------------------------------
 * 内部辅助：根据 groupId 查找插槽
 * ---------------------------------------------------------------- */
static SoAd_InternalSocket_t* find_slot(SoAd_SoConGroupIdType groupId)
{
    for (uint8_t i = 0; i < SOAD_MAX_SOCON; ++i) {
        if (s_sockets[i].inUse && s_sockets[i].groupId == groupId) {
            return &s_sockets[i];
        }
    }
    return NULL;
}

static SoAd_InternalSocket_t* alloc_slot(void)
{
    for (uint8_t i = 0; i < SOAD_MAX_SOCON; ++i) {
        if (!s_sockets[i].inUse) return &s_sockets[i];
    }
    return NULL;
}

/* ================================================================
 * SoAd_Init
 * ================================================================ */
void SoAd_Init(void)
{
    for (uint8_t i = 0; i < SOAD_MAX_SOCON; ++i) {
        s_sockets[i].fd     = -1;
        s_sockets[i].inUse  = 0u;
        s_sockets[i].groupId = 0u;
    }
    s_state = SOAD_STATE_INIT;
    printf("[SoAd ] Initialized\n");
}

/* ================================================================
 * SoAd_OpenSoConGroup
 * ================================================================ */
Std_ReturnType SoAd_OpenSoConGroup(SoAd_SoConGroupIdType groupId,
                                   const SoAd_SoConConfigType* cfg)
{
    if (s_state != SOAD_STATE_INIT) {
        Det_ReportError(SOAD_MODULE_ID, SOAD_INSTANCE_ID, 0x01u, 0x01u);
        return E_NOT_OK;
    }
    if (cfg == NULL) return E_NOT_OK;
    if (find_slot(groupId) != NULL) return E_OK; /* 已打开，幂等 */

    SoAd_InternalSocket_t* slot = alloc_slot();
    if (slot == NULL) {
        printf("[SoAd ] ERROR: no free socket slot\n");
        return E_NOT_OK;
    }

    /* 创建 UDP socket */
    int type = cfg->isTcp ? SOCK_STREAM : SOCK_DGRAM;
    int fd   = socket(AF_INET, type, 0);
    if (fd < 0) {
        printf("[SoAd ] socket() failed: %s\n", strerror(errno));
        return E_NOT_OK;
    }

    /* 允许端口复用（多进程测试） */
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    /* 绑定本地地址 */
    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family      = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr(cfg->localAddr);
    localAddr.sin_port        = htons(cfg->localPort);

    if (bind(fd, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        printf("[SoAd ] bind(%s:%u) failed: %s\n",
               cfg->localAddr, cfg->localPort, strerror(errno));
        close(fd);
        return E_NOT_OK;
    }

    /* 非阻塞 */
    fcntl(fd, F_SETFL, O_NONBLOCK);

    /* 记录远端地址 */
    memset(&slot->remoteAddr, 0, sizeof(slot->remoteAddr));
    slot->remoteAddr.sin_family      = AF_INET;
    slot->remoteAddr.sin_addr.s_addr = inet_addr(cfg->remoteAddr);
    slot->remoteAddr.sin_port        = htons(cfg->remotePort);

    slot->fd      = fd;
    slot->groupId = groupId;
    slot->inUse   = 1u;

    printf("[SoAd ] Opened SoConGroup=0x%02X  local=%s:%u  remote=%s:%u  %s\n",
           groupId, cfg->localAddr, cfg->localPort,
           cfg->remoteAddr, cfg->remotePort,
           cfg->isTcp ? "TCP" : "UDP");
    return E_OK;
}

/* ================================================================
 * SoAd_IfTransmit
 * ================================================================ */
Std_ReturnType SoAd_IfTransmit(SoAd_SoConGroupIdType groupId,
                                const uint8* data, uint16 len)
{
    if (s_state != SOAD_STATE_INIT) return E_NOT_OK;

    SoAd_InternalSocket_t* slot = find_slot(groupId);
    if (slot == NULL || slot->fd < 0) return E_NOT_OK;

    ssize_t sent = sendto(slot->fd,
                          (const void*)data, (size_t)len, 0,
                          (struct sockaddr*)&slot->remoteAddr,
                          sizeof(slot->remoteAddr));
    if (sent < 0) {
        /* EAGAIN 在非阻塞下是正常的（缓冲区满），其他情况报错 */
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            printf("[SoAd ] sendto error: %s\n", strerror(errno));
            return E_NOT_OK;
        }
        return E_NOT_OK;
    }
    return E_OK;
}

/* ================================================================
 * SoAd_IfReceive
 * ================================================================ */
uint16 SoAd_IfReceive(SoAd_SoConGroupIdType groupId,
                      uint8* buf, uint16 maxLen)
{
    if (s_state != SOAD_STATE_INIT) return 0u;

    SoAd_InternalSocket_t* slot = find_slot(groupId);
    if (slot == NULL || slot->fd < 0) return 0u;

    ssize_t n = recv(slot->fd, (void*)buf, (size_t)maxLen, 0);
    if (n <= 0) return 0u;
    return (uint16)n;
}

/* ================================================================
 * SoAd_CloseSoConGroup
 * ================================================================ */
void SoAd_CloseSoConGroup(SoAd_SoConGroupIdType groupId)
{
    SoAd_InternalSocket_t* slot = find_slot(groupId);
    if (slot == NULL) return;

    if (slot->fd >= 0) {
        close(slot->fd);
        slot->fd = -1;
    }
    slot->inUse = 0u;
    printf("[SoAd ] Closed SoConGroup=0x%02X\n", groupId);
}

/* ================================================================
 * SoAd_DeInit
 * ================================================================ */
void SoAd_DeInit(void)
{
    for (uint8_t i = 0; i < SOAD_MAX_SOCON; ++i) {
        if (s_sockets[i].inUse) {
            SoAd_CloseSoConGroup(s_sockets[i].groupId);
        }
    }
    s_state = SOAD_STATE_UNINIT;
    printf("[SoAd ] DeInitialized\n");
}
