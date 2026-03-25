/**
 * @file    Dio.h
 * @brief   AUTOSAR CP R25-11 — Digital I/O Driver
 * @details Implements AUTOSAR_CP_SWS_DIODriver (R25-11)
 *          Functional Cluster: MCAL / I/O Hardware Abstraction
 *
 * AUTOSAR Document: AUTOSAR_CP_SWS_DIODriver
 * Version        : R25-11
 */
#ifndef DIO_H
#define DIO_H

#include "Std_Types.h"

/* ================================================================
 * Type Definitions  [SWS_Dio_00182]
 * ================================================================ */
typedef uint8  Dio_ChannelType;      /**< DIO channel ID */
typedef uint16 Dio_PortType;         /**< DIO port ID */
typedef uint8  Dio_LevelType;        /**< STD_HIGH / STD_LOW */
typedef uint8  Dio_PortLevelType;    /**< Port level bitmask */

/** Channel group — SWS_Dio_00061 */
typedef struct {
    Dio_PortLevelType mask;       /**< bitmask for the port */
    uint8             offset;     /**< bit offset within port */
    Dio_PortType      port;       /**< port ID */
} Dio_ChannelGroupType;

/* ================================================================
 * Symbolic Names
 * ================================================================ */
#define DIO_CHANNEL_LED_STATUS      ((Dio_ChannelType)0x00u)
#define DIO_CHANNEL_IGNITION_IN     ((Dio_ChannelType)0x01u)
#define DIO_CHANNEL_RELAY_OUT       ((Dio_ChannelType)0x02u)
#define DIO_CHANNEL_CAN_STBY        ((Dio_ChannelType)0x03u)

#define DIO_PORT_A                  ((Dio_PortType)0x00u)
#define DIO_PORT_B                  ((Dio_PortType)0x01u)

/* ================================================================
 * API — SWS_Dio_00133 .. SWS_Dio_00177
 * ================================================================ */

/**
 * @brief   Read a DIO channel.
 * @param   ChannelId   Channel to read [SWS_Dio_00133]
 * @return  STD_HIGH / STD_LOW
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType ChannelId);

/**
 * @brief   Write a DIO channel.
 * @param   ChannelId   Channel to write [SWS_Dio_00134]
 * @param   Level       STD_HIGH or STD_LOW
 */
void Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);

/**
 * @brief   Flip the level of a DIO channel and return new level.
 *          [SWS_Dio_00191]
 */
Dio_LevelType Dio_FlipChannel(Dio_ChannelType ChannelId);

/**
 * @brief   Read all channels of a DIO port [SWS_Dio_00135]
 */
Dio_PortLevelType Dio_ReadPort(Dio_PortType PortId);

/**
 * @brief   Write all channels of a DIO port [SWS_Dio_00136]
 */
void Dio_WritePort(Dio_PortType PortId, Dio_PortLevelType Level);

/**
 * @brief   Read channel group [SWS_Dio_00137]
 */
Dio_PortLevelType Dio_ReadChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr);

/**
 * @brief   Write channel group [SWS_Dio_00138]
 */
void Dio_WriteChannelGroup(const Dio_ChannelGroupType* ChannelGroupIdPtr, Dio_PortLevelType Level);

/**
 * @brief   Return DIO driver version info [SWS_Dio_00139]
 */
void Dio_GetVersionInfo(Std_VersionInfoType* VersionInfo);

#endif /* DIO_H */
