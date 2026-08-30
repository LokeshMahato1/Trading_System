cmake_minimum_required(VERSION 3.16)
project(TradingManagementSystem CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# =========================================================================
# trading_core: the engine, as a static library, linked into trading_gui.
# =========================================================================
file(GLOB_RECURSE TMS_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/Engine/*.cpp"
)

add_library(trading_core STATIC ${TMS_CORE_SOURCES})

target_include_directories(trading_core PUBLIC
    "${CMAKE_SOURCE_DIR}/Engine"
)

find_package(SQLite3 REQUIRED)
target_link_libraries(trading_core PUBLIC SQLite::SQLite3)

find_package(Threads REQUIRED)
target_link_libraries(trading_core PUBLIC Threads::Threads)

find_package(OpenSSL QUIET)
if(OpenSSL_FOUND)
    message(STATUS "OpenSSL found: enabling HTTPS support for market data fetches.")
    target_compile_definitions(trading_core PUBLIC CPPHTTPLIB_OPENSSL_SUPPORT)
    target_link_libraries(trading_core PUBLIC OpenSSL::SSL OpenSSL::Crypto)
else()
    message(WARNING
        "OpenSSL not found: building WITHOUT HTTPS support. "
        "Live market-data fetches will fail. Install libssl-dev and reconfigure to enable it."
    )
endif()

if(WIN32)
    target_link_libraries(trading_core PUBLIC ws2_32 crypt32)
endif()

if(MSVC)
    target_compile_options(trading_core PRIVATE /W4)
else()
    target_compile_options(trading_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# =========================================================================
# trading_gui: the "Artha Bazaar" Qt Quick/QML front-end (single
# executable, single entry point at Gui/main.cpp). Requires Qt6 Quick.
# =========================================================================
find_package(Qt6 REQUIRED COMPONENTS Quick)
qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(trading_gui
    "${CMAKE_SOURCE_DIR}/Gui/main.cpp"
    "${CMAKE_SOURCE_DIR}/Gui/Bridge.h"
    "${CMAKE_SOURCE_DIR}/Gui/Bridge.cpp"
)

# Each QML/asset file gets an explicit flat alias (e.g. "Main.qml" instead
# of "Gui/Main.qml") so the compiled resource layout is qrc:/<uri>/Main.qml
# regardless of which directory this CMakeLists.txt lives in - this lets
# everything stay in one CMakeLists.txt instead of needing a Gui/CMakeLists.txt.
set(TMS_QML_FILES
    "${CMAKE_SOURCE_DIR}/Gui/Main.qml"
    "${CMAKE_SOURCE_DIR}/Gui/LoginPage.qml"
    "${CMAKE_SOURCE_DIR}/Gui/DashboardPage.qml"
    "${CMAKE_SOURCE_DIR}/Gui/AdminPage.qml"
    "${CMAKE_SOURCE_DIR}/Gui/TradeDialog.qml"
    "${CMAKE_SOURCE_DIR}/Gui/OrdersDialog.qml"
    "${CMAKE_SOURCE_DIR}/Gui/CollateralDialog.qml"
    "${CMAKE_SOURCE_DIR}/Gui/PortfolioDialog.qml"
)
foreach(qmlFile ${TMS_QML_FILES})
    get_filename_component(qmlAlias "${qmlFile}" NAME)
    set_source_files_properties(${qmlFile} PROPERTIES QT_RESOURCE_ALIAS "${qmlAlias}")
endforeach()

set_source_files_properties(
    "${CMAKE_SOURCE_DIR}/Gui/loginLogo.png"
    PROPERTIES QT_RESOURCE_ALIAS "loginLogo.png"
)

qt_add_qml_module(trading_gui
    URI com.arthabazar.app
    VERSION 1.0
    QML_FILES ${TMS_QML_FILES}
    RESOURCES "${CMAKE_SOURCE_DIR}/Gui/loginLogo.png"
)

target_link_libraries(trading_gui PRIVATE trading_core Qt6::Quick)
