#include "pkbehaviors.h"

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
/////// 守门员行为
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

PKGoalieBehavior::
PKGoalieBehavior(const std::string teamName, // 队伍名称
                 int uNum, // 球员编号
                 const map<string, string>& namedParams_, // 命名参数
                 const string& rsg_) // RSG文件路径
    : NaoBehavior(teamName, uNum, namedParams_, rsg_) { // 调用父类构造函数
}

void PKGoalieBehavior::
beam(double& beamX, double& beamY, double& beamAngle) { // 设置守门员的上场位置
    beamX = -HALF_FIELD_X + .5; // 上场位置的X坐标（靠近球门）
    beamY = 0; // 上场位置的Y坐标为0
    beamAngle = 0; // 上场时的角度为0
}

SkillType PKGoalieBehavior::
selectSkill() { // 选择守门员的技能
    return SKILL_STAND; // 默认行为是站立
}

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
/////// 射手行为
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
PKShooterBehavior::
PKShooterBehavior(const std::string teamName, // 队伍名称
                  int uNum, // 球员编号
                  const map<string, string>& namedParams_, // 命名参数
                  const string& rsg_) // RSG文件路径
    : NaoBehavior(teamName, uNum, namedParams_, rsg_) { // 调用父类构造函数
}

void PKShooterBehavior::
beam(double& beamX, double& beamY, double& beamAngle) { // 设置射手的上场位置
    beamX = -0.5; // 上场位置的X坐标（靠近中场）
    beamY = 0; // 上场位置的Y坐标为0
    beamAngle = 0; // 上场时的角度为0
}

SkillType PKShooterBehavior::
selectSkill() { // 选择射手的技能
    return SKILL_STAND; // 默认行为是站立
}