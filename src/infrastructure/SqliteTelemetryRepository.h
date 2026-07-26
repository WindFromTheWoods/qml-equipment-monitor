/**
 * @file SqliteTelemetryRepository.h
 * @brief Declares the SQLite telemetry repository adapter.
 */

#pragma once

#include "application/ITelemetryRepository.h"

namespace equipment {

class SqliteDatabase;

/**
 * @brief Persists normalized telemetry samples in SQLite.
 */
class SqliteTelemetryRepository final : public ITelemetryRepository {
public:
    /**
     * @brief Creates a repository using an open database wrapper.
     * @param database Non-owning SQLite database pointer.
     */
    explicit SqliteTelemetryRepository(SqliteDatabase *database);

    /** @copydoc ITelemetryRepository::save */
    bool save(
        const TelemetrySample &sample,
        QString *errorMessage = nullptr) override;

    /** @copydoc ITelemetryRepository::recentSamples */
    [[nodiscard]] QList<TelemetrySample> recentSamples(
        const QUuid &deviceId,
        const QString &metricKey,
        qsizetype limit,
        QString *errorMessage = nullptr) const override;

    /** @copydoc ITelemetryRepository::sampleCount */
    [[nodiscard]] qint64 sampleCount(
        const QUuid &deviceId,
        const QString &metricKey,
        QString *errorMessage = nullptr) const override;

private:
    SqliteDatabase *m_database; ///< Non-owning database wrapper pointer.
};

} // namespace equipment
