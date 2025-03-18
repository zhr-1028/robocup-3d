/*这段代码定义了 Behavior 类的接口，是 RoboCup 3D仿真比赛中机器人行为决策的核心模块。
通过继承 Behavior 类并实现其纯虚函数，可以实现复杂的行为逻辑，帮助机器人在比赛中做出智能决策，从而提升团队的整体表现。*/

#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <string>

class Behavior {

public:

    Behavior();  // 构造函数
    virtual ~Behavior();  // 虚析构函数

    /** 
     * 初始化方法，在首次连接到服务器时调用一次
     * 返回初始化结果（字符串）
     */
    virtual std::string Init() = 0;

    /** 
     * 思考方法，每次从服务器接收到消息时调用
     * @param message 从服务器接收到的消息
     * @return 返回要执行的动作（字符串）
     */
    virtual std::string Think(const std::string& message) = 0;

    /** 
     * 获取发送到服务器监控端口的消息
     * @return 返回要发送的消息（字符串）
     */
    virtual std::string getMonMessage() = 0;
};

#endif // BEHAVIOR_H