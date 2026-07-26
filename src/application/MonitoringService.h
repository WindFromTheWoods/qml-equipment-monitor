/**
 * @file MonitoringService.h
 * @brief Declares the application service that coordinates monitoring.
 */

#pragma once

#include "application/AlarmEngine.h"
#include "application/IAlarmRepository.h"
#include "application/IDeviceTransport.h"
#include "application/ITelemetryRepository.h"

#include <QObject>

#include <optional>

namespace equipment {

/**
 * @brief Coordinates transport, alarm evaluation, and persistent history.
 *
 * The service owns monitoring use-case state but does not own its transport or
 * repositories. It publishes domain changes that presentation adapters can
 * format without knowing how telemetry is transported or stored.
 */
class MonitoringService final : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Creates a monitoring service for one device.
     * @param transport Non-owning telemetry transport pointer.
     * @param telemetryRepository Non-owning telemetry repository pointer.
     * @param alarmRepository Non-owning alarm repository pointer.
     * @param deviceId Stable identifier of the monitored device.
     * @param alarmRule Initial rule evaluated for incoming samples.
     * @param parent Optional Qt object owner.
     */
    explicit MonitoringService(
        IDeviceTransport *transport,
        ITelemetryRepository *telemetryRepository,
        IAlarmRepository *alarmRepository,
        QUuid deviceId,
        AlarmRule alarmRule,
        QObject *parent = nullptr);

    /** @return The stable identifier of the monitored device. */
    [[nodiscard]] QUuid deviceId() const;

    /** @return The current transport connection state. */
    [[nodiscard]] DeviceStatus status() const noexcept;

    /** @return The alarm rule currently evaluated by the service. */
    [[nodiscard]] AlarmRule alarmRule() const;

    /** @return The latest accepted sample, if telemetry has been received. */
    [[nodiscard]] const std::optional<TelemetrySample> &lastSample() const noexcept;

    /** @return The current unresolved alarm, if one exists. */
    [[nodiscard]] const std::optional<Alarm> &currentAlarm() const noexcept;

    /**
     * @brief Restores recent telemetry and unresolved alarm state.
     * @param maximumSamples Maximum number of chart samples to load.
     * @return `true` when every repository query completed successfully.
     */
    [[nodiscard]] bool restoreHistory(qsizetype maximumSamples = 120);

    /** @brief Starts the configured telemetry transport. */
    void start();

    /** @brief Stops the configured telemetry transport. */
    void stop();

    /**
     * @brief Replaces the active alarm rule and re-evaluates the latest sample.
     * @param alarmRule New rule to apply.
     */
    void setAlarmRule(const AlarmRule &alarmRule);

    /** @brief Acknowledges the current active alarm and persists the change. */
    void acknowledgeAlarm();

signals:
    /**
     * @brief Publishes a live sample after it is accepted by the service.
     * @param sample Accepted telemetry sample.
     */
    void sampleAccepted(const equipment::TelemetrySample &sample);

    /**
     * @brief Publishes restored history to presentation adapters.
     * @param samples Recent samples ordered from oldest to newest.
     * @param totalSampleCount Total number of stored samples for the metric.
     */
    void historyRestored(
        const QList<equipment::TelemetrySample> &samples,
        qint64 totalSampleCount);

    /**
     * @brief Reports a transport state transition.
     * @param status New transport state.
     */
    void statusChanged(equipment::DeviceStatus status);

    /** @brief Reports that the alarm rule changed. */
    void alarmRuleChanged();

    /** @brief Reports that the current alarm lifecycle state changed. */
    void alarmChanged();

    /**
     * @brief Reports a recoverable persistence failure.
     * @param message Human-readable diagnostic provided by a repository.
     */
    void persistenceError(const QString &message);

private slots:
    /**
     * @brief Processes a normalized sample received from the transport.
     * @param sample Incoming telemetry sample.
     */
    void onTelemetryReceived(const equipment::TelemetrySample &sample);

    /**
     * @brief Forwards a transport state transition.
     * @param status New transport state.
     */
    void onDeviceStatusChanged(equipment::DeviceStatus status);

private:
    /**
     * @brief Updates the current alarm lifecycle for a sample.
     * @param sample Latest accepted telemetry sample.
     */
    void evaluateAlarm(const TelemetrySample &sample);

    /**
     * @brief Persists the supplied alarm and reports repository errors.
     * @param alarm Alarm state to persist.
     */
    void persistAlarm(const Alarm &alarm);

    IDeviceTransport *m_transport; ///< Non-owning telemetry transport pointer.
    ITelemetryRepository *m_telemetryRepository; ///< Non-owning telemetry storage pointer.
    IAlarmRepository *m_alarmRepository; ///< Non-owning alarm storage pointer.
    QUuid m_deviceId; ///< Identifier accepted by this service instance.
    AlarmEngine m_alarmEngine; ///< Stateless threshold evaluation service.
    AlarmRule m_alarmRule; ///< Rule evaluated for incoming telemetry.
    std::optional<TelemetrySample> m_lastSample; ///< Latest accepted sample.
    std::optional<Alarm> m_currentAlarm; ///< Current unresolved alarm.
    DeviceStatus m_status{DeviceStatus::Offline}; ///< Current transport state.
};

} // namespace equipment
