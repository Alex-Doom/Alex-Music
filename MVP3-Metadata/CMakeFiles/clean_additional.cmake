# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "AlexMusic_autogen"
  "CMakeFiles\\AlexMusic_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\AlexMusic_autogen.dir\\ParseCache.txt"
  )
endif()
