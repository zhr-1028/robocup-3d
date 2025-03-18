#!/bin/bash
#
#在3D仿真比赛中启动点球守门员的脚本
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
# 定义默认的参数文件路径
paramsfile=paramfiles/defaultParams.txt
# 定义用于发送绘图命令的监视器的IP地址，默认为本地主机
mhost="localhost"

# 强制终止所有正在运行的球员代理程序（当前被注释掉）
#killall -9 $AGENT_BINARY

# 显示脚本使用说明的函数
usage()
{
  (echo "用法: $0 [选项]"
   echo "可用选项:"
   echo "  --help                       显示此帮助信息"
   echo "  HOST                         指定服务器主机名（默认: localhost）"
   echo "  -p, --port PORT              指定服务器端口号（默认: 3100）"
   echo "  -t, --team TEAMNAME          指定球队名称"
   echo "  -mh, --mhost HOST            指定用于发送绘图命令的监视器IP地址（默认: localhost）"
   echo "  -pf, --paramsfile FILENAME   指定要加载的参数文件名称（默认: paramfiles/defaultParams.txt）") 1>&2
}

# 标记是否已经解析过主机名的标志变量
fParsedHost=false
# 初始化参数文件的命令行参数
paramsfile_args="--paramsfile ${paramsfile}"

# 循环处理命令行参数
while [ $# -gt 0 ]
do
  case $1 in

    --help)
      # 显示帮助信息并退出脚本
      usage
      exit 0
      ;;

    -mh|--mhost)
      # 检查是否提供了监视器主机名
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      mhost="${2}"
      shift 1
      ;;

    -p|--port)
      # 检查是否提供了服务器端口号
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      port="${2}"
      shift 1
      ;;

    -t|--team)
      # 检查是否提供了球队名称
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      team="${2}"
      shift 1
      ;;

    -pf|--paramsfile)
      # 检查是否提供了参数文件名称
      if [ $# -lt 2 ]; then
        usage
        exit 1
      fi
      # 获取参数文件所在目录的绝对路径
      DIR_PARAMS="$( cd "$( dirname "$2" )" && pwd )"
      # 构建参数文件的完整路径
      PARAMS_FILE=$DIR_PARAMS/$(basename $2)
      # 更新参数文件的命令行参数
      paramsfile_args="${paramsfile_args} --paramsfile ${PARAMS_FILE}"
      shift 1
      ;;
    *)
      if $fParsedHost;
      then
        echo 1>&2
        echo "无效选项 \"${1}\"." 1>&2
        echo 1>&2
        usage
        exit 1
      else
        host="${1}"
        fParsedHost=true
      fi
      ;;
  esac

  shift 1
done

# 构建球员代理程序的命令行选项
opt="${opt} --host=${host} --port ${port} --team ${team} ${paramsfile_args} --mhost=${mhost}"

# 获取脚本所在目录的绝对路径
DIR="$( cd "$( dirname "$0" )" && pwd )" 
# 切换到脚本所在的目录
cd $DIR

# 启动一个球员代理程序作为点球守门员，球员编号为1，类型为4
"$BINARY_DIR/$AGENT_BINARY" $opt --unum 1 --type 4 --paramsfile paramfiles/defaultParams_t4.txt --pkgoalie &#> /dev/null &
# 下面一行是另一种启动方式，将标准输出和标准错误输出重定向到文件（当前被注释掉）
#"$BINARY_DIR/$AGENT_BINARY" $opt --unum 1 --type 4 --paramsfile paramfiles/defaultParams_t4.txt --pkgoalie > stdoutgk 2> stderrgk &

# 脚本暂停2秒
sleep 2