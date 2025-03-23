#ifndef NAOBEHAVIOR_H
#define NAOBEHAVIOR_H

#include "behavior.h"
#include "../headers/headers.h"
#include "../parser/parser.h"
#include "../worldmodel/worldmodel.h"
#include "../bodymodel/bodymodel.h"
#include "../particlefilter/PFLocalization.h"
#include "../skills/skill.h"

// 用于UT Walk
#include <MotionCore.h>
#include <memory/Memory.h>
#include <memory/FrameInfoBlock.h>
#include <memory/SensorBlock.h>
#include <memory/JointBlock.h>
#include <memory/JointCommandBlock.h>
#include <memory/SimEffectorBlock.h>
#include <memory/WalkRequestBlock.h>

using namespace std;

// TODO: 临时存放。不确定这是否是最好的位置。
struct WalkVelocity
{
    WalkRequestBlock::ParamSet paramSet; // 行走参数集

    double x;   // X方向速度（单位未指定）
    double y;   // Y方向速度
    double rot;   // 绕Z轴的旋转速度

    WalkVelocity() : paramSet(WalkRequestBlock::PARAMS_DEFAULT), x(0), y(0), rot(0) {}

    WalkVelocity(const double& velX, const double& velY, const double& velRot) :
        paramSet(WalkRequestBlock::PARAMS_DEFAULT), x(velX), y(velY), rot(velRot) {}

    WalkVelocity(WalkRequestBlock::ParamSet param, const double& velX, const double& velY, const double& velRot) :
        paramSet(param), x(velX), y(velY), rot(velRot) {}

    friend std::ostream& operator<<(std::ostream &out, const WalkVelocity& v)
    {
        out << "参数集: " << v.paramSet << " 平移: (" << v.x << ", " << v.y << ") | 旋转: " << v.rot;
        return out;
    }
};

class NaoBehavior : public Behavior {
    friend class KickClassifier;
protected:

    double currentFallStateStartTime; // 当前跌倒状态的开始时间

    // TODO: 消除这些并使用更好的解决方案
    string classname;

    map< SkillType, boost::shared_ptr<Skill> > skills; // 技能映射
    const map<string, string>& namedParams; // 命名参数
    string rsg; // RSG文件路径

    std::string agentTeamName; // 球员所属队伍名称
    int agentUNum; // 球员编号

    Parser *parser; // 解析器
    WorldModel *worldModel; // 世界模型
    BodyModel *bodyModel; // 身体模型
    PFLocalization* particleFilter; // 粒子滤波器

    // 用于UT Walk
    MotionCore* core; // 运动核心
    Memory *memory_; // 内存
    FrameInfoBlock* frame_info_; // 帧信息块
    FrameInfoBlock* vision_frame_info_; // 视觉帧信息块
    SensorBlock* raw_sensors_; // 原始传感器数据
    JointBlock* raw_joint_angles_; // 原始关节角度
    JointCommandBlock* raw_joint_commands_; // 原始关节命令
    JointBlock* processed_joint_angles_; // 处理后的关节角度
    JointCommandBlock* processed_joint_commands_; // 处理后的关节命令
    SimEffectorBlock* sim_effectors_; // 模拟效应器
    WalkVelocity velocity; // 行走速度

    // 用于UT Walk
    void calculateAngles(); // 计算角度
    void preProcessJoints(); // 预处理关节
    void postProcessJoints(); // 后处理关节

    double hoverTime; // 悬停时间
    bool mInit; // 初始化标志
    bool initBeamed; // 是否已初始化上场
    double beamTime; // 上场时间

    bool initialized; // 是否已初始化

    SkillType skill; // 当前技能
    int skillState; // 技能状态

    int fallState; // 跌倒状态
    bool fallenLeft, fallenRight, fallenDown, fallenUp; // 跌倒方向
    double fallTimeStamp; // 跌倒时间戳
    double fallTimeWait; // 跌倒等待时间

    VecPosition kickDirection; // 踢球方向
    int kickType; // 踢球类型
    VecPosition kickTarget; // 踢球目标
    double kickVerticalAngle; // 踢球垂直角度

    double lastGetupRecoveryTime; // 上次恢复站立的时间

    SkillType currentKick; // 当前踢球技能
    int currentKickType; // 当前踢球类型

    VecPosition me; // 自身位置
    VecPosition myXDirection, myYDirection, myZDirection; // 自身坐标系的方向

    VecPosition ball; // 球的位置

    // 比分
    int scoreMe; // 己方得分
    int scoreOpp; // 对方得分

    string monMsg; // 监控消息

    bool fParsedVision; // 是否已解析视觉数据
    string composeAction(); // 组合动作

    virtual void resetSkills(); // 重置技能
    void resetScales(); // 重置比例
    void refresh(); // 刷新
    void act(); // 执行动作

    // ----------------------------------------------------
    // ---------  以下函数需要由子类重写...
    virtual SkillType selectSkill(); // 选择技能
    virtual void beam(double& beamX, double& beamY, double& beamAngle); // 设置上场位置
    virtual void updateFitness() {} // 更新适应度
   

    // ----------------------------------------------------

    bool checkingFall(); // 检查是否跌倒

    /**
     * 将值限制在[min, max]范围内。
     *
     * value - 需要限制的值。
     * min   - 最小值。
     * max   - 最大值。
     */
    double trim(const double& value, const double& min, const double& max);

