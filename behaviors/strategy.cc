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
    double midfieldX = -HALF_FIELD_X + 7.0; // 中场靠前，增加到距离底线7米，更靠前
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
        double baseY = (playerNum == 4) ? -6.0 : (playerNum == 5) ? 0.0 : 6.0; // 扩大y轴分布
        // 增加随机偏移
        double randomOffset = (static_cast<double>(std::rand()) / RAND_MAX) * 2.0; 
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
            double baseY = (playerNum == 7) ? -6.0 : 
                           (playerNum == 8) ? -3.0 :
                           (playerNum == 10) ? 3.0 : 
                           6.0; // 扩大y轴分布
            double randomOffset = (static_cast<double>(std::rand()) / RAND_MAX) * 3.0; 
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
    int playerNum = worldModel->getUNum();

    // 守门员（球员1）
    if (playerNum == 1) {
        return keeper();
    }
    // 后卫（球员2、3）
    else if (playerNum == 2 || playerNum == 3) {
        return defenderBehavior();
    }
    // 中场（球员4、5、6）
    else if (playerNum >=4 && playerNum <=6) {  // 修正条件判断
        return strikerBehavior(playerNum);       // 传递playerNum参数
    }
    // 其他球员（前锋
    else if (playerNum == 7) {
    return test();
    }
    else if (playerNum == 8) {
        return test();
        }
        else if (playerNum == 9) {
            return test();
            }
            else if (playerNum == 10) {
                return test();
                }
                else if (playerNum == 11) {
                    return test();
                    }
    else {
        return SKILL_STAND;
    }
}




  


    





//守门员%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
SkillType NaoBehavior::keeper() {
    VecPosition ball = worldModel->getBall();
    VecPosition myPos = worldModel->getMyPosition();
    double ballDist = myPos.getDistanceTo(ball);
    double currentAngle = worldModel->getMyAngDeg(); // 当前身体朝向
    
    // 计算球相对于自身朝向的角度差（规范化到[-180,180]）
    double angleToBall = (ball - myPos).getTheta();
    double angleDiff = angleToBall - currentAngle;
    while(angleDiff > 180) angleDiff -= 360;
    while(angleDiff < -180) angleDiff += 360;

    // 核心逻辑
    if(ballDist < 0.7 && fabs(angleDiff) < 30) {
        return kickBall(KICK_FORWARD, VecPosition(HALF_FIELD_X, 0));
    } 
    else if(ballDist < 2.0) {
        if(fabs(angleDiff) > 20) {
            // 方案一：使用goToTargetRelative
            return goToTargetRelative(VecPosition(0,0), angleToBall);
            
           
           
        }
        return goToTarget(ball);
    }
    else {
        VecPosition homePos(-HALF_FIELD_X+0.5, 0);
        return goToTarget(homePos);
    }
}








