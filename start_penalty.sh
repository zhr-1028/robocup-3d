#!/bin/bash
#
# 在3D仿真比赛中启动点球守门员和点球射手的脚本
#

# 定义球员代理程序的二进制文件名
AGENT_BINARY=agentspark
# 定义二进制文件所在的目录
BINARY_DIR="."
# 定义库文件所在的目录
LIBS_DIR="./libs"
# 将库文件目录添加到动态链接库搜索路径中
export LD_LIBRARY_PATH=$LIBS_DIR:$LD_LIBRARY_PATH

# 定义球队名称
team="Ahjzu"
# 定义服务器主机名，默认为本地主机
host="localhost"
# 定义服务器端口号，默认为3100
port=3100
# 定义默认的参数文件路径（仅需指定defaultParams.txt）
paramsfile=paramfiles/defaultParams.txt
# 定义用于发送绘图命令的监视器的IP地址，默认为本地主机
mhost="localhost"

# 显示脚本使用说明的函数
usage()
{
  (echo "用法: $0 [选项]"
   echo "可用选项:"
   echo "  --help                       显示此帮助信息"
   echo "  -h, --host HOST              指定服务器主机名（默认: localhost)"
   echo "  -p, --port PORT              指定服务器端口号（默认: 3100)"
   echo "  -t, --team TEAMNAME          指定球队名称"
   echo "  -mh, --mhost HOST            指定用于发送绘图命令的监视器IP地址(默认: localhost)"
   echo "  -pf, --paramsfile FILENAME   指定要加载的参数文件名称（默认: paramfiles/defaultParams.txt)") 1>&2
}

# 初始化参数文件的命令行参数
paramsfile_args="--paramsfile ${paramsfile}"

# 循环处理命令行参数
while [ $# -gt 0 ]
do
  case $1 in

    --help)
      usage
      exit 0
      ;;

    -h|--host)
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      host="${2}"
      shift 2
      ;;

    -mh|--mhost)
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      mhost="${2}"
      shift 2
      ;;

    -p|--port)
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      port="${2}"
      shift 2
      ;;

    -t|--team)
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      team="${2}"
      shift 2
      ;;

    -pf|--paramsfile)
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      DIR_PARAMS="$( cd "$( dirname "$2" )" && pwd )"
      PARAMS_FILE=$DIR_PARAMS/$(basename $2)
      paramsfile_args="${paramsfile_args} --paramsfile ${PARAMS_FILE}"
      shift 2
      ;;

    *)
      echo "无效选项 \"${1}\"." 1>&2
      usage
      exit 1
      ;;
  esac
done

# 构建公共命令行选项
common_opt="--host=${host} --port ${port} --team ${team} ${paramsfile_args} --mhost=${mhost}"

# 切换到脚本所在目录（确保paramfiles在此目录下）
cd "$( dirname "$0" )"

# 启动点球守门员（自动加载defaultParams_t4.txt）
"$BINARY_DIR/$AGENT_BINARY" $common_opt --unum 1 --type 4 --pkgoalie &> /dev/null &
echo "已启动守门员(unum=1, type=4)"

# 启动点球射手（自动加载defaultParams_t4.txt）
echo "当前工作目录: $(pwd)"
echo "即将启动射手，使用的命令: $BINARY_DIR/$AGENT_BINARY $common_opt --unum 2 --type 4 --pkshooter"
"$BINARY_DIR/$AGENT_BINARY" $common_opt --unum 2 --type 4 --pkshooter

sleep 2
echo "脚本执行完成：$(date)"#!/bin/bash
#
# 在3D仿真比赛中启动点球守门员和点球射手的脚本
#

# 定义球员代理程序的二进制文件名
AGENT_BINARY=agentspark
# 定义二进制文件所在的目录
BINARY_DIR="."
# 定义库文件所在的目录
LIBS_DIR="./libs"
# 将库文件目录添加到动态链接库搜索路径中
export LD_LIBRARY_PATH=$LIBS_DIR:$LD_LIBRARY_PATH

# 定义球队名称
team="Ahjzu"
# 定义服务器主机名，默认为本地主机
host="localhost"
# 定义服务器端口号，默认为3100
port=3100
# 定义默认的参数文件路径（仅需指定defaultParams.txt）
paramsfile=paramfiles/defaultParams.txt
# 定义用于发送绘图命令的监视器的IP地址，默认为本地主机
mhost="localhost"

# 显示脚本使用说明的函数
usage()
{
  (echo "用法: $0 [选项]"
   echo "可用选项:"
   echo "  --help                       显示此帮助信息"
   echo "  -h, --host HOST              指定服务器主机名（默认: localhost）"
   echo "  -p, --port PORT              指定服务器端口号（默认: 3100）"
   echo "  -t, --team TEAMNAME          指定球队名称"
   echo "  -mh, --mhost HOST            指定用于发送绘图命令的监视器IP地址（默认: localhost）"
   echo "  -pf, --paramsfile FILENAME   指定要加载的参数文件名称（默认: paramfiles/defaultParams.txt）") 1>&2
}

# 初始化参数文件的命令行参数
paramsfile_args="--paramsfile ${paramsfile}"

# 循环处理命令行参数
while [ $# -gt 0 ]
do
  case $1 in

    --help)
      usage
      exit 0
      ;;

    -h|--host)
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      host="${2}"
      shift 2
      ;;

    -mh|--mhost)
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      mhost="${2}"
      shift 2
      ;;

    -p|--port)
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      port="${2}"
      shift 2
      ;;

    -t|--team)
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      team="${2}"
      shift 2
      ;;

    -pf|--paramsfile)
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      DIR_PARAMS="$( cd "$( dirname "$2" )" && pwd )"
      PARAMS_FILE=$DIR_PARAMS/$(basename $2)
      paramsfile_args="${paramsfile_args} --paramsfile ${PARAMS_FILE}"
      shift 2
      ;;

    *)
      echo "无效选项 \"${1}\"." 1>&2
      usage
      exit 1
      ;;
  esac
done

# 构建公共命令行选项
common_opt="--host=${host} --port ${port} --team ${team} ${paramsfile_args} --mhost=${mhost}"

# 切换到脚本所在目录（确保paramfiles在此目录下）
cd "$( dirname "$0" )"

# 启动点球守门员（自动加载defaultParams_t4.txt）
"$BINARY_DIR/$AGENT_BINARY" $common_opt --unum 1 --type 4 --pkgoalie &> /dev/null &
echo "已启动守门员(unum=1, type=4)"

# 启动点球射手（自动加载defaultParams_t4.txt）
"$BINARY_DIR/$AGENT_BINARY" $common_opt --unum 2 --type 4 --pkshooter &> /dev/null &
echo "已启动射手(unum=2, type=5)"

sleep 2
echo "脚本执行完成：$(date)"