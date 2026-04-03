# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\PersonalDateAssisant_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\PersonalDateAssisant_autogen.dir\\ParseCache.txt"
  "PersonalDateAssisant_autogen"
  )
endif()