    /**
     * 返回能够最好地近似所需行走方向和旋转的技能。
     * 该函数设计用于selectSkill()，允许更通用的实现，不依赖于当前实现的行走。
     * 该函数提供最快的速度，因此不适用于对齐/微调。
     *
     * direction - 行走方向的角度（相对于机器人当前朝向）。
     * rotation  - 机器人旋转的角度。
     * speed     - 行走速度的百分比（0到1之间，默认1）。该参数不影响旋转速度。
     * fAllowOver180Turn - 是否允许超过180度的旋转（而不是映射到其补角）。
     */
    SkillType getWalk(const double& direction, const double& rotation, double speed = 1.0, bool fAllowOver180Turn=false);
    SkillType getWalk(WalkRequestBlock::ParamSet paramSet, const double& direction, double rotation, double speed, bool fAllowOver180Turn=false);

    /**
     * 返回到达目标位置并面向目标旋转所需的技能。
     * 所有目标都是相对于机器人当前位置和朝向的偏移量。
     * 注意：(globalTarget - me) 不是正确的本地目标，应使用 g2l(globalTarget)。
     */
    SkillType goToTargetRelative(const VecPosition& targetLoc, const double& targetRot, double speed=1, bool fAllowOver180Turn=false, WalkRequestBlock::ParamSet paramSet=WalkRequestBlock::PARAMS_DEFAULT);

    SkillType goToTarget(const VecPosition &target); // 走向目标

    VecPosition collisionAvoidance(bool avoidTeammate, bool avoidOpponent, bool avoidBall, double PROXIMITY_THRESH, double COLLISION_THRESH, VecPosition target, bool fKeepDistance=true); // 碰撞避免
    VecPosition collisionAvoidanceCorrection(VecPosition start, double PROXIMITY_THRESH, double COLLISION_THRESH, VecPosition target, VecPosition obstacle); // 碰撞避免修正
    VecPosition collisionAvoidanceApproach(double PROXIMITY_THRESH, double COLLISION_THRESH, VecPosition target, VecPosition obstacle); // 碰撞避免接近
    VecPosition collisionAvoidanceApproach(VecPosition start, double PROXIMITY_THRESH, double COLLISION_THRESH, VecPosition target, VecPosition obstacle); // 碰撞避免接近（带起点）
    VecPosition navigateAroundBall(VecPosition target, double PROXIMITY_THRESH, double COLLISION_THRESH ); // 绕球导航
    void resetKickState(); // 重置踢球状态

    double computeKickCost(VecPosition target, SkillType kickType); // 计算踢球成本
    SkillType kickBall(const int kickTypeToUse, const VecPosition &target, const double kickVerticalAngle=0.0); // 踢球
    SkillType kickBallAtPresetTarget(); // 向预设目标踢球

    void getTargetDistanceAndAngle(const VecPosition &target, double &distance, double &angle); // 获取目标距离和角度

    SkillType kickBallAtTargetSimplePositioning(const VecPosition &targetToKickAt, SkillType kick_skill, int kickType); // 简单定位踢球

    /**
     * 返回在保持最大前进速度的情况下可以行走的最大方向角度。
     * 换句话说，返回角度theta，使得如果我们以[-theta, theta]范围内的任何方向行走，
     * 我们的前进速度将是行走引擎允许的最大值。
     */
    double getLimitingAngleForward();

    bool beamablePlayMode(); // 是否可以上场
    bool improperPlayMode(); // 是否是不合适的比赛模式
    bool improperPlayMode(int pm); // 是否是不合适的比赛模式（指定模式）
    bool kickPlayMode(); // 是否是踢球模式
    bool kickPlayMode(int pm, bool eitherTeam=false); // 是否是踢球模式（指定模式）
    bool isIndirectKick(); // 是否是间接任意球
    bool isIndirectKick(int pm); // 是否是间接任意球（指定模式）

    void readSkillsFromFile(const std::string& filename); // 从文件读取技能

    bool isRightSkill(SkillType skill); // 是否是右技能
    bool isLeftSkill(SkillType skill); // 是否是左技能

    double getParameter(const std::string& name); // 获取参数
    double getStdNameParameter(const SkillType kick_skill, const std::string& parameter); // 获取标准名称参数
    void getSkillsForKickType(int kickType, SkillType skillsForType[]); // 获取踢球类型对应的技能













    //#################################################################################################################
    SkillType keeper();
    SkillType turnToAngle(double targetAngle);
    SkillType defenderBehavior();

    VecPosition findBestPassTarget() ;

    SkillType midfieldDecisionMaking(int playerNum);
    bool checkBallControl() ;
    bool shouldAttemptShot(int num);
    SkillType executeStrategicPass(int num) ;
    bool shouldPressOpponent(int num);


    //中锋$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
    SkillType strikerBehavior(int playerNum) ;
    bool validatePassPath(const VecPosition& target) ;




    //前锋******************************************************
    SkillType test();
  
   

   
   
   

 
   








    













    
   



public:

    NaoBehavior(const std::string teamName, int uNum, const map<string, string>& namedParams_, const string& rsg_); // 构造函数
    virtual ~NaoBehavior(); // 析构函数

    virtual std::string Init(); // 初始化
    virtual std::string Think(const std::string& message); // 思考

    void setMonMessage(const std::string& msg); // 设置监控消息
    string getMonMessage(); // 获取监控消息

    inline MotionCore* getCore() {
        return core; // 获取运动核心
    }
};

#endif // NAOBEHAVIOR_H