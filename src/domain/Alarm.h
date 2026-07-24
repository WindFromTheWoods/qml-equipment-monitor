/**
 * @file Alarm.h
 * @brief Defines alarm lifecycle state and alarm event data.
 */

#pragma once

#include "domain/AlarmRule.h"

#include <QDateTime>
#include <QString>
#include <QUuid>

namespace equipment {

/**
 * @brief Describes the operator-visible lifecycle of an alarm.
 */
enum class AlarmState {
    Active, ///< The abnormal condition is active and not acknowledged.
    Acknowledged, ///< An operator has acknowledged the active condition.
    Resolved ///< The measurement has returned to its permitted range.
};

/**
 * @brief Captures a concrete alarm raised for a telemetry sample.
 */
struct Alarm {
    QUuid id; ///< Globally unique alarm identifier.
    QUuid deviceId; ///< Identifier of the device that raised the alarm.
    QString metricKey; ///< Metric responsible for the alarm.
    double measuredValue{0.0}; ///< Value observed when the alarm was evaluated.
    double threshold{0.0}; ///< Rule threshold that was violated.
    AlarmSeverity severity{AlarmSeverity::Warning}; ///< Operational severity.
    AlarmState state{AlarmState::Active}; ///< Current lifecycle state.
    QDateTime raisedAt; ///< UTC time at which the alarm was raised.
};

} // namespace equipment
