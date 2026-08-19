
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was LeviLinuxConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

# Find required dependencies
find_dependency(Qt6 COMPONENTS Core Widgets Network Concurrent)

# Find optional dependencies
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(OPENGL gl)
    pkg_check_modules(VULKAN vulkan)
    pkg_check_modules(MBEDTLS mbedtls)
    pkg_check_modules(LIBCURL libcurl)
    pkg_check_modules(ZLIB zlib)
endif()

# Import targets
include("${CMAKE_CURRENT_LIST_DIR}/LeviLinuxTargets.cmake")

# Version info
set(LEVI_LINUX_VERSION "1.0.0")
set(LEVI_LINUX_VERSION_MAJOR "1")
set(LEVI_LINUX_VERSION_MINOR "0")
set(LEVI_LINUX_VERSION_PATCH "0")

check_required_components(LeviLinux)
