/**
 * @file IAlarmRepository.h
 * @brief Declares the persistence port for alarm lifecycle state.
 */

#pragma once

#include "domain/Alarm.h"

#include <QString>

#include <optional>

namespace equipment {

/**
 * @brief Abstracts persistent storage of alarms from the application layer.
 */
class IAlarmRepository {
public:
    /** @brief Destroys the alarm repository interface. */
    virtual ~IAlarmRepository() = default;

    /**
     * @brief Inserts or updates an alarm by its stable identifier.
     * @param alarm Complete alarm state to persist.
     * @param errorMessage Optional destination for a diagnostic on failure.
     * @return `true` when the alarm state was stored successfully.
     */
    virtual bool save(
        const Alarm &alarm,
        QString *errorMessage = nullptr) = 0;

    /**
     * @brief Loads the most recently raised unresolved alarm for a metric.
     * @param deviceId Device whose alarm must be loaded.
     * @param metricKey Metric whose alarm must be loaded.
     * @param errorMessage Optional destination for a diagnostic on failure.
     * @return The unresolved alarm, or `std::nullopt` when none exists.
     */
    [[nodiscard]] virtual std::optional<Alarm> activeAlarm(
        const QUuid &deviceId,
        const QString &metricKey,
        QString *errorMessage = nullptr) const = 0;
};

} // namespace equipment
