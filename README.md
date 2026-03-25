# MyAutoSarCP

基于 **AUTOSAR Classic Platform（CP）R25-11** 规范的 ECU 软件参考实现框架。

[![Build](https://img.shields.io/badge/build-passing-brightgreen)]()
[![AUTOSAR](https://img.shields.io/badge/AUTOSAR-CP%20R25--11-blue)]()
[![C Standard](https://img.shields.io/badge/C-C11-lightgrey)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

---

## 架构概述

```
┌──────────────────────────────────────────────────────────┐
│               Application Layer (SWC)                    │
│   SwcEngine (发动机控制)   SwcBattery (电池监控)          │
├──────────────────────────────────────────────────────────┤
│          Runtime Environment (RTE)  ←  R25-11            │
├──────────────────────────────────────────────────────────┤
│                   Basic Software (BSW)                   │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────┐  │
│  │System Svcs   │ │ Comm Svcs    │ │ Memory / Diag    │  │
│  │ OS  EcuM Det │ │ ComM CanIf   │ │ NvM  Dcm  Dem    │  │
│  │ WdgM BswM    │ │ Com  PduR    │ │ Csm              │  │
│  └──────────────┘ └──────────────┘ └──────────────────┘  │
├──────────────────────────────────────────────────────────┤
│              MCAL (Micro-Controller Abstraction)         │
│           Mcu   Dio   Can   Adc   Spi   Pwm              │
└──────────────────────────────────────────────────────────┘
             ↕  Hardware (ECU / MCU)
```

---

## 目录结构

```
MyAutoSarCP/
├── MCAL/               # 微控制器抽象层
│   ├── Mcu/            # MCU 驱动
│   ├── Dio/            # 数字 I/O 驱动
│   ├── Can/            # CAN 驱动
│   ├── Adc/            # ADC 驱动
│   ├── Spi/            # SPI 驱动
│   └── Pwm/            # PWM 驱动
├── BSW/                # 基础软件层
│   ├── SystemServices/ # OS / EcuM / Det / WdgM / BswM
│   ├── CommunicationServices/ # ComM / Com / PduR / CanIf / CanNm / CanTp / SoAd
│   ├── MemoryServices/ # NvM / MemIf / Ea
│   ├── DiagnosticServices/    # Dcm / Dem
│   └── CryptoServices/ # Csm
├── RTE/                # 运行时环境（stub，真实项目由工具链生成）
├── Application/        # 应用层 SWC
│   ├── SwcEngine/      # 发动机控制软件组件
│   ├── SwcBattery/     # 电池监控软件组件
│   └── main.c          # ECU 主入口（启动序列仿真）
├── tests/              # 单元测试（GoogleTest）
├── scripts/            # CI 脚本
├── config/             # ARXML / 配置文件（预留）
└── CMakeLists.txt
```

---

## 实现模块

| 层次 | 模块 | AUTOSAR SWS 文档 | 状态 |
|------|------|-----------------|------|
| MCAL | Mcu  | AUTOSAR_CP_SWS_MCUDriver (R25-11) | ✅ 头文件 |
| MCAL | Dio  | AUTOSAR_CP_SWS_DIODriver (R25-11) | ✅ 头文件 |
| MCAL | Can  | AUTOSAR_CP_SWS_CANDriver (R25-11) | ✅ 头文件 |
| BSW  | Os   | AUTOSAR_CP_SWS_OS (R25-11)        | ✅ 头文件 |
| BSW  | EcuM | AUTOSAR_CP_SWS_ECUStateManager (R25-11) | ✅ 完整 stub |
| BSW  | Det  | AUTOSAR_CP_SWS_DefaultErrorTracer (R25-11) | ✅ 头文件 |
| BSW  | WdgM | AUTOSAR_CP_SWS_WatchdogManager (R25-11) | ✅ 头文件 |
| BSW  | ComM | AUTOSAR_CP_SWS_CommunicationManager (R25-11) | ✅ 头文件 |
| BSW  | NvM  | AUTOSAR_CP_SWS_NVRAMManager (R25-11) | ✅ 头文件 |
| BSW  | Dcm  | AUTOSAR_CP_SWS_DiagnosticCommunicationManager (R25-11) | ✅ 头文件 |
| BSW  | Dem  | AUTOSAR_CP_SWS_DiagnosticEventManager (R25-11) | ✅ 完整 stub |
| RTE  | Rte  | AUTOSAR_CP_SWS_RTE (R25-11)       | ✅ 完整 stub |
| App  | SwcEngine  | —                           | ✅ 完整实现 |
| App  | SwcBattery | —                           | ✅ 完整实现 |

---

## 快速开始

### 环境要求

- CMake ≥ 3.16
- Apple Clang / GCC（支持 C11）
- GoogleTest（可选，用于单元测试）：`brew install googletest`
- lcov（可选，用于覆盖率报告）：`brew install lcov`

### 编译

```bash
# Release 构建
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 运行 ECU 仿真
./build/MyAutoSarCP
```

### 一键 CI（构建 + 测试 + 覆盖率）

```bash
bash scripts/ci.sh          # 完整 CI
bash scripts/ci.sh build    # 仅 Release 构建
bash scripts/ci.sh test     # Debug 构建 + 单元测试
bash scripts/ci.sh coverage # Debug + 测试 + lcov HTML 报告
```

### 运行输出示例

```
==============================================
  MyAutoSarCP  —  AUTOSAR CP R25-11
  ECU Startup Simulation (Host Build)
==============================================

[EcuM ] Init (pre-OS)
[EcuM ] Phase-0 init complete
[RTE  ] Starting RTE...
[SWC  ] SwcEngine_Init
[SWC  ] SwcBattery_Init
[EcuM ] Requesting RUN state (user=0)
[EcuM ] State -> APP_RUN
[OS   ] Entering cyclic task loop (Ctrl+C to stop)

[   0s] Engine: state=0  speed=0 rpm  temp=25.0°C  | Battery: 12.60 V  0.5 A  state=0
[   1s] Engine: state=1  speed=400 rpm  temp=25.0°C | Battery: 13.80 V -8.0 A  state=0
```

---

## 设计原则

1. **符合 AUTOSAR CP R25-11 规范**：所有 API 签名、类型定义、错误码均参照 AUTOSAR 官方 SWS 文档
2. **分层架构**：严格遵守 MCAL / BSW / RTE / Application 层次，层间仅通过标准接口通信
3. **可移植**：BSW 模块在 host build 中以 stub 形式运行，生产代码只需替换 MCAL/BSW 实现
4. **可测试**：RTE stub 提供纯内存通信，单元测试无需硬件
5. **零警告**：编译时开启 `-Wall -Wextra -Wpedantic`

---

## 规范参考

- AUTOSAR CP R25-11 主包: https://www.autosar.org/standards/classic-platform
- AUTOSAR_CP_SWS_BSWGeneral (R25-11)
- AUTOSAR_CP_SWS_OS (R25-11)
- AUTOSAR_CP_SWS_ECUStateManager (R25-11)
- AUTOSAR_CP_SWS_DefaultErrorTracer (R25-11)
- AUTOSAR_CP_SWS_WatchdogManager (R25-11)
- AUTOSAR_CP_SWS_CommunicationManager (R25-11)
- AUTOSAR_CP_SWS_NVRAMManager (R25-11)
- AUTOSAR_CP_SWS_DiagnosticCommunicationManager (R25-11)
- AUTOSAR_CP_SWS_DiagnosticEventManager (R25-11)
- AUTOSAR_CP_SWS_RTE (R25-11)
- AUTOSAR_CP_SWS_CANDriver (R25-11)
- AUTOSAR_CP_SWS_DIODriver (R25-11)
- AUTOSAR_CP_SWS_MCUDriver (R25-11)

---

## License

MIT
