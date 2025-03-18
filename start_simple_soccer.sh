#!/bin/bash
#
# 德克萨斯大学奥斯汀分校（UT Austin）足球队在3D仿真比赛中的简单足球行为启动脚本
#

AGENT_BINARY=agentspark
BINARY_DIR="."
LIBS_DIR="./libs"
NUM_PLAYERS=11

team="UTAustinVilla_Base"
host="localhost"
port=3100
paramsfile=paramfiles/defaultParams.txt
mhost="localhost"

# 导出动态链接库搜索路径，将当前脚本的libs目录添加到LD_LIBRARY_PATH中
export LD_LIBRARY_PATH=$LIBS_DIR:$LD_LIBRARY_PATH

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

fParsedHost=false
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
      DIR_PARAMS="$( cd "$( dirname "$2" )" && pwd )"
      PARAMS_FILE=$DIR_PARAMS/$(basename $2)
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

opt="${opt} --host=${host} --port ${port} --team ${team} ${paramsfile_args} --mhost=${mhost}"

# 获取脚本所在的目录并切换到该目录
DIR="$( cd "$( dirname "$0" )" && pwd )" 
cd $DIR

# 循环启动11个球员代理
for ((i=1;i<=$NUM_PLAYERS;i++)); do
    case $i in
        1|2)
            echo "正在运行球员编号 $i -- 类型 0"
            "$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 0 --paramsfile paramfiles/defaultParams_t0.txt --simplesoccer &#> /dev/null &
            #"$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 0 --paramsfile paramfiles/defaultParams_t0.txt > stdout$i 2> stderr$i &
            ;;
        3|4)
            echo "正在运行球员编号 $i -- 类型 1"
            "$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 1 --paramsfile paramfiles/defaultParams_t1.txt --simplesoccer &#>  /dev/null &
            #"$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 1 --paramsfile paramfiles/defaultParams_t1.txt > stdout$i 2> stderr$i &
            ;;
        5|6)
            echo "正在运行球员编号 $i -- 类型 2"
            "$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 2 --paramsfile paramfiles/defaultParams_t2.txt --simplesoccer &#> /dev/null &
            #"$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 2 --paramsfile paramfiles/defaultParams_t2.txt > stdout$i 2> stderr$i &
            ;;
        7|8)
            echo "正在运行球员编号 $i -- 类型 3"
            "$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 3 --paramsfile paramfiles/defaultParams_t3.txt --simplesoccer  &#> /dev/null &
            #"$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 3 --paramsfile paramfiles/defaultParams_t3.txt > stdout$i 2> stderr$i &
            ;;
        *)
            echo "正在运行球员编号 $i -- 类型 4"
            "$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 4 --paramsfile paramfiles/defaultParams_t4.txt --simplesoccer &#> /dev/null &
            #"$BINARY_DIR/$AGENT_BINARY" $opt --unum $i --type 4 --paramsfile paramfiles/defaultParams_t4.txt > stdout$i 2> stderr$i &
            ;;
    
    esac
    sleep 1
done