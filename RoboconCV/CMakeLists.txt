cmake_minimum_required(VERSION 3.0)

set(PROJ_NAME RoboconCV)
project(${PROJ_NAME})

FILE(GLOB HEADER_LIST "src/*.h")
FILE(GLOB SOURCE_LIST "src/*.cpp")

add_executable(${PROJ_NAME} ${SOURCE_LIST} ${HEADER_LIST})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJ_NAME})

#OpenCV
find_package(OpenCV REQUIRED)

IF(OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
  target_link_libraries(${PROJ_NAME} ${OpenCV_LIBS})
endif()