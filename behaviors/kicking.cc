#include "naobehavior.h"
#include <list>
#include <algorithm>
#include "../rvdraw/rvdraw.h"

#include <sys/time.h>

extern int agentBodyType;

extern bool fFatProxy;

// 重要的踢球参数名称
const string& ANGLE = "angle";
const string& CW_ANGLE_THRESH = "cw_angle_thresh";
const string& CCW_ANGLE_THRESH = "ccw_angle_thresh";
const string& OFFSET_X = "xoffset";
const string& OFFSET_Y = "yoffset";
const string& BOUNDING_BOX_RIGHT = "max_displacement_right"; // 对于右腿踢球，左右是翻转的
const string& BOUNDING_BOX_LEFT = "max_displacement_left";
const string& BOUNDING_BOX_TOP = "max_displacement_top";
const string& BOUNDING_BOX_BOTTOM = "max_displacement_bottom";


/*
 * 向目标位置踢球（或带球）的基本方法
 */
SkillType NaoBehavior::kickBall(const int kickTypeToUse, const VecPosition &target, const double verticalAngle) {
    kickTarget = target;
    kickVerticalAngle = verticalAngle;  // 仅在胖代理模式下使用
    kickDirection = (kickTarget - ball).normalize();

    kickType = kickTypeToUse;

    if (me.getDistanceTo(ball) > 1) {
        // 离球较远，所以朝着球偏移后的目标位置行走
        VecPosition approachBallTarget = ball - kickDirection*atof(namedParams.find("drib_target")->second.c_str());
        return goToTarget(approachBallTarget);
    }

    return kickBallAtPresetTarget();
}


/*计算到达合适位置以执行踢球动作的估计成本
  */
double NaoBehavior::computeKickCost(VecPosition target, SkillType kickType) {
    const double TURN_PENALTY = .5;
    double angle = getStdNameParameter(kickType, ANGLE);

    VecPosition x_vec = (target - ball).normalize();
    VecPosition y_vec = x_vec.rotateAboutZ(-90);

    VecPosition offX = x_vec * getStdNameParameter(kickType, OFFSET_X);
    VecPosition offY = y_vec * (isRightSkill(kickType) ? -1 : 1) * getStdNameParameter(kickType, OFFSET_Y);
    VecPosition stand_pos = ball + offX + offY;
    SIM::AngDeg walkRotation = abs(x_vec.getTheta() + angle - worldModel->getMyAngDeg());
    if (walkRotation > 180.0) {
        walkRotation -= 180.0;
    }
    double cost = me.getDistanceTo(stand_pos) + walkRotation / 180.0 * TURN_PENALTY;
    bool ballInPath =  abs(VecPosition(0, 0, 0).getAngleBetweenPoints(stand_pos - ball, me - ball)) > 90;
    if (ballInPath) {
        cost += .5;
    }
    return cost;
}

/**
 * 将智能体定位，以便按照 this->kickTarget 和 this->kickType 参数化的方式踢球
 *
 * 定位后，执行踢球动作
 */
