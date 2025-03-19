#include "naobehavior.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cctype>
#include <exception>

#include "../skills/skillparser.h"
#include "../rvdraw/rvdraw.h"
#include <assert.h>

// 用于 UT 步行
#include <common/InterfaceInfo.h>
#include <motion/MotionModule.h>

extern int agentBodyType;

/*
 * namedParams_ 是参数与其值之间的映射
 */
NaoBehavior::
NaoBehavior(const std::string teamName, int uNum, const map<string, string>& namedParams_, const string& rsg_) :
    namedParams( namedParams_ ),
    rsg( rsg_ )
{

    //cout << "Constructing of Nao Behavior" << endl;

    srand ((unsigned)time(NULL) );
    srand48((unsigned)time(NULL));

    classname = "NaoBehavior"; //TODO: 消除它...

    mInit = false;
    initBeamed = false;

    agentTeamName = teamName;
    agentUNum = uNum;

    scoreMe = 0;
    scoreOpp = 0;

    worldModel = new WorldModel();
    bodyModel = new BodyModel(worldModel);

    memory_ = new Memory(false,true);

    memory_->getOrAddBlockByName(frame_info_,"frame_info");
    memory_->getOrAddBlockByName(vision_frame_info_,"vision_frame_info");
    frame_info_->source = MEMORY_SIM; // 设置为模拟器
    vision_frame_info_->source = MEMORY_SIM;

    memory_->getOrAddBlockByName(raw_sensors_,"raw_sensors");
    memory_->getOrAddBlockByName(raw_joint_angles_,"raw_joint_angles");
    memory_->getOrAddBlockByName(processed_joint_angles_,"processed_joint_angles");
    memory_->getOrAddBlockByName(raw_joint_commands_,"raw_joint_commands");
    memory_->getOrAddBlockByName(processed_joint_commands_,"processed_joint_commands");
    memory_->getOrAddBlockByName(sim_effectors_,"sim_effectors");

    core = new MotionCore(CORE_SIM, true, *memory_);
    fParsedVision = false;
    particleFilter = new PFLocalization( worldModel, bodyModel, core);

    parser = new Parser(worldModel, bodyModel, teamName, particleFilter,
                        vision_frame_info_,
                        frame_info_,
                        raw_joint_angles_,
                        raw_sensors_ );

    initBeamed = false;
    initialized = false;
    beamTime = -1;
    hoverTime = 2.25;

    fallState = 0;
    fallenLeft = false;
    fallenRight = false;
    fallenDown = false;
    fallenUp = false;
    fallTimeStamp = -1;
    fallTimeWait = -1;

    lastGetupRecoveryTime = -1.0;

    monMsg = "";

    // TODO: 更正确地处理路径？（独立于系统的方式）
    try {
        // 从文件中读取技能
        readSkillsFromFile( "./skills/stand.skl" );
        readSkillsFromFile( "./skills/kick.skl" );

        // ik 技能
        readSkillsFromFile( "./skills/kick_ik_0.skl" );
        // 结束 ik 技能

    }
    catch( std::string& what ) {
        cerr << "捕获到异常: " << what << endl;
        exit(1);
    }
    catch (std::exception& e)
    {
        cerr << e.what() << endl;
        exit(1);
    }

    // 初始化，以免重置技能时发生段错误
    skill = SKILL_STAND;
    resetSkills();
    resetKickState();

    // 取消注释以使用地面真值数据进行定位
    //worldModel->setUseGroundTruthDataForLocalization(true);
}

NaoBehavior::~NaoBehavior() {
    // 释放内存
    delete parser;
    delete worldModel;
    delete bodyModel;
    delete particleFilter;
    delete core;
}

// 初始化函数，返回加载场景的消息
string NaoBehavior::Init() {
    cout << "加载 rsg: " << "(scene " << rsg << ")" << endl;
    return "(scene " + rsg + ")";
}

