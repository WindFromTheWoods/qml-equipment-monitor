/**
 * @file SimulatedTransport.h
 * @brief Declares a deterministic-style telemetry source for local development.
 */

#pragma once

#include "application/IDeviceTransport.h"
#include "domain/Device.h"

#include <QTimer>

namespace equipment {

/**
 * @brief Generates realistic temperature samples without external equipment.
 *
 * The adapter implements the same application port as future MQTT or
 * WebSocket transports, allowing the complete monitoring flow to be exercised
 * locally.
 */
class SimulatedTransport final : public IDeviceTransport {
    Q_OBJECT

public:
    /**
     * @brief Creates a stopped simulator for a virtual compressor.
     * @param parent Optional Qt object owner.
     */
    explicit SimulatedTransport(QObject *parent = nullptr);

    /** @brief Connects the simulated device and begins publishing samples. */
    void start() override;

    /** @brief Stops publishing samples and marks the device offline. */
    void stop() override;

    /**
     * @brief Returns metadata for the simulated device.
     * @return A reference that remains valid for the lifetime of the transport.
     */
    [[nodiscard]] const Device &device() const noexcept;

private:
    /** @brief Generates and publishes the next temperature sample. */
    void produceSample();

    QTimer m_timer; ///< Controls the telemetry publication interval.
    Device m_device; ///< Metadata and current state of the virtual device.
    int m_sampleIndex{0}; ///< Monotonic position in the generated waveform.
};

} // namespace equipment
