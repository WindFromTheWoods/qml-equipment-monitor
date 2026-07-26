# Qt/QML Equipment Monitoring System

[![CI](https://github.com/WindFromTheWoods/qml-equipment-monitor/actions/workflows/ci.yml/badge.svg)](https://github.com/WindFromTheWoods/qml-equipment-monitor/actions/workflows/ci.yml)

A desktop application for monitoring industrial equipment. The current version implements a persistent vertical slice using Clean Architecture and MVVM: a simulated device sends telemetry to an application service, SQLite stores telemetry and alarm state, and QML renders both restored history and live measurements.

## Features

- Qt 6.8, C++20, QML, and Qt Quick Controls;
- separation into `domain`, `application`, `infrastructure`, and `presentation` layers;
- application-level `MonitoringService` coordinating transport, alarms, and persistence;
- simulated compressor temperature published every 500 ms;
- live chart containing the latest 120 measurements;
- SQLite telemetry history with automatic versioned schema migrations;
- restoration of recent telemetry and unresolved alarms after restart;
- configurable high-temperature threshold;
- persistent active alarm creation, resolution, and operator acknowledgement;
- monitoring start and stop controls;
- unit tests for the domain-level `AlarmEngine`;
- SQLite integration tests covering repository behavior and restart restoration.

## Architecture

```text
src/
  app/                 Application composition root
  domain/              Device, TelemetrySample, Alarm, and AlarmRule
  application/         Monitoring use cases and transport/storage ports
  infrastructure/      Simulated transport and SQLite repository adapters
  presentation/        QML-facing view model and user interface
tests/                  Qt Test unit tests
```

The dependency direction is:

```text
QML View -> TelemetryViewModel -> MonitoringService -> Domain
                                         |       |
                                         v       v
                                    Transport  Repository ports
                                         ^       ^
                                         |       |
                                  Infrastructure adapters
```

QML contains presentation logic only. `MonitoringService` coordinates the monitoring use case through `IDeviceTransport`, `ITelemetryRepository`, and `IAlarmRepository`. The current infrastructure layer supplies a simulated transport and SQLite adapters, while `TelemetryViewModel` formats restored and live state for QML.

The SQLite database is created in the platform-specific application data directory. Migration 1 creates the telemetry, alarm, and migration-history tables automatically. Telemetry samples and alarm acknowledgement state are restored when the application starts again.

## Requirements

- Qt 6.8.0 or newer with the Core, Gui, Quick, Quick Controls 2, SQL, and Test modules;
- CMake 3.21 or newer;
- a C++20-compatible compiler;
- Ninja or another CMake-supported build tool.

## Build

Open the root `CMakeLists.txt` in Qt Creator and select a Qt 6.8 desktop kit, or build from PowerShell:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="W:\Qt\6.8.0\mingw_64"
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run

When running outside Qt Creator, make the Qt runtime libraries available on `PATH`:

```powershell
$env:PATH = "W:\Qt\6.8.0\mingw_64\bin;W:\Qt\Tools\mingw1310_64\bin;$env:PATH"
.\build\equipment-monitor.exe
```

Qt Creator configures the required runtime paths automatically.

## Test

The test suite validates metric filtering, strict threshold behavior, alarm creation, SQLite migrations, limited history ordering, alarm lifecycle persistence, and restoration across database restarts:

```powershell
ctest --test-dir build --output-on-failure
```

## Continuous Integration

GitHub Actions runs automatically for every push to `main`, every pull request targeting `main`, and manual `workflow_dispatch` runs. The CI matrix builds the project with Ubuntu/GCC, Windows/MSVC 2022, and Windows/MinGW 13.1. Every variant performs the following checks:

- configures and builds the project in Release mode;
- treats compiler warnings as errors;
- runs all CTest unit tests and fails when no tests are discovered;
- runs the generated Qt `all_qmllint` target;
- generates Doxygen documentation on Ubuntu;
- packages the Windows executable with its Qt runtime dependencies.

Successful Windows jobs publish separate `equipment-monitor-windows-msvc-x64` and `equipment-monitor-windows-mingw-x64` artifacts that are retained for 14 days. GitHub Actions dependencies are pinned to immutable commit SHAs and monitored by Dependabot.

## Generate API Documentation

When Doxygen is installed, CMake provides an optional `docs` target:

```powershell
cmake --build build --target docs
```

The generated HTML documentation is written to `build/docs/html/index.html`.

## Next Milestone

The next vertical slice will introduce multiple monitored devices through a `QAbstractListModel`, device selection in QML, per-device history, and isolated connection and alarm state.
