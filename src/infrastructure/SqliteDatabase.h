/**
 * @file SqliteDatabase.h
 * @brief Declares SQLite connection ownership and schema migration support.
 */

#pragma once

#include <QSqlDatabase>
#include <QString>

namespace equipment {

/**
 * @brief Owns one named SQLite connection and applies schema migrations.
 *
 * Repository adapters share this object but create their own lightweight
 * QSqlDatabase handles for individual queries.
 */
class SqliteDatabase final {
public:
    /**
     * @brief Creates a closed SQLite database wrapper.
     * @param databasePath SQLite file path or `:memory:` for an in-memory DB.
     * @param connectionName Optional Qt SQL connection name.
     */
    explicit SqliteDatabase(
        QString databasePath,
        QString connectionName = {});

    /** @brief Closes and unregisters the owned Qt SQL connection. */
    ~SqliteDatabase();

    /** @brief SQLite connections cannot be copied. */
    SqliteDatabase(const SqliteDatabase &) = delete;

    /** @brief SQLite connections cannot be copy-assigned. */
    SqliteDatabase &operator=(const SqliteDatabase &) = delete;

    /**
     * @brief Opens the database and applies all pending migrations.
     * @param errorMessage Optional destination for a diagnostic on failure.
     * @return `true` when the database is ready for repository queries.
     */
    bool open(QString *errorMessage = nullptr);

    /** @return `true` when the underlying SQLite connection is open. */
    [[nodiscard]] bool isOpen() const;

    /**
     * @brief Returns a handle to the owned Qt SQL connection.
     * @return Lightweight handle suitable for constructing QSqlQuery objects.
     */
    [[nodiscard]] QSqlDatabase connection() const;

    /** @return The configured SQLite file path. */
    [[nodiscard]] QString databasePath() const;

private:
    /**
     * @brief Applies versioned schema migrations in transactions.
     * @param errorMessage Optional destination for a diagnostic on failure.
     * @return `true` when the schema is current.
     */
    bool migrate(QString *errorMessage);

    QString m_databasePath; ///< SQLite file path or in-memory marker.
    QString m_connectionName; ///< Unique Qt SQL connection name.
    QSqlDatabase m_database; ///< Owned connection handle.
};

} // namespace equipment
