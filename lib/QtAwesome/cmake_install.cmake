# Install script for directory: C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/build/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/D/software/QT/Tools/mingw1310_64/bin/objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome/libQtAwesome.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QtAwesome" TYPE FILE FILES
    "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome/QtAwesome/QtAwesome.h"
    "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome/QtAwesome/QtAwesomeAnim.h"
    "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome/QtAwesome/QtAwesomeEnumGenerated.h"
    "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome/QtAwesome/QtAwesomeStringGenerated.h"
    "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome/QtAwesome/QtAwesomeQuickImageProvider.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/licenses/QtAwesome" TYPE FILE RENAME "LICENSE" FILES "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome/LICENSE.md")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtAwesome/QtAwesomeConfig.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtAwesome/QtAwesomeConfig.cmake"
         "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome/CMakeFiles/Export/0e9df93fb2de51680e781cd4501de1ff/QtAwesomeConfig.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtAwesome/QtAwesomeConfig-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QtAwesome/QtAwesomeConfig.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/QtAwesome" TYPE FILE FILES "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome/CMakeFiles/Export/0e9df93fb2de51680e781cd4501de1ff/QtAwesomeConfig.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/QtAwesome" TYPE FILE FILES "C:/CODE/QTS/Projects/DateAssistant/DataAssistant/lib/QtAwesome/CMakeFiles/Export/0e9df93fb2de51680e781cd4501de1ff/QtAwesomeConfig-release.cmake")
  endif()
endif()

