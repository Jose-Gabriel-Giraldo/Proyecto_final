# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\proyecto_final_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\proyecto_final_autogen.dir\\ParseCache.txt"
  "proyecto_final_autogen"
  )
endif()
