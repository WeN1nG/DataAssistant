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
    find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network Quick Qml Concurrent)
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets Sql Network Concurrent)
    set(QT_VERSION_MAJOR 5)
endif()

# Python 查找 (用于邮件模块) - 硬编码路径
set(PYTHON_INCLUDE_DIR "C:/Users/32660/AppData/Local/Python/pythoncore-3.14-64/include")
set(PYTHON_LIBRARY "C:/Users/32660/AppData/Local/Python/pythoncore-3.14-64/libs/python314.lib")
set(PYTHON_DLL "C:/Users/32660/AppData/Local/Python/pythoncore-3.14-64/python314.dll")

if(EXISTS "${PYTHON_INCLUDE_DIR}/Python.h")
    message(STATUS "Found Python.h at: ${PYTHON_INCLUDE_DIR}")
    set(PYTHON_FOUND TRUE)
else()
    message(WARNING "Python.h not found at: ${PYTHON_INCLUDE_DIR}")
    set(PYTHON_FOUND FALSE)
endif()

# libcurl 库查找（跨平台支持）
find_package(CURL QUIET)

# Windows 特定：尝试查找 libcurl
if(WIN32 AND NOT CURL_FOUND)
    # MSYS2/MinGW libcurl 安装路径
    set(CURL_MSYS2_ROOT "C:/CodeSpace/Compile/Mysy2/ucrt64")
    
    # 查找 libcurl 库文件
    find_library(CURL_LIBRARY
        NAMES curl libcurl
        PATHS "${CURL_MSYS2_ROOT}/lib"
        NO_DEFAULT_PATH
    )
    
    # 查找 libcurl 头文件目录
    set(CURL_INCLUDE_DIR "${CURL_MSYS2_ROOT}/include")
    
    if(CURL_LIBRARY)
        set(CURL_FOUND TRUE)
        set(CURL_LIBRARIES ${CURL_LIBRARY})
        message(STATUS "Found libcurl: ${CURL_LIBRARY}")
        message(STATUS "libcurl include: ${CURL_INCLUDE_DIR}")
        
        # 将 libcurl DLL 复制到输出目录
        add_custom_command(TARGET PersonalDateAssisant POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CURL_MSYS2_ROOT}/bin/libcurl-4.dll"
                "$<TARGET_FILE_DIR:PersonalDateAssisant>"
            COMMENT "Copying libcurl DLL to output directory"
        )
    else()
        message(WARNING "libcurl not found at ${CURL_MSYS2_ROOT}. Please install it using: pacman -S mingw-w64-ucrt-x86_64-libcurl")
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
    ResourceCode/email/EmailModels.h
    ResourceCode/email/EmailDataManager.h
    ResourceCode/email/EmailDataManager.cpp
    ResourceCode/email/EmailBoxWindow.h
    ResourceCode/email/EmailBoxWindow.cpp
    ResourceCode/email/EmailListItemNew.h
    ResourceCode/email/EmailListItemNew.cpp
    ResourceCode/email/EmailCountWindow.h
    ResourceCode/email/EmailCountWindow.cpp
    ResourceCode/email/EmailSendWindow.h
    ResourceCode/email/EmailSendWindow.cpp
    ResourceCode/email/AddAccountDialog.h
    ResourceCode/email/AddAccountDialog.cpp
    ResourceCode/email/EmailProtocolTest.h
    ResourceCode/email/EmailProtocolTest.cpp
    ResourceCode/Database/DatabaseManagerExtra.cpp
    PythonEmail/PythonEmailClient.cpp
    PythonEmail/PythonEmailClient.h
    PythonEmail/PythonEmbedder.cpp
    PythonEmail/PythonEmbedder.h
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
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/ResourceCode/email"
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/lib/QtAwesome/QtAwesome"
    "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/PythonEmail"
)

# 添加 Python 库依赖
if(PYTHON_FOUND)
    target_include_directories(PersonalDateAssisant PRIVATE ${PYTHON_INCLUDE_DIR})
    
    if(WIN32)
        target_link_libraries(PersonalDateAssisant PRIVATE "${PYTHON_LIBRARY}")
        add_custom_command(TARGET PersonalDateAssisant POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PYTHON_DLL}"
                "$<TARGET_FILE_DIR:PersonalDateAssisant>"
            COMMENT "Copying Python DLL to output directory"
        )
        add_custom_command(TARGET PersonalDateAssisant POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "C:/Users/32660/AppData/Local/Python/pythoncore-3.14-64/DLLs"
                "$<TARGET_FILE_DIR:PersonalDateAssisant>/DLLs"
            COMMENT "Copying Python DLLs directory"
        )
    else()
        target_link_libraries(PersonalDateAssisant PRIVATE ${PYTHON_LIBRARY})
    endif()
endif()

target_link_libraries(PersonalDateAssisant PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Concurrent
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
