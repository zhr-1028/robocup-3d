#include "gazebobehavior.h"
// GazeboBehavior 类的构造函数
// 参数:
// teamName: 队伍名称
// uNum: 球员编号
// namedParams_: 命名参数的映射
// rsg_: RSG 文件路径
GazeboBehavior::
GazeboBehavior( const std::string teamName,
                int uNum,
                const map<string, string>& namedParams_,
                const string& rsg_)
    : NaoBehavior( teamName,
                   uNum,
                   namedParams_,
                   rsg_) {
}
// 该函数用于设置球员的初始位置和朝向
// 参数:
// beamX: 球员初始位置的 x 坐标
// beamY: 球员初始位置的 y 坐标
// beamAngle: 球员初始的朝向角度
void GazeboBehavior::
beam( double& beamX, double& beamY, double& beamAngle ) {
    // 将球员初始位置的 x 坐标设置为球场半场长度减去 0.5
    beamX = -HALF_FIELD_X+.5;
    // 将球员初始位置的 y 坐标设置为 0
    beamY = 0;
    // 将球员初始的朝向角度设置为 0
    beamAngle = 0;
}

// 该函数用于选择球员要执行的技能
// 返回值:
// 选择的技能类型
SkillType GazeboBehavior::
selectSkill() {
    // 向球的位置移动
    return goToTarget(ball);

    // 注释掉的代码：让球员保持站立状态
    //return SKILL_STAND;
}


//这段代码定义了 GazeboBehavior 类，该类继承自 NaoBehavior 类，主要用于在 RoboCup 3D 比赛中控制机器人球员的行为.
