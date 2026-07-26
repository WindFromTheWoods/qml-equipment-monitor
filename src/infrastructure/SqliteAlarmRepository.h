/**
 * @file SqliteAlarmRepository.h
 * @brief Declares the SQLite alarm repository adapter.
 */

#pragma once

#include "application/IAlarmRepository.h"

namespace equipment {

class SqliteDatabase;

/**
 * @brief Persists and restores alarm lifecycle state in SQLite.
 */
class SqliteAlarmRepository final : public IAlarmRepository {
public:
    /**
     * @brief Creates a repository using an open database wrapper.
     * @param database Non-owning SQLite database pointer.
     */
    explicit SqliteAlarmRepository(SqliteDatabase *database);

    /** @copydoc IAlarmRepository::save */
    bool save(
        const Alarm &alarm,
        QString *errorMessage = nullptr) override;

    /** @copydoc IAlarmRepository::activeAlarm */
    [[nodiscard]] std::optional<Alarm> activeAlarm(
        const QUuid &deviceId,
        const QString &metricKey,
        QString *errorMessage = nullptr) const override;

private:
    SqliteDatabase *m_database; ///< Non-owning database wrapper pointer.
};

} // namespace equipment
