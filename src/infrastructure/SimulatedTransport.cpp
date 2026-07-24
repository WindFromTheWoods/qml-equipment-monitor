/**
 * @file SimulatedTransport.cpp
 * @brief Implements the local simulated device transport.
 */

#include "infrastructure/SimulatedTransport.h"

#include "domain/TelemetrySample.h"

#include <QDateTime>
#include <QRandomGenerator>

#include <cmath>

namespace equipment {

SimulatedTransport::SimulatedTransport(QObject *parent)
    : IDeviceTransport(parent)
    , m_device{
          .id = QUuid(QStringLiteral("{9ad485f6-d455-4e19-92df-828aacdac061}")),
          .name = QStringLiteral("Compressor A-17"),
          .type = QStringLiteral("Simulated sensor"),
          .status = DeviceStatus::Offline,
      }
{
    m_timer.setInterval(500);
    m_timer.setTimerType(Qt::PreciseTimer);

    connect(&m_timer, &QTimer::timeout, this, &SimulatedTransport::produceSample);
}

void SimulatedTransport::start()
{
    if (m_timer.isActive()) {
        return;
    }

    m_device.status = DeviceStatus::Online;
    emit statusChanged(m_device.status);

    produceSample();
    m_timer.start();
}

void SimulatedTransport::stop()
{
    if (!m_timer.isActive() && m_device.status == DeviceStatus::Offline) {
        return;
    }

    m_timer.stop();
    m_device.status = DeviceStatus::Offline;
    emit statusChanged(m_device.status);
}

const Device &SimulatedTransport::device() const noexcept
{
    return m_device;
}

void SimulatedTransport::produceSample()
{
    const double wave = std::sin(static_cast<double>(m_sampleIndex) * 0.23);
    const double noise = (QRandomGenerator::global()->generateDouble() * 2.0) - 1.0;
    const double temperature = 72.0 + (13.0 * wave) + noise;

    emit telemetryReceived(TelemetrySample{
        .deviceId = m_device.id,
        .metricKey = QStringLiteral("temperature"),
        .value = temperature,
        .unit = QStringLiteral("°C"),
        .timestamp = QDateTime::currentDateTimeUtc(),
    });

    ++m_sampleIndex;
}

} // namespace equipment
