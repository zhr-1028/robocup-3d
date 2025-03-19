#include "naobehavior.h"

/*
  这个函数如果正在采取措施来避免或从跌倒中恢复则返回 true；否则返回 false。

  如果 fallState 为 0，表示我们仍然安全，并且会沿着四个方向（上、下、右、左）进行跌倒检测。
  如果这些方向中的任何一个指示发生了跌倒，fallState 变为 1，并且机器人会向侧面伸展它的手臂。
  这个动作通常会使机器人处于向上或向下跌倒的位置，即不是向侧面跌倒。fallState 为 1 会导致进入 fallState 为 2 的状态，此时会重新评估跌倒的方向。

  从 fallenUp 或 fallenDown 状态开始，fallState 为 3 会导致一系列的 fallStates，最终会重置为 0。
 */
bool NaoBehavior::checkingFall() {

    VecPosition accel = bodyModel->getAccelRates();
    if(fallState == 0 || beamablePlayMode()) {
        // 检查是否正在跌倒。如果正在跌倒，只会设置一个标志。
        fallenUp = (accel.getX() < -6.5);
        fallenDown = !fallenUp && (accel.getX() > 7.5);
        fallenRight = !fallenUp && !fallenDown && (accel.getY() < -6.5);
        fallenLeft = !fallenUp && !fallenDown && !fallenRight && (accel.getY() >  6.5);

        /*
        if(fallenLeft){
          cout << "----------FALLEN LEFT-------------\n";
        }
        else if(fallenRight){
          cout << "----------FALLEN RIGHT-------------\n";
        }
        else if(fallenUp){
          cout << "----------FALLEN UP-------------\n";
        }
        else if(fallenDown){
          cout << "----------FALLEN DOWN-------------\n";
        }
        */

        if(fallenUp || fallenDown || fallenLeft || fallenRight) {
            fallState = 1;
            currentFallStateStartTime = -1.0;
            worldModel->setFallen(true);
            worldModel->setLocalized(false);
            if (beamablePlayMode()) {
                fallState = 0;
                return false;
            }
        }
        else {
            fallState = 0;
            worldModel->setFallen(false);
            return false;
        }
    }

    // 保持头部向前
    bodyModel->setScale(EFF_H1, 0.5);
    bodyModel->setTargetAngle(EFF_H1, 0);

    if(fallState == 1) {

        if(currentFallStateStartTime < 0) {
            currentFallStateStartTime = worldModel->getTime();
        }

        bodyModel->setInitialArm(ARM_LEFT);
        bodyModel->setInitialArm(ARM_RIGHT);
        bodyModel->setInitialLeg(LEG_LEFT);
        bodyModel->setInitialLeg(LEG_RIGHT);

        bodyModel->setTargetAngle(EFF_LA2, 90);
        bodyModel->setTargetAngle(EFF_RA2, -90);

        bodyModel->setTargetAngle(EFF_LA4, 0);
        bodyModel->setTargetAngle(EFF_RA4, 0);

        if (fallTimeWait < 0) {
            fallTimeWait = worldModel->getTime();
        }

        if(worldModel->getTime() - currentFallStateStartTime > 0.2) { // 调整
            fallState = 2;
            currentFallStateStartTime = -1.0;
            fallTimeStamp = worldModel->getTime();
            fallTimeWait = -1;
        }
    }
    else if(fallState == 2) {

        if(currentFallStateStartTime < 0) {
            currentFallStateStartTime = worldModel->getTime();
        }

        if(worldModel->getTime() > (fallTimeStamp + (fallenDown? atof(namedParams.find("getup_parms_stateDownInitialWait")->second.c_str()) : atof(namedParams.find("getup_parms_stateUpInitialWait")->second.c_str())))) {
            fallTimeStamp = -1;
            fallState = 1;
            currentFallStateStartTime = -1.0;

            // 再次检查。如果仍然是向右侧或左侧跌倒，就当作是向上跌倒来处理。
            fallenUp = (accel.getX() < -6.5);
            fallenDown = !fallenUp && (accel.getX() > 6.5);
            fallenRight = !fallenUp && !fallenDown && (accel.getY() < -6.5);
            fallenLeft = !fallenUp && !fallenDown && !fallenRight && (accel.getY() >  6.5);

            if(fallenUp || fallenRight || fallenLeft) {
                fallenUp = true;
                fallenRight = false;
                fallenLeft = false;
                fallState = 3;
                currentFallStateStartTime = -1.0;
            }
            else if(fallenDown) {
                fallState = 3;
                currentFallStateStartTime = -1.0;
            }
            else { // 神奇的是，我们站起来了！
                fallState = 0;
                currentFallStateStartTime = -1.0;
            }
        }
    }


    // 恢复过程
    if(fallenDown) {

        if(fallState == 3) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LA1, atof(namedParams.find("getup_parms_stateDown3A1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA1, atof(namedParams.find("getup_parms_stateDown3A1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LA2, 0);
            bodyModel->setTargetAngle(EFF_RA2, 0);

            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateDown3L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateDown3L3")->second.c_str()));

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateDown3L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateDown3L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateDown3MinTime")->second.c_str())) {
                fallState = 4;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 4) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            if(worldModel->getTime() > (fallTimeStamp + 0)) {
                fallTimeStamp = -1;
                fallState = 5;
                currentFallStateStartTime = -1.0;
            }
        }
        else if(fallState == 5) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateDown5L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateDown5L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL5, 0);
            bodyModel->setTargetAngle(EFF_RL5, 0);

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateDown5L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateDown5L7")->second.c_str()));
            }


            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateDown5MinTime")->second.c_str())) {
                fallState = 6;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 6) {
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            if(worldModel->getTime() > (fallTimeStamp + 0.25)) {
                fallTimeStamp = -1;
                fallState = 7;
                currentFallStateStartTime = -1.0;
            }
        }
        else if(fallState == 7) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateDown7L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateDown7L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateDown7L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateDown7L3")->second.c_str()));

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateDown7L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateDown7L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateDown7MinTime")->second.c_str())) {
                fallState = 8;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 8) {
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            if(worldModel->getTime() > (fallTimeStamp + 0)) {
                fallTimeStamp = -1;
                fallState = 9;
                currentFallStateStartTime = -1.0;
            }
        }
        else if(fallState == 9) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setInitialArm(ARM_LEFT);
            bodyModel->setInitialArm(ARM_RIGHT);
            bodyModel->setInitialLeg(LEG_LEFT);
            bodyModel->setInitialLeg(LEG_RIGHT);

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateDown10MinTime")->second.c_str())) {
                fallState = 10;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 10) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            if (worldModel->getTime() < (fallTimeStamp + 1.0)) {
                VecPosition gyroRates = bodyModel->getGyroRates();
                // 检查稳定性，如果陀螺仪速率较高，则不表示我们已经恢复。
                if (gyroRates.getX() >.5 || gyroRates.getY() >.5 || gyroRates.getZ() >.5) {
                    return true;
                }
            }

            if(worldModel->getTime() > (fallTimeStamp + 0.1)) {
                fallTimeStamp = -1;
                currentFallStateStartTime = -1.0;
                fallState = 0;
                fallenDown = false;
                lastGetupRecoveryTime = worldModel->getTime();
                // 递归...
                return checkingFall();

            }
        }
    }

    if(fallenUp) {

        if(fallState == 3) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LA1, atof(namedParams.find("getup_parms_stateUp3A1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA1, atof(namedParams.find("getup_parms_stateUp3A1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LA2, atof(namedParams.find("getup_parms_stateUp3A2")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA2, -atof(namedParams.find("getup_parms_stateUp3A2")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LA4, atof(namedParams.find("getup_parms_stateUp3A4")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA4, -atof(namedParams.find("getup_parms_stateUp3A4")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateUp3L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateUp3L3")->second.c_str()));

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp3L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp3L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp3MinTime")->second.c_str())) {
                fallState = 4;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 4) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            if(worldModel->getTime() > (fallTimeStamp + 0)) {
                fallTimeStamp = -1;
                fallState = 5;
                currentFallStateStartTime = -1.0;
            }
        }
        else if(fallState == 5) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateUp5L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateUp5L3")->second.c_str()));

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp5L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp5L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp5MinTime")->second.c_str())) {
                fallState = 6;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 6) {

            
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 当当前时间超过跌倒时间戳时，更新状态
            if(worldModel->getTime() > (fallTimeStamp + 0)) {
                fallTimeStamp = -1;
                fallState = 7;
                currentFallStateStartTime = -1.0;
            }
        }
        else if(fallState == 7) {
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 设置左臂和右臂的目标角度为 0
            bodyModel->setTargetAngle(EFF_LA1, 0);
            bodyModel->setTargetAngle(EFF_RA1, 0);

            bodyModel->setTargetAngle(EFF_LA2, 0);
            bodyModel->setTargetAngle(EFF_RA2, 0);

            // 设置左腿和右腿的目标角度，从参数中获取对应值
            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateUp7L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateUp7L1")->second.c_str()));

            // 如果机器人有脚趾，设置脚趾的目标角度
            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp7L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp7L7")->second.c_str()));
            }

            // 如果跌倒等待时间未记录，则记录当前时间
            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            // 当经过的时间超过指定的最小时间，更新状态
            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp7MinTime")->second.c_str())) {
                fallState = 8;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 8) {
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 当当前时间超过跌倒时间戳时，更新状态
            if(worldModel->getTime() > (fallTimeStamp + 0)) {
                fallTimeStamp = -1;
                fallState = 9;
                currentFallStateStartTime = -1.0;
            }
        }
        else if(fallState == 9) {
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 设置左臂和右臂的目标角度，从参数中获取对应值
            bodyModel->setTargetAngle(EFF_LA1, atof(namedParams.find("getup_parms_stateUp9A1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA1, atof(namedParams.find("getup_parms_stateUp9A1")->second.c_str()));

            // 设置左腿和右腿的目标角度，从参数中获取对应值
            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateUp9L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateUp9L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL4, atof(namedParams.find("getup_parms_stateUp9L4")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL4, atof(namedParams.find("getup_parms_stateUp9L4")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL5, atof(namedParams.find("getup_parms_stateUp9L5")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL5, atof(namedParams.find("getup_parms_stateUp9L5")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL6, atof(namedParams.find("getup_parms_stateUp9L6")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL6, -atof(namedParams.find("getup_parms_stateUp9L6")->second.c_str()));

            // 如果机器人有脚趾，设置脚趾的目标角度
            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp9L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp9L7")->second.c_str()));
            }

            // 如果跌倒等待时间未记录，则记录当前时间
            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            // 当经过的时间超过指定的最小时间，更新状态
            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp9MinTime")->second.c_str())) {
                fallState = 10;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 10) {
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 当当前时间超过跌倒时间戳时，更新状态
            if(worldModel->getTime() > (fallTimeStamp + 0)) {
                fallTimeStamp = -1;
                fallState = 11;
                currentFallStateStartTime = -1.0;
            }
        }
        else if(fallState == 11) {
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 设置左臂和右臂的目标角度，从参数中获取对应值
            bodyModel->setTargetAngle(EFF_LA1, atof(namedParams.find("getup_parms_stateUp11A1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA1, atof(namedParams.find("getup_parms_stateUp11A1")->second.c_str()));

            // 设置左腿和右腿的目标角度，从参数中获取对应值
            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateUp11L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateUp11L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL5, atof(namedParams.find("getup_parms_stateUp11L5")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL5, atof(namedParams.find("getup_parms_stateUp11L5")->second.c_str()));

            // 如果机器人有脚趾，设置脚趾的目标角度
            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp11L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp11L7")->second.c_str()));
            }

            // 如果跌倒等待时间未记录，则记录当前时间
            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            // 当经过的时间超过指定的最小时间，更新状态
            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp11MinTime")->second.c_str())) {
                fallState = 12;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 12) {
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 当当前时间超过跌倒时间戳时，更新状态
            if(worldModel->getTime() > (fallTimeStamp + 0)) {
                fallTimeStamp = -1;
                fallState = 13;
                currentFallStateStartTime = -1.0;
            }
        }
        else if(fallState == 13) {
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 设置左臂和右臂的目标角度，从参数中获取对应值
            bodyModel->setTargetAngle(EFF_LA1, atof(namedParams.find("getup_parms_stateUp13A1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA1, atof(namedParams.find("getup_parms_stateUp13A1")->second.c_str()));

            // 设置左腿和右腿的目标角度，从参数中获取对应值
            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateUp13L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateUp13L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateUp13L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateUp13L3")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL4, atof(namedParams.find("getup_parms_stateUp13L4")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL4, atof(namedParams.find("getup_parms_stateUp13L4")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL5, atof(namedParams.find("getup_parms_stateUp13L5")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL5, atof(namedParams.find("getup_parms_stateUp13L5")->second.c_str()));

            // 如果机器人有脚趾，设置脚趾的目标角度
            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp13L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp13L7")->second.c_str()));
            }

            // 如果跌倒等待时间未记录，则记录当前时间
            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            // 当经过的时间超过指定的最小时间，更新状态
            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp13MinTime")->second.c_str())) {
                fallState = 14;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 14) {
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 当当前时间超过跌倒时间戳时，更新状态
            if(worldModel->getTime() > (fallTimeStamp + 0)) {
                fallTimeStamp = -1;
                fallState = 15;
                currentFallStateStartTime = -1.0;
            }
        }
        else if(fallState == 15) {
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 将左臂、右臂、左腿、右腿设置为初始状态
            bodyModel->setInitialArm(ARM_LEFT);
            bodyModel->setInitialArm(ARM_RIGHT);
            bodyModel->setInitialLeg(LEG_LEFT);
            bodyModel->setInitialLeg(LEG_RIGHT);

            // 如果跌倒等待时间未记录，则记录当前时间
            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            // 当经过的时间超过指定的最小时间，更新状态
            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp15MinTime")->second.c_str())) {
                fallState = 16;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 16) {
            // 如果当前跌倒状态开始时间未记录，则记录当前时间
            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            // 在一定时间内检查陀螺仪数据，判断是否稳定
            if (worldModel->getTime() < (fallTimeStamp + 1.0)) {
                VecPosition gyroRates = bodyModel->getGyroRates();
                // 检查稳定性，如果陀螺仪速率较高，则认为还未恢复
                if (gyroRates.getX() > .5 || gyroRates.getY() > .5 || gyroRates.getZ() > .5) {
                    return true;
                }
            }

            // 当超过一定时间后，重置状态并递归调用检查函数
            if(worldModel->getTime() > (fallTimeStamp + 0.1)) {
                fallTimeStamp = -1;
                fallState = 0;
                currentFallStateStartTime = -1.0;
                fallenUp = false;
                lastGetupRecoveryTime = worldModel->getTime();
                // 递归调用检查函数
                return checkingFall();
            }
        }
    }

    return true;
}