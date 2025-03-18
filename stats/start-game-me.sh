#!/bin/bash
#
# 此脚本用于运行SimSpark足球模拟器，让我们的球队与对手进行比赛。
# 调用 start-stats.sh 启动脚本来运行我们的球队，这样代理程序会使用 RecordStatsBehavior 并收集比赛统计信息。
# 统计信息会写入一个输出文件。
#
# 使用方法: start-game-me.sh <统计信息输出文件> <对手球队目录> <left | right>

# 检查命令行参数数量是否为3
if [ "$#" -ne 3 ]; then
  echo "使用方法: $0 <统计信息输出文件> <对手球队目录> <left | right>"
  exit 1
fi

# 获取脚本的绝对路径
SCRIPT=$(readlink -f $0)
# 获取脚本所在的目录
SCRIPT_DIR=`dirname $SCRIPT`

# 统计信息输出文件
OUTPUT_FILE=$1
# 对手球队代理程序所在的目录
OPPONENT_AGENT_DIR=$2
# 我方球队所在的半场（left 或 right）
AGENT_SIDE=$3

# 获取当前主机名
HOST=`hostname`

# 记录脚本开始执行的时间（以秒为单位）
start_time=`date +%s`

# 强制终止所有正在运行的 rcssserver3d 进程
killall -9 rcssserver3d

# 启动足球模拟器服务器
rcssserver3d &

# 获取服务器进程的 PID
PID=$!

# 等待服务器启动，睡眠5秒
sleep 5

# 切换到脚本所在的目录
cd $SCRIPT_DIR                                                                

# 如果我方球队在右侧半场
if [ "$AGENT_SIDE" == "right" ]
then 
    # 切换到对手球队代理程序所在的目录
    cd $OPPONENT_AGENT_DIR
    # 启动对手球队代理程序
    ./start.sh 127.0.0.1
    # 等待1秒
    sleep 1
fi

# 切换回脚本所在的目录
cd $SCRIPT_DIR
# 启动我方球队代理程序，并指定统计信息输出文件
./start-stats.sh 127.0.0.1 $OUTPUT_FILE
# 等待1秒
sleep 1

# 如果我方球队不在右侧半场（即左侧半场）
if [ "$AGENT_SIDE" != "right" ] # 应该是 left
then 
    # 切换到对手球队代理程序所在的目录
    cd $OPPONENT_AGENT_DIR
    # 启动对手球队代理程序
    ./start.sh 127.0.0.1
    # 等待1秒
    sleep 1
fi

# 等待10秒，让球队代理程序有足够时间初始化
sleep 10
# 执行 kickoff.pl 脚本进行开球，开始比赛
perl $SCRIPT_DIR/kickoff.pl 

# 最大等待时间（秒）
max_wait_time_secs=1500

# 循环检查统计信息输出文件是否存在，并且等待时间未超过最大等待时间
while [ ! -f $OUTPUT_FILE ] && [ `expr \`date +%s\` - $start_time` -lt $max_wait_time_secs ]
do 
  # 检查服务器进程是否还在运行
  if ! kill -0 $PID &> /dev/null
  then
      echo "服务器已停止，因此关闭程序。"
      break
  fi 
  # 每隔5秒检查一次
  sleep 5
done 

# 如果统计信息输出文件不存在
if [ ! -f $OUTPUT_FILE ]
then
  echo "等待比赛结束超时，当前等待时间为 `expr \`date +%s\` - $start_time` 秒。"
else
  echo "比赛完成，等待时间为 `expr \`date +%s\` - $start_time` 秒。"
fi

# 等待1秒
sleep 1

# 输出正在终止我方球队代理程序的信息
echo "正在终止我方球队代理程序"
# 切换到脚本所在目录的上一级目录
cd $SCRIPT_DIR/..
# 执行 kill.sh 脚本终止我方球队代理程序
./kill.sh

# 输出正在终止对手球队代理程序的信息
echo "正在终止对手球队代理程序"
# 切换到对手球队代理程序所在的目录
cd $OPPONENT_AGENT_DIR
# 执行 kill.sh 脚本终止对手球队代理程序
./kill.sh

# 输出正在终止模拟器的信息
echo "正在终止模拟器"
# 向服务器进程发送 SIGINT 信号（信号编号为2），终止服务器进程
kill -s 2 $PID

# 等待2秒
sleep 2
# 输出完成信息
echo "完成"