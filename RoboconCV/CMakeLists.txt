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
set (OpenCV_DIR "C:/ST42Data/Code/opencv3Src/install/x86/vc15/lib") # 更改此路径为OpenCVConfig.cmake的路径（库目录）
find_package(OpenCV REQUIRED)

if(OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
  target_link_libraries(${PROJ_NAME} ${OpenCV_LIBS})
endif()

# Camera Driver
include_directories(${CMAKE_SOURCE_DIR}/MindVisionAPI/Include)
target_link_libraries(${PROJ_NAME} ${CMAKE_SOURCE_DIR}/MindVisionAPI/Library/MVCAMSDK.lib)

# 多核处理
set_target_properties(${PROJ_NAME} PROPERTIES COMPILE_FLAGS "/MP")

# Administrator
#set_target_properties(${PROJ_NAME} PROPERTIES LINK_FLAGS "/level='requireAdministrator'") 

# Unicode编码
Add_Definitions(-DUNICODE -D_UNICODE)