//后卫**********************************************************************************************************
SkillType NaoBehavior::defenderBehavior() {
    const int playerNum = worldModel->getUNum();
    VecPosition ball = worldModel->getBall();
    VecPosition myPos = worldModel->getMyPosition();
    const VecPosition homeGoal(-HALF_FIELD_X, 0);

    // 防御参数配置
    const double DANGER_ZONE_RADIUS = 3.5;
    const double CLEAR_DISTANCE = 1.2;
    const double BASE_DEFEND_X = -HALF_FIELD_X + 4.0;
    const double SIDE_OFFSET = 3.0;

    // 动态防御位置（自动左右分配）
    VecPosition defendPos(
        BASE_DEFEND_X + (ball.getX() - BASE_DEFEND_X) * 0.3,
        (playerNum % 2 == 0) ? -SIDE_OFFSET : SIDE_OFFSET
    );

    // 态势感知
    const bool ballInDanger = ball.getDistanceTo(homeGoal) < DANGER_ZONE_RADIUS;
    const bool canClear = myPos.getDistanceTo(ball) < CLEAR_DISTANCE;

    // 危险区域处理
    if (ballInDanger) {
        if (canClear) {
            // 侧翼解围方向选择
            VecPosition clearTarget = (defendPos.getY() < 0) 
                ? VecPosition(HALF_FIELD_X, HALF_FIELD_Y/2)
                : VecPosition(HALF_FIELD_X, -HALF_FIELD_Y/2);

            // 角度校准
            double targetAngle = (clearTarget - myPos).getTheta();
            double currentAngle = worldModel->getMyAngDeg();
            double angleDiff = VecPosition::normalizeAngle(targetAngle - currentAngle);

            if (fabs(angleDiff) < 30.0) {
                return kickBall(KICK_FORWARD, clearTarget);
            }
            return goToTargetRelative(VecPosition(), angleDiff);
        }
        
        // 使用球位置预测（基础线性预测）
        VecPosition ballPrediction = ball + (ball - myPos).normalize() * 0.7;
        return goToTarget(ballPrediction);
    }

    // 中场协防模式
    if (ball.getX() < -HALF_FIELD_X/3) {
        // 动态防御位置生成
        VecPosition defendDir = (ball - defendPos).normalize();
        VecPosition targetPos = defendPos + defendDir * 1.5;
        
        // 添加编号扰动防止重叠
        targetPos.setY(targetPos.getY() + (playerNum % 3 - 1) * 0.4);
        
        return goToTarget(targetPos);
    }

    // 组织进攻阶段
    VecPosition bestPassTarget = homeGoal;  // 默认目标
    double maxPassScore = -INFINITY;
    
    // 遍历有效队友（使用框架原生方法）
    for (int i = WO_TEAMMATE1; i <= WO_TEAMMATE11; ++i) {
        WorldObject* teammate = worldModel->getWorldObject(i);
        if (!teammate || i == WO_TEAMMATE1 + playerNum - 1) continue;

        // 使用框架原生位置有效性判断
        if (teammate->validPosition && 
            teammate->pos.getX() > -HALF_FIELD_X + 5.0) {
            // 传球评分：位置优势 + 安全系数
            double forwardScore = teammate->pos.getX() * 1.2;
            double safetyScore = 1.0 / (teammate->pos.getDistanceTo(ball) + 0.1);
            double totalScore = forwardScore + safetyScore;
            
            if (totalScore > maxPassScore) {
                maxPassScore = totalScore;
                bestPassTarget = teammate->pos;
            }
        }
    }

    // 执行传球
    if (maxPassScore > 0) {
        VecPosition leadPassTarget = bestPassTarget + 
                                   (bestPassTarget - myPos).normalize() * 0.5;
        
        if (myPos.getDistanceTo(leadPassTarget) < 5.0) {
            double passAngle = (leadPassTarget - myPos).getTheta();
            double angleDiff = VecPosition::normalizeAngle(passAngle - worldModel->getMyAngDeg());
            
            if (fabs(angleDiff) < 25.0) {
                return kickBall(KICK_IK, leadPassTarget);
            }
            return goToTargetRelative(VecPosition(), angleDiff);
        }
    }

    // 默认回防行为
    return goToTarget(defendPos);
}











//中锋#####################################################################################################3

SkillType NaoBehavior::strikerBehavior(int playerNum) {
    VecPosition ball = worldModel->getBall();
    VecPosition myPos = worldModel->getMyPosition();
    VecPosition oppGoal(HALF_FIELD_X, 0);
    static const double SHOOT_DIST = 8.0;
    static const double PRESS_DIST = 2.5;

    // 位置有效性检查函数
    auto isValidPosition = [](const VecPosition& pos) {
        return (pos.getX() > -HALF_FIELD_X*1.1 && pos.getX() < HALF_FIELD_X*1.1 &&
                pos.getY() > -HALF_FIELD_Y*1.1 && pos.getY() < HALF_FIELD_Y*1.1);
    };

    if (playerNum == 5) { // 核心中场
        if (myPos.getDistanceTo(ball) < 1.2) {
            VecPosition bestTarget = findBestPassTarget();
            if (isValidPosition(bestTarget)) {
                double passAngle = (bestTarget - myPos).getTheta();
                double angleDiff = VecPosition::normalizeAngle(passAngle - worldModel->getMyAngDeg());
                
                if (fabs(angleDiff) < 25.0) {
                    return kickBall(KICK_IK, bestTarget);
                }
                return goToTargetRelative(VecPosition(), angleDiff);
            }

            // 带球实现：推进时保持控球
            if (myPos.getX() > -HALF_FIELD_X/2) {
                VecPosition dribbleTarget = myPos + (oppGoal - myPos).normalize() * 1.0;
                double kickPower = min(0.3, myPos.getDistanceTo(oppGoal)/10.0);
                return kickBall(KICK_DRIBBLE, dribbleTarget, kickPower);
            }
        }

        VecPosition midPos(-HALF_FIELD_X/3, 0);
        return goToTarget(midPos);
    } 
    else { // 边中场（4/6号）
        double sideSign = (playerNum == 4) ? -1.0 : 1.0;
        VecPosition attackBase(-HALF_FIELD_X + 8.0, sideSign * 3.0);

        // 防守回撤
        if (ball.getX() < -HALF_FIELD_X/2) {
            VecPosition defendPos(-HALF_FIELD_X + 5.0, sideSign * 4.0);
            return goToTarget(defendPos);
        }

        // 进攻处理
        if (myPos.getDistanceTo(ball) < PRESS_DIST) {
            if (myPos.getX() > 0 && myPos.getDistanceTo(oppGoal) < SHOOT_DIST) {
                return kickBall(KICK_FORWARD, oppGoal);
            }
            
            // 带球推进
            VecPosition advanceDir = (oppGoal - myPos).normalize();
            VecPosition dribbleTarget = myPos + advanceDir * 1.5;
            return kickBall(KICK_DRIBBLE, dribbleTarget, 0.4);
        }

        // 动态跑位：根据球的位置调整
        VecPosition runPos = attackBase;
        if (ball.getX() > -HALF_FIELD_X/3) {
            runPos += VecPosition(2.0, sideSign * 1.5);
        }
        return goToTarget(runPos);
    }
}

