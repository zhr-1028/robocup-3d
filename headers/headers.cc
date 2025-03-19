#include "headers.h"

// 此文件包含枚举类型与字符串之间的映射，主要供解析器在从文件读取数据时使用

// 为 BodyParts 枚举类型特化 EnumParser 类的构造函数
template<>
EnumParser<BodyParts>::EnumParser()
{
    // 将字符串 "TORSO" 映射到枚举值 TORSO
    string2enum["TORSO"] = TORSO;
    // 将枚举值 TORSO 映射到字符串 "TORSO"
    enum2String[TORSO] = "TORSO";

    string2enum["HEAD"] = HEAD;
    enum2String[HEAD] = "HEAD";

    string2enum["ARM_LEFT"] = ARM_LEFT;
    enum2String[ARM_LEFT] = "ARM_LEFT";

    string2enum["ARM_RIGHT"] = ARM_RIGHT;
    enum2String[ARM_RIGHT] = "ARM_RIGHT";

    string2enum["LEG_LEFT"] = LEG_LEFT;
    enum2String[LEG_LEFT] = "LEG_LEFT";

    string2enum["LEG_RIGHT"] = LEG_RIGHT;
    enum2String[LEG_RIGHT] = "LEG_RIGHT";

    string2enum["FOOT_LEFT"] = FOOT_LEFT;
    enum2String[FOOT_LEFT] = "FOOT_LEFT";

    string2enum["FOOT_RIGHT"] = FOOT_RIGHT;
    enum2String[FOOT_RIGHT] = "FOOT_RIGHT";

    string2enum["TOE_LEFT"] = TOE_LEFT;
    enum2String[TOE_LEFT] = "TOE_LEFT";

    string2enum["TOE_RIGHT"] = TOE_RIGHT;
    enum2String[TOE_RIGHT] = "TOE_RIGHT";
}
// 显式实例化 EnumParser<BodyParts> 类模板
template class EnumParser<BodyParts>;
// 定义一个静态常量解析器实例
template<>
const EnumParser<BodyParts> EnumParser<BodyParts>::parser = EnumParser<BodyParts>();

//EnumParser<BodyParts> a = EnumParser<BodyParts>::parser;

// 为 SkillType 枚举类型特化 EnumParser 类的构造函数
template<>
EnumParser<SkillType>::EnumParser()
{
    string2enum["SKILL_WALK_OMNI"] = SKILL_WALK_OMNI;
    enum2String[SKILL_WALK_OMNI] = "SKILL_WALK_OMNI";

    string2enum["SKILL_STAND"] = SKILL_STAND;
    enum2String[SKILL_STAND] = "SKILL_STAND";

    string2enum["SKILL_KICK_LEFT_LEG"] = SKILL_KICK_LEFT_LEG;
    enum2String[SKILL_KICK_LEFT_LEG] = "SKILL_KICK_LEFT_LEG";

    string2enum["SKILL_KICK_RIGHT_LEG"] = SKILL_KICK_RIGHT_LEG;
    enum2String[SKILL_KICK_RIGHT_LEG] = "SKILL_KICK_RIGHT_LEG";

    // 逆运动学踢球技能
    string2enum["SKILL_KICK_IK_0_LEFT_LEG"] = SKILL_KICK_IK_0_LEFT_LEG;
    enum2String[SKILL_KICK_IK_0_LEFT_LEG] = "SKILL_KICK_IK_0_LEFT_LEG";

    string2enum["SKILL_KICK_IK_0_RIGHT_LEG"] = SKILL_KICK_IK_0_RIGHT_LEG;
    enum2String[SKILL_KICK_IK_0_RIGHT_LEG] = "SKILL_KICK_IK_0_RIGHT_LEG";

    // 逆运动学踢球技能结束
    string2enum["SKILL_NONE"] = SKILL_NONE;
    enum2String[SKILL_NONE] = "SKILL_NONE";

}
// 显式实例化 EnumParser<SkillType> 类模板
template class EnumParser<SkillType>;
// 定义一个静态常量解析器实例
template<>
const EnumParser<SkillType> EnumParser<SkillType>::parser = EnumParser<SkillType>();

