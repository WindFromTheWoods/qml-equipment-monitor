/**
 * @file test_alarm_engine.cpp
 * @brief Contains unit tests for telemetry alarm rule evaluation.
 */

#include "application/AlarmEngine.h"

#include <QTest>

namespace equipment {

/**
 * @brief Verifies the protocol-independent behavior of AlarmEngine.
 */
class TestAlarmEngine final : public QObject {
    Q_OBJECT

private slots:
    /** @brief Verifies that rules ignore samples from a different metric. */
    void ignoresAnotherMetric();

    /** @brief Verifies that a value above a strict threshold raises an alarm. */
    void createsAlarmAboveThreshold();

    /** @brief Verifies strict greater-than behavior at the exact threshold. */
    void doesNotCreateAlarmAtStrictThreshold();

    /** @brief Verifies support for less-than-or-equal comparisons. */
    void supportsLowerBoundRules();
};

void TestAlarmEngine::ignoresAnotherMetric()
{
    const AlarmEngine engine;
    const TelemetrySample sample{
        .deviceId = QUuid::createUuid(),
        .metricKey = QStringLiteral("pressure"),
        .value = 95.0,
        .unit = QStringLiteral("bar"),
        .timestamp = QDateTime::currentDateTimeUtc(),
    };
    const AlarmRule rule{
        .metricKey = QStringLiteral("temperature"),
        .operation = ComparisonOperation::GreaterThan,
        .threshold = 80.0,
        .severity = AlarmSeverity::Critical,
    };

    QVERIFY(!engine.evaluate(sample, rule).has_value());
}

void TestAlarmEngine::createsAlarmAboveThreshold()
{
    const AlarmEngine engine;
    const QUuid deviceId = QUuid::createUuid();
    const QDateTime timestamp = QDateTime::currentDateTimeUtc();
    const TelemetrySample sample{
        .deviceId = deviceId,
        .metricKey = QStringLiteral("temperature"),
        .value = 80.1,
        .unit = QStringLiteral("°C"),
        .timestamp = timestamp,
    };
    const AlarmRule rule{
        .metricKey = QStringLiteral("temperature"),
        .operation = ComparisonOperation::GreaterThan,
        .threshold = 80.0,
        .severity = AlarmSeverity::Critical,
    };

    const auto alarm = engine.evaluate(sample, rule);

    QVERIFY(alarm.has_value());
    QCOMPARE(alarm->deviceId, deviceId);
    QCOMPARE(alarm->metricKey, QStringLiteral("temperature"));
    QCOMPARE(alarm->measuredValue, 80.1);
    QCOMPARE(alarm->threshold, 80.0);
    QCOMPARE(alarm->severity, AlarmSeverity::Critical);
    QCOMPARE(alarm->state, AlarmState::Active);
    QCOMPARE(alarm->raisedAt, timestamp);
    QVERIFY(!alarm->id.isNull());
}

void TestAlarmEngine::doesNotCreateAlarmAtStrictThreshold()
{
    const AlarmEngine engine;
    const TelemetrySample sample{
        .deviceId = QUuid::createUuid(),
        .metricKey = QStringLiteral("temperature"),
        .value = 80.0,
        .unit = QStringLiteral("°C"),
        .timestamp = QDateTime::currentDateTimeUtc(),
    };
    const AlarmRule rule{
        .metricKey = QStringLiteral("temperature"),
        .operation = ComparisonOperation::GreaterThan,
        .threshold = 80.0,
        .severity = AlarmSeverity::Critical,
    };

    QVERIFY(!engine.evaluate(sample, rule).has_value());
}

void TestAlarmEngine::supportsLowerBoundRules()
{
    const AlarmEngine engine;
    const TelemetrySample sample{
        .deviceId = QUuid::createUuid(),
        .metricKey = QStringLiteral("pressure"),
        .value = 1.9,
        .unit = QStringLiteral("bar"),
        .timestamp = QDateTime::currentDateTimeUtc(),
    };
    const AlarmRule rule{
        .metricKey = QStringLiteral("pressure"),
        .operation = ComparisonOperation::LessThanOrEqual,
        .threshold = 2.0,
        .severity = AlarmSeverity::Warning,
    };

    QVERIFY(engine.matches(sample, rule));
}

} // namespace equipment

QTEST_APPLESS_MAIN(equipment::TestAlarmEngine)

#include "test_alarm_engine.moc"
