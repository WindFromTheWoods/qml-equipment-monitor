/**
 * @file MonitoringService.cpp
 * @brief Implements monitoring use-case coordination and persistence.
 */

#include "application/MonitoringService.h"

#include <QtGlobal>

#include <utility>

namespace equipment {

MonitoringService::MonitoringService(
    IDeviceTransport *transport,
    ITelemetryRepository *telemetryRepository,
    IAlarmRepository *alarmRepository,
    QUuid deviceId,
    AlarmRule alarmRule,
    QObject *parent)
    : QObject(parent)
    , m_transport(transport)
    , m_telemetryRepository(telemetryRepository)
    , m_alarmRepository(alarmRepository)
    , m_deviceId(std::move(deviceId))
    , m_alarmRule(std::move(alarmRule))
{
    Q_ASSERT(m_transport != nullptr);
    Q_ASSERT(m_telemetryRepository != nullptr);
    Q_ASSERT(m_alarmRepository != nullptr);
    Q_ASSERT(!m_deviceId.isNull());

    connect(
        m_transport,
        &IDeviceTransport::telemetryReceived,
        this,
        &MonitoringService::onTelemetryReceived);
    connect(
        m_transport,
        &IDeviceTransport::statusChanged,
        this,
        &MonitoringService::onDeviceStatusChanged);
}

QUuid MonitoringService::deviceId() const
{
    return m_deviceId;
}

DeviceStatus MonitoringService::status() const noexcept
{
    return m_status;
}

AlarmRule MonitoringService::alarmRule() const
{
    return m_alarmRule;
}

const std::optional<TelemetrySample> &MonitoringService::lastSample() const noexcept
{
    return m_lastSample;
}

const std::optional<Alarm> &MonitoringService::currentAlarm() const noexcept
{
    return m_currentAlarm;
}

bool MonitoringService::restoreHistory(qsizetype maximumSamples)
{
    QString errorMessage;
    const QList<TelemetrySample> samples = m_telemetryRepository->recentSamples(
        m_deviceId,
        m_alarmRule.metricKey,
        maximumSamples,
        &errorMessage);
    if (!errorMessage.isEmpty()) {
        emit persistenceError(errorMessage);
        return false;
    }

    const qint64 totalSampleCount = m_telemetryRepository->sampleCount(
        m_deviceId,
        m_alarmRule.metricKey,
        &errorMessage);
    if (!errorMessage.isEmpty()) {
        emit persistenceError(errorMessage);
        return false;
    }

    if (!samples.isEmpty()) {
        m_lastSample = samples.constLast();
    }
    emit historyRestored(samples, totalSampleCount);

    m_currentAlarm = m_alarmRepository->activeAlarm(
        m_deviceId,
        m_alarmRule.metricKey,
        &errorMessage);
    if (!errorMessage.isEmpty()) {
        emit persistenceError(errorMessage);
        return false;
    }

    emit alarmChanged();
    return true;
}

void MonitoringService::start()
{
    m_transport->start();
}

void MonitoringService::stop()
{
    m_transport->stop();
}

void MonitoringService::setAlarmRule(const AlarmRule &alarmRule)
{
    if (m_alarmRule.metricKey == alarmRule.metricKey
        && m_alarmRule.operation == alarmRule.operation
        && qFuzzyCompare(m_alarmRule.threshold, alarmRule.threshold)
        && m_alarmRule.severity == alarmRule.severity) {
        return;
    }

    m_alarmRule = alarmRule;
    emit alarmRuleChanged();

    if (m_lastSample.has_value()) {
        evaluateAlarm(*m_lastSample);
    }
}

void MonitoringService::acknowledgeAlarm()
{
    if (!m_currentAlarm.has_value()
        || m_currentAlarm->state != AlarmState::Active) {
        return;
    }

    m_currentAlarm->state = AlarmState::Acknowledged;
    persistAlarm(*m_currentAlarm);
    emit alarmChanged();
}

void MonitoringService::onTelemetryReceived(const TelemetrySample &sample)
{
    if (sample.deviceId != m_deviceId) {
        return;
    }

    QString errorMessage;
    if (!m_telemetryRepository->save(sample, &errorMessage)) {
        emit persistenceError(errorMessage);
    }

    m_lastSample = sample;
    emit sampleAccepted(sample);
    evaluateAlarm(sample);
}

void MonitoringService::onDeviceStatusChanged(DeviceStatus status)
{
    if (m_status == status) {
        return;
    }

    m_status = status;
    emit statusChanged(status);
}

void MonitoringService::evaluateAlarm(const TelemetrySample &sample)
{
    if (sample.metricKey != m_alarmRule.metricKey) {
        return;
    }

    const std::optional<Alarm> evaluatedAlarm = m_alarmEngine.evaluate(sample, m_alarmRule);

    if (!evaluatedAlarm.has_value()) {
        if (m_currentAlarm.has_value()
            && m_currentAlarm->metricKey == m_alarmRule.metricKey) {
            m_currentAlarm->state = AlarmState::Resolved;
            persistAlarm(*m_currentAlarm);
            m_currentAlarm.reset();
            emit alarmChanged();
        }
        return;
    }

    if (!m_currentAlarm.has_value()) {
        m_currentAlarm = evaluatedAlarm;
        persistAlarm(*m_currentAlarm);
        emit alarmChanged();
        return;
    }

    m_currentAlarm->measuredValue = sample.value;
    m_currentAlarm->threshold = m_alarmRule.threshold;
    m_currentAlarm->severity = m_alarmRule.severity;
    persistAlarm(*m_currentAlarm);
    emit alarmChanged();
}

void MonitoringService::persistAlarm(const Alarm &alarm)
{
    QString errorMessage;
    if (!m_alarmRepository->save(alarm, &errorMessage)) {
        emit persistenceError(errorMessage);
    }
}

} // namespace equipment
