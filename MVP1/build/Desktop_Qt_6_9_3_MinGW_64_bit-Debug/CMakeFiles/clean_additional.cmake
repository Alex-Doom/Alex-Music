# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\CURS_WORK_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CURS_WORK_autogen.dir\\ParseCache.txt"
  "CURS_WORK_autogen"
  )
endif()
