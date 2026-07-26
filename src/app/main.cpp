/**
 * @file main.cpp
 * @brief Composes and starts the Equipment Monitor desktop application.
 */

#include "application/MonitoringService.h"
#include "infrastructure/SimulatedTransport.h"
#include "infrastructure/SqliteAlarmRepository.h"
#include "infrastructure/SqliteDatabase.h"
#include "infrastructure/SqliteTelemetryRepository.h"
#include "presentation/TelemetryViewModel.h"

#include <QDebug>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QStandardPaths>
#include <QVariantMap>

#include <cstdlib>

/**
 * @brief Creates application services, exposes the view model, and runs Qt.
 * @param argc Number of command-line arguments.
 * @param argv Command-line argument values.
 * @return The Qt event loop exit code.
 */
int main(int argc, char *argv[])
{
    QGuiApplication application(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("Equipment Monitor"));
    QGuiApplication::setOrganizationName(QStringLiteral("Equipment Monitor"));

    qRegisterMetaType<equipment::TelemetrySample>();
    qRegisterMetaType<equipment::DeviceStatus>();

    const QString applicationDataPath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    const QString databasePath = QDir(applicationDataPath).filePath(
        QStringLiteral("equipment-monitor.sqlite"));

    equipment::SqliteDatabase database(databasePath);
    QString databaseError;
    if (!database.open(&databaseError)) {
        qCritical().noquote() << databaseError;
        return EXIT_FAILURE;
    }

    equipment::SqliteTelemetryRepository telemetryRepository(&database);
    equipment::SqliteAlarmRepository alarmRepository(&database);
    equipment::SimulatedTransport transport;
    equipment::MonitoringService monitoringService(
        &transport,
        &telemetryRepository,
        &alarmRepository,
        transport.device().id,
        equipment::AlarmRule{
            .metricKey = QStringLiteral("temperature"),
            .operation = equipment::ComparisonOperation::GreaterThan,
            .threshold = 80.0,
            .severity = equipment::AlarmSeverity::Critical,
        });
    equipment::TelemetryViewModel viewModel(
        &monitoringService,
        transport.device().name,
        transport.device().type);

    QObject::connect(
        &monitoringService,
        &equipment::MonitoringService::persistenceError,
        &application,
        [](const QString &message) {
            qWarning().noquote() << message;
        });

    if (!monitoringService.restoreHistory()) {
        qWarning() << "The application will continue without restored history.";
    }

    QQmlApplicationEngine engine;
    engine.setInitialProperties(QVariantMap{
        {QStringLiteral("viewModel"), QVariant::fromValue(&viewModel)},
    });

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &application,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule(QStringLiteral("EquipmentMonitor"), QStringLiteral("Main"));
    monitoringService.start();

    return application.exec();
}
