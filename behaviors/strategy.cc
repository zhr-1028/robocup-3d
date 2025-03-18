#include "naobehavior.h"
#include "../rvdraw/rvdraw.h"



extern int agentBodyType;

/*
 * 实际比赛中的上场位置设置。
 * 填充参数：x, y, angle
 */
#include <cstdlib>  // 用于随机数生成
#include <ctime>    // 用于初始化随机数种子

#include <cmath>
#include <cstdlib>
#include <ctime>

void NaoBehavior::beam(double& beamX, double& beamY, double& beamAngle) {
    int playerNum = worldModel->getUNum();
    double maxFrontX = -0.5; // 极限靠前位置（距离中线0.5米，确保不越位）
    double midfieldX = -HALF_FIELD_X + 6.0; // 中场靠前
    const double centerCircleRadius = 2.0; // RoboCup 3D 场地中心圆圈半径

    // 初始化随机数种子
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 守门员（球员1）
    if (playerNum == 1) {
        beamX = -HALF_FIELD_X + 0.3; // 紧贴球门
        beamY = 0.0;
        beamAngle = 0.0;
    }
    // 后卫（球员2、3）：压缩防守
    else if (playerNum == 2 || playerNum == 3) {
        beamX = -HALF_FIELD_X + 1.5;
        // 增加一些随机偏移，避免完全对称
        double randomOffset = (static_cast<double>(std::rand()) / RAND_MAX) * 0.5; 
        beamY = (playerNum == 2) ? -1.5 - randomOffset : 1.5 + randomOffset;
        beamAngle = 0.0;
    }
    // 中场（球员4、5、6）：极简中场，全部压上
    else if (playerNum == 4 || playerNum == 5 || playerNum == 6) {
        beamX = midfieldX;
        double baseY = (playerNum == 4) ? -4.0 : (playerNum == 5) ? 0.0 : 4.0;
        // 增加随机偏移
        double randomOffset = (static_cast<double>(std::rand()) / RAND_MAX) * 1.0; 
        beamY = baseY + ((playerNum % 2 == 0) ? randomOffset : -randomOffset);
        beamAngle = (playerNum == 5) ? 0.0 : (playerNum == 4) ? 15.0 : -15.0; // 边中场斜向站位
    }
    // 前锋（球员7 - 11）：极限压上，形成进攻箭头
    else if (playerNum >= 7 && playerNum <= 11) {
        if (playerNum == 9) {
            // 核心前锋：居中靠前（允许略微突前）
            beamX = maxFrontX + 0.2; // 微超中场但仍在合法范围
            beamY = 0.0;
            beamAngle = 0.0;
        } else {
            // 其他前锋：非对称三角站位基础上增加随机偏移
            double baseY = (playerNum == 7) ? -3.5 : 
                           (playerNum == 8) ? -1.5 :
                           (playerNum == 10) ? 1.5 : 
                           3.5;
            double randomOffset = (static_cast<double>(std::rand()) / RAND_MAX) * 1.5; 
            beamY = baseY + ((playerNum % 2 == 0) ? randomOffset : -randomOffset);
            beamX = maxFrontX + ((static_cast<double>(std::rand()) / RAND_MAX) * 0.5); // 让前锋在x轴上也有一定分散
            beamAngle = (playerNum % 2 == 0) ? 10.0 : -10.0; // 斜向对方球门
        }
    }
    // 默认位置（如有更多球员）
    else {
        beamX = -HALF_FIELD_X + playerNum;
        beamY = 0.0;
        beamAngle = 0.0;
    }

    // 检查是否在中心圆圈内，如果在则进行调整
    if (std::sqrt(beamX * beamX + beamY * beamY) < centerCircleRadius) {
        // 计算从当前位置到圆心的向量
        double vectorLength = std::sqrt(beamX * beamX + beamY * beamY);
        double scaleFactor = centerCircleRadius / vectorLength;

        // 将位置调整到圆圈边缘
        beamX *= scaleFactor;
        beamY *= scaleFactor;
    }
}













 SkillType NaoBehavior::selectSkill() {
    return demoKickingCircle();
    
//         const int playerNum = worldModel->getUNum();
//         VecPosition myPos = worldModel->getMyPosition();
//         const int playMode = worldModel->getPlayMode();
       
    
//         // 特殊比赛模式处理（点球等）
//         if (playMode == PM_PENALTY_SETUP || playMode == PM_PENALTY_READY) {
//             return handlePenaltyMode();
//         }
    
//         // 守门员专用逻辑
//         if (playerNum == 1) {
//             return goalieBehavior();
//         }
    
//         // 持球违规检测与处理
//         if (checkBallHoldingViolation()) {
//             return releaseBall();
//         }
    
//         // 根据比赛状态选择策略
//         if (playMode == PM_PLAY_ON) {
//             return activePlayBehavior();
//         }
        
//         return defaultPositioning();

    }
    
    // 守门员行为策略
    // 守门员行为策略（简化版）
