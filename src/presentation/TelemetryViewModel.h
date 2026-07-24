/**
 * @file TelemetryViewModel.h
 * @brief Declares the QML-facing state and commands for live telemetry.
 */

#pragma once

#include "application/AlarmEngine.h"
#include "application/IDeviceTransport.h"
#include "domain/Alarm.h"
#include "domain/AlarmRule.h"
#include "domain/TelemetrySample.h"

#include <QObject>
#include <QVariantList>

#include <optional>

namespace equipment {

/**
 * @brief Adapts device telemetry and alarm state for declarative QML bindings.
 *
 * The view model subscribes to the transport application port, keeps a bounded
 * chart window, evaluates the active temperature rule, and exposes only
 * presentation-ready properties and commands to QML.
 */
class TelemetryViewModel final : public QObject {
    Q_OBJECT

    /** @brief Human-readable device name. */
    Q_PROPERTY(QString deviceName READ deviceName CONSTANT)

    /** @brief Human-readable device type. */
    Q_PROPERTY(QString deviceType READ deviceType CONSTANT)

    /** @brief Latest temperature value in degrees Celsius. */
    Q_PROPERTY(double currentTemperature READ currentTemperature NOTIFY telemetryChanged)

    /** @brief Bounded sequence of values rendered by the live chart. */
    Q_PROPERTY(QVariantList temperatureSeries READ temperatureSeries NOTIFY telemetryChanged)

    /** @brief Total number of accepted temperature samples. */
    Q_PROPERTY(int sampleCount READ sampleCount NOTIFY telemetryChanged)

    /** @brief Local time of the latest sample formatted for display. */
    Q_PROPERTY(QString lastUpdate READ lastUpdate NOTIFY telemetryChanged)

    /** @brief Indicates whether the device transport is online. */
    Q_PROPERTY(bool online READ online NOTIFY statusChanged)

    /** @brief Human-readable transport connection state. */
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)

    /** @brief Configurable high-temperature alarm threshold. */
    Q_PROPERTY(double alarmThreshold READ alarmThreshold WRITE setAlarmThreshold NOTIFY thresholdChanged)

    /** @brief Indicates whether an unresolved alarm exists. */
    Q_PROPERTY(bool alarmActive READ alarmActive NOTIFY alarmChanged)

    /** @brief Indicates whether the active alarm was acknowledged. */
    Q_PROPERTY(bool alarmAcknowledged READ alarmAcknowledged NOTIFY alarmChanged)

    /** @brief Presentation-ready summary of the current alarm state. */
    Q_PROPERTY(QString alarmMessage READ alarmMessage NOTIFY alarmChanged)

public:
    /**
     * @brief Creates a view model for a single monitored device.
     * @param transport Non-owning pointer to the telemetry transport.
     * @param deviceName Human-readable device name.
     * @param deviceType Human-readable device type.
     * @param parent Optional Qt object owner.
     */
    explicit TelemetryViewModel(
        IDeviceTransport *transport,
        QString deviceName,
        QString deviceType,
        QObject *parent = nullptr);

    /** @return The human-readable device name. */
    [[nodiscard]] QString deviceName() const;

    /** @return The human-readable device type. */
    [[nodiscard]] QString deviceType() const;

    /** @return The latest temperature value in degrees Celsius. */
    [[nodiscard]] double currentTemperature() const noexcept;

    /** @return A copy of the bounded temperature series displayed by QML. */
    [[nodiscard]] QVariantList temperatureSeries() const;

    /** @return The total number of accepted temperature samples. */
    [[nodiscard]] int sampleCount() const noexcept;

    /** @return The local time of the latest sample formatted for display. */
    [[nodiscard]] QString lastUpdate() const;

    /** @return `true` when the monitored device is online. */
    [[nodiscard]] bool online() const noexcept;

    /** @return A human-readable representation of the transport state. */
    [[nodiscard]] QString statusText() const;

    /** @return The configured high-temperature alarm threshold. */
    [[nodiscard]] double alarmThreshold() const noexcept;

    /** @return `true` when an unresolved temperature alarm exists. */
    [[nodiscard]] bool alarmActive() const noexcept;

    /** @return `true` when the active alarm was acknowledged by an operator. */
    [[nodiscard]] bool alarmAcknowledged() const noexcept;

    /** @return A presentation-ready summary of the current alarm state. */
    [[nodiscard]] QString alarmMessage() const;

    /**
     * @brief Updates the high-temperature alarm threshold.
     * @param threshold Requested threshold in degrees Celsius; values are
     * clamped to the supported range of 40–120 degrees.
     */
    void setAlarmThreshold(double threshold);

    /** @brief Starts receiving telemetry through the configured transport. */
    Q_INVOKABLE void startMonitoring();

    /** @brief Stops receiving telemetry through the configured transport. */
    Q_INVOKABLE void stopMonitoring();

    /** @brief Marks the current active alarm as acknowledged. */
    Q_INVOKABLE void acknowledgeAlarm();

signals:
    /** @brief Notifies QML that the latest sample or chart series changed. */
    void telemetryChanged();

    /** @brief Notifies QML that the device connection state changed. */
    void statusChanged();

    /** @brief Notifies QML that the alarm threshold changed. */
    void thresholdChanged();

    /** @brief Notifies QML that the active alarm state changed. */
    void alarmChanged();

private slots:
    /**
     * @brief Processes a telemetry sample published by the transport.
     * @param sample Normalized device measurement.
     */
    void onTelemetryReceived(const equipment::TelemetrySample &sample);

    /**
     * @brief Applies a transport state update to the presentation model.
     * @param status New connection state.
     */
    void onDeviceStatusChanged(equipment::DeviceStatus status);

private:
    /**
     * @brief Re-evaluates the current alarm lifecycle for a sample.
     * @param sample Latest accepted temperature measurement.
     */
    void evaluateAlarm(const TelemetrySample &sample);

    /** @brief Maximum number of values retained for the live chart. */
    static constexpr qsizetype maximumVisibleSamples = 120;

    IDeviceTransport *m_transport; ///< Non-owning telemetry transport pointer.
    QString m_deviceName; ///< Device name exposed to QML.
    QString m_deviceType; ///< Device type exposed to QML.
    AlarmEngine m_alarmEngine; ///< Stateless alarm evaluation service.
    AlarmRule m_temperatureRule; ///< Active high-temperature rule.
    std::optional<Alarm> m_currentAlarm; ///< Current unresolved alarm, if any.
    std::optional<TelemetrySample> m_lastSample; ///< Latest accepted sample.
    QVariantList m_temperatureSeries; ///< Bounded values used by the chart.
    double m_currentTemperature{0.0}; ///< Latest temperature in degrees Celsius.
    int m_sampleCount{0}; ///< Total number of accepted samples.
    QString m_lastUpdate{QStringLiteral("—")}; ///< Formatted latest sample time.
    DeviceStatus m_status{DeviceStatus::Offline}; ///< Current transport state.
};

} // namespace equipment