// 思考函数，处理接收到的消息并生成动作消息
string NaoBehavior::Think(const std::string& message) {

    //  cout << "(NaoBehavior) received message " << message << endl;

    fParsedVision = false;
    // 解析消息
    bool parseSuccess = parser->parse(message, fParsedVision);
    if(!parseSuccess && (worldModel->getPlayMode() != PM_BEFORE_KICK_OFF)) {
//    cout << "****************************************\n";
//    cout << "无法解析消息: " << message << "\n";
//    cout << "****************************************\n";
    }

    //  cout << "\nparseSuccess: " << parseSuccess << "\n";
    //  worldModel->display();
    // 刷新身体模型
    bodyModel->refresh();
    if(fParsedVision) {
        if (!worldModel->isFallen()) {
            // 处理视觉信息
            parser->processVision();
        } else {
            // 处理视线信息，忽略视觉
            parser->processSightings(true /*fIgnoreVision*/);
        }
    }
    // 更新适应度
    this->updateFitness();
    //  bodyModel->display();
    //  bodyModel->displayDerived();

    // 示例：使用 roboviz 绘图系统和 RVSender 绘制智能体位置和方向
    /*
    worldModel->getRVSender()->clearStaticDrawings();
    VecPosition pos = worldModel->getMyPosition();
    VecPosition dir = VecPosition(1,0,0);
    dir = dir.rotateAboutZ(-worldModel->getMyAngDeg());
    worldModel->getRVSender()->drawPoint(pos.getX(), pos.getY(), 10);
    worldModel->getRVSender()->drawLine(pos.getX(), pos.getY(), pos.getX()+dir.getX(), pos.getY()+dir.getY());
    */

    // 计算角度
    calculateAngles();

    if (frame_info_->start_time == -1) {
        frame_info_->start_time = frame_info_->seconds_since_start;
        vision_frame_info_->start_time = frame_info_->start_time;
    }
    frame_info_->seconds_since_start= frame_info_->seconds_since_start - frame_info_->start_time;

    raw_joint_angles_->values_[RHipYawPitch] = raw_joint_angles_->values_[LHipYawPitch];

    // 预处理关节角度
    preProcessJoints();  // 对关节角度应用正确的符号

    // 后处理关节角度
    postProcessJoints(); // 翻转关节角度

    string action;

    if (!mInit) {

        mInit = true;
        stringstream ss;
        ss << "(init (unum " << agentUNum << ")(teamname " << agentTeamName << "))";
        action = ss.str();
        return action;
    }

    if (worldModel->getLastPlayMode() != worldModel->getPlayMode() &&
            (worldModel->getPlayMode() == PM_BEFORE_KICK_OFF ||
             worldModel->getPlayMode() == PM_GOAL_LEFT ||
             worldModel->getPlayMode() == PM_GOAL_RIGHT)) {
        initBeamed = false;
    }

    // 记录比赛得分
    if (worldModel->getScoreLeft() != -1 && worldModel->getScoreRight() != -1) {
        scoreMe = worldModel->getSide() == SIDE_LEFT ? worldModel->getScoreLeft() : worldModel->getScoreRight();
        scoreOpp = worldModel->getSide() == SIDE_LEFT ? worldModel->getScoreRight() : worldModel->getScoreLeft();
    }

    if ((worldModel->getPlayMode() == PM_GOAL_LEFT || worldModel->getPlayMode() == PM_GOAL_RIGHT || worldModel->getPlayMode() == PM_BEFORE_KICK_OFF) && worldModel->getLastPlayMode() != worldModel->getPlayMode()) {
        beamTime = worldModel->getTime() + hoverTime;
    }
    else if(beamTime >= 0 && worldModel->getTime() >= beamTime) {
        //initialized = false;
        initBeamed = false;
        beamTime = -1.0;
    }

    if (worldModel->getPlayMode() != worldModel->getLastPlayMode()) {
        worldModel->setLastDifferentPlayMode(worldModel->getLastPlayMode());
    }
    worldModel->setLastPlayMode(worldModel->getPlayMode());

    if(!initialized) {
        if(!worldModel->getUNumSet() || !worldModel->getSideSet()) {
            //      cout << "尚未收到球员编号和队伍侧信息.\n";
            action = "";
            return action;
        }

        if(!initBeamed) {
            initBeamed = true;

            double beamX, beamY, beamAngle;

            // 调用虚函数
            // 它可以在此处实现（真实比赛）
            // 或在继承类中实现
            // 参数在 beam 函数中填充
            this->beam( beamX, beamY, beamAngle );
            stringstream ss;
            ss << "(beam " << beamX << " " << beamY << " " << beamAngle << ")";
            particleFilter->setForBeam(beamX, beamY, beamAngle);
            action = ss.str();
            return action;
        }
        else {
            // 未初始化
            bodyModel->setInitialHead();
            bodyModel->setInitialArm(ARM_LEFT);
            bodyModel->setInitialArm(ARM_RIGHT);
            bodyModel->setInitialLeg(LEG_LEFT);
            bodyModel->setInitialLeg(LEG_RIGHT);
            initialized = true;
        }
    }

    if(!initBeamed) {
        initBeamed = true;

        double beamX, beamY, beamAngle;

        // 调用虚函数
        // 它可以在此处实现（真实比赛）
        // 或在继承类中实现 - 用于优化智能体
        // 参数在 beam 函数中填充
        this->beam( beamX, beamY, beamAngle );
        stringstream ss;
        ss << "(beam " << beamX << " " << beamY << " " << beamAngle << ")";
        particleFilter->setForBeam(beamX, beamY, beamAngle);
        action = ss.str();
    }

    frame_info_->frame_id++;
    // 执行动作
    act();

    worldModel->getRVSender()->refresh();

    action = action + composeAction();

    //std::cout << "Sending action: " << action << "\n";
    return action;
}

