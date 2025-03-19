# 设置 CMake 的最低版本要求为 2.8
cmake_minimum_required(VERSION 2.8)
# 设置项目名称为 UTAustinVillaBase，使用 C++ 和 C 语言
project(UTAustinVillaBase CXX C)

# 设置 CMake 模块的搜索路径为项目源目录下的 cmake 文件夹
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

# 查找并要求使用 Threads 库
find_package(Threads REQUIRED)
# 查找并要求使用 Boost 库的 system 组件
find_package(Boost COMPONENTS system REQUIRED)
# 查找并要求使用 Rcssnet3d 库
find_package(Rcssnet3d REQUIRED)

# 如果没有指定构建类型，默认设置为 Release
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set (CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# 如果没有指定 C++ 编译选项，设置默认的编译选项
if(NOT CMAKE_CXX_FLAGS)
  set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused")
endif(NOT CMAKE_CXX_FLAGS)

# 设置可执行文件的链接选项
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")

# 设置核心代码所在的路径
set(CORE_PATH utwalk)

# 设置包含目录，以便编译器能够找到头文件
include_directories(
${CORE_PATH}
${RCSSNET3D_INCLUDE_DIR}
)

# 定义核心代码文件列表
set(CORE_FILES
  ${CORE_PATH}/MotionCore.cpp
  ${CORE_PATH}/Module.cpp  
  ${CORE_PATH}/common/NMatrix.cpp 
  ${CORE_PATH}/math/MVTools.cpp 
  ${CORE_PATH}/math/RotationMatrix.cpp 
  ${CORE_PATH}/memory/Lock.cpp 
  ${CORE_PATH}/memory/Logger.cpp   
  ${CORE_PATH}/memory/Memory.cpp  
  ${CORE_PATH}/memory/MemoryBlock.cpp  
  ${CORE_PATH}/memory/PrivateMemory.cpp  
  ${CORE_PATH}/memory/SharedMemory.cpp 
  ${CORE_PATH}/motion/MotionModule.cpp
  ${CORE_PATH}/motion/UTWalkEngine.cpp 
  ${CORE_PATH}/sensor/SensorModule.cpp  
  ${CORE_PATH}/sensor/InertialFilter.cpp 
  ${CORE_PATH}/kinematics/InverseKinematics.cpp 
  ${CORE_PATH}/kinematics/ForwardKinematics.cpp  
  ${CORE_PATH}/kinematics/KinematicsModule.cpp  
  ${CORE_PATH}/math/Geometry.cpp  
  ${CORE_PATH}/common/PIDController.cpp 
  ${CORE_PATH}/motion/WalkEngineParameters.cpp 
)

# 定义项目源文件列表
set(SRCS
  main.cc 
  behaviors/behavior.cc
  behaviors/naobehavior.cc
  behaviors/checkfall.cc 
  behaviors/kicking.cc 
  behaviors/strategy.cc
  behaviors/pkbehaviors.cc
  behaviors/gazebobehavior.cc
  behaviors/simplesoccer.cc
  servercomm/primitives.cc
  parser/parser.cc 
  math/hctmatrix.cc
  math/vecposition.cc
  math/Geometry.cc
  worldmodel/worldmodel.cc
  bodymodel/bodymodel.cc
  particlefilter/PFLocalization.cc
  particlefilter/Particle.cc
  skills/skill.cc 
  skills/curve3d.cc
  ikfast/ikfast.cpp
  headers/headers.cc
  audio/audio.cc 
  rvdraw/rvdraw.cc 
  ${CORE_FILES}
  kalman/BallKF.cpp
  kalman/PlayerKF.cpp 
  kalman/OrigKalmanFilter.cpp
  optimization/optimizationbehaviors.cc
  stats/recordstatsbehavior.cc
)

# 定义生成的可执行文件的名称
set(BINARY agentspark)

# 添加可执行文件的构建目标，使用之前定义的源文件列表
add_executable(${BINARY}
${SRCS}
)

# 链接可执行文件所需的库
target_link_libraries(${BINARY}
${RCSSNET3D_LIBRARY}
${Boost_SYSTEM_LIBRARY}
dl
${CMAKE_THREAD_LIBS_INIT}
rt 
)

# 以下代码被注释掉了，其作用是在每次构建后自动将链接的共享对象复制到 libs 目录
#add_custom_command(TARGET ${BINARY}
#  POST_BUILD
#  COMMAND ${CMAKE_SOURCE_DIR}/scripts/copy_libs.sh ARGS ${BINARY} libs
#)
# 设置目录属性，将 libs 目录添加到 make clean 要清理的文件列表中
#set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES libs)