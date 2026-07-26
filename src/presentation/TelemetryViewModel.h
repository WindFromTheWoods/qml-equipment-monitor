/**
 * @file TelemetryViewModel.h
 * @brief Declares the QML-facing state and commands for live telemetry.
 */

#pragma once

#include "application/MonitoringService.h"
#include "domain/TelemetrySample.h"

#include <QObject>
#include <QVariantList>

namespace equipment {

/**
 * @brief Adapts monitoring use-case state for declarative QML bindings.
 *
 * The view model subscribes to MonitoringService, keeps presentation-ready
 * chart values, and exposes formatted state and commands to QML.
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

    /** @brief Total number of persisted and live temperature samples. */
    Q_PROPERTY(qint64 sampleCount READ sampleCount NOTIFY telemetryChanged)

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
     * @param monitoringService Non-owning application service pointer.
     * @param deviceName Human-readable device name.
     * @param deviceType Human-readable device type.
     * @param parent Optional Qt object owner.
     */
    explicit TelemetryViewModel(
        MonitoringService *monitoringService,
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

    /** @return The total number of persisted and live temperature samples. */
    [[nodiscard]] qint64 sampleCount() const noexcept;

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
     * clamped to the supported range of 40-120 degrees.
     */
    void setAlarmThreshold(double threshold);

    /** @brief Starts receiving telemetry through the monitoring service. */
    Q_INVOKABLE void startMonitoring();

    /** @brief Stops receiving telemetry through the monitoring service. */
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
     * @brief Applies a live sample published by the monitoring service.
     * @param sample Accepted normalized device measurement.
     */
    void onSampleAccepted(const equipment::TelemetrySample &sample);

    /**
     * @brief Replaces the chart state with restored persistent history.
     * @param samples Recent temperature samples in chronological order.
     * @param totalSampleCount Total number of persisted temperature samples.
     */
    void onHistoryRestored(
        const QList<equipment::TelemetrySample> &samples,
        qint64 totalSampleCount);

    /**
     * @brief Applies a transport state update to the presentation model.
     * @param status New connection state.
     */
    void onDeviceStatusChanged(equipment::DeviceStatus status);

private:
    /** @brief Maximum number of values retained for the live chart. */
    static constexpr qsizetype maximumVisibleSamples = 120;

    MonitoringService *m_monitoringService; ///< Non-owning application service pointer.
    QString m_deviceName; ///< Device name exposed to QML.
    QString m_deviceType; ///< Device type exposed to QML.
    QVariantList m_temperatureSeries; ///< Bounded values used by the chart.
    double m_currentTemperature{0.0}; ///< Latest temperature in degrees Celsius.
    qint64 m_sampleCount{0}; ///< Total number of persisted and live samples.
    QString m_lastUpdate{QStringLiteral("\u2014")}; ///< Formatted latest sample time.
};

} // namespace equipment