// 执行动作函数
void NaoBehavior::act() {
    // 刷新状态
    refresh();

    const double LAST_LINE_SIGHTING_THRESH = 0.1;
    if (worldModel->getTime()-worldModel->getLastLineSightingTime() > LAST_LINE_SIGHTING_THRESH) {
        worldModel->setLocalized(false);
    }

    // 如果球太远，重置踢球状态
    if(me.getDistanceTo(ball) > 1) {
        resetKickState();
    }

    //worldModel->getRVSender()->drawPoint("me", me.getX(), me.getY(), 20);
    int pm = worldModel->getPlayMode();
    bool resetForKickoff = pm == PM_BEFORE_KICK_OFF || pm == PM_GOAL_LEFT || pm == PM_GOAL_RIGHT;

    if(checkingFall()) {
        resetSkills();
        bodyModel->setUseOmniWalk(false);
        return;
    }
    else if(resetForKickoff) {
        if (beamablePlayMode() && (worldModel->isFallen() || worldModel->getTime() <= beamTime)) {
            initBeamed = false;
        }
        resetSkills();
        skill = SKILL_STAND;
        core->move(0,0,0);
        velocity.paramSet = WalkRequestBlock::PARAMS_DEFAULT;
    }
    else {
        if(skills[skill]->done( bodyModel, worldModel) ||
                bodyModel->useOmniWalk()) {
            skills[skill]->reset();
            resetScales();
            // 选择技能
            SkillType currentSkill = selectSkill();

            if (currentSkill != SKILL_WALK_OMNI) {
                velocity.x = 0;
                velocity.y = 0;
                velocity.rot = 0;
                velocity.paramSet = WalkRequestBlock::PARAMS_DEFAULT;
            }

            bodyModel->setUseOmniWalk(true);
            switch(currentSkill) {
            case SKILL_WALK_OMNI:
                core->move(velocity.paramSet, velocity.x, velocity.y, velocity.rot);
                break;
            case SKILL_STAND:
                core->move(0,0,0);
                break;
            default:
                bodyModel->setUseOmniWalk(false);
            }

            if (bodyModel->useOmniWalk()) {
                resetSkills();
            } else {

                /*EnumParser<SkillType> enumParser;
                cout << "Skill: " << enumParser.getStringFromEnum(skill) << endl;*/

                // 状态转换，实现有限状态机...

                SkillType lastSkill = worldModel->getLastSkill();
                skill = currentSkill;
            }
        }
    }
    //  cout << "Executing: " << EnumParser<SkillType>::getStringFromEnum(skill) << endl;
    //  cerr << "Selected skill: " << SkillType2Str[skill] << " time: " << worldModel->getTime() << endl;
    //    LOG_ST(skill);
    // 执行技能
    skills[skill]->execute( bodyModel, worldModel );

    worldModel->setLastSkill(skill);
    // 用于里程计
    if (bodyModel->useOmniWalk()) {
        worldModel->addExecutedSkill(SKILL_WALK_OMNI);
    } else {
        worldModel->addExecutedSkill( skill );
    }

    // 设置头部转动行为
    VecPosition me = worldModel->getMyPosition();
    me.setZ(0);
    ball = worldModel->getBall();
    ball.setZ(0);
    // 当前每 2 秒
    static double panOffset = drand48() * 4.0;
    int panState = ( static_cast<int>( worldModel->getTime()+panOffset ) ) % 4;
    double ballDistance, ballAngle;

    getTargetDistanceAndAngle(ball, ballDistance, ballAngle);
    //SkillType lastSkill = worldModel->getLastSkill();

    if (worldModel->isFallen()) {
        bodyModel->setScale(EFF_H1, 0.5);
        bodyModel->setTargetAngle(EFF_H1, 0);
    } else if (ballDistance < 1.0 && worldModel->getWorldObject(WO_BALL)->validPosition) {
        // 靠近球，专注于球并转动头部 30 度
        if( panState == 0 || panState == 2 ) {
            bodyModel->setScale(EFF_H1, 0.3);
            bodyModel->setTargetAngle(EFF_H1, ballAngle);
        } else {
            int direction = (panState == 1) ? 1 : -1;
            bodyModel->setScale(EFF_H1, 0.3);
            bodyModel->setTargetAngle(EFF_H1, ballAngle+(direction*30.0));
        }
    } else {
        // 默认行为
        if( panState == 0 || panState == 2 ) {
            bodyModel->setScale(EFF_H1, 0.3);
            bodyModel->setTargetAngle(EFF_H1, 0);
        } else {
            int direction = (panState == 1) ? 1 : -1;
            bodyModel->setScale(EFF_H1, 0.3);
            bodyModel->setTargetAngle(EFF_H1, direction*120);// 30.0); // 120.0);
        }
    }
}

/*
 * 抛出字符串异常
 */
