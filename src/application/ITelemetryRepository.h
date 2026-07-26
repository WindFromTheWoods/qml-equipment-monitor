/**
 * @file ITelemetryRepository.h
 * @brief Declares the persistence port for telemetry history.
 */

#pragma once

#include "domain/TelemetrySample.h"

#include <QList>
#include <QString>

namespace equipment {

/**
 * @brief Abstracts persistent storage of normalized telemetry samples.
 *
 * Application services depend on this interface instead of a concrete SQL
 * implementation, which keeps storage technology out of the use-case layer.
 */
class ITelemetryRepository {
public:
    /** @brief Destroys the telemetry repository interface. */
    virtual ~ITelemetryRepository() = default;

    /**
     * @brief Persists one normalized telemetry sample.
     * @param sample Sample to store.
     * @param errorMessage Optional destination for a diagnostic on failure.
     * @return `true` when the sample was stored successfully.
     */
    virtual bool save(
        const TelemetrySample &sample,
        QString *errorMessage = nullptr) = 0;

    /**
     * @brief Loads the most recent samples in chronological order.
     * @param deviceId Device whose history must be loaded.
     * @param metricKey Metric whose history must be loaded.
     * @param limit Maximum number of samples to return.
     * @param errorMessage Optional destination for a diagnostic on failure.
     * @return Up to @p limit samples ordered from oldest to newest.
     */
    [[nodiscard]] virtual QList<TelemetrySample> recentSamples(
        const QUuid &deviceId,
        const QString &metricKey,
        qsizetype limit,
        QString *errorMessage = nullptr) const = 0;

    /**
     * @brief Counts all stored samples for one device metric.
     * @param deviceId Device whose samples must be counted.
     * @param metricKey Metric whose samples must be counted.
     * @param errorMessage Optional destination for a diagnostic on failure.
     * @return Number of matching samples, or zero when the query fails.
     */
    [[nodiscard]] virtual qint64 sampleCount(
        const QUuid &deviceId,
        const QString &metricKey,
        QString *errorMessage = nullptr) const = 0;
};

} // namespace equipment
