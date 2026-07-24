/**
 * @file main.cpp
 * @brief Composes and starts the Equipment Monitor desktop application.
 */

#include "infrastructure/SimulatedTransport.h"
#include "presentation/TelemetryViewModel.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QVariantMap>

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

    equipment::SimulatedTransport transport;
    equipment::TelemetryViewModel viewModel(
        &transport,
        transport.device().name,
        transport.device().type);

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
    viewModel.startMonitoring();

    return application.exec();
}