SkillType NaoBehavior::kickBallAtPresetTarget() {
    static SkillType lastKickSelected = SKILL_NONE;
    static double lastKickSelectedTime = -1;
    double angleThreshold;
    double lateralThreshold;
    SkillType tempSkill;
    VecPosition target;

    SIM::Point2D ballVel = worldModel->getWorldObject(WO_BALL)->absVel;

    // 如果我们离球在 1 米以内且不是在带球，我们需要选择一个踢球方式并定位自己
    // 带球操作稍后处理
    if (me.getDistanceTo(ball) <= 1 && KICK_DRIBBLE != kickType) {
        // 确定使用哪种踢球方式（左脚或右脚）
        SkillType kick;
        double ball_dist;
        double ball_angle;
        double target_dist;
        double target_angle;
        getTargetDistanceAndAngle(ball, ball_dist, ball_angle);
        getTargetDistanceAndAngle(kickTarget, target_dist, target_angle);
        SkillType kick_array[2];
        getSkillsForKickType(kickType, kick_array);
        const int NUM_KICKS = sizeof(kick_array)/sizeof(SkillType);
        double lowestKickCost = 99999999;
        SkillType kickSkill = kick_array[0];
        for (int k = 0; k < NUM_KICKS; k++) {
            SkillType kick = kick_array[k];
            double kickCost = computeKickCost(kickTarget, kick);
            if (kickCost < lowestKickCost) {
                lowestKickCost = kickCost;
                kickSkill = kick;
            }
        }

        static double timeLastSetTarget = -100;
        static VecPosition currentTarget = VecPosition(0,0,0);

        // 每五秒或踢球类型改变时才可以改变目标
        // 当改变目标时重置所有内容
        VecPosition targetToKickAt;
        if (worldModel->getTime()-timeLastSetTarget > 5  || currentKickType != kickType || currentKick == SKILL_NONE) {
            timeLastSetTarget = worldModel->getTime();
            currentTarget = kickTarget;
            lastKickSelected = kickSkill;
            lastKickSelectedTime = worldModel->getTime();
            currentKick = kickSkill;
            currentKickType = kickType;

        }


        if (lastKickSelected != kickSkill) {
            lastKickSelected = kickSkill;
            lastKickSelectedTime = worldModel->getTime();
        }

        // 踢球技能滞后要求我们在切换之前保持某个踢球技能一段时间（但不切换踢球类型）
        if (worldModel->getTime() - lastKickSelectedTime > .5 && currentKickType == kickType) {
            currentKick = lastKickSelected;
        }

        return kickBallAtTargetSimplePositioning(currentTarget, currentKick, currentKickType);
    } else {
        // 这里是我们如何带球的方法

        currentKickType = KICK_DRIBBLE;
        angleThreshold = 10;

        VecPosition localBall = worldModel->g2l(ball);
        localBall.setZ(0);
        SIM::AngDeg localBallAngle = atan2Deg(localBall.getY(), localBall.getX());
        VecPosition me2Ball = ball - me;

        SIM::AngDeg angleError = abs(VecPosition(0, 0, 0).getAngleBetweenPoints(kickDirection, me2Ball));
        SIM::AngDeg turnAngle = localBallAngle;

        if (angleError < angleThreshold) {
            // 带球
            return goToTargetRelative(localBall, turnAngle);
        }

        VecPosition ballTarget = ball;

        target = ballTarget - (VecPosition(kickDirection)  * atof(namedParams.find("drib_target")->second.c_str()));
        target.setZ(0);

        VecPosition originalTarget = target;
        target = navigateAroundBall(target, .5 /*PROXIMITY_TRESH*/,atof(namedParams.find("drib_coll_thresh")->second.c_str()));
        target = collisionAvoidance(true /*Avoid teamate*/, false /*Avoid opponent*/, false /*Avoid ball*/, .5, .5, target,
                                    false /*fKeepDistance*/);
        target = collisionAvoidance(false /*Avoid teamate*/, false /*Avoid opponent*/, true /*Avoid ball*/, .5 /*PROXIMITY_TRESH*/,
                                    atof(namedParams.find("drib_coll_thresh")->second.c_str()), target);
        VecPosition localTarget = worldModel->g2l(target);
        SIM::AngDeg localTargetAngle = atan2Deg(localTarget.getY(), localTarget.getX());
        //cout << "CIRCLE\t" << worldModel->getGameTime() << "\n";
        /*
            if (me.getDistanceTo(ball) < me.getDistanceTo(originalTarget)) {
                // 如果需要，朝着本地目标而不是球行走
                return goToTargetRelative(localTarget, localTargetAngle, 1.0, false, WalkRequestBlock::PARAMS_POSITIONING);
            }
        */
        return goToTargetRelative(localTarget, turnAngle, 1.0, false /*fAllowOver180Turn*/, WalkRequestBlock::PARAMS_POSITIONING);
    }
}