// 从文件中读取技能
void NaoBehavior::readSkillsFromFile( const std::string& filename) {
//  cerr << "Loading skills from file " << filename << endl;

    // 将技能文件加载到内存。假设文件小于 4K

    int buffsize = 65536;
    char buff[buffsize];
    int numRead;

    fstream skillFile( filename.c_str(), ios_base::in );
    skillFile.read( buff, buffsize );
    if( !skillFile.eof() ) {
        throw "无法读取整个技能文件 " + filename;
    }
    numRead = skillFile.gcount();

    // 在末尾填充 \0
    buff[numRead] = '\0';

    // 预处理：用值替换参数
    string skillDescription("");
    skillDescription.reserve( buffsize );
    for( int i = 0; i < numRead; ++i ) {
        char c = buff[i];
        if( c == '$' ) {
            // 参数 - 替换它
            string param("");
            i += 1;
            while( i < numRead && ( isalnum( buff[i] ) || buff[i] == '_' ) ) {
                param += buff[i];
                ++i;
            }

            map<string, string>::const_iterator it = namedParams.find( param );
            if( it == namedParams.end() ) {
                throw "技能文件 " + filename + " 中缺少参数: " + param;
            }
            skillDescription += it->second;

            if( i < numRead )
                skillDescription += buff[i];

        } else {
            // 不是参数，直接拼接字符
            skillDescription += c;
        }
    }

    // 解析
    SkillParser parser( skills, bodyModel );
    parse_info<iterator_t> info = parse( skillDescription.c_str(),
                                         parser,
                                         ( space_p | comment_p("#") )
                                       );

    // 检查解析结果
    if (info.hit)
    {
//    cout << "-------------------------\n";
//    cout << "解析成功\n";
//    cout << "-------------------------\n";
//    cout << "stop "  << info.stop << endl;
//    cout << "full " << info.full << endl;
//    cout << "length " << info.length << endl;
    }
    else
    {
        cout << "-------------------------\n";
        cout << "解析失败\n";
        //            cout << "stopped at: \": " << info.stop << "\"\n";
        cout << "-------------------------\n";
//    throw "Parsing failed";
    }
}

// 判断技能是否为右肢技能
bool NaoBehavior::isRightSkill( SkillType skill ) {
    string skillStr = EnumParser<SkillType>::getStringFromEnum( skill );
    return skillStr.find("RIGHT") != string::npos;
}

// 判断技能是否为左肢技能
bool NaoBehavior::isLeftSkill( SkillType skill ) {
    string skillStr = EnumParser<SkillType>::getStringFromEnum( skill );
    return skillStr.find("LEFT") != string::npos;
}

// 修剪值，使其在指定的最小值和最大值之间
double NaoBehavior::
trim(const double& value, const double& min, const double&max)
{
    double ret;
    if (value > max)
        ret = max;
    else if (value < min)
        ret = min;
    else
        ret = value;

    return ret;
}

// 计算角度
void NaoBehavior::calculateAngles() {
    float  accX = raw_sensors_->values_[accelX];
    float  accY = raw_sensors_->values_[accelY];
    float  accZ = raw_sensors_->values_[accelZ];

    raw_sensors_->values_[angleX] = atan2(accY,accZ);
    raw_sensors_->values_[angleY] = -atan2(accX,accZ);

    //raw_sensors_->values_[gyroX] = 0; // = 1000000.0;
    //raw_sensors_->values_[gyroY] = 0; //= 1000000.0;
}

// 预处理关节角度
void NaoBehavior::preProcessJoints() {
    for (int i=0; i<NUM_JOINTS; i++) {
        processed_joint_angles_->values_[i] = spark_joint_signs[i] * raw_joint_angles_->values_[i];
    }
}

// 后处理关节角度
void NaoBehavior::postProcessJoints() {
    raw_joint_commands_->angle_time_ = processed_joint_commands_->angle_time_;
    raw_joint_commands_->stiffness_time_ = processed_joint_commands_->stiffness_time_;
    for (int i=0; i<NUM_JOINTS; i++) {
        raw_joint_commands_->angles_[i] = spark_joint_signs[i] * processed_joint_commands_->angles_[i]; // 应用关节符号转换到机器人的参考系
        raw_joint_commands_->stiffness_[i] = processed_joint_commands_->stiffness_[i];
    }
    raw_joint_commands_->send_stiffness_ = processed_joint_commands_->send_stiffness_;
    processed_joint_commands_->send_stiffness_ = false;
}

// 重置技能
void NaoBehavior::resetSkills() {
    skills[skill]->reset();

    skill = SKILL_STAND;
    skillState = 0;

    kickDirection = VecPosition(1.0, 0, 0);

    resetScales();

    kickType = KICK_FORWARD;

    skills[worldModel->getLastSkill()]->reset();
    worldModel->setLastSkill(skill);

    bodyModel->setUseOmniWalk(true);
}

