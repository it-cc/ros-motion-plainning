cmake_minimum_required(VERSION 3.0.2)
project(trajectory_planner)

get_filename_component(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
if(EXISTS "${PROJECT_ROOT_DIR}/3rd/conanbuildinfo.cmake")
  include(${PROJECT_ROOT_DIR}/3rd/conanbuildinfo.cmake)
  conan_basic_setup()
endif()

find_package(catkin REQUIRED COMPONENTS
  angles
  roscpp
  costmap_2d
  common
  path_processor
)

find_package(Eigen3 REQUIRED)

catkin_package(
 INCLUDE_DIRS include
 LIBRARIES trajectory_generation
 CATKIN_DEPENDS common path_processor
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
)

# optimization
file(GLOB trajectory_generation_src_files src/trajectory_generation/*.cpp)

# trajectory generation
add_library(trajectory_generation
  ${trajectory_generation_src_files}
)

target_include_directories(trajectory_generation PUBLIC
  ${CONAN_INCLUDE_DIRS}
)

target_link_libraries(trajectory_generation
  ${catkin_LIBRARIES}
  ${CONAN_LIBS}
)

set(BUILDING_WITH_TEST Off)
if (${BUILDING_WITH_TEST})
  add_subdirectory(test)
endif()