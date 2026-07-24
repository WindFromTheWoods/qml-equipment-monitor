/**
 * @file AlarmRule.h
 * @brief Defines alarm comparison rules and severity levels.
 */

#pragma once

#include <QString>

namespace equipment {

/**
 * @brief Lists the comparisons supported by the alarm engine.
 */
enum class ComparisonOperation {
    GreaterThan, ///< Trigger when the value is greater than the threshold.
    GreaterThanOrEqual, ///< Trigger when the value is greater than or equal to the threshold.
    LessThan, ///< Trigger when the value is less than the threshold.
    LessThanOrEqual ///< Trigger when the value is less than or equal to the threshold.
};

/**
 * @brief Expresses the operational importance of an alarm.
 */
enum class AlarmSeverity {
    Information, ///< Informational event that requires no immediate action.
    Warning, ///< Abnormal condition that should be investigated.
    Critical ///< Critical condition that requires immediate attention.
};

/**
 * @brief Configures when a metric must raise an alarm.
 */
struct AlarmRule {
    QString metricKey; ///< Metric to which the rule applies.
    ComparisonOperation operation{ComparisonOperation::GreaterThan}; ///< Threshold comparison.
    double threshold{0.0}; ///< Numeric threshold used by the comparison.
    AlarmSeverity severity{AlarmSeverity::Warning}; ///< Severity assigned to a matching alarm.
};

} // namespace equipment
