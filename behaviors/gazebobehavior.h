// 防止头文件被重复包含的预处理器指令
#ifndef _GAZEBO_BEHAVIOR_H
#define _GAZEBO_BEHAVIOR_H

// 包含 Nao 机器人行为的头文件
#include "naobehavior.h"

// 定义 GazeboBehavior 类，它继承自 NaoBehavior 类
class GazeboBehavior : public NaoBehavior {
public:
    // 构造函数，接收队伍名称、球员编号、命名参数映射和 RSG 文件路径作为参数
    GazeboBehavior(const std::string teamName, int uNum, const map<string, string>& namedParams_, const string& rsg_);

    // 虚函数，用于设置球员的初始位置和朝向
    // 参数：
    // beamX：球员初始位置的 x 坐标
    // beamY：球员初始位置的 y 坐标
    // beamAngle：球员初始的朝向角度
    virtual void beam( double& beamX, double& beamY, double& beamAngle );

    // 虚函数，用于选择球员要执行的技能
    // 返回值：
    // 选择的技能类型
    virtual SkillType selectSkill();
};

// 结束防止头文件重复包含的预处理器指令
#endif