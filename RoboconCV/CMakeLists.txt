cmake_minimum_required(VERSION 3.0)

# Project Name
set(PROJ_NAME RoboconCV)
project(${PROJ_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

FILE(GLOB HEADER_LIST "src/*.h"   "src/SerialPort/*.h"  )
FILE(GLOB SOURCE_LIST "src/*.cpp" "src/SerialPort/*.cpp")

add_executable(${PROJ_NAME} ${SOURCE_LIST} ${HEADER_LIST})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJ_NAME})

# OpenCV
find_package(OpenCV REQUIRED)

if(OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
  target_link_libraries(${PROJ_NAME} ${OpenCV_LIBS})
endif()

# Camera Driver
include_directories(${CMAKE_SOURCE_DIR}/MindVisionAPI/Include)
#target_link_libraries(${PROJ_NAME} ${CMAKE_SOURCE_DIR}/MindVisionAPI/Library/MVCAMSDK.lib)
target_link_libraries(${PROJ_NAME} ${CMAKE_SOURCE_DIR}/MindVisionAPI/Library/MVCAMSDK_X64.lib)
#include_directories($ENV{MV_DIR}\\Include)
#target_link_libraries(${PROJ_NAME} $ENV{MV_DIR}\\MVCAMSDK.lib)

# ∂‡∫À¥¶¿Ì
set_target_properties(${PROJ_NAME} PROPERTIES COMPILE_FLAGS "/MP")

# Administrator
#set_target_properties(${PROJ_NAME} PROPERTIES LINK_FLAGS "/level='requireAdministrator'") 

# Unicode Encoder
Add_Definitions(-DUNICODE -D_UNICODE)