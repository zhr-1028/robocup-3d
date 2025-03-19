#ifndef HEADERS_H
#define HEADERS_H

#include <map>
#include <string>
#include <vector>
#include <ostream>
#include <sstream>

using std::map;
using std::string;
using std::stringstream;

// TODO: 是否有更好的方法将字符串映射到枚举类型？？？
// 该类的作用是：将字符串和枚举类型之间进行映射
template<typename T>
class EnumParser
{
    map<string, T> string2enum;  // 存储字符串到枚举的映射
    map<T, string> enum2String;  // 存储枚举到字符串的映射

    EnumParser();  // 私有构造函数
    EnumParser(const EnumParser& that);  // 禁用复制构造函数

    // 根据字符串值获取枚举类型
    T _getEnumFromString(const string &value) const {
        if( string2enum.find( value ) != string2enum.end() ) {
            return string2enum.find( value )->second;
        } else {
            throw "string->enum mapping doesn't exist for " + value;  // 如果映射不存在，抛出异常
        }
    }

    // 根据枚举值获取字符串
    string _getStringFromEnum(const T &value) const {
        if( enum2String.find( value ) != enum2String.end() ) {
            return enum2String.find( value )->second;
        } else {
            stringstream ss;
            ss << "enum->string mapping doesn't exist. ";
            ss << value;
            throw ss.str();  // 如果映射不存在，抛出异常
        }
    }

public:
    // [nstiurca] 实现单例模式
    static const EnumParser<T> parser;
    
    // 获取枚举值对应的字符串
    static string getStringFromEnum(const T &value) {
        return parser._getStringFromEnum(value);
    }

    // 获取字符串对应的枚举值
    static T getEnumFromString(const string &value) {
        return parser._getEnumFromString(value);
    }
};

// Hinge Joints（铰链关节）定义

#define HJ_H1  0
#define HJ_H2  1

#define HJ_LA1 2
#define HJ_LA2 3
#define HJ_LA3 4
#define HJ_LA4 5

#define HJ_RA1 6
#define HJ_RA2 7
#define HJ_RA3 8
#define HJ_RA4 9

#define HJ_LL1 10
#define HJ_LL2 11
#define HJ_LL3 12
#define HJ_LL4 13
#define HJ_LL5 14
#define HJ_LL6 15
#define HJ_LL7 16

#define HJ_RL1 17
#define HJ_RL2 18
#define HJ_RL3 19
#define HJ_RL4 20
#define HJ_RL5 21
#define HJ_RL6 22
#define HJ_RL7 23

#define HJ_NUM 24  // 总共24个铰链关节

// Effectors（效应器）枚举
enum Effectors {
    EFF_H1,  // Hinge Joint 1
    EFF_H2,  // Hinge Joint 2

    EFF_LA1, EFF_LA2, EFF_LA3, EFF_LA4,  // 左臂效应器
    EFF_RA1, EFF_RA2, EFF_RA3, EFF_RA4,  // 右臂效应器

    EFF_LL1, EFF_LL2, EFF_LL3, EFF_LL4, EFF_LL5, EFF_LL6, EFF_LL7,  // 左腿效应器
    EFF_RL1, EFF_RL2, EFF_RL3, EFF_RL4, EFF_RL5, EFF_RL6, EFF_RL7,  // 右腿效应器

    EFF_NUM  // 总共效应器数量
};

// Flags（标志）定义
#define FLAG_NUM  4  // 标志的数量

// Goal Posts（球门）定义
#define GOALPOST_NUM  4  // 球门的数量

// Team Playing Side（球队的进攻方）定义
#define SIDE_LEFT    0  // 左方进攻
#define SIDE_RIGHT   1  // 右方进攻

#define TEAMMATE_NUM 11  // 队友人数
#define OPPONENT_NUM 11  // 对手人数

// Body Components（身体部件）定义
#define COMP_TORSO     0  // 躯干
#define COMP_NECK      1  // 颈部
#define COMP_HEAD      2  // 头部

#define COMP_LSHOULDER 3  // 左肩
#define COMP_LUPPERARM 4  // 左上臂
#define COMP_LELBOW    5  // 左肘部
#define COMP_LLOWERARM 6  // 左下臂

#define COMP_RSHOULDER 7  // 右肩
#define COMP_RUPPERARM 8  // 右上臂
#define COMP_RELBOW    9  // 右肘部
#define COMP_RLOWERARM 10 // 右下臂

#define COMP_LHIP1     11 // 左臀部1
#define COMP_LHIP2     12 // 左臀部2
#define COMP_LTHIGH    13 // 左大腿
#define COMP_LSHANK    14 // 左小腿
#define COMP_LANKLE    15 // 左脚踝
#define COMP_LFOOT     16 // 左脚

#define COMP_RHIP1     17 // 右臀部1
#define COMP_RHIP2     18 // 右臀部2
#define COMP_RTHIGH    19 // 右大腿
#define COMP_RSHANK    20 // 右小腿
#define COMP_RANKLE    21 // 右脚踝
#define COMP_RFOOT     22 // 右脚

