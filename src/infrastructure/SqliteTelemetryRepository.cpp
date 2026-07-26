/**
 * @file SqliteTelemetryRepository.cpp
 * @brief Implements telemetry persistence with Qt SQL and SQLite.
 */

#include "infrastructure/SqliteTelemetryRepository.h"

#include "infrastructure/SqliteDatabase.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QtGlobal>

namespace equipment {

SqliteTelemetryRepository::SqliteTelemetryRepository(SqliteDatabase *database)
    : m_database(database)
{
    Q_ASSERT(m_database != nullptr);
}

bool SqliteTelemetryRepository::save(
    const TelemetrySample &sample,
    QString *errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    QSqlQuery query(m_database->connection());
    query.prepare(QStringLiteral(
        "INSERT INTO telemetry_samples("
        "device_id, metric_key, value, unit, timestamp_utc) "
        "VALUES(:device_id, :metric_key, :value, :unit, :timestamp_utc)"));
    query.bindValue(
        QStringLiteral(":device_id"),
        sample.deviceId.toString(QUuid::WithoutBraces));
    query.bindValue(QStringLiteral(":metric_key"), sample.metricKey);
    query.bindValue(QStringLiteral(":value"), sample.value);
    query.bindValue(QStringLiteral(":unit"), sample.unit);
    query.bindValue(
        QStringLiteral(":timestamp_utc"),
        sample.timestamp.toUTC().toString(Qt::ISODateWithMs));

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot persist telemetry sample: %1")
                                .arg(query.lastError().text());
        }
        return false;
    }

    return true;
}

QList<TelemetrySample> SqliteTelemetryRepository::recentSamples(
    const QUuid &deviceId,
    const QString &metricKey,
    qsizetype limit,
    QString *errorMessage) const
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    QList<TelemetrySample> samples;
    if (limit <= 0) {
        return samples;
    }

    QSqlQuery query(m_database->connection());
    query.prepare(QStringLiteral(
        "SELECT device_id, metric_key, value, unit, timestamp_utc "
        "FROM telemetry_samples "
        "WHERE device_id = :device_id AND metric_key = :metric_key "
        "ORDER BY timestamp_utc DESC, id DESC LIMIT :limit"));
    query.bindValue(
        QStringLiteral(":device_id"),
        deviceId.toString(QUuid::WithoutBraces));
    query.bindValue(QStringLiteral(":metric_key"), metricKey);
    query.bindValue(QStringLiteral(":limit"), static_cast<qlonglong>(limit));

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot load telemetry history: %1")
                                .arg(query.lastError().text());
        }
        return {};
    }

    while (query.next()) {
        samples.prepend(TelemetrySample{
            .deviceId = QUuid(query.value(0).toString()),
            .metricKey = query.value(1).toString(),
            .value = query.value(2).toDouble(),
            .unit = query.value(3).toString(),
            .timestamp = QDateTime::fromString(
                             query.value(4).toString(),
                             Qt::ISODateWithMs)
                             .toUTC(),
        });
    }

    return samples;
}

qint64 SqliteTelemetryRepository::sampleCount(
    const QUuid &deviceId,
    const QString &metricKey,
    QString *errorMessage) const
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    QSqlQuery query(m_database->connection());
    query.prepare(QStringLiteral(
        "SELECT COUNT(*) FROM telemetry_samples "
        "WHERE device_id = :device_id AND metric_key = :metric_key"));
    query.bindValue(
        QStringLiteral(":device_id"),
        deviceId.toString(QUuid::WithoutBraces));
    query.bindValue(QStringLiteral(":metric_key"), metricKey);

    if (!query.exec() || !query.next()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot count telemetry samples: %1")
                                .arg(query.lastError().text());
        }
        return 0;
    }

    return query.value(0).toLongLong();
}

} // namespace equipment
