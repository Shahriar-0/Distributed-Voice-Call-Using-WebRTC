# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\appsrc_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appsrc_autogen.dir\\ParseCache.txt"
  "appsrc_autogen"
  )
endif()