// 重置缩放比例
void NaoBehavior::resetScales() {
    for (int e = int(EFF_H1); e < int(EFF_NUM); e++) {
        bodyModel->setScale(e, 1.0);
    }
}

// 确定向目标移动时是否会发生碰撞，并在必要时进行相应调整
VecPosition NaoBehavior::collisionAvoidance(bool avoidTeammate, bool avoidOpponent, bool avoidBall, double PROXIMITY_THRESH, double COLLISION_THRESH, VecPosition target, bool fKeepDistance) {
    // 障碍物回避
    VecPosition closestObjPos = VecPosition(100, 100, 0);
    double closestObjDistance = me.getDistanceTo(closestObjPos);

    // 如果设置了标志，则避开球
    if(avoidBall) {
        if (abs(me.getAngleBetweenPoints(target, ball)) < 90.0
                && (fKeepDistance || me.getDistanceTo(ball) <= me.getDistanceTo(target))) {
            closestObjPos = ball;
            closestObjDistance = me.getDistanceTo(ball);
        }
    }

    // 如果设置了标志，则避开所有队友
    if(avoidTeammate) {
        for(int i = WO_TEAMMATE1; i <= WO_TEAMMATE11; ++i) {
            // 跳过自己
            if (worldModel->getUNum() == i - WO_TEAMMATE1 + 1) {
                continue;
            }
            WorldObject* teammate = worldModel->getWorldObject( i );
            if (teammate->validPosition == true) {
                VecPosition temp = teammate->pos;
                temp.setZ(0);
                if (abs(me.getAngleBetweenPoints(target, temp)) < 90.0) {
                    if (!fKeepDistance && me.getDistanceTo(temp) > me.getDistanceTo(target)) {
                        continue;
                    }
                    double distance = me.getDistanceTo(temp);
                    if (distance < closestObjDistance) {
                        closestObjDistance = distance;
                        closestObjPos = temp;
                    }
                }
            }
        }
    }

    // 如果设置了标志，则避开对手
    if(avoidOpponent) {
        if (closestObjDistance > PROXIMITY_THRESH) {
            for(int i = WO_OPPONENT1; i <= WO_OPPONENT11; ++i) {
                WorldObject* opponent = worldModel->getWorldObject( i );
                if (opponent->validPosition == true) {
                    VecPosition temp = opponent->pos;
                    temp.setZ(0);
                    if (abs(me.getAngleBetweenPoints(target, temp)) < 90.0 &&
                            me.getDistanceTo(temp) < me.getDistanceTo(target)) {
                        double distance = me.getDistanceTo(temp);
                        if (distance < closestObjDistance) {
                            closestObjDistance = distance;
                            closestObjPos = temp;
                        }
                    }
                }
            }
        }
    }

    // 确定需要移动到哪里以避开最近的障碍物
    if (closestObjDistance <= PROXIMITY_THRESH) {
        VecPosition originalTarget = target;
        target = collisionAvoidanceCorrection(me, PROXIMITY_THRESH, COLLISION_THRESH, target, closestObjPos);
    }

    return target;
}

// 碰撞回避修正函数
VecPosition NaoBehavior::collisionAvoidanceCorrection(VecPosition start, double PROXIMITY_THRESH, double COLLISION_THRESH, VecPosition target, VecPosition obstacle) {
    double obstacleDist = start.getDistanceTo(obstacle);

    if (abs(start.getAngleBetweenPoints(target, obstacle)) >= 90.0 ||
            obstacleDist > PROXIMITY_THRESH) {
        return target;
    }

    VecPosition obstacleDir = (obstacle-start).normalize();

    VecPosition left90 = start + VecPosition(0, 0, 1).crossProduct(obstacleDir)*1.0;
    VecPosition right90 = start - VecPosition(0, 0, 1).crossProduct(obstacleDir)*1.0;
    if (target.getDistanceTo(left90) > target.getDistanceTo(right90)) {
        target = right90;
    } else {
        target = left90;
    }

    if (obstacleDist <= COLLISION_THRESH) {
        // 太近了，也向后退
        target += (start-obstacle).normalize()*1.0;
    }
    return target;
}

// 接近时的碰撞回避函数
VecPosition NaoBehavior::collisionAvoidanceApproach(double PROXIMITY_THRESH, double COLLISION_THRESH, VecPosition target, VecPosition obstacle) {
    return collisionAvoidanceApproach(me, PROXIMITY_THRESH, COLLISION_THRESH, target, obstacle);
}

