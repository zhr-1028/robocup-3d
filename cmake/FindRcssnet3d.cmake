# - 查找 Rcssnet3d
# 查找原生 rcssnet 的头文件和库文件
#
#  RCSSNET3D_INCLUDE_DIR     - 存放 rcssnet 头文件的路径
#  RCSSNET3D_LIBRARIES       - 使用 rcssnet 时所需的库文件列表
#  RCSSNET3D_FOUND           - 如果找到 rcssnet，则为 TRUE

# 设置 SPARK_DIR 变量，首先尝试从环境变量中获取，若未获取到则使用默认路径
SET(SPARK_DIR $ENV{SPARK_DIR} "C:/Program Files/simspark" "C:/Program Files (x86)/simspark" "C:/library/simspark")

# rcssnet3D
IF (RCSSNET3D_INCLUDE_DIR)
  # 若已在缓存中，保持安静模式
  SET(RCSSNET3D_FIND_QUIETLY TRUE)
ENDIF (RCSSNET3D_INCLUDE_DIR)

# 查找 rcssnet 的头文件目录，目标文件为 rcssnet/addr.hpp
# HINTS 表示从环境变量 SPARK_DIR 中获取提示路径
# PATHS 表示要搜索的路径列表
# PATH_SUFFIXES 表示在搜索路径后追加的子路径
FIND_PATH(RCSSNET3D_INCLUDE_DIR rcssnet/addr.hpp
  HINTS ENV SPARK_DIR
  PATHS ${SPARK_DIR}
  PATH_SUFFIXES simspark include/simspark)

# 定义 rcssnet 库文件的可能名称
SET(RCSSNET3D_NAMES rcssnet3D rcssnet3D_debug)
# 查找 rcssnet 的库文件
# NAMES 表示要查找的库文件名称列表
# HINTS 表示从环境变量 SPARK_DIR 中获取提示路径
# PATHS 表示要搜索的路径列表
# PATH_SUFFIXES 表示在搜索路径后追加的子路径
FIND_LIBRARY(RCSSNET3D_LIBRARY NAMES ${RCSSNET3D_NAMES}
  HINTS ENV SPARK_DIR
  PATHS ${SPARK_DIR}
  PATH_SUFFIXES simspark lib/simspark)

# 处理 QUIETLY 和 REQUIRED 参数，并根据指定的变量判断是否找到 rcssnet
# 如果所有指定变量都为 TRUE，则将 RCSSNET3D_FOUND 设置为 TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RCSSNET3D DEFAULT_MSG RCSSNET3D_LIBRARY
         RCSSNET3D_INCLUDE_DIR)

IF(RCSSNET3D_FOUND)
  # 如果找到 rcssnet，设置 RCSSNET3D_LIBRARIES 变量为找到的库文件
  SET( RCSSNET3D_LIBRARIES ${RCSSNET3D_LIBRARY} )
ELSE(RCSSNET3D_FOUND)
  # 如果未找到 rcssnet，清空 RCSSNET3D_LIBRARIES 变量
  SET( RCSSNET3D_LIBRARIES )
ENDIF(RCSSNET3D_FOUND)

# 将 RCSSNET3D_LIBRARY 和 RCSSNET3D_INCLUDE_DIR 标记为高级变量，在普通配置界面中隐藏
MARK_AS_ADVANCED( RCSSNET3D_LIBRARY RCSSNET3D_INCLUDE_DIR )