cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(systematicstools VERSION 02.00.01 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ Standard to use when compiling")

#Changes default install path to be a subdirectory of the build dir.
#Can set build dir at configure time with -DCMAKE_INSTALL_PREFIX=/install/path
if(CMAKE_INSTALL_PREFIX STREQUAL "" OR CMAKE_INSTALL_PREFIX STREQUAL
  "/usr/local")
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
elseif(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE DebWithRelInfo)
endif()

message(STATUS "INFO: CMAKE_C_COMPILER ${CMAKE_C_COMPILER}")
message(STATUS "INFO: CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER}")
message(STATUS "INFO: CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD}")
message(STATUS "INFO: CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
include(CPM)

find_package(ROOT 6.10 REQUIRED)

## Check if we have fhiclcpp from ups
find_package(fhiclcpp QUIET)
if(NOT fhiclcpp_FOUND)
  ## Find fhicl_cpp_standalone
  find_package(fhiclcppstandalone)
else()
  ## If found, rename the target to fhiclcpp::standalone
  add_library(fhiclcpp::standalone ALIAS fhiclcpp::fhiclcpp)
endif()

###### Compiler set up
add_library(systematicstools_dependencies INTERFACE)
target_link_libraries(systematicstools_dependencies INTERFACE 
  fhiclcpp::standalone)
target_include_directories(systematicstools_dependencies INTERFACE 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include> )
set_target_properties(systematicstools_dependencies PROPERTIES EXPORT_NAME commondeps)
add_library(systtools::commondeps ALIAS systematicstools_dependencies)

install(TARGETS 
          systematicstools_dependencies
    EXPORT systtools-targets)

add_subdirectory(src/systematicstools)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/systematicstoolsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/systematicstoolsConfig.cmake.in"
  "${PROJECT_BINARY_DIR}/systematicstoolsConfig.cmake"
  INSTALL_DESTINATION cmake
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(EXPORT systtools-targets
        NAMESPACE systtools::
        DESTINATION lib/cmake/systematicstools)
install(FILES "${PROJECT_BINARY_DIR}/systematicstoolsConfigVersion.cmake"
              "${PROJECT_BINARY_DIR}/systematicstoolsConfig.cmake"
        DESTINATION lib/cmake/systematicstools)

configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Templates/setup.systematicstools.sh.in 
  ${PROJECT_BINARY_DIR}/setup.systematicstools.sh @ONLY)
install(PROGRAMS ${PROJECT_BINARY_DIR}/setup.systematicstools.sh DESTINATION bin)

install(DIRECTORY fcl DESTINATION fcl)