// SkillType NaoBehavior::goalieBehavior() {
//     // 基础参数
//     const VecPosition myPos = worldModel->getMyPosition();
//     const VecPosition ballPos = worldModel->getBall();
//     const double catchRadius = 0.18; // 接球半径

//     // 简化的禁区检测（X轴坐标判断）
//     bool inPenaltyArea = myPos.getX() < -HALF_FIELD_X + 1.8; // 1.8米是禁区深度

//     // 基础接球逻辑
//     if (inPenaltyArea) {
//         // 计算球与守门员的距离
//         double ballDistance = myPos.getDistanceTo(ballPos); 
        
//         // 简单接球条件：球在可接范围内且位于禁区
//         if (ballDistance < catchRadius) {
//             return SKILL_CATCH; // 使用基础接球技能
//         }

//         // 移动至球门中心防守位置
//         VecPosition defendPos(-HALF_FIELD_X + 0.5, 0, 0);
//         return goToTarget(defendPos);
//     }

//     // 默认返回站立技能
//     return SKILL_STAND;
// }
    
//     // 主动比赛行为
//     SkillType NaoBehavior::activePlayBehavior() {
//         const int playerNum = worldModel->getUNum();
//         const VecPosition ballPos = worldModel->getBall();
        
//         // 进攻/防守状态判断
//         if (isAttackingPhase()) {
//             // 前锋进攻策略
//             if (playerNum >= 7) {
//                 return offensiveAttackerBehavior();
//             }
//             // 中场过渡策略
//             return midfieldSupportBehavior();
//         } else {
//             // 后卫防守策略
//             if (playerNum <= 3) {
//                 return defensiveBackBehavior();
//             }
//             // 中场回防
//             return midfieldDefenseBehavior();
//         }
//     }
    
//     // 前锋进攻策略
//     SkillType NaoBehavior::offensiveAttackerBehavior() {
//         // 传球模式激活判断
//         if (canActivatePassMode()) {
//             worldModel->sendPassCommand();
//         }
    
//         // 带球突破逻辑
//         if (isBallPossession()) {
//             VecPosition target = calculateBreakthroughTarget();
//             return dribbleToTarget(target);
//         }
    
//         // 无球跑位
//         return dynamicPositioning();
//     }
    
//     // 动态跑位算法
//     SkillType NaoBehavior::dynamicPositioning() {
//         VecPosition optimalPos = calculateOptimalPosition();
        
//         // 禁区人数监测
//         if (countDefendersInPenaltyArea() >= 3) {
//             optimalPos = adjustPositionForDefenseLimit(optimalPos);
//         }
    
//         // 带碰撞避免的导航
//         return navigateWithCollisionAvoidance(optimalPos);
//     }
    
