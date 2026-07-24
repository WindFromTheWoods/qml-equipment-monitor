/**
 * @file IDeviceTransport.h
 * @brief Declares the application port used to receive device telemetry.
 */

#pragma once

#include "domain/Device.h"
#include "domain/TelemetrySample.h"

#include <QObject>

namespace equipment {

/**
 * @brief Abstracts a live connection to one or more monitored devices.
 *
 * Infrastructure adapters implement this port and publish normalized domain
 * samples. Presentation and application code therefore remain independent of
 * the concrete protocol, such as MQTT, WebSocket, or a simulator.
 */
class IDeviceTransport : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Creates a transport port object.
     * @param parent Optional Qt object owner.
     */
    explicit IDeviceTransport(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    /** @brief Destroys the transport interface. */
    ~IDeviceTransport() override = default;

    /** @brief Starts the device connection and telemetry stream. */
    virtual void start() = 0;

    /** @brief Stops the telemetry stream and disconnects the device. */
    virtual void stop() = 0;

signals:
    /**
     * @brief Publishes a normalized telemetry measurement.
     * @param sample Measurement received from the device.
     */
    void telemetryReceived(const equipment::TelemetrySample &sample);

    /**
     * @brief Reports a device transport state transition.
     * @param status New connection state.
     */
    void statusChanged(equipment::DeviceStatus status);
};

} // namespace equipment