// 接近时的碰撞回避函数重载
VecPosition NaoBehavior::collisionAvoidanceApproach(VecPosition start, double PROXIMITY_THRESH, double COLLISION_THRESH, VecPosition target, VecPosition obstacle) {
    double distanceToObstacle = start.getDistanceTo(obstacle);
    if (fabs(start.getAngleBetweenPoints(target, obstacle)) >= 90.0 ||
            distanceToObstacle > start.getDistanceTo(target)) {
        return target;
    }

    if (distanceToObstacle <= PROXIMITY_THRESH) {
        return collisionAvoidanceCorrection(start, PROXIMITY_THRESH, COLLISION_THRESH, target, obstacle);
    }

    VecPosition start2Target = target-start;
    VecPosition start2TargetDir = VecPosition(start2Target).normalize();
    VecPosition start2Obstacle = obstacle-start;
    VecPosition start2ObstacleDir = VecPosition(start2Obstacle).normalize();

    VecPosition closestPathPoint = start+
                                   (start2TargetDir*(start2Obstacle.dotProduct(start2TargetDir)));

    double pathDistanceFromObstacle = (obstacle-closestPathPoint).getMagnitude();
    VecPosition originalTarget = target;
    if (pathDistanceFromObstacle < PROXIMITY_THRESH) {
        target = obstacle + (closestPathPoint-obstacle).normalize()*PROXIMITY_THRESH;
    }
    return target;
}

// 获取步行技能
SkillType NaoBehavior::getWalk(const double& direction, const double& rotation, double speed, bool fAllowOver180Turn)
{
    return getWalk(WalkRequestBlock::PARAMS_DEFAULT, direction, rotation, speed, fAllowOver180Turn);
}

// 获取步行技能重载
SkillType NaoBehavior::getWalk(WalkRequestBlock::ParamSet paramSet, const double& direction, double rotation, double speed, bool fAllowOver180Turn)
{
    double reqDirection, relSpeed;

    if (worldModel->getTime()-lastGetupRecoveryTime < 1.0 && abs(direction) > 90) {
        // 如果刚起身，不要尝试向后走，因为可能不稳定
        speed = 0;
    }

    // 将方向角度转换到 [0, 360) 范围
    reqDirection = fmod(direction, 360.0);
    reqDirection += (reqDirection < 0) ? 360 : 0;
    assert((reqDirection >= 0) && (reqDirection <= 360));

    // 修剪相对速度
    relSpeed = trim(speed, 0, 1);

    double tanReqDirection, tanMaxSpeed;
    double maxSpeedX, maxSpeedY, maxRot;

    // 期望的速度和旋转作为最大速度的百分比
    double relSpeedX, relSpeedY, relRot;

    // 获取最大速度
    maxSpeedX = core->motion_->getMaxXSpeed(); //core->walkEngine.p.speedMax.translation.x;
    maxSpeedY = core->motion_->getMaxYSpeed(); // core->walkEngine.p.speedMax.translation.y;

    relRot = rotation;
    // 没有理由请求超过 180 或小于 -180 的转弯，因为在这种情况下
    // 我们应该向另一个方向转弯
    if (!fAllowOver180Turn) {
        if (relRot > 180) {
            relRot -= 360.0;
        } else if (relRot < -180) {
            relRot += 360.0;
        }
    }

    relRot = rotation / 180;

    // 截断到 (+/-)1.0
    relRot = trim(relRot, -1, 1);

    // 计算正切值。由于浮点误差和步行引擎处理只有一个非零分量的步行请求的特殊方式，
    // 有必要为 90 的倍数的方向请求显式设置值
    if ((reqDirection == 0) || (reqDirection == 180))
        tanReqDirection = 0;
    else if ((reqDirection == 90) || (reqDirection == 270))
        tanReqDirection = INFINITY;
    else
        tanReqDirection = abs(tanDeg(reqDirection));

    tanMaxSpeed = maxSpeedY / maxSpeedX;

    // 确定将导致以适当方向步行的最大相对速度
    if (tanReqDirection < tanMaxSpeed)
    {
        relSpeedX = 1;
        relSpeedY = tanReqDirection / tanMaxSpeed;
    }
    else
    {
        relSpeedX = tanMaxSpeed / tanReqDirection;
        relSpeedY = 1;
    }


    // 确保符号正确。向前为正 X 方向，向左为正 Y 方向
    if (reqDirection > 180)
        relSpeedY *= -1;

    if ((reqDirection > 90) && (reqDirection < 270))
        relSpeedX *= -1;

    // 使用接近球的步行参数集时，突然停止或改变方向可能会导致不稳定
    // 因此需要检查这种情况，并在必要时进行稳定处理
    static WalkRequestBlock::ParamSet lastWalkParamSet = WalkRequestBlock::PARAMS_DEFAULT;
    static bool fLastWalkParamRequestWasApproach = false;
    static double lastWalkParamRequestApproachTime = 999999999;
    bool fStabilize = false;
    if (paramSet == WalkRequestBlock::PARAMS_APPROACH_BALL) {
        if (!fLastWalkParamRequestWasApproach) {
            lastWalkParamRequestApproachTime = worldModel->getTime();
        }
        fLastWalkParamRequestWasApproach = true;

        if (lastWalkParamSet != WalkRequestBlock::PARAMS_APPROACH_BALL && (speed < .5 || abs(direction) > 45)) {
            if (worldModel->getTime() - lastWalkParamRequestApproachTime < .5) {
                paramSet = WalkRequestBlock::PARAMS_DEFAULT;
                fStabilize = true;
                relSpeed = 0;
                relRot = 0;
            }
        }
    } else {
        fLastWalkParamRequestWasApproach = false;
    }

    if (lastWalkParamSet != paramSet) {
        lastWalkParamSet = paramSet;
    }

    // 合理性检查。这些变量的绝对值必须 <= 1。
    // 但是，由于浮点误差，它们可能会略大于 1。
    assert(abs(relSpeedX) < 1.001);
    assert(abs(relSpeedY) < 1.001);
    assert(abs(relRot) < 1.001);

    // 记录期望的速度，并返回全向步行技能
    // 当调用 SKILL_WALK_OMNI 时，NaoBehavior::act() 会使用 velocity 中的速度分量
    // 向全向步行引擎生成请求
    velocity = WalkVelocity(paramSet, relSpeed * relSpeedX, relSpeed * relSpeedY, relRot);

    if (fStabilize) {
        // 进行稳定处理
        return SKILL_STAND;
    }

    return SKILL_WALK_OMNI;
}

