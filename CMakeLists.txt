cmake_minimum_required(VERSION 3.16)
project(PersonalDateAssisant VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib/QtAwesome)

# Qt 版本兼容性支持
find_package(Qt6 QUIET)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network Quick Qml)
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets Sql Network)
    set(QT_VERSION_MAJOR 5)
endif()

# libcurl 库查找（跨平台支持）
find_package(CURL)

# Windows 特定：尝试查找 libcurl
if(WIN32 AND NOT CURL_FOUND)
    # 尝试多个常见的 MSYS2/MinGW libcurl 位置
    set(CURL_SEARCH_PATHS
        "C:/CodeSpace/Compile/Msys2/ucrt64/lib"
        "C:/CodeSpace/Compile/Msys2/ucrt64/lib/cmake/curl"
        "C:/msys64/ucrt64/lib"
        "C:/msys64/mingw64/lib"
        "C:/Qt/Tools/mingw*/lib"
        "$ENV{MSYSTEM_PREFIX}/lib"
    )
    
    find_library(CURL_LIBRARY
        NAMES curl libcurl
        PATHS ${CURL_SEARCH_PATHS}
        PATH_SUFFIXES lib
    )
    
    if(CURL_LIBRARY)
        set(CURL_FOUND TRUE)
        set(CURL_LIBRARIES ${CURL_LIBRARY})
        message(STATUS "Found libcurl: ${CURL_LIBRARY}")
    else()
        message(WARNING "libcurl not found. Weather network features may not work.")
    endif()
endif()

# 源文件列表
set(PROJECT_SOURCES
    ResourceCode/Core/main.cpp
    ResourceCode/Core/Widget.cpp
    ResourceCode/Core/Widget.h
    ResourceCode/Core/Widget.ui
    ResourceCode/Database/DatabaseManager.cpp
    ResourceCode/Database/DatabaseManager.h
    ResourceCode/Schedule/ScheduleDialog.cpp
    ResourceCode/Schedule/ScheduleDialog.h
    ResourceCode/Schedule/ScheduleDialog.ui
    ResourceCode/Notification/ReminderManager.cpp
    ResourceCode/Notification/ReminderManager.h
    ResourceCode/Notification/SystemTrayNotification.cpp
    ResourceCode/Notification/SystemTrayNotification.h
    ResourceCode/Weather/WeatherManager.cpp
    ResourceCode/Weather/WeatherManager.h
    ResourceCode/Settings/SettingsDialog.cpp
    ResourceCode/Settings/SettingsDialog.h
    ResourceCode/Settings/SettingsDialog.ui
    ResourceCode/Schedule/ScheduleDetailDialog.cpp
    ResourceCode/Schedule/ScheduleDetailDialog.h
    ResourceCode/Schedule/ScheduleListDialog.cpp
    ResourceCode/Schedule/ScheduleListDialog.h
    ResourceCode/Calendar/CalendarDelegate.cpp
    ResourceCode/Calendar/CalendarDelegate.h
    ResourceCode/Calendar/LunarCalendar.cpp
    ResourceCode/Calendar/LunarCalendar.h
)

qt_add_executable(PersonalDateAssisant MANUAL_FINALIZATION ${PROJECT_SOURCES})

# 为每个模块子目录添加 include 路径
target_include_directories(PersonalDateAssisant PRIVATE
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/ResourceCode"
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/ResourceCode/Core"
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/ResourceCode/Database"
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/ResourceCode/Schedule"
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/ResourceCode/Notification"
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/ResourceCode/Weather"
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/ResourceCode/Settings"
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/ResourceCode/Calendar"
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/lib/QtAwesome/QtAwesome"
)

target_link_libraries(PersonalDateAssisant PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    QtAwesome
)

# 平台特定的库依赖
if(WIN32)
    target_link_libraries(PersonalDateAssisant PRIVATE
        ws2_32
        wldap32
        winmm.lib
    )
    if(CURL_FOUND)
        target_link_libraries(PersonalDateAssisant PRIVATE ${CURL_LIBRARIES})
    endif()
elseif(UNIX AND NOT APPLE)
    target_link_libraries(PersonalDateAssisant PRIVATE curl pthread)
endif()

set_target_properties(PersonalDateAssisant PROPERTIES
    WIN32_EXECUTABLE TRUE
)

qt_finalize_executable(PersonalDateAssisant)