#define COMP_NUM       23  // 总共23个身体部件

// Body Parts（身体部件索引）枚举
enum BodyParts {
    TORSO,        // 躯干
    HEAD,         // 头部
    ARM_LEFT,     // 左臂
    ARM_RIGHT,    // 右臂
    LEG_LEFT,     // 左腿
    LEG_RIGHT,    // 右腿
    FOOT_LEFT,    // 左脚make
    FOOT_RIGHT,   // 右脚
    TOE_LEFT,     // 左脚趾
    TOE_RIGHT     // 右脚趾
};

// 错误的腿部索引异常类
class bad_leg_index : public std::exception {
    std::string what_;
public:
    bad_leg_index(const int &legIndex) throw() {
        stringstream ss;
        ss << "Bad leg index: " << legIndex << ". Expected LEG_LEFT("
           << LEG_LEFT << ") or LEG_RIGHT(" << LEG_RIGHT << ").";
        what_ = ss.str();
    }

    virtual const char* what() const throw() {
        return what_.c_str();
    }

    virtual ~bad_leg_index() throw() {}
};

// 浮动点比较容差
const double EPSILON = 0.0001;  // 小数点比较精度
#define INF 1e16  // 无穷大常量

// HCT 矩阵创建模式
#define HCT_IDENTITY            0
#define HCT_ROTATE_X            1
#define HCT_ROTATE_Y            2
#define HCT_ROTATE_Z            3
#define HCT_GENERALIZED_ROTATE  4
#define HCT_TRANSLATE           5

// Play Modes（比赛模式）定义
#define PM_BEFORE_KICK_OFF 0  // 开球前
#define PM_KICK_OFF_LEFT   1  // 左方开球
#define PM_KICK_OFF_RIGHT  2  // 右方开球
#define PM_PLAY_ON         3  // 比赛进行中
#define PM_KICK_IN_LEFT    4  // 左方界外球
#define PM_KICK_IN_RIGHT   5  // 右方界外球
#define PM_GOAL_LEFT       6  // 左方进球
#define PM_GOAL_RIGHT      7  // 右方进球
#define PM_GAME_OVER       8  // 比赛结束

// Extra added（额外模式）
#define PM_CORNER_KICK_LEFT       9  // 左方角球
#define PM_CORNER_KICK_RIGHT      10 // 右方角球
#define PM_GOAL_KICK_LEFT         11 // 左方门球
#define PM_GOAL_KICK_RIGHT        12 // 右方门球
#define PM_OFFSIDE_LEFT           13 // 左方越位
#define PM_OFFSIDE_RIGHT          14 // 右方越位
#define PM_FREE_KICK_LEFT         15 // 左方任意球
#define PM_FREE_KICK_RIGHT        16 // 右方任意球
#define PM_DIRECT_FREE_KICK_LEFT  17 // 左方直接任意球
#define PM_DIRECT_FREE_KICK_RIGHT 18 // 右方直接任意球
#define PM_PASS_LEFT              19 // 左方传球
#define PM_PASS_RIGHT             20 // 右方传球

// Directions（方向）定义
#define DIR_LEFT  0  // 左
#define DIR_RIGHT 1  // 右
#define DIR_BACK  2  // 后

// 技能类型（SkillType）枚举
enum SkillType {
    SKILL_WALK_OMNI,           // 全向行走技能

    SKILL_STAND,               // 站立技能

    SKILL_KICK_LEFT_LEG,       // 左腿踢球
    SKILL_KICK_RIGHT_LEG,      // 右腿踢球

    SKILL_KICK_IK_0_LEFT_LEG,  // 左腿踢球（IK模型）
    SKILL_KICK_IK_0_RIGHT_LEG, // 右腿踢球（IK模型）

    SKILL_NONE                 // 无技能
};

// 踢球参数
#define KICK_NONE -1  // 无踢球
#define KICK_DRIBBLE 0  // 运球
#define KICK_FORWARD 1  // 向前踢球
#define KICK_IK 2  // IK 踢球

// 队员数量
#define NUM_AGENTS 11

#define GAZEBO_AGENT_TYPE -1  // Gazebo 代理类型

// 输出流重载函数，用于输出 vector 类型
template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& t) {
    out << "[";
    for(typename std::vector<T>::const_iterator it = t.begin(); it != t.end(); ++it) {
        out << (*it) << ", ";
    }
    return out << "]";
}

// 日志宏定义
#define LOG_BASE        clog << __FILE__ << ":" << __FUNCTION__ << "():" << __LINE__ << ":: "
#define LOG_STR(str)    LOG_BASE << str << endl
#define LOG(x)          LOG_BASE << (#x) << ": " << (x) << endl
#define LOG_MSG(msg, x) LOG_BASE << msg << ": " << x << endl
#define LOG_ST(var)     LOG_BASE << #var << ": " \
						<< EnumParser<SkillType>::getStringFromEnum(var) << endl

// 判断是否是踢球技能
bool isKickSkill(SkillType skill);

// 判断是否是 IK 踢球技能
bool isKickIKSkill(SkillType skill);

#endif // HEADERS_H