// 目前未进行调优。例如，没有减速逻辑...
SkillType NaoBehavior::goToTargetRelative(const VecPosition& targetLoc, const double& targetRot, const double speed, bool fAllowOver180Turn, WalkRequestBlock::ParamSet paramSet)
{
    double walkDirection, walkRotation, walkSpeed;

    walkDirection = targetLoc.getTheta();
    walkRotation = targetRot;
    walkSpeed = speed;

    // 修剪步行速度，使其在 0.1 到 1 之间
    walkSpeed = trim(walkSpeed, 0.1, 1);

    if (targetLoc.getMagnitude() == 0)
        walkSpeed = 0;

    return getWalk(paramSet, walkDirection, walkRotation, walkSpeed, fAllowOver180Turn);
}

// 假设目标位置的 z 坐标为 0。可能需要进一步调优
SkillType NaoBehavior::goToTarget(const VecPosition &target) {
    double distance, angle;
    // 获取到目标的距离和角度
    getTargetDistanceAndAngle(target, distance, angle);

    const double distanceThreshold = 1;
    const double angleThreshold = getLimitingAngleForward() * .9;
    VecPosition relativeTarget = VecPosition(distance, angle, 0, POLAR);

    // 首先转向我们想要步行的角度，因为如果可能的话，我们希望以最大的前进速度行走。
    /*if (abs(angle) > angleThreshold)
    {
    return goToTargetRelative(VecPosition(), angle);
    }*/

    // [patmac] 速度/敏捷性调整
    // 目前只是朝着目标方向全速前进，同时转向目标方向
    SIM::AngDeg turnAngle = angle;

    // 如果我们距离目标在 distanceThreshold 以内，我们直接走向目标
    if (me.getDistanceTo(target) < distanceThreshold) {
        turnAngle = 0;
    }

    // 朝着我们想要的方向行走
    return goToTargetRelative(relativeTarget, turnAngle);
}

// 获取向前行走的极限角度
double NaoBehavior::getLimitingAngleForward() {
    double maxSpeedX = core->motion_->getMaxXSpeed(); // 核心运动模块的最大 X 方向速度
    double maxSpeedY = core->motion_->getMaxYSpeed(); // 核心运动模块的最大 Y 方向速度
    return abs(atan2Deg(maxSpeedY, maxSpeedX));
}

// 刷新智能体的状态信息
void NaoBehavior::refresh() {
    myXDirection = worldModel->l2g(VecPosition(1.0, 0, 0)) - worldModel->l2g(VecPosition(0, 0, 0));
    myXDirection.setZ(0);
    myXDirection.normalize();

    myYDirection = worldModel->l2g(VecPosition(0, 1.0, 0)) - worldModel->l2g(VecPosition(0, 0, 0));
    myYDirection.setZ(0);
    myYDirection.normalize();

    // 异常情况
    myZDirection = worldModel->l2g(VecPosition(0, 0, 1.0)) - worldModel->l2g(VecPosition(0, 0, 0));
    myZDirection.normalize();

    me = worldModel->getMyPosition(); // 之前这里使用 l2g 有一致性问题
    me.setZ(0);

    ball = worldModel->getBall();
    ball.setZ(0);
}

