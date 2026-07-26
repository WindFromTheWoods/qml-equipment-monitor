cmake_minimum_required(VERSION 3.21)

project(EquipmentMonitor
    VERSION 0.1.0
    DESCRIPTION "Desktop equipment telemetry monitor"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.8 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    Sql
    Test
)
find_package(Doxygen QUIET)

qt_standard_project_setup(REQUIRES 6.8)
enable_testing()

add_library(equipment_domain INTERFACE)
target_include_directories(equipment_domain INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(equipment_domain INTERFACE Qt6::Core)

qt_add_library(equipment_application STATIC
    src/application/AlarmEngine.cpp
    src/application/AlarmEngine.h
    src/application/IAlarmRepository.h
    src/application/IDeviceTransport.h
    src/application/ITelemetryRepository.h
    src/application/MonitoringService.cpp
    src/application/MonitoringService.h
)
target_include_directories(equipment_application PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(equipment_application PUBLIC
    equipment_domain
    Qt6::Core
)

qt_add_library(equipment_infrastructure STATIC
    src/infrastructure/SimulatedTransport.cpp
    src/infrastructure/SimulatedTransport.h
    src/infrastructure/SqliteAlarmRepository.cpp
    src/infrastructure/SqliteAlarmRepository.h
    src/infrastructure/SqliteDatabase.cpp
    src/infrastructure/SqliteDatabase.h
    src/infrastructure/SqliteTelemetryRepository.cpp
    src/infrastructure/SqliteTelemetryRepository.h
)
target_include_directories(equipment_infrastructure PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(equipment_infrastructure PUBLIC
    equipment_application
    Qt6::Core
    Qt6::Sql
)

qt_add_library(equipment_presentation STATIC
    src/presentation/TelemetryViewModel.cpp
    src/presentation/TelemetryViewModel.h
)
target_include_directories(equipment_presentation PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(equipment_presentation PUBLIC
    equipment_application
    Qt6::Core
)

qt_add_executable(equipment-monitor
    src/app/main.cpp
)

set_source_files_properties(
    src/presentation/qml/Main.qml
    PROPERTIES QT_RESOURCE_ALIAS Main.qml
)

qt_add_qml_module(equipment-monitor
    URI EquipmentMonitor
    VERSION 1.0
    QML_FILES
        src/presentation/qml/Main.qml
)

target_link_libraries(equipment-monitor PRIVATE
    equipment_infrastructure
    equipment_presentation
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
)

set_target_properties(equipment-monitor PROPERTIES
    WIN32_EXECUTABLE TRUE
)

qt_add_executable(test_alarm_engine
    tests/test_alarm_engine.cpp
)
target_link_libraries(test_alarm_engine PRIVATE
    equipment_application
    Qt6::Test
)
add_test(NAME AlarmEngine COMMAND test_alarm_engine)

if(Doxygen_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_GENERATE_TREEVIEW YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
    set(DOXYGEN_WARN_AS_ERROR YES)

    doxygen_add_docs(docs
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        COMMENT "Generate Equipment Monitor API documentation"
    )
endif()

if(MSVC)
    target_compile_options(equipment_application PRIVATE /W4 /permissive-)
    target_compile_options(equipment_infrastructure PRIVATE /W4 /permissive-)
    target_compile_options(equipment_presentation PRIVATE /W4 /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(equipment_application PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(equipment_infrastructure PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(equipment_presentation PRIVATE -Wall -Wextra -Wpedantic)
endif()
