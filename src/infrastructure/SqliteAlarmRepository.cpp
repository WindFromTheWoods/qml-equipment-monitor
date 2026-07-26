/**
 * @file SqliteAlarmRepository.cpp
 * @brief Implements alarm persistence with Qt SQL and SQLite.
 */

#include "infrastructure/SqliteAlarmRepository.h"

#include "infrastructure/SqliteDatabase.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QtGlobal>

namespace equipment {

SqliteAlarmRepository::SqliteAlarmRepository(SqliteDatabase *database)
    : m_database(database)
{
    Q_ASSERT(m_database != nullptr);
}

bool SqliteAlarmRepository::save(const Alarm &alarm, QString *errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    QSqlQuery query(m_database->connection());
    query.prepare(QStringLiteral(
        "INSERT INTO alarms("
        "id, device_id, metric_key, measured_value, threshold, severity, state, raised_at_utc) "
        "VALUES(:id, :device_id, :metric_key, :measured_value, :threshold, "
        ":severity, :state, :raised_at_utc) "
        "ON CONFLICT(id) DO UPDATE SET "
        "device_id = excluded.device_id, "
        "metric_key = excluded.metric_key, "
        "measured_value = excluded.measured_value, "
        "threshold = excluded.threshold, "
        "severity = excluded.severity, "
        "state = excluded.state, "
        "raised_at_utc = excluded.raised_at_utc"));
    query.bindValue(QStringLiteral(":id"), alarm.id.toString(QUuid::WithoutBraces));
    query.bindValue(
        QStringLiteral(":device_id"),
        alarm.deviceId.toString(QUuid::WithoutBraces));
    query.bindValue(QStringLiteral(":metric_key"), alarm.metricKey);
    query.bindValue(QStringLiteral(":measured_value"), alarm.measuredValue);
    query.bindValue(QStringLiteral(":threshold"), alarm.threshold);
    query.bindValue(QStringLiteral(":severity"), static_cast<int>(alarm.severity));
    query.bindValue(QStringLiteral(":state"), static_cast<int>(alarm.state));
    query.bindValue(
        QStringLiteral(":raised_at_utc"),
        alarm.raisedAt.toUTC().toString(Qt::ISODateWithMs));

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot persist alarm: %1")
                                .arg(query.lastError().text());
        }
        return false;
    }

    return true;
}

std::optional<Alarm> SqliteAlarmRepository::activeAlarm(
    const QUuid &deviceId,
    const QString &metricKey,
    QString *errorMessage) const
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    QSqlQuery query(m_database->connection());
    query.prepare(QStringLiteral(
        "SELECT id, device_id, metric_key, measured_value, threshold, "
        "severity, state, raised_at_utc "
        "FROM alarms "
        "WHERE device_id = :device_id AND metric_key = :metric_key "
        "AND state != :resolved_state "
        "ORDER BY raised_at_utc DESC LIMIT 1"));
    query.bindValue(
        QStringLiteral(":device_id"),
        deviceId.toString(QUuid::WithoutBraces));
    query.bindValue(QStringLiteral(":metric_key"), metricKey);
    query.bindValue(
        QStringLiteral(":resolved_state"),
        static_cast<int>(AlarmState::Resolved));

    if (!query.exec()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot load active alarm: %1")
                                .arg(query.lastError().text());
        }
        return std::nullopt;
    }

    if (!query.next()) {
        return std::nullopt;
    }

    return Alarm{
        .id = QUuid(query.value(0).toString()),
        .deviceId = QUuid(query.value(1).toString()),
        .metricKey = query.value(2).toString(),
        .measuredValue = query.value(3).toDouble(),
        .threshold = query.value(4).toDouble(),
        .severity = static_cast<AlarmSeverity>(query.value(5).toInt()),
        .state = static_cast<AlarmState>(query.value(6).toInt()),
        .raisedAt = QDateTime::fromString(
                        query.value(7).toString(),
                        Qt::ISODateWithMs)
                        .toUTC(),
    };
}

} // namespace equipment
