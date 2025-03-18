#ifndef _SIMPLE_SOCCER_H
#define _SIMPLE_SOCCER_H

#include "naobehavior.h"

// 定义机器人在足球比赛中的角色枚举
enum Role {
    GOALIE,       // 守门员
    SUPPORTER,    // 支援者
    BACK_LEFT,    // 左后卫
    BACK_RIGHT,   // 右后卫
    MID_LEFT,     // 左中场
    MID_RIGHT,    // 右中场
    WING_LEFT,    // 左边锋
    WING_RIGHT,   // 右边锋
    FORWARD_LEFT, // 左前锋
    FORWARD_RIGHT,// 右前锋
    ON_BALL,      // 控球者
    NUM_ROLES     // 角色数量
};

// 定义 SimpleSoccerBehavior 类，继承自 NaoBehavior 类
class SimpleSoccerBehavior : public NaoBehavior {
public:
    // 构造函数，用于初始化 SimpleSoccerBehavior 对象
    // 参数包括队伍名称、球员编号、命名参数映射和 RSG 文件路径
    SimpleSoccerBehavior(const std::string teamName, int uNum, const map<string, string>& namedParams_, const string& rsg_);

    // 虚函数，用于确定机器人重新定位（beam）时的位置和角度
    virtual void beam( double& beamX, double& beamY, double& beamAngle );

    // 虚函数，用于选择机器人要执行的技能
    virtual SkillType selectSkill();

protected:
    // 获取离球最近的球员编号
    int getPlayerClosestToBall();

    // 让机器人前往指定空间位置
    SkillType goToSpace(VecPosition space);

    // 让机器人监视指定位置
    SkillType watchPosition(VecPosition pos);

    // 根据角色获取对应的空间位置
    VecPosition getSpaceForRole(Role role);

    // 为每个球员分配角色
    map<int, Role> getAgentRoles();

    // 计算指定球员到指定角色位置的距离
    double getAgentDistanceToRole(int uNum, Role role);
};

#endif