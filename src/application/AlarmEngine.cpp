/**
 * @file AlarmEngine.cpp
 * @brief Implements telemetry alarm rule evaluation.
 */

#include "application/AlarmEngine.h"

namespace equipment {

bool AlarmEngine::matches(
    const TelemetrySample &sample,
    const AlarmRule &rule) const
{
    if (sample.metricKey != rule.metricKey) {
        return false;
    }

    switch (rule.operation) {
    case ComparisonOperation::GreaterThan:
        return sample.value > rule.threshold;
    case ComparisonOperation::GreaterThanOrEqual:
        return sample.value >= rule.threshold;
    case ComparisonOperation::LessThan:
        return sample.value < rule.threshold;
    case ComparisonOperation::LessThanOrEqual:
        return sample.value <= rule.threshold;
    }

    return false;
}

std::optional<Alarm> AlarmEngine::evaluate(
    const TelemetrySample &sample,
    const AlarmRule &rule) const
{
    if (!matches(sample, rule)) {
        return std::nullopt;
    }

    return Alarm{
        .id = QUuid::createUuid(),
        .deviceId = sample.deviceId,
        .metricKey = sample.metricKey,
        .measuredValue = sample.value,
        .threshold = rule.threshold,
        .severity = rule.severity,
        .state = AlarmState::Active,
        .raisedAt = sample.timestamp,
    };
}

} // namespace equipment
