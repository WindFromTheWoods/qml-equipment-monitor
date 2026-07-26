/**
 * @file SqliteDatabase.cpp
 * @brief Implements SQLite connection ownership and schema migrations.
 */

#include "infrastructure/SqliteDatabase.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

#include <utility>

namespace equipment {

SqliteDatabase::SqliteDatabase(QString databasePath, QString connectionName)
    : m_databasePath(std::move(databasePath))
    , m_connectionName(connectionName.isEmpty()
              ? QStringLiteral("equipment-monitor-%1")
                    .arg(QUuid::createUuid().toString(QUuid::WithoutBraces))
              : std::move(connectionName))
{
}

SqliteDatabase::~SqliteDatabase()
{
    if (m_database.isValid()) {
        m_database.close();
        m_database = QSqlDatabase{};
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool SqliteDatabase::open(QString *errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (isOpen()) {
        return true;
    }

    if (m_databasePath != QStringLiteral(":memory:")) {
        const QFileInfo databaseFile(m_databasePath);
        QDir parentDirectory = databaseFile.absoluteDir();
        if (!parentDirectory.exists() && !parentDirectory.mkpath(QStringLiteral("."))) {
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Cannot create database directory: %1")
                                    .arg(parentDirectory.absolutePath());
            }
            return false;
        }
    }

    if (!m_database.isValid()) {
        m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
        m_database.setDatabaseName(m_databasePath);
    }
    if (!m_database.open()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot open SQLite database: %1")
                                .arg(m_database.lastError().text());
        }
        return false;
    }

    QSqlQuery pragmaQuery(m_database);
    if (!pragmaQuery.exec(QStringLiteral("PRAGMA foreign_keys = ON"))) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot enable SQLite foreign keys: %1")
                                .arg(pragmaQuery.lastError().text());
        }
        m_database.close();
        return false;
    }
    if (m_databasePath != QStringLiteral(":memory:")
        && !pragmaQuery.exec(QStringLiteral("PRAGMA journal_mode = WAL"))) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot enable SQLite WAL mode: %1")
                                .arg(pragmaQuery.lastError().text());
        }
        m_database.close();
        return false;
    }
    pragmaQuery.finish();

    if (!migrate(errorMessage)) {
        m_database.close();
        return false;
    }

    return true;
}

bool SqliteDatabase::isOpen() const
{
    return m_database.isValid() && m_database.isOpen();
}

QSqlDatabase SqliteDatabase::connection() const
{
    return m_database;
}

QString SqliteDatabase::databasePath() const
{
    return m_databasePath;
}

bool SqliteDatabase::migrate(QString *errorMessage)
{
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS schema_migrations ("
            "version INTEGER PRIMARY KEY, "
            "applied_at_utc TEXT NOT NULL)"))) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot create migration table: %1")
                                .arg(query.lastError().text());
        }
        return false;
    }

    if (!query.exec(QStringLiteral(
            "SELECT COALESCE(MAX(version), 0) FROM schema_migrations"))
        || !query.next()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot read schema version: %1")
                                .arg(query.lastError().text());
        }
        return false;
    }

    const int currentVersion = query.value(0).toInt();
    query.finish();
    if (currentVersion >= 1) {
        return true;
    }

    if (!m_database.transaction()) {
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot start schema migration: %1")
                                .arg(m_database.lastError().text());
        }
        return false;
    }

    const QStringList migrationStatements{
        QStringLiteral(
            "CREATE TABLE telemetry_samples ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "device_id TEXT NOT NULL, "
            "metric_key TEXT NOT NULL, "
            "value REAL NOT NULL, "
            "unit TEXT NOT NULL, "
            "timestamp_utc TEXT NOT NULL)"),
        QStringLiteral(
            "CREATE INDEX idx_telemetry_device_metric_time "
            "ON telemetry_samples(device_id, metric_key, timestamp_utc DESC)"),
        QStringLiteral(
            "CREATE TABLE alarms ("
            "id TEXT PRIMARY KEY, "
            "device_id TEXT NOT NULL, "
            "metric_key TEXT NOT NULL, "
            "measured_value REAL NOT NULL, "
            "threshold REAL NOT NULL, "
            "severity INTEGER NOT NULL, "
            "state INTEGER NOT NULL, "
            "raised_at_utc TEXT NOT NULL)"),
        QStringLiteral(
            "CREATE INDEX idx_alarms_device_metric_state_time "
            "ON alarms(device_id, metric_key, state, raised_at_utc DESC)"),
    };

    for (const QString &statement : migrationStatements) {
        if (!query.exec(statement)) {
            m_database.rollback();
            if (errorMessage != nullptr) {
                *errorMessage = QStringLiteral("Cannot apply schema migration 1: %1")
                                    .arg(query.lastError().text());
            }
            return false;
        }
    }

    query.prepare(QStringLiteral(
        "INSERT INTO schema_migrations(version, applied_at_utc) "
        "VALUES(1, :applied_at_utc)"));
    query.bindValue(
        QStringLiteral(":applied_at_utc"),
        QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    if (!query.exec()) {
        m_database.rollback();
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot record schema migration 1: %1")
                                .arg(query.lastError().text());
        }
        return false;
    }

    if (!m_database.commit()) {
        m_database.rollback();
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Cannot commit schema migration 1: %1")
                                .arg(m_database.lastError().text());
        }
        return false;
    }

    return true;
}

} // namespace equipment
