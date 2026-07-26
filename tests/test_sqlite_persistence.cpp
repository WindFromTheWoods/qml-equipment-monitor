/**
 * @file test_sqlite_persistence.cpp
 * @brief Contains SQLite repository and monitoring restoration tests.
 */

#include "application/MonitoringService.h"
#include "infrastructure/SqliteAlarmRepository.h"
#include "infrastructure/SqliteDatabase.h"
#include "infrastructure/SqliteTelemetryRepository.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

namespace equipment {

/**
 * @brief Provides controllable telemetry events for integration tests.
 */
class FakeTransport final : public IDeviceTransport {
public:
    /**
     * @brief Creates a stopped fake transport.
     * @param parent Optional Qt object owner.
     */
    explicit FakeTransport(QObject *parent = nullptr)
        : IDeviceTransport(parent)
    {
    }

    /** @brief Marks the fake transport online. */
    void start() override
    {
        emit statusChanged(DeviceStatus::Online);
    }

    /** @brief Marks the fake transport offline. */
    void stop() override
    {
        emit statusChanged(DeviceStatus::Offline);
    }

    /**
     * @brief Publishes a sample synchronously to connected services.
     * @param sample Normalized sample to publish.
     */
    void publish(const TelemetrySample &sample)
    {
        emit telemetryReceived(sample);
    }
};

/**
 * @brief Creates a temperature sample with stable test metadata.
 * @param deviceId Identifier of the sample producer.
 * @param value Temperature value in degrees Celsius.
 * @param timestamp UTC sample timestamp.
 * @return A normalized temperature sample.
 */
[[nodiscard]] TelemetrySample temperatureSample(
    const QUuid &deviceId,
    double value,
    const QDateTime &timestamp)
{
    return TelemetrySample{
        .deviceId = deviceId,
        .metricKey = QStringLiteral("temperature"),
        .value = value,
        .unit = QStringLiteral("\u00B0C"),
        .timestamp = timestamp,
    };
}

/**
 * @brief Creates the temperature alarm rule used by persistence tests.
 * @return A strict high-temperature rule with an 80 degree threshold.
 */
[[nodiscard]] AlarmRule temperatureRule()
{
    return AlarmRule{
        .metricKey = QStringLiteral("temperature"),
        .operation = ComparisonOperation::GreaterThan,
        .threshold = 80.0,
        .severity = AlarmSeverity::Critical,
    };
}

/**
 * @brief Verifies SQLite repositories and restart restoration behavior.
 */
class TestSqlitePersistence final : public QObject {
    Q_OBJECT

private slots:
    /** @brief Verifies limited history order and total sample counting. */
    void loadsRecentTelemetryInChronologicalOrder();

    /** @brief Verifies that history and alarm state survive a DB restart. */
    void restoresMonitoringStateAfterRestart();
};

void TestSqlitePersistence::loadsRecentTelemetryInChronologicalOrder()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    SqliteDatabase database(temporaryDirectory.filePath(QStringLiteral("history.sqlite")));
    QString errorMessage;
    QVERIFY2(database.open(&errorMessage), qPrintable(errorMessage));
    SqliteTelemetryRepository repository(&database);

    const QUuid deviceId = QUuid::createUuid();
    const QDateTime baseTime = QDateTime::fromString(
        QStringLiteral("2026-01-01T12:00:00.000Z"),
        Qt::ISODateWithMs);
    QVERIFY(repository.save(temperatureSample(deviceId, 71.0, baseTime), &errorMessage));
    QVERIFY(repository.save(temperatureSample(deviceId, 72.0, baseTime.addSecs(1)), &errorMessage));
    QVERIFY(repository.save(temperatureSample(deviceId, 73.0, baseTime.addSecs(2)), &errorMessage));

