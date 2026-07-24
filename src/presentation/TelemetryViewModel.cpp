/**
 * @file TelemetryViewModel.cpp
 * @brief Implements the QML-facing live telemetry view model.
 */

#include "presentation/TelemetryViewModel.h"

#include <QtGlobal>

namespace equipment {

TelemetryViewModel::TelemetryViewModel(
    IDeviceTransport *transport,
    QString deviceName,
    QString deviceType,
    QObject *parent)
    : QObject(parent)
    , m_transport(transport)
    , m_deviceName(std::move(deviceName))
    , m_deviceType(std::move(deviceType))
    , m_temperatureRule{
          .metricKey = QStringLiteral("temperature"),
          .operation = ComparisonOperation::GreaterThan,
          .threshold = 80.0,
          .severity = AlarmSeverity::Critical,
      }
{
    Q_ASSERT(m_transport != nullptr);

    connect(
        m_transport,
        &IDeviceTransport::telemetryReceived,
        this,
        &TelemetryViewModel::onTelemetryReceived);
    connect(
        m_transport,
        &IDeviceTransport::statusChanged,
        this,
        &TelemetryViewModel::onDeviceStatusChanged);
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

int TelemetryViewModel::sampleCount() const noexcept
{
    return m_sampleCount;
}

QString TelemetryViewModel::lastUpdate() const
{
    return m_lastUpdate;
}

bool TelemetryViewModel::online() const noexcept
{
    return m_status == DeviceStatus::Online;
}

QString TelemetryViewModel::statusText() const
{
    switch (m_status) {
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
    return m_temperatureRule.threshold;
}

bool TelemetryViewModel::alarmActive() const noexcept
{
    return m_currentAlarm.has_value()
        && m_currentAlarm->state != AlarmState::Resolved;
}

bool TelemetryViewModel::alarmAcknowledged() const noexcept
{
    return alarmActive()
        && m_currentAlarm->state == AlarmState::Acknowledged;
}

QString TelemetryViewModel::alarmMessage() const
{
    if (!alarmActive()) {
        return QStringLiteral("All measurements are within the permitted range");
    }

    if (alarmAcknowledged()) {
        return QStringLiteral("Alarm acknowledged by the operator");
    }

    return QStringLiteral("Temperature %1 °C exceeded the %2 °C threshold")
        .arg(m_currentAlarm->measuredValue, 0, 'f', 1)
        .arg(m_currentAlarm->threshold, 0, 'f', 0);
}

void TelemetryViewModel::setAlarmThreshold(double threshold)
{
    const double normalizedThreshold = qBound(40.0, threshold, 120.0);
    if (qFuzzyCompare(m_temperatureRule.threshold, normalizedThreshold)) {
        return;
    }

    m_temperatureRule.threshold = normalizedThreshold;
    emit thresholdChanged();

    if (m_lastSample.has_value()) {
        evaluateAlarm(*m_lastSample);
    }
}

void TelemetryViewModel::startMonitoring()
{
    m_transport->start();
}

void TelemetryViewModel::stopMonitoring()
{
    m_transport->stop();
}

void TelemetryViewModel::acknowledgeAlarm()
{
    if (!alarmActive() || alarmAcknowledged()) {
        return;
    }

    m_currentAlarm->state = AlarmState::Acknowledged;
    emit alarmChanged();
}

void TelemetryViewModel::onTelemetryReceived(const TelemetrySample &sample)
{
    if (sample.metricKey != QStringLiteral("temperature")) {
        return;
    }

    m_lastSample = sample;
    m_currentTemperature = sample.value;
    m_temperatureSeries.append(sample.value);
    if (m_temperatureSeries.size() > maximumVisibleSamples) {
        m_temperatureSeries.removeFirst();
    }

    ++m_sampleCount;
    m_lastUpdate = sample.timestamp.toLocalTime().toString(QStringLiteral("HH:mm:ss"));
    emit telemetryChanged();

    evaluateAlarm(sample);
}

void TelemetryViewModel::onDeviceStatusChanged(DeviceStatus status)
{
    if (m_status == status) {
        return;
    }

    m_status = status;
    emit statusChanged();
}

void TelemetryViewModel::evaluateAlarm(const TelemetrySample &sample)
{
    const auto evaluatedAlarm = m_alarmEngine.evaluate(sample, m_temperatureRule);

    if (!evaluatedAlarm.has_value()) {
        if (m_currentAlarm.has_value()) {
            m_currentAlarm.reset();
            emit alarmChanged();
        }
        return;
    }

    if (!alarmActive()) {
        m_currentAlarm = evaluatedAlarm;
        emit alarmChanged();
        return;
    }

    m_currentAlarm->measuredValue = sample.value;
    m_currentAlarm->threshold = m_temperatureRule.threshold;
    emit alarmChanged();
}

} // namespace equipment