VecPosition NaoBehavior::findBestPassTarget() {
    VecPosition bestTarget(999, 999); // 无效位置标记
    double maxScore = -INFINITY;
    
    // 新增：获取当前球员位置
    VecPosition myPos = worldModel->getMyPosition(); // <-- 修复未定义标识符问题
    
    for (int i = WO_TEAMMATE1; i <= WO_TEAMMATE11; ++i) {
        WorldObject* teammate = worldModel->getWorldObject(i);
        if (!teammate || teammate->pos == myPos)  // 使用本地myPos
            continue;
        
        VecPosition tPos = teammate->pos;
        double forwardScore = tPos.getX() * 1.5;
        
        // 修正后的角度评分计算
        double angleToTeammate = (tPos - myPos).getTheta();
        double currentAngle = worldModel->getMyAngDeg();
        double angleDiff = fabs(VecPosition::normalizeAngle(angleToTeammate - currentAngle));
        double angleScore = 1.0 / (angleDiff + 0.1); // 避免除以零
        
        double distScore = 1.0/(tPos.getDistanceTo(myPos) + 0.5);
        
        double total = forwardScore + angleScore*0.8 + distScore*0.5;
        
        if (total > maxScore && validatePassPath(tPos)) {
            maxScore = total;
            bestTarget = tPos;
        }
    }
    return bestTarget;
}

bool NaoBehavior::validatePassPath(const VecPosition& target) {
    VecPosition currentPos = worldModel->getMyPosition();
    VecPosition dir = (target - currentPos).normalize();
    
    for (double t = 0.5; t <= 5.0; t += 0.8) {
        VecPosition checkPos = currentPos + dir * t;
        
        // 边界检查（使用标准绝对值函数）
        if (fabs(checkPos.getX()) > HALF_FIELD_X || 
            fabs(checkPos.getY()) > HALF_FIELD_Y) {
            return false;
        }

        // 对手检查
        for (int i = WO_OPPONENT1; i <= WO_OPPONENT11; ++i) {
            WorldObject* opp = worldModel->getWorldObject(i);
            if (opp && opp->pos.getDistanceTo(checkPos) < 1.8) {
                return false;
            }
        }
    }
    return true;
}




//前锋策略&&&%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

