#ifndef _FIELD_H
#define _FIELD_H

#include "../worldmodel/WorldObject.h"
#include "../math/Geometry.h"
#include <stdio.h>

// 设置场地尺寸、地标位置等常量
// 新场地

// 场地的 Y 轴长度
const double FIELD_Y = 20;
// 场地的 X 轴长度
const double FIELD_X = 30;

// 场地 Y 轴长度的一半
const double HALF_FIELD_Y = FIELD_Y/2.0;
// 场地 X 轴长度的一半
const double HALF_FIELD_X = FIELD_X/2.0;

// 球门的宽度
const double GOAL_Y = 2.1;
// 球门的深度
const double GOAL_X = .6;
// 球门的高度
const double GOAL_Z = .8;
// 球门宽度的一半
const double HALF_GOAL_Y = GOAL_Y / 2.0;

// 罚球区的 Y 轴长度
const double PENALTY_Y = 6.0;// 5.8; //3.9;
// 罚球区的 X 轴长度
const double PENALTY_X = 1.8;

// 场地中心的 X 坐标
const double FIELD_CENTER_X = 0;
// 场地中心的 Y 坐标
const double FIELD_CENTER_Y = 0;

// 角球区的 Y 坐标相关值
const double CORNER_Y = 5.5;

// 一些矩形区域

// 定义场地矩形区域
// 左上角坐标为 (-FIELD_X / 2,  FIELD_Y / 2)，右下角坐标为 ( FIELD_X / 2, -FIELD_Y / 2)
const SIM::Rectangle FIELD =
    SIM::Rectangle( SIM::Point2D( -FIELD_X / 2,  FIELD_Y / 2 ),
                    SIM::Point2D(  FIELD_X / 2, -FIELD_Y / 2 ) );

// 我们不想将粒子限制在边界内
// 所以这个区域是场地的两倍大
// 左上角坐标为 (-FIELD_X,  FIELD_Y)，右下角坐标为 ( FIELD_X, -FIELD_Y)
const SIM::Rectangle GRASS =
    SIM::Rectangle( SIM::Point2D( -FIELD_X ,  FIELD_Y  ),
                    SIM::Point2D(  FIELD_X , -FIELD_Y  ) );

#endif