void NaoBehavior::resetKickState() {
    currentKick = SKILL_NONE;
    currentKickType = KICK_NONE;
    kickTarget = VecPosition(0, 0, 0);
    kickVerticalAngle = 0.0;
}



SkillType NaoBehavior::kickBallAtTargetSimplePositioning(const VecPosition &targetToKickAt, SkillType kick_skill, int kickType) {

    if (kick_skill == SKILL_NONE || kickType == KICK_NONE) {
        // 安全检查，确保我们没有错误的值
        cerr << "Bad kick skill/type: kick_skill = " <<  EnumParser<SkillType>::getStringFromEnum( kick_skill ) << "\tkick type = " << kickType << "\n";
        kick_skill = SKILL_KICK_LEFT_LEG;
        kickType = KICK_FORWARD;
    }

    VecPosition x_vec = (targetToKickAt - ball).normalize();
    VecPosition y_vec = x_vec.rotateAboutZ( -90 );

    VecPosition offX = x_vec * getStdNameParameter(kick_skill, OFFSET_X);
    VecPosition offY =   y_vec * (isRightSkill(kick_skill) ? -1 : 1) * getStdNameParameter(kick_skill, OFFSET_Y);
    VecPosition stand_pos = ball + offX + offY;

    SIM::AngDeg walkRotation = x_vec.getTheta() + (isRightSkill(kick_skill) ? -1 : 1) * getStdNameParameter(kick_skill, ANGLE) - worldModel->getMyAngDeg();


    if (walkRotation > 180) {
        walkRotation -= 360;
    } else if (walkRotation < -180) {
        walkRotation += 360;
    }

    VecPosition spToMe = (me - stand_pos);
    VecPosition spToBallX = (ball - stand_pos).normalize();
    VecPosition spToBallY = spToBallX;
    spToBallY.setX(-spToBallX.getY());
    spToBallY.setY(spToBallX.getX());
    spToBallY.normalize(); // 如果球在地面上，应该已经归一化了

    VecPosition yOffset = spToMe.project(spToBallY); // 左边是 0 度
    VecPosition xOffset = spToMe.project(spToBallX); // 前方是 0 度
    bool inPosY = true;
    if (!isRightSkill(kick_skill)) {
        if (getStdNameParameter(kick_skill, OFFSET_Y) > 0) { // 站立位置在球的右边
            inPosY = yOffset.getMagnitude() < getStdNameParameter(kick_skill, (yOffset.getTheta() == spToBallY.getTheta() ? BOUNDING_BOX_LEFT : BOUNDING_BOX_RIGHT));
        } else {
            inPosY = yOffset.getMagnitude() < getStdNameParameter(kick_skill, (yOffset.getTheta() == spToBallY.getTheta() ? BOUNDING_BOX_RIGHT : BOUNDING_BOX_LEFT));
        }
    } else { // 对于右脚，所有东西都是翻转的
        if (getStdNameParameter(kick_skill, OFFSET_Y) > 0) { // 站立位置在球的右边
            inPosY = yOffset.getMagnitude() < getStdNameParameter(kick_skill, (yOffset.getTheta() == spToBallY.getTheta() ? BOUNDING_BOX_RIGHT : BOUNDING_BOX_LEFT));
        } else {
            inPosY = yOffset.getMagnitude() < getStdNameParameter(kick_skill, (yOffset.getTheta() == spToBallY.getTheta() ? BOUNDING_BOX_LEFT : BOUNDING_BOX_RIGHT));
        }
    }
    // 假设 xOffset 总是负的（在球的后面）
    bool inPosX = xOffset.getMagnitude() < getStdNameParameter(kick_skill, (xOffset.getTheta() == spToBallX.getTheta() ? BOUNDING_BOX_TOP : BOUNDING_BOX_BOTTOM));
    bool inPosAngle = true;
    if (isRightSkill(kick_skill)) { // 对于右脚踢球是翻转的
        inPosAngle = abs(walkRotation) < getStdNameParameter(kick_skill, walkRotation < 0 ? CW_ANGLE_THRESH : CCW_ANGLE_THRESH);
    } else {
        inPosAngle = abs(walkRotation) < getStdNameParameter(kick_skill, walkRotation < 0 ? CCW_ANGLE_THRESH : CW_ANGLE_THRESH);
    }
    bool inPosition = inPosY && inPosX && inPosAngle;
    if (fFatProxy) {
        inPosition = me.getDistanceTo(ball) <= 0.25;
    }

    bool canExecute = true;

    SIM::Point2D ballVel = worldModel->getWorldObject(WO_BALL)->absVel;
    const double BALL_VEL_THRESH = .1;

    // 不要尝试踢一个移动的球
    if (ballVel.getMagnitude() > BALL_VEL_THRESH && !fFatProxy) {
        canExecute = false;
    }

    // 如果可以踢球，确保稳定后再踢
    bool fBallCurrentlySeen = worldModel->getWorldObject(WO_BALL)->currentlySeen;
    if(fBallCurrentlySeen && inPosition && canExecute) {
        skills[kick_skill]->reset();
        return kick_skill;
    }

    VecPosition targetLoc = worldModel->g2l(stand_pos);
    double walkSpeed = 1;

    if (ball.getAngleBetweenPoints(stand_pos, me) >= atof(namedParams.find("kick_gen_approach_navBallAngle")->second.c_str())) { // 如果我们不在球的后面，考虑绕过它导航
        double navBallDist = atof(namedParams.find("kick_gen_approach_navBallDist")->second.c_str());
        targetLoc = worldModel->g2l(navigateAroundBall(stand_pos, navBallDist, atof(namedParams.find("kick_gen_approach_navBallCollision")->second.c_str())));
    }

    SIM::AngDeg walkDirection = targetLoc.getTheta();
    targetLoc.setZ(0);

    if (me.getDistanceTo(stand_pos) > atof(namedParams.find("kick_gen_approach_turnDist")->second.c_str())) { // 朝着站立位置（便于走向它），直到足够近。然后面向正确的角度。
        VecPosition localStandPos = worldModel->g2l(stand_pos);
        localStandPos.setZ(0);
        SIM::AngDeg localSPAngle = atan2Deg(localStandPos.getY(), localStandPos.getX());
        walkRotation = localSPAngle;
        if (ball.getAngleBetweenPoints(stand_pos, me) > 135) {
            walkRotation = atan2Deg(targetLoc.getY(), targetLoc.getX());
        }
    }

    const double MPS_SCALE = 43.674733875; // 将粒子滤波器的速度估计乘以这个值得到 m/s
    /*const*/
    double MAX_DECEL_X = atof(namedParams.find("kick_gen_approach_maxDecelX")->second.c_str()); // m/s^2
    const double MAX_DECEL_Y = atof(namedParams.find("kick_gen_approach_maxDecelY")->second.c_str()); //m/s^2 // TODO: The command getWalk(90, 0, 1) doesn't actually cause the robot to walk directly to the side...
    const double MAX_VELOCITY_Y = 0.3;//core->motion_->getMaxYSpeed(); // m/s
    const double MAX_VELOCITY_X = 0.8;//core->motion_->getMaxXSpeed(); // m/s
    /*const*/
    double BUFFER_DIST = atof(namedParams.find("kick_gen_approach_buff")->second.c_str()); // 尝试在离球这么远的地方停下来

    VecPosition estimatedVelocity = particleFilter->getOdometryDisplacementEstimateXY() * MPS_SCALE + atof(namedParams.find("kick_gen_approach_estVelCorrection")->second.c_str());

    double theta = targetLoc.getTheta();
    if (targetLoc.getX() >= 0 && targetLoc.getY() >= 0) {
        // 什么也不做
    } else if (targetLoc.getX() >= 0 && targetLoc.getY() < 0) {
        theta = 360 - theta; // 我们希望 sin 和 cos 最终为正
    } else if (targetLoc.getX() < 0 && targetLoc.getY() >= 0) {
        theta = 180 - theta;
    } else { // targetLoc.getX() < 0 && targetLoc.getY() < 0
        theta = theta - 180;
    }
    theta = Deg2Rad(theta);


    // 计算到目标位置的 X 轴距离
    double distToTargetX = abs(targetLoc.getX());
    // 如果到目标的 X 轴距离大于 2 倍的缓冲距离，则减去缓冲距离
    if (distToTargetX > 2 * BUFFER_DIST) distToTargetX -= BUFFER_DIST;
    // 计算到目标位置的 Y 轴距离
    double distToTargetY = abs(targetLoc.getY());
    // 如果到目标的 Y 轴距离大于 2 倍的缓冲距离，则减去缓冲距离
    if (distToTargetY > 2 * BUFFER_DIST) distToTargetY -= BUFFER_DIST;
    // 根据匀变速直线运动公式 v^2 = 2ax 计算 X 轴期望速度
    double velX = sqrt(2 * MAX_DECEL_X * distToTargetX);
    // 根据匀变速直线运动公式 v^2 = 2ay 计算 Y 轴期望速度
    double velY = sqrt(2 * MAX_DECEL_Y * distToTargetY);
    // 计算期望的总速度
    double desiredVel = hypot(velX, velY);
    // 如果目标位置不在原点
    if (targetLoc.getX() != 0 && targetLoc.getY() != 0) {
        // 如果期望总速度在 X 轴的分量大于计算的 X 轴速度
        if (desiredVel * cos(theta) > velX) {
            // 重新计算 Y 轴速度
            velY = velX * tan(theta);
        } 
        // 如果期望总速度在 Y 轴的分量大于计算的 Y 轴速度
        else if (desiredVel * sin(theta) > velY) {
            // 重新计算 X 轴速度
            velX = velY / tan(theta);
        }
    }
    // 重新计算期望的总速度
    desiredVel = hypot(velX, velY);
    // 根据期望速度和最大速度计算步行速度
    walkSpeed = desiredVel > estimatedVelocity.getMagnitude() ? desiredVel / hypot(MAX_VELOCITY_X, MAX_VELOCITY_Y) : 0; 
    // TODO: 确实需要知道步行引擎当前试图达到的步行速度，以便有足够的超调量来达到期望速度
    // 返回步行请求，参数包括接近球的参数、步行方向、旋转角度和步行速度
    return getWalk(WalkRequestBlock::PARAMS_APPROACH_BALL, walkDirection, walkRotation, walkSpeed);
    }

    // 根据参数名称从命名参数映射中获取对应的参数值，并转换为 double 类型返回
    double NaoBehavior::getParameter(const string& name) {
        return atof(namedParams.find(name)->second.c_str());
    }

    // 根据踢球技能类型和参数名称获取对应的标准化参数值
    double NaoBehavior::getStdNameParameter(const SkillType kick_skill, const string& parameter) {
        string prefix = "";
        // 根据不同的踢球技能类型设置参数前缀
        switch(kick_skill) {
        case SKILL_KICK_IK_0_LEFT_LEG:
        case SKILL_KICK_IK_0_RIGHT_LEG:
            prefix = "kick_ik_0_";
            break;
        case SKILL_KICK_LEFT_LEG:
        case SKILL_KICK_RIGHT_LEG:
            prefix = "kick_";
            break;
        default:
            // 若为不支持的踢球技能类型，输出错误信息
            cerr << "Tried to get a parameter for unsupported kick: " + EnumParser<SkillType>::getStringFromEnum(kick_skill) << endl;
            break;
        }
        // 调用 getParameter 函数获取带前缀的参数值
        return getParameter(prefix + parameter);
    }

    // 根据踢球类型获取对应的踢球技能
    void NaoBehavior::getSkillsForKickType(int kickType, SkillType skills[]) {
        // 根据不同的踢球类型设置对应的技能
        switch(kickType) {
        case KICK_IK:
            skills[0] = SKILL_KICK_IK_0_LEFT_LEG;
            skills[1] = SKILL_KICK_IK_0_RIGHT_LEG;
            break;
        case KICK_FORWARD:
        default:
            skills[0] = SKILL_KICK_LEFT_LEG;
            skills[1] = SKILL_KICK_RIGHT_LEG;
            break;
        }
    }

    // 确定在向球移动时是否会发生碰撞，并在必要时进行相应调整
    VecPosition NaoBehavior::navigateAroundBall(VecPosition target, double PROXIMITY_THRESH, double COLLISION_THRESHOLD) {
        // 计算智能体到球的距离
        double distanceToBall = me.getDistanceTo(ball);

        // 如果智能体、目标和球形成的角度大于等于 90 度，或者到球的距离大于到目标的距离
        if (fabs(me.getAngleBetweenPoints(target, ball)) >= 90.0 ||
                distanceToBall > me.getDistanceTo(target)) {
            // 从当前位置移动到目标时不应撞到球
            target = collisionAvoidance(true/*避免队友*/, false/*不避免对手*/, false/*不避免球*/, 0.5, 0.5, target, false /*不保持距离*/);
            // 避免球的碰撞，返回调整后的目标位置
            return collisionAvoidance(false/*不避免队友*/, false/*不避免对手*/, true/*避免球*/, PROXIMITY_THRESH, COLLISION_THRESHOLD, target);
        }

        // 确定需要移动到哪里以避免撞到球
        VecPosition me2Target = target - me;
        VecPosition me2TargetDir = VecPosition(me2Target).normalize();
        VecPosition me2Ball = ball - me;
        VecPosition me2BallDir = VecPosition(me2Ball).normalize();

        // 如果到球的距离大于接近阈值
        if (distanceToBall > PROXIMITY_THRESH) {
            // 进行接近球时的碰撞避免操作
            target = collisionAvoidanceApproach(PROXIMITY_THRESH, COLLISION_THRESHOLD, target, ball);

            // 计算从智能体到目标路径上离球最近的点
            VecPosition closestPathPoint = me + (me2TargetDir * (me2Ball.dotProduct(me2TargetDir)));

            // 计算路径上离球最近的点到球的距离
            double pathDistanceFromBall = (ball - closestPathPoint).getMagnitude();

            VecPosition originalTarget = target;
            // 如果路径上离球最近的点到球的距离小于接近阈值
            if (pathDistanceFromBall < PROXIMITY_THRESH) {
                // 计算需要调整的角度
                double distanceToBall = me2Ball.getMagnitude();
                SIM::AngDeg correctionAngle = fabs(Rad2Deg(asin(min(PROXIMITY_THRESH / distanceToBall, 0.999))));
                // 计算向左调整后的位置
                VecPosition correctLeft = ball + me2TargetDir.rotateAboutZ(correctionAngle) * PROXIMITY_THRESH;
                // 计算向右调整后的位置
                VecPosition correctRight = ball + me2TargetDir.rotateAboutZ(-correctionAngle) * PROXIMITY_THRESH;

                // 选择离原目标较近的调整位置
                if (correctLeft.getDistanceTo(target) < correctRight.getDistanceTo(target)) {
                    target = correctLeft;
                } else {
                    target = correctRight;
                }
            }
        } else { 
            // 到球的距离小于等于接近阈值时的处理逻辑，此处为空
        }

        // 避免队友的碰撞
        target = collisionAvoidance(true/*避免队友*/, false/*不避免对手*/, false/*不避免球*/, 0.5, 0.5, target, false /*不保持距离*/);
        // 避免球的碰撞
        target = collisionAvoidance(false/*不避免队友*/, false/*不避免对手*/, true/*避免球*/, PROXIMITY_THRESH, atof(namedParams.find("drib_coll_thresh")->second.c_str()), target);
        // 返回最终调整后的目标位置
        return target;
    }