/**
 * @file AlarmEngine.h
 * @brief Declares the domain service that evaluates telemetry alarm rules.
 */

#pragma once

#include "domain/Alarm.h"
#include "domain/AlarmRule.h"
#include "domain/TelemetrySample.h"

#include <optional>

namespace equipment {

/**
 * @brief Evaluates telemetry samples against protocol-independent alarm rules.
 *
 * The service has no mutable state, which makes it deterministic and easy to
 * test independently from transports, storage, and the user interface.
 */
class AlarmEngine {
public:
    /**
     * @brief Checks whether a sample satisfies an alarm rule.
     * @param sample Telemetry measurement to inspect.
     * @param rule Rule that defines the metric, comparison, and threshold.
     * @return `true` when the rule applies and its comparison succeeds.
     */
    [[nodiscard]] bool matches(
        const TelemetrySample &sample,
        const AlarmRule &rule) const;

    /**
     * @brief Creates an active alarm when a sample satisfies a rule.
     * @param sample Telemetry measurement to inspect.
     * @param rule Rule that defines the alarm condition and severity.
     * @return A populated alarm, or `std::nullopt` when the rule does not match.
     */
    [[nodiscard]] std::optional<Alarm> evaluate(
        const TelemetrySample &sample,
        const AlarmRule &rule) const;
};

} // namespace equipment
