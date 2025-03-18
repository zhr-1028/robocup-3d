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
    int playerNum = worldModel->getUNum(); // 获取球员编号
    VecPosition ballPos = worldModel->getBall(); // 获取球的位置
    VecPosition myPos = worldModel->getMyPosition(); // 获取自身位置

    // 守门员行为
    if (playerNum == 1) {
        if (ballPos.getX() < -HALF_FIELD_X + 2.0) { // 球在禁区附近
            return kickBall(KICK_FORWARD, VecPosition(HALF_FIELD_X, 0, 0)); // 尝试解围
        } else {
            return SKILL_STAND; // 守门员站立
        }
    }
    // 后卫行为
    else if (playerNum == 2 || playerNum == 3) {
        if (ballPos.getX() < -HALF_FIELD_X + 5.0) { // 球在后场
            return goToTarget(ballPos); // 拦截球
        } else {
            return SKILL_STAND; // 防守站位
        }
    }
    // 中场行为
    else if (playerNum == 4 || playerNum == 5 || playerNum == 6) {
        if (ballPos.getX() < 0) { // 球在中场
            return goToTarget(ballPos); // 支援进攻
        } else {
            return SKILL_STAND; // 防守站位
        }
    }
    // 前锋行为
    else if (playerNum >= 7 && playerNum <= 11) {
        if (ballPos.getX() > 0) { // 球在前场
            return kickBall(KICK_FORWARD, VecPosition(HALF_FIELD_X, 0, 0)); // 尝试射门
        } else {
            return goToTarget(ballPos); // 支援进攻
        }
    }

    // 默认行为
    return SKILL_STAND;
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
  








