//     // 带碰撞避免的导航
//     SkillType NaoBehavior::navigateWithCollisionAvoidance(VecPosition target) {
//         // 多层避障策略
//         target = collisionAvoidance(true, true, true, 1.5, 0.8, target);
//         target = avoidStaticObstacles(target);
        
//         // 动态路径规划
//         if (needPathReplanning(target)) {
//             target = calculateAlternativePath(target);
//         }
        
//         return goToTarget(target);
//     }
    
//     // 持球违规检测
//     bool NaoBehavior::checkBallHoldingViolation() {
//         const double holdRadius = 0.12;
//         const double maxHoldTime = (playerNum == 1 && inPenaltyArea()) ? 10.0 : 5.0;
        
//         if (me.getDistanceTo(ball) < holdRadius) {
//             if (!ballHoldingTimerStarted) {
//                 ballHoldStartTime = worldModel->getTime();
//                 ballHoldingTimerStarted = true;
//             }
            
//             double holdDuration = worldModel->getTime() - ballHoldStartTime;
//             if (holdDuration > maxHoldTime) {
//                 return true;
//             }
//         } else {
//             ballHoldingTimerStarted = false;
//         }
//         return false;
//     }
    
//     // 自动调整防守位置
//     VecPosition NaoBehavior::calculateOptimalDefensePosition() {
//         // 基于球速和位置的预测算法
//         VecPosition predictedBallPos = predictBallTrajectory(2.0); // 预测未来2秒位置
//         return calculateDefensePosition(predictedBallPos);
//     }
    
//     // 实时禁区人数统计
//     int NaoBehavior::countDefendersInPenaltyArea() {
//         int defenders = 0;
//         for (int i = WO_TEAMMATE1; i <= WO_TEAMMATE11; ++i) {
//             if (worldModel->getWorldObject(i)->pos.getX() < -HALF_FIELD_X + PENALTY_AREA_LENGTH) {
//                 defenders++;
//             }
//         }
//         return defenders;
//     }
    
//     // 激活传球模式条件判断
//     bool NaoBehavior::canActivatePassMode() {
//         const double maxBallSpeed = 0.05;
//         const double maxBallDist = 0.5;
//         const double minOpponentDist = 1.0;
        
//         return worldModel->getBallSpeed().magnitude() < maxBallSpeed &&
//                me.getDistanceTo(ball) < maxBallDist &&
//                nearestOpponentDistance() > minOpponentDist &&
//                worldModel->getTime() - lastPassTime > 3.0;
//     }
    

//     SkillType NaoBehavior::handlePenaltyMode() {
//         const int playMode = worldModel->getPlayMode();
//         const int playerNum = worldModel->getUNum();
//         const VecPosition ballPos = worldModel->getBall();
    
//         // 射手逻辑（假设点球时球员编号为9）
//         if (playerNum == 9) {
//             // 计算最佳射门角度
//             VecPosition target = VecPosition(HALF_FIELD_X, 0, 0); // 直射对方球门中心
//             return kickBall(KICK_IK, target);
//         }
    
//         // 守门员逻辑（编号1）
//         if (playerNum == 1) {
//             // 预测球路并移动
//             VecPosition defendPos = ballPos.getX() > -HALF_FIELD_X + 2.0 
//                                    ? ballPos 
//                                    : VecPosition(-HALF_FIELD_X + 0.5, 0, 0);
//             return goToTarget(defendPos);
//         }
    
//         // 其他球员保持静止
//         return SKILL_STAND;
//     }
//     SkillType NaoBehavior::releaseBall() {
//         // 安全释放策略
//         if (worldModel->isGoalieHoldingBall()) {
//             // 守门员大脚开球
//             VecPosition target = VecPosition(HALF_FIELD_X, 0, 0);
//             return kickBall(KICK_FORWARD, target);
//         } else {
//             // 普通球员轻推释放
//             VecPosition safeDirection = VecPosition(1.0, 0, 0); // 向前方安全区域
//             return kickBall(KICK_DRIBBLE, safeDirection);
//         }
//     }