// 假设目标的 z 坐标为 0，获取到目标的距离和角度
void NaoBehavior::getTargetDistanceAndAngle(const VecPosition &target, double &distance, double &angle) {
    VecPosition targetDirection = VecPosition(target) - me;
    targetDirection.setZ(0);

    // 计算距离
    distance = targetDirection.getMagnitude();

    // 计算角度
    targetDirection.normalize();

    angle = VecPosition(0, 0, 0).getAngleBetweenPoints(myXDirection, targetDirection);
    if (isnan(angle)) {
        //cout << "错误的角度!\n";
        angle = 0;
    }
    if(myYDirection.dotProduct(targetDirection) < 0) {
        angle = -angle;
    }
}

// 判断当前比赛模式是否允许重新定位（beam）
bool NaoBehavior::beamablePlayMode() {
    int pm = worldModel->getPlayMode();
    return pm == PM_BEFORE_KICK_OFF || pm == PM_GOAL_LEFT || pm == PM_GOAL_RIGHT;
}

// 判断当前比赛模式是否不适合进行某些操作
bool NaoBehavior::improperPlayMode() {
    return improperPlayMode(worldModel->getPlayMode());
}

/* 比赛模式不允许触球或者比赛结束（不适合触球）的情况 */
bool NaoBehavior::improperPlayMode(int pm) {

    if(pm == PM_BEFORE_KICK_OFF) {
        return true;
    }
    else if(pm == PM_GAME_OVER) {
        return true;
    }
    else if(pm == PM_GOAL_LEFT) {
        return true;
    }
    else if(pm == PM_GOAL_RIGHT) {
        return true;
    }

    if(worldModel->getSide() == SIDE_LEFT) {

        if(pm == PM_KICK_OFF_RIGHT) {
            return true;
        }
        else if(pm == PM_KICK_IN_RIGHT) {
            return true;
        }
        else if(pm == PM_CORNER_KICK_RIGHT) {
            return true;
        }
        else if(pm == PM_GOAL_KICK_RIGHT) {
            return true;
        }
        else if(pm == PM_OFFSIDE_LEFT) {
            return true;
        }
        else if(pm == PM_FREE_KICK_RIGHT) {
            return true;
        }
        else if(pm == PM_DIRECT_FREE_KICK_RIGHT) {
            return true;
        }
        else if(pm == PM_PASS_RIGHT) {
            return true;
        }
    }
    else if(worldModel->getSide() == SIDE_RIGHT) {

        if(pm == PM_KICK_OFF_LEFT) {
            return true;
        }
        else if(pm == PM_KICK_IN_LEFT) {
            return true;
        }
        else if(pm == PM_CORNER_KICK_LEFT) {
            return true;
        }
        else if(pm == PM_GOAL_KICK_LEFT) {
            return true;
        }
        else if(pm == PM_OFFSIDE_RIGHT) {
            return true;
        }
        else if(pm == PM_FREE_KICK_LEFT) {
            return true;
        }
        else if(pm == PM_DIRECT_FREE_KICK_LEFT) {
            return true;
        }
        else if(pm == PM_PASS_LEFT) {
            return true;
        }
    }

    return false;
}

// 判断当前比赛模式是否允许踢球
bool NaoBehavior::kickPlayMode() {
    return kickPlayMode(worldModel->getPlayMode());
}

// 判断指定比赛模式是否允许踢球
bool NaoBehavior::kickPlayMode(int pm, bool eitherTeam) {
    if(!eitherTeam && improperPlayMode(pm)) {
        return false;
    }

    return pm == PM_CORNER_KICK_LEFT || pm == PM_CORNER_KICK_RIGHT || pm == PM_KICK_IN_LEFT || pm == PM_KICK_IN_RIGHT || pm == PM_FREE_KICK_LEFT || pm == PM_FREE_KICK_RIGHT || pm == PM_DIRECT_FREE_KICK_LEFT || pm == PM_DIRECT_FREE_KICK_RIGHT || pm == PM_GOAL_KICK_LEFT || pm == PM_GOAL_KICK_RIGHT || pm == PM_KICK_OFF_LEFT || pm == PM_KICK_OFF_RIGHT;
}

// 判断当前比赛模式是否为间接任意球
bool NaoBehavior::isIndirectKick() {
    return isIndirectKick(worldModel->getPlayMode());
}

// 判断指定比赛模式是否为间接任意球
bool NaoBehavior::isIndirectKick(int pm) {
    if(!kickPlayMode(pm, true)) {
        return false;
    }

    return !(pm == PM_DIRECT_FREE_KICK_LEFT || pm == PM_DIRECT_FREE_KICK_RIGHT || pm == PM_CORNER_KICK_LEFT || pm == PM_CORNER_KICK_RIGHT || pm == PM_GOAL_KICK_LEFT || pm == PM_GOAL_KICK_RIGHT);
}

/* 设置要发送到监控端口的消息 */
void NaoBehavior::setMonMessage(const std::string& msg) {
    monMsg = msg;
}

/* 获取要发送到监控端口的消息，并清空消息 */
string NaoBehavior::getMonMessage() {
    string ret = monMsg;
    monMsg = "";
    return ret;
}


//************************************************************* */