    const QList<TelemetrySample> samples = repository.recentSamples(
        deviceId,
        QStringLiteral("temperature"),
        2,
        &errorMessage);

    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
    QCOMPARE(samples.size(), 2);
    QCOMPARE(samples.at(0).value, 72.0);
    QCOMPARE(samples.at(1).value, 73.0);
    QCOMPARE(samples.at(0).timestamp, baseTime.addSecs(1));
    QCOMPARE(samples.at(1).timestamp, baseTime.addSecs(2));
    QCOMPARE(
        repository.sampleCount(
            deviceId,
            QStringLiteral("temperature"),
            &errorMessage),
        3);
    QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
}

void TestSqlitePersistence::restoresMonitoringStateAfterRestart()
{
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());

    const QString databasePath = temporaryDirectory.filePath(
        QStringLiteral("restart.sqlite"));
    const QUuid deviceId = QUuid::createUuid();
    const QDateTime baseTime = QDateTime::fromString(
        QStringLiteral("2026-01-01T12:00:00.000Z"),
        Qt::ISODateWithMs);

    {
        SqliteDatabase database(databasePath);
        QString errorMessage;
        QVERIFY2(database.open(&errorMessage), qPrintable(errorMessage));
        SqliteTelemetryRepository telemetryRepository(&database);
        SqliteAlarmRepository alarmRepository(&database);
        FakeTransport transport;
        MonitoringService service(
            &transport,
            &telemetryRepository,
            &alarmRepository,
            deviceId,
            temperatureRule());
        QSignalSpy persistenceErrors(&service, &MonitoringService::persistenceError);

        transport.publish(temperatureSample(deviceId, 72.0, baseTime));
        transport.publish(temperatureSample(deviceId, 85.0, baseTime.addSecs(1)));
        QVERIFY(service.currentAlarm().has_value());
        QCOMPARE(service.currentAlarm()->state, AlarmState::Active);

        service.acknowledgeAlarm();
        QCOMPARE(service.currentAlarm()->state, AlarmState::Acknowledged);
        QCOMPARE(persistenceErrors.count(), 0);
    }

    {
        SqliteDatabase database(databasePath);
        QString errorMessage;
        QVERIFY2(database.open(&errorMessage), qPrintable(errorMessage));
        SqliteTelemetryRepository telemetryRepository(&database);
        SqliteAlarmRepository alarmRepository(&database);
        FakeTransport transport;
        MonitoringService service(
            &transport,
            &telemetryRepository,
            &alarmRepository,
            deviceId,
            temperatureRule());
        QList<TelemetrySample> restoredSamples;
        qint64 restoredSampleCount = -1;
        connect(
            &service,
            &MonitoringService::historyRestored,
            &service,
            [&restoredSamples, &restoredSampleCount](
                const QList<TelemetrySample> &samples,
                qint64 totalSampleCount) {
                restoredSamples = samples;
                restoredSampleCount = totalSampleCount;
            });
        QSignalSpy persistenceErrors(&service, &MonitoringService::persistenceError);

        QVERIFY(service.restoreHistory());
        QCOMPARE(restoredSamples.size(), 2);
        QCOMPARE(restoredSamples.constFirst().value, 72.0);
        QCOMPARE(restoredSamples.constLast().value, 85.0);
        QCOMPARE(restoredSampleCount, 2);
        QVERIFY(service.currentAlarm().has_value());
        QCOMPARE(service.currentAlarm()->state, AlarmState::Acknowledged);

        transport.publish(temperatureSample(deviceId, 70.0, baseTime.addSecs(2)));
        QVERIFY(!service.currentAlarm().has_value());
        QVERIFY(!alarmRepository.activeAlarm(
            deviceId,
            QStringLiteral("temperature"),
            &errorMessage).has_value());
        QVERIFY2(errorMessage.isEmpty(), qPrintable(errorMessage));
        QCOMPARE(persistenceErrors.count(), 0);
    }
}

} // namespace equipment

QTEST_GUILESS_MAIN(equipment::TestSqlitePersistence)

#include "test_sqlite_persistence.moc"