//     SkillType NaoBehavior::defaultPositioning() {
//         const int playerNum = worldModel->getUNum();
//         const VecPosition myPos = worldModel->getMyPosition();
        
//         // 角色基准位置
//         VecPosition basePos;
        
//         // 根据球员号码分配默认位置
//         switch(playerNum) {
//             case 1:  // 守门员
//                 basePos = VecPosition(-HALF_FIELD_X + 0.5, 0, 0);
//                 break;
                
//             case 2:  // 左后卫
//                 basePos = VecPosition(-HALF_FIELD_X + 2.0, -3.0, 0);
//                 break;
                
//             case 3:  // 右后卫
//                 basePos = VecPosition(-HALF_FIELD_X + 2.0, 3.0, 0);
//                 break;
                
//             case 4:  // 左中场
//                 basePos = VecPosition(-HALF_FIELD_X + 6.0, -4.0, 0);
//                 break;
                
//             case 5:  // 中场核心
//                 basePos = VecPosition(-HALF_FIELD_X + 8.0, 0, 0);
//                 break;
                
//             case 6:  // 右中场
//                 basePos = VecPosition(-HALF_FIELD_X + 6.0, 4.0, 0);
//                 break;
                
//             case 7:  // 左边锋
//                 basePos = VecPosition(-HALF_FIELD_X + 10.0, -5.0, 0);
//                 break;
                
//             case 8:  // 影锋
//                 basePos = VecPosition(-HALF_FIELD_X + 12.0, 0, 0);
//                 break;
                
//             case 9:  // 中锋
//                 basePos = VecPosition(-HALF_FIELD_X + 14.0, 0, 0);
//                 break;
                
//             case 10: // 右前锋
//                 basePos = VecPosition(-HALF_FIELD_X + 10.0, 5.0, 0);
//                 break;
                
//             default: // 其他球员居中
//                 basePos = VecPosition(-HALF_FIELD_X + 8.0, 0, 0);
//         }
    
//         // 动态调整策略
//         if (playerNum >= 7 && playerNum <= 11) { // 前锋线
//             // 如果距离目标位置较近则保持面向球门
//             if (myPos.getDistanceTo(basePos) < 1.0) {
//                 VecPosition goalTarget(HALF_FIELD_X, 0, 0);
//                 return goToTargetRelative(worldModel->g2l(goalTarget), 0);
//             }
//         }
    
//         // 带碰撞避免的导航
//         return navigateWithCollisionAvoidance(basePos);
//     }
    





