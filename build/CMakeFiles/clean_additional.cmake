# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "RelWithDebInfo")
  file(REMOVE_RECURSE
  "src/ui/CMakeFiles/LeviLinuxUI_autogen.dir/AutogenUsed.txt"
  "src/ui/CMakeFiles/LeviLinuxUI_autogen.dir/ParseCache.txt"
  "src/ui/LeviLinuxUI_autogen"
  )
endif()