// 为 Effectors 枚举类型特化 EnumParser 类的构造函数
template<>
EnumParser<Effectors>::EnumParser()
{
    string2enum["EFF_H1"] = EFF_H1;
    enum2String[EFF_H1] = "EFF_H1";

    string2enum["EFF_H2"] = EFF_H2;
    enum2String[EFF_H2] = "EFF_H2";

    string2enum["EFF_LA1"] = EFF_LA1;
    enum2String[EFF_LA1] = "EFF_LA1";

    string2enum["EFF_LA2"] = EFF_LA2;
    enum2String[EFF_LA2] = "EFF_LA2";

    string2enum["EFF_LA3"] = EFF_LA3;
    enum2String[EFF_LA3] = "EFF_LA3";

    string2enum["EFF_LA4"] = EFF_LA4;
    enum2String[EFF_LA4] = "EFF_LA4";

    string2enum["EFF_RA1"] = EFF_RA1;
    enum2String[EFF_RA1] = "EFF_RA1";

    string2enum["EFF_RA2"] = EFF_RA2;
    enum2String[EFF_RA2] = "EFF_RA2";

    string2enum["EFF_RA3"] = EFF_RA3;
    enum2String[EFF_RA3] = "EFF_RA3";

    string2enum["EFF_RA4"] = EFF_RA4;
    enum2String[EFF_RA4] = "EFF_RA4";

    string2enum["EFF_LL1"] = EFF_LL1;
    enum2String[EFF_LL1] = "EFF_LL1";

    string2enum["EFF_LL2"] = EFF_LL2;
    enum2String[EFF_LL2] = "EFF_LL2";

    string2enum["EFF_LL3"] = EFF_LL3;
    enum2String[EFF_LL3] = "EFF_LL3";

    string2enum["EFF_LL4"] = EFF_LL4;
    enum2String[EFF_LL4] = "EFF_LL4";

    string2enum["EFF_LL5"] = EFF_LL5;
    enum2String[EFF_LL5] = "EFF_LL5";

    string2enum["EFF_LL6"] = EFF_LL6;
    enum2String[EFF_LL6] = "EFF_LL6";

    string2enum["EFF_LL7"] = EFF_LL7;
    enum2String[EFF_LL7] = "EFF_LL7";

    string2enum["EFF_RL1"] = EFF_RL1;
    enum2String[EFF_RL1] = "EFF_RL1";

    string2enum["EFF_RL2"] = EFF_RL2;
    enum2String[EFF_RL2] = "EFF_RL2";

    string2enum["EFF_RL3"] = EFF_RL3;
    enum2String[EFF_RL3] = "EFF_RL3";

    string2enum["EFF_RL4"] = EFF_RL4;
    enum2String[EFF_RL4] = "EFF_RL4";

    string2enum["EFF_RL5"] = EFF_RL5;
    enum2String[EFF_RL5] = "EFF_RL5";

    string2enum["EFF_RL6"] = EFF_RL6;
    enum2String[EFF_RL6] = "EFF_RL6";

    string2enum["EFF_RL7"] = EFF_RL7;
    enum2String[EFF_RL7] = "EFF_RL7";

}
// 显式实例化 EnumParser<Effectors> 类模板
template class EnumParser<Effectors>;
// 定义一个静态常量解析器实例
template<>
const EnumParser<Effectors> EnumParser<Effectors>::parser = EnumParser<Effectors>();

// 判断给定的技能类型是否为踢球技能
bool isKickSkill(SkillType skill)
{
    // 将技能类型的枚举值转换为字符串
    string skillStr = EnumParser<SkillType>::getStringFromEnum( skill );
    // 检查字符串中是否包含 "KICK"
    return skillStr.find("KICK") != string::npos;
}

// 判断给定的技能类型是否为逆运动学踢球技能
bool isKickIKSkill(SkillType skill)
{
    // 将技能类型的枚举值转换为字符串
    string skillStr = EnumParser<SkillType>::getStringFromEnum( skill );
    // 检查字符串中是否包含 "KICK_IK"
    return skillStr.find("KICK_IK") != string::npos;
}