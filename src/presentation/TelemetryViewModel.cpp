/**
 * @file TelemetryViewModel.cpp
 * @brief Implements the QML-facing live telemetry view model.
 */

#include "presentation/TelemetryViewModel.h"

#include <QtGlobal>

#include <optional>
#include <utility>

namespace equipment {

TelemetryViewModel::TelemetryViewModel(
    MonitoringService *monitoringService,
    QString deviceName,
    QString deviceType,
    QObject *parent)
    : QObject(parent)
    , m_monitoringService(monitoringService)
    , m_deviceName(std::move(deviceName))
    , m_deviceType(std::move(deviceType))
{
    Q_ASSERT(m_monitoringService != nullptr);

    connect(
        m_monitoringService,
        &MonitoringService::sampleAccepted,
        this,
        &TelemetryViewModel::onSampleAccepted);
    connect(
        m_monitoringService,
        &MonitoringService::historyRestored,
        this,
        &TelemetryViewModel::onHistoryRestored);
    connect(
        m_monitoringService,
        &MonitoringService::statusChanged,
        this,
        &TelemetryViewModel::onDeviceStatusChanged);
    connect(
        m_monitoringService,
        &MonitoringService::alarmRuleChanged,
        this,
        &TelemetryViewModel::thresholdChanged);
    connect(
        m_monitoringService,
        &MonitoringService::alarmChanged,
        this,
        &TelemetryViewModel::alarmChanged);
}

QString TelemetryViewModel::deviceName() const
{
    return m_deviceName;
}

QString TelemetryViewModel::deviceType() const
{
    return m_deviceType;
}

double TelemetryViewModel::currentTemperature() const noexcept
{
    return m_currentTemperature;
}

QVariantList TelemetryViewModel::temperatureSeries() const
{
    return m_temperatureSeries;
}

qint64 TelemetryViewModel::sampleCount() const noexcept
{
    return m_sampleCount;
}

QString TelemetryViewModel::lastUpdate() const
{
    return m_lastUpdate;
}

bool TelemetryViewModel::online() const noexcept
{
    return m_monitoringService->status() == DeviceStatus::Online;
}

QString TelemetryViewModel::statusText() const
{
    switch (m_monitoringService->status()) {
    case DeviceStatus::Offline:
        return QStringLiteral("Disconnected");
    case DeviceStatus::Connecting:
        return QStringLiteral("Connecting");
    case DeviceStatus::Online:
        return QStringLiteral("Online");
    case DeviceStatus::Error:
        return QStringLiteral("Error");
    }

    return QStringLiteral("Unknown");
}

double TelemetryViewModel::alarmThreshold() const noexcept
{
    return m_monitoringService->alarmRule().threshold;
}

bool TelemetryViewModel::alarmActive() const noexcept
{
    const std::optional<Alarm> &alarm = m_monitoringService->currentAlarm();
    return alarm.has_value() && alarm->state != AlarmState::Resolved;
}

bool TelemetryViewModel::alarmAcknowledged() const noexcept
{
    const std::optional<Alarm> &alarm = m_monitoringService->currentAlarm();
    return alarmActive() && alarm->state == AlarmState::Acknowledged;
}

QString TelemetryViewModel::alarmMessage() const
{
    if (!alarmActive()) {
        return QStringLiteral("All measurements are within the permitted range");
    }

    if (alarmAcknowledged()) {
        return QStringLiteral("Alarm acknowledged by the operator");
    }

    const Alarm &alarm = *m_monitoringService->currentAlarm();
    return QStringLiteral("Temperature %1 \u00B0C exceeded the %2 \u00B0C threshold")
        .arg(alarm.measuredValue, 0, 'f', 1)
        .arg(alarm.threshold, 0, 'f', 0);
}

void TelemetryViewModel::setAlarmThreshold(double threshold)
{
    const double normalizedThreshold = qBound(40.0, threshold, 120.0);
    AlarmRule alarmRule = m_monitoringService->alarmRule();
    if (qFuzzyCompare(alarmRule.threshold, normalizedThreshold)) {
        return;
    }

    alarmRule.threshold = normalizedThreshold;
    m_monitoringService->setAlarmRule(alarmRule);
}

void TelemetryViewModel::startMonitoring()
{
    m_monitoringService->start();
}

void TelemetryViewModel::stopMonitoring()
{
    m_monitoringService->stop();
}

void TelemetryViewModel::acknowledgeAlarm()
{
    m_monitoringService->acknowledgeAlarm();
}

void TelemetryViewModel::onSampleAccepted(const TelemetrySample &sample)
{
    if (sample.metricKey != QStringLiteral("temperature")) {
        return;
    }

    m_currentTemperature = sample.value;
    m_temperatureSeries.append(sample.value);
    if (m_temperatureSeries.size() > maximumVisibleSamples) {
        m_temperatureSeries.removeFirst();
    }

    ++m_sampleCount;
    m_lastUpdate = sample.timestamp.toLocalTime().toString(QStringLiteral("HH:mm:ss"));
    emit telemetryChanged();
}

void TelemetryViewModel::onHistoryRestored(
    const QList<TelemetrySample> &samples,
    qint64 totalSampleCount)
{
    m_temperatureSeries.clear();
    for (const TelemetrySample &sample : samples) {
        if (sample.metricKey == QStringLiteral("temperature")) {
            m_temperatureSeries.append(sample.value);
        }
    }

    m_sampleCount = totalSampleCount;
    if (!samples.isEmpty()) {
        const TelemetrySample &latestSample = samples.constLast();
        m_currentTemperature = latestSample.value;
        m_lastUpdate = latestSample.timestamp.toLocalTime().toString(QStringLiteral("HH:mm:ss"));
    }
    emit telemetryChanged();
}

void TelemetryViewModel::onDeviceStatusChanged(DeviceStatus status)
{
    Q_UNUSED(status)
    emit statusChanged();
}

} // namespace equipment
