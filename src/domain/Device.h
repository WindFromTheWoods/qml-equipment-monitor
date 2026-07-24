/**
 * @file Device.h
 * @brief Defines equipment identity and connection state types.
 */

#pragma once

#include <QMetaType>
#include <QString>
#include <QUuid>

namespace equipment {

/**
 * @brief Describes the current connection state of a monitored device.
 */
enum class DeviceStatus {
    Offline,   ///< The device is not connected.
    Connecting, ///< A connection attempt is in progress.
    Online,    ///< The device is connected and can provide telemetry.
    Error      ///< The transport encountered an unrecoverable error.
};

/**
 * @brief Contains the stable identity and descriptive metadata of a device.
 */
struct Device {
    QUuid id; ///< Globally unique device identifier.
    QString name; ///< Human-readable device name.
    QString type; ///< Device type presented to the operator.
    DeviceStatus status{DeviceStatus::Offline}; ///< Current connection state.
};

} // namespace equipment

Q_DECLARE_METATYPE(equipment::DeviceStatus)