SkillType NaoBehavior::test() {
    // Parameters for circle
    VecPosition center = VecPosition(HALF_FIELD_X/2.0+2, 0, 0);
    double circleRadius = 5.0;
    double rotateRate = 2.5;

    // Find closest player to ball
    int playerClosestToBall = -1;
    double closestDistanceToBall = 10000;
    for(int i = WO_TEAMMATE1; i < WO_TEAMMATE1+NUM_AGENTS; ++i) {
        VecPosition temp;
        int playerNum = i - WO_TEAMMATE1 + 1;
        if (worldModel->getUNum() == playerNum) {
            // This is us
            temp = worldModel->getMyPosition();
        } else {
            WorldObject* teammate = worldModel->getWorldObject( i );
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
        // Have closest player kick the ball toward the center
        return kickBall(KICK_FORWARD, VecPosition(15,0,0));
    } else {
        // Move to circle position around center and face the center
        VecPosition localCenter = worldModel->g2l(center);
        SIM::AngDeg localCenterAngle = atan2Deg(localCenter.getY(), localCenter.getX());

        // Our desired target position on the circle
        // Compute target based on uniform number, rotate rate, and time
        VecPosition target = center + VecPosition(circleRadius,0,0).rotateAboutZ(360.0/(NUM_AGENTS-1)*(worldModel->getUNum()-(worldModel->getUNum() > playerClosestToBall ? 1 : 0)) + worldModel->getTime()*rotateRate);

        // Adjust target to not be too close to teammates or the ball
        target = collisionAvoidance(true /*teammate*/, false/*opponent*/, true/*ball*/, 1/*proximity thresh*/, .5/*collision thresh*/, target, true/*keepDistance*/);

        if (me.getDistanceTo(target) < .25 && abs(localCenterAngle) <= 10) {
            // Close enough to desired position and orientation so just stand
            return SKILL_STAND;
        } else if (me.getDistanceTo(target) < .5) {
            // Close to desired position so start turning to face center
            return goToTargetRelative(worldModel->g2l(target), localCenterAngle);
        } else {
            // Move toward target location
            return goToTarget(target);
        }
    }
}







// SkillType NaoBehavior::demoKickingCircle() {
//      // 圆圈的参数
//    VecPosition center = VecPosition(-HALF_FIELD_X/2.0, 0, 0); // 圆心位置
//     double circleRadius = 5.0; // 圆的半径
//     double rotateRate = 2.5; // 旋转速率

//      // 找到离球最近的球员
//      int playerClosestToBall = -1;
//      double closestDistanceToBall = 10000;
//      for(int i = WO_TEAMMATE1; i < WO_TEAMMATE1+NUM_AGENTS; ++i) {
//          VecPosition temp;
//         int playerNum = i - WO_TEAMMATE1 + 1;
//         if (worldModel->getUNum() == playerNum) {
//             // 这是自己
//             temp = worldModel->getMyPosition();
//         } else {
//             WorldObject* teammate = worldModel->getWorldObject(i);
//             if (teammate->validPosition) {
//                 temp = teammate->pos;
//             } else {
//                 continue;
//             }
//         }
//         temp.setZ(0);

//         double distanceToBall = temp.getDistanceTo(ball);
//         if (distanceToBall < closestDistanceToBall) {
//             playerClosestToBall = playerNum;
//             closestDistanceToBall = distanceToBall;
//         }
//     }

//     if (playerClosestToBall == worldModel->getUNum()) {
//         // 让离球最近的球员向圆心踢球
//         return kickBall(KICK_FORWARD, center);
//     } else {
//         // 移动到圆心周围的圆圈位置并面向圆心
//         VecPosition localCenter = worldModel->g2l(center);
//         SIM::AngDeg localCenterAngle = atan2Deg(localCenter.getY(), localCenter.getX());

//         // 我们在圆圈上的目标位置
//         // 根据球员编号、旋转速率和时间计算目标位置
//         VecPosition target = center + VecPosition(circleRadius,0,0).rotateAboutZ(360.0/(NUM_AGENTS-1)*(worldModel->getUNum()-(worldModel->getUNum() > playerClosestToBall ? 1 : 0)) + worldModel->getTime()*rotateRate);

//         // 调整目标位置，避免与队友或球太近
//         target = collisionAvoidance(true /*队友*/, false/*对手*/, true/*球*/, 1/*接近阈值*/, .5/*碰撞阈值*/, target, true/*保持距离*/);

//         if (me.getDistanceTo(target) < .25 && abs(localCenterAngle) <= 10) {
//             // 距离目标位置和朝向足够近，原地站立
//             return SKILL_STAND;
//         } else if (me.getDistanceTo(target) < .5) {
//             // 接近目标位置，开始转向面向圆心
//             return goToTargetRelative(worldModel->g2l(target), localCenterAngle);
//         } else {
//             // 向目标位置移动
//             return goToTarget(target);
//         }
//     }
// }










































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
  








































