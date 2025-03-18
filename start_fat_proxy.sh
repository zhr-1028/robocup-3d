保持原代码不变，将一些注释改为中文，并说明此代码在robocup3d比赛中的作用#!/bin/bash
#
# 德克萨斯大学奥斯汀分校（UT Austin Villa）在3D仿真比赛中启动胖代理（Fat Proxy）的脚本
#

# 定义球员代理程序的二进制文件名
AGENT_BINARY=agentspark
# 定义二进制文件所在的目录
BINARY_DIR="."
# 定义库文件所在的目录
LIBS_DIR="./libs"
# 定义参赛球员的数量
NUM_PLAYERS=11

# 定义球队名称
team="Ahjzu"
# 定义服务器主机名，默认为本地主机
host="localhost"
# 定义胖代理服务器端口号，后续会根据命令行参数修改
port=3100
# 定义默认的参数文件路径
paramsfile=paramfiles/defaultParams.txt
# 定义用于发送绘图命令的监视器的IP地址，默认为本地主机
mhost="localhost"

# 将库文件目录添加到动态链接库搜索路径中
export LD_LIBRARY_PATH=$LIBS_DIR:$LD_LIBRARY_PATH

# 显示脚本使用说明的函数
usage()
{
  (echo "用法: $0 [选项]"
   echo "可用选项:"
   echo "  --help                       显示此帮助信息"
   echo "  HOST                         指定服务器主机名（默认: localhost）"
   echo "  -p, --port PORT              指定胖代理服务器端口号"
   echo "  -t, --team TEAMNAME          指定球队名称"
   echo "  -mh, --mhost HOST            指定用于发送绘图命令的监视器IP地址（默认: localhost）"
   echo "  -pf, --paramsfile FILENAME   指定要加载的参数文件名称（默认: paramfiles/defaultParams.txt）") 1>&2
}

# 标记是否已经解析过主机名的标志变量
fParsedHost=false
# 标记是否已经解析过端口号的标志变量
fParsedPort=false
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
      fParsedPort=true
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

# 如果未指定胖代理服务器端口号，则输出错误信息并退出脚本
if ! $fParsedPort
then
  echo "必须指定胖代理服务器端口号: -p, --port PORT" 1>&2
  exit 1
fi

# 构建球员代理程序的命令行选项，添加 --fatproxy 标志
opt="${opt} --host=${host} --port ${port} --team ${team} ${paramsfile_args} --mhost=${mhost} --fatproxy"

# 获取脚本所在目录的绝对路径
DIR="$( cd "$( dirname "$0" )" && pwd )" 
# 切换到脚本所在的目录
cd $DIR

# 循环启动球员代理程序
for ((i=1;i<=$NUM_PLAYERS;i++)); do
    case $i in
        2|7)
            # 输出正在启动的球员编号及类型信息
            echo "正在运行球员编号 $i -- 类型 0"
            # 启动球员代理程序，指定球员编号、类型和对应的参数文件
            "$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 0 --paramsfile paramfiles/defaultParams_t0.txt &#> /dev/null &
            # 下面一行是另一种启动方式，将标准输出和标准错误输出重定向到文件（当前被注释掉）
            #"$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 0 --paramsfile paramfiles/defaultParams_t0.txt > stdout$i 2> stderr$i &
            ;;
        -1)
            echo "正在运行球员编号 $i -- 类型 1"
            "$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 1 --paramsfile paramfiles/defaultParams_t1.txt &#>  /dev/null &
            #"$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 1 --paramsfile paramfiles/defaultParams_t1.txt > stdout$i 2> stderr$i &
            ;;
        8|9)
            echo "正在运行球员编号 $i -- 类型 2"
            "$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 2 --paramsfile paramfiles/defaultParams_t2.txt &#> /dev/null &
            #"$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 2 --paramsfile paramfiles/defaultParams_t2.txt > stdout$i 2> stderr$i &
            ;;
        -1)
            echo "正在运行球员编号 $i -- 类型 3"
            "$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 3 --paramsfile paramfiles/defaultParams_t3.txt &#> /dev/null &
            #"$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 3 --paramsfile paramfiles/defaultParams_t3.txt > stdout$i 2> stderr$i &
            ;;
        *)
            echo "正在运行球员编号 $i -- 类型 4"
            "$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 4 --paramsfile paramfiles/defaultParams_t4.txt &#> /dev/null &
            #"$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 4 --paramsfile paramfiles/defaultParams_t4.txt > stdout$i 2> stderr$i &
            ;;
    esac
    # 每次启动间隔1秒
    sleep 1
done