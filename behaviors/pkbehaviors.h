#ifndef _PK_BEHAVIORS_H  // 防止头文件重复包含
#define _PK_BEHAVIORS_H

#include "naobehavior.h"  // 包含基类 NaoBehavior 的头文件

// 射手行为类，继承自 NaoBehavior
class PKShooterBehavior : public NaoBehavior {
public:
    // 构造函数
    PKShooterBehavior(const std::string teamName, int uNum, const map<string, string>& namedParams_, const string& rsg_);

    // 重写基类的 beam 函数，用于设置射手的初始位置和角度
    virtual void beam(double& beamX, double& beamY, double& beamAngle);

    // 重写基类的 selectSkill 函数，用于选择射手的技能
    virtual SkillType selectSkill();
};

// 守门员行为类，继承自 NaoBehavior
class PKGoalieBehavior : public NaoBehavior {
public:
    // 构造函数
    PKGoalieBehavior(const std::string teamName, int uNum, const map<string, string>& namedParams_, const string& rsg_);

    // 重写基类的 beam 函数，用于设置守门员的初始位置和角度
    virtual void beam(double& beamX, double& beamY, double& beamAngle);

    // 重写基类的 selectSkill 函数，用于选择守门员的技能
    virtual SkillType selectSkill();
};

#endif  // 结束头文件定义