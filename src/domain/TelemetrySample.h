/**
 * @file TelemetrySample.h
 * @brief Defines a timestamped telemetry measurement.
 */

#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QUuid>

namespace equipment {

/**
 * @brief Represents one metric value received from a monitored device.
 */
struct TelemetrySample {
    QUuid deviceId; ///< Identifier of the device that produced the measurement.
    QString metricKey; ///< Stable machine-readable metric identifier.
    double value{0.0}; ///< Numeric metric value.
    QString unit; ///< Unit displayed with the value.
    QDateTime timestamp; ///< UTC time at which the measurement was produced.
};

} // namespace equipment

Q_DECLARE_METATYPE(equipment::TelemetrySample)