SkillType NaoBehavior::demoKickingCircle() {
//     // 圆圈的参数
   VecPosition center = VecPosition(-HALF_FIELD_X/2.0, 0, 0); // 圆心位置
    double circleRadius = 5.0; // 圆的半径
    double rotateRate = 2.5; // 旋转速率

//     // 找到离球最近的球员
     int playerClosestToBall = -1;
     double closestDistanceToBall = 10000;
     for(int i = WO_TEAMMATE1; i < WO_TEAMMATE1+NUM_AGENTS; ++i) {
         VecPosition temp;
        int playerNum = i - WO_TEAMMATE1 + 1;
        if (worldModel->getUNum() == playerNum) {
            // 这是自己
            temp = worldModel->getMyPosition();
        } else {
            WorldObject* teammate = worldModel->getWorldObject(i);
            if (teammate->validPosition) {
                temp = teammate->pos;
            } else {
                continue;
            }
        }
        temp.setZ(0);

        double distanceToBall = temp.getDistanceTo(ball);
        if (distanceToBall < closestDistanceToBall) {
            playerClosestToBall = playerNum;
            closestDistanceToBall = distanceToBall;
        }
    }

    if (playerClosestToBall == worldModel->getUNum()) {
        // 让离球最近的球员向圆心踢球
        return kickBall(KICK_FORWARD, center);
    } else {
        // 移动到圆心周围的圆圈位置并面向圆心
        VecPosition localCenter = worldModel->g2l(center);
        SIM::AngDeg localCenterAngle = atan2Deg(localCenter.getY(), localCenter.getX());

        // 我们在圆圈上的目标位置
        // 根据球员编号、旋转速率和时间计算目标位置
        VecPosition target = center + VecPosition(circleRadius,0,0).rotateAboutZ(360.0/(NUM_AGENTS-1)*(worldModel->getUNum()-(worldModel->getUNum() > playerClosestToBall ? 1 : 0)) + worldModel->getTime()*rotateRate);

        // 调整目标位置，避免与队友或球太近
        target = collisionAvoidance(true /*队友*/, false/*对手*/, true/*球*/, 1/*接近阈值*/, .5/*碰撞阈值*/, target, true/*保持距离*/);

        if (me.getDistanceTo(target) < .25 && abs(localCenterAngle) <= 10) {
            // 距离目标位置和朝向足够近，原地站立
            return SKILL_STAND;
        } else if (me.getDistanceTo(target) < .5) {
            // 接近目标位置，开始转向面向圆心
            return goToTargetRelative(worldModel->g2l(target), localCenterAngle);
        } else {
            // 向目标位置移动
            return goToTarget(target);
        }
    }
}










































    // 获取我的位置和角度
    //cout << worldModel->getUNum() << ": " << worldModel->getMyPosition() << ",\t" << worldModel->getMyAngDeg() << "\n";

    // 获取球的位置
    //cout << worldModel->getBall() << "\n";

    // 使用roboviz绘图系统和rvdarw.cc中的RVSender的示例。
    // 球员绘制他们认为球的位置
    // 也可以在naobehavior.cc中查看绘制球员位置和朝向的示例。
    /*
    worldModel->getRVSender()->clear(); // 清除上一周期的绘图
    worldModel->getRVSender()->drawPoint("ball", ball.getX(), ball.getY(), 10.0f, RVSender::MAGENTA);
    */

    // ### 示例行为 ###

    // 向不同方向行走
    //return goToTargetRelative(VecPosition(1,0,0), 0); // 向前
    //return goToTargetRelative(VecPosition(-1,0,0), 0); // 向后
    //return goToTargetRelative(VecPosition(0,1,0), 0); // 向左
    //return goToTargetRelative(VecPosition(0,-1,0), 0); // 向右
    //return goToTargetRelative(VecPosition(1,1,0), 0); // 对角线
    //return goToTargetRelative(VecPosition(0,1,0), 90); // 逆时针旋转
    //return goToTargetRelative(VecPosition(0,-1,0), -90); // 顺时针旋转
    //return goToTargetRelative(VecPosition(1,0,0), 15); // 绕圈

    // 走向球
    //return goToTarget(ball);

    // 原地转向面对球
    /*double distance, angle;
    getTargetDistanceAndAngle(ball, distance, angle);
    if (abs(angle) > 10) {
      return goToTargetRelative(VecPosition(), angle);
    } else {
      return SKILL_STAND;
    }*/

    // 走向球的同时保持面向前方
    //return goToTargetRelative(worldModel->g2l(ball), -worldModel->getMyAngDeg());

    // 带球向对方球门移动
    //return kickBall(KICK_DRIBBLE, VecPosition(HALF_FIELD_X, 0, 0));

    // 向对方球门踢球
    //return kickBall(KICK_FORWARD, VecPosition(HALF_FIELD_X, 0, 0)); // 基本踢球
    //return kickBall(KICK_IK, VecPosition(HALF_FIELD_X, 0, 0)); // IK踢球

    // 原地站立
    //return SKILL_STAND;

    // 示例行为：球员形成一个旋转的圆圈并来回踢球
    //return demoKickingCircle();
    //if(worldModel->getUNum()==9){
    //return kickBall(KICK_FORWARD,VecPosition(15,0));}
  








































