#ifndef BODY_MODEL_H
#define BODY_MODEL_H

#include "../headers/headers.h"
#include "../math/vecposition.h"
#include "../math/hctmatrix.h"
#include "../worldmodel/worldmodel.h"
#include "../ikfast/ikfast.h"

#include <sstream>

// 用于 UT Walk
class SimEffectorBlock;
class JointBlock;
class JointCommandBlock;

// 表示一个关节的结构体
struct SIMJoint {
    // 关节角度
    double angle;

    // 构造函数，初始化关节角度为 0
    SIMJoint() {
        angle = 0;
    }
};

/**
 * Pos6DOF 表示一个 6 自由度的位姿。
 */
struct Pos6DOF
{
    // 位姿的 XYZ 分量
    VecPosition xyz;      
    // 位姿的滚转 - 俯仰 - 偏航分量
    VecPosition rpy;      

    // 反射当前位姿
    Pos6DOF reflect() const;
};

// 重载输出运算符，用于输出 Pos6DOF 结构体
std::ostream& operator<<(std::ostream &out, const Pos6DOF &pos);

// 表示一个执行器的结构体
struct Effector {
    // 执行器的最小角度
    double minAngle;
    // 执行器的最大角度
    double maxAngle;

    // 当前角度
    double currentAngle;
    // 目标角度
    double targetAngle;

    // PID 控制的常数
    double k1, k2, k3;

    // 误差项
    double currentError;  // 对应 k1
    double cumulativeError; // 对应 k2
    double previousError; // 对应 k3

    // 设定点容差
    double errorTolerance;

    // 缩放比例
    double scale;

    // 构造函数，初始化执行器的参数
    Effector(const double &minAngle, const double &maxAngle, const double &k1, const double &k2, const double &k3, const double &errorTolerance) {
        this->minAngle = minAngle;
        this->maxAngle = maxAngle;

        currentAngle = 0;
        targetAngle = 0;

        this->k1 = k1;
        this->k2 = k2;
        this->k3 = k3;

        resetErrors();
        this->errorTolerance = errorTolerance;

        scale = 1.0;
    }

    // 重置误差项
    void resetErrors() {
        currentError = 0;
        previousError = 0;
        cumulativeError = 0;
    }

    // 更新误差项
    void updateErrors() {
        previousError = currentError;
        currentError = targetAngle - currentAngle;
        cumulativeError += currentError;
    }

    // 设置目标角度，确保角度在最小和最大角度范围内
    void setTargetAngle(const double &angle) {
        if(angle < minAngle) {
            targetAngle = minAngle;
        }
        else if(angle > maxAngle) {
            targetAngle = maxAngle;
        }
        else {
            targetAngle = angle;
        }
    }

    // 更新当前角度
    void update(const double &angle) {
        currentAngle = angle;
    }

    // 判断是否达到目标角度
    bool targetReached() {
        return (fabs(targetAngle - currentAngle) < errorTolerance);
    }

    // 判断给定角度是否在执行器的有效范围内
    bool isAngleInRange(const double &angle) const {
        return (minAngle <= angle) && (angle <= maxAngle);
    }
};

// 表示机器人身体组件的结构体
struct Component {
    // 父组件的索引
    int parent;

    // 组件的质量
    double mass;
    // 相对于父组件的平移向量
    VecPosition translation;
    // 锚点位置
    VecPosition anchor;
    // 旋转轴
    VecPosition axis;

    // 从父组件到当前组件的变换矩阵
    HCTMatrix transformFromParent;
    // 从根组件到当前组件的变换矩阵
    HCTMatrix transformFromRoot;

    // 平移矩阵
    HCTMatrix translateMatrix;
    // 反向平移矩阵
    HCTMatrix backTranslateMatrix;

    // 构造函数，初始化组件的参数
    Component(const int &parent, const double &mass, const VecPosition &translation, const VecPosition &anchor, const VecPosition &axis) {
        this->parent = parent;
        this->mass = mass;

        this->translation = translation;
        this->anchor = anchor;
        this->axis = axis;

        transformFromParent = HCTMatrix();
        transformFromRoot = HCTMatrix();

        translateMatrix = HCTMatrix(HCT_TRANSLATE, VecPosition(translation) + VecPosition(anchor));
        backTranslateMatrix = HCTMatrix(HCT_TRANSLATE, -VecPosition(anchor));
    }
};

// 表示机器人身体模型的类
class BodyModel {
private:
    // 指向世界模型的指针
    WorldModel *worldModel;

    // 关节向量
    std::vector<SIMJoint> joint;

    // 执行器向量
    std::vector<Effector> effector;

    // 反射执行器的索引向量
    std::vector<int> reflectedEffector;
    // 是否需要反射角度的布尔向量
    std::vector<bool> toReflectAngle;

    // 组件向量
    std::vector<Component> component;

    // 陀螺仪速率
    VecPosition gyroRates;
    // 加速度计速率
    VecPosition accelRates;
    // 左脚的地面反作用力中心
    VecPosition FRPCentreLeft;
    // 左脚的地面反作用力
    VecPosition FRPForceLeft;
    // 右脚的地面反作用力中心
    VecPosition FRPCentreRight;
    // 右脚的地面反作用力
    VecPosition FRPForceRight;
    // 左脚的另一个地面反作用力中心
    VecPosition FRPCentreLeft1;
    // 左脚的另一个地面反作用力
    VecPosition FRPForceLeft1;
    // 右脚的另一个地面反作用力中心
    VecPosition FRPCentreRight1;
    // 右脚的另一个地面反作用力
    VecPosition FRPForceRight1;

    // 身体与世界的接口变换矩阵
    HCTMatrix bodyWorldInterface;

    // 左腿的变换矩阵
    HCTMatrix ll1, ll2, ll3, ll4, ll5;
    // 右腿的变换矩阵
    HCTMatrix rl1, rl2, rl3, rl4, rl5;

    // 跌倒角度
    double fallAngle;

    // 是否使用全向行走的标志
    bool fUseOmniWalk;

    // 刷新躯干的变换矩阵
    void refreshTorso();
    // 刷新指定索引组件的变换矩阵
    void refreshComponent(const int &index);
    // 刷新头部的变换矩阵
    void refreshHead();
    // 刷新左臂的变换矩阵
    void refreshLeftArm();
    // 刷新右臂的变换矩阵
    void refreshRightArm();
    // 刷新左腿的变换矩阵
    void refreshLeftLeg();
    // 刷新右腿的变换矩阵
    void refreshRightLeg();

    // 计算跌倒角度
    inline void computeFallAngle() {
        VecPosition zAxis = VecPosition(0, 0, 1.0);
        VecPosition bodyZAxis = (worldModel->l2g(VecPosition(0, 0, 1.0)) - worldModel->l2g(VecPosition(0, 0, 0))).normalize();
        fallAngle = VecPosition(0, 0, 0).getAngleBetweenPoints(zAxis, bodyZAxis);
    }

public:
    // 构造函数，初始化身体模型
    BodyModel(WorldModel *worldModel);
    // 另一个构造函数，用于基于当前身体模型和腿部角度创建新的身体模型
    BodyModel(const BodyModel *current, const int legIndex, const double a1, const double a2, const double a3, const double a4, const double a5, const double a6);
    // 析构函数
    ~BodyModel();

    // 初始化执行器
    void initialiseEffectors();
    // 初始化组件
    void initialiseComponents();

    // 获取指定关节的角度
    inline double getJointAngle(const int &i) {
        return joint[i].angle;
    }
    // 设置指定关节的角度
    inline void setJointAngle(const int &i, const double &a) {
        joint[i].angle = a;
    }

    // 显示身体模型的信息
    void display();
    // 显示派生的身体模型信息
    void displayDerived();

    // 获取指定执行器的当前角度
    inline double getCurrentAngle(const int &EffectorID) const {
        return effector[EffectorID].currentAngle;
    }
    // 设置指定执行器的当前角度
    inline void setCurrentAngle(const int &EffectorID, const double &angle) {
        effector[EffectorID].currentAngle = angle;
    }

    // 获取指定执行器的目标角度
    inline double getTargetAngle(const int &EffectorID) const {
        return effector[EffectorID].targetAngle;
    }
    // 设置指定执行器的目标角度
    inline void setTargetAngle(const int &EffectorID, const double &angle) {
        effector[EffectorID].setTargetAngle(angle);
    }
    // 增加指定执行器的目标角度
    inline void increaseTargetAngle(const int &EffectorID, const double &increase) {
        effector[EffectorID].setTargetAngle(effector[EffectorID].targetAngle + increase);
    };

    // 获取指定执行器的缩放比例
    inline double getScale(const int &EffectorID) {
        return effector[EffectorID].scale;
    }
    // 设置指定执行器的缩放比例
    inline void setScale(const int &EffectorID, double s) {
        effector[EffectorID].scale = s;
    }

    // 判断指定执行器是否达到目标角度
    inline bool getTargetReached(const int &EffectorID) {
        return effector[EffectorID].targetReached();
    }

    // 计算指定执行器的扭矩
    double computeTorque(const int &effectorID);

    // 用于 UT Walk，计算指定执行器的扭矩
    double computeTorque(const int &effID, SimEffectorBlock* sim_effectors_, JointBlock* raw_joint_angles_, JointCommandBlock* raw_joint_commands_);
    // 设置是否使用全向行走
    inline void setUseOmniWalk(bool fUseOmniWalk) {
        this->fUseOmniWalk = fUseOmniWalk;
    }
    // 判断是否使用全向行走
    inline bool useOmniWalk() {
        return fUseOmniWalk;
    }

    // 获取陀螺仪速率
    inline VecPosition getGyroRates() {
        return gyroRates;
    }
    // 设置陀螺仪速率
    inline void setGyroRates(const double &rateX, const double &rateY, const double &rateZ) {
        gyroRates = VecPosition(rateX, rateY, rateZ);
    }

    // 获取加速度计速率
    inline VecPosition getAccelRates() {
        return accelRates;
    }
    // 设置加速度计速率
    inline void setAccelRates(const double &rateX, const double &rateY, const double &rateZ) {
        accelRates = VecPosition(rateX, rateY, rateZ);
    }

    // 获取左脚的地面反作用力中心
    inline VecPosition getFRPCentreLeft() const {
        return FRPCentreLeft;
    }
    // 获取左脚的地面反作用力
    inline VecPosition getFRPForceLeft() const {
        return FRPForceLeft;
    }
    // 获取右脚的地面反作用力中心
    inline VecPosition getFRPCentreRight() const {
        return FRPCentreRight;
    }
    // 获取右脚的地面反作用力
    inline VecPosition getFRPForceRight() const {
        return FRPForceRight;
    }
    // 获取左脚的另一个地面反作用力中心
    inline VecPosition getFRPCentreLeft1() const {
        return FRPCentreLeft1;
    }
    // 获取左脚的另一个地面反作用力
    inline VecPosition getFRPForceLeft1() const {
        return FRPForceLeft1;
    }
    // 获取右脚的另一个地面反作用力中心
    inline VecPosition getFRPCentreRight1() const {
        return FRPCentreRight1;
    }
    // 获取右脚的另一个地面反作用力
    inline VecPosition getFRPForceRight1() const {
        return FRPForceRight1;
    }
    // 设置左脚的地面反作用力
    inline void setFRPLeft(const VecPosition &centre, const VecPosition &force) {
        FRPCentreLeft = VecPosition(centre);
        FRPForceLeft = VecPosition(force);
    }
    // 设置右脚的地面反作用力
    inline void setFRPRight(const VecPosition &centre, const VecPosition &force) {
        FRPCentreRight = VecPosition(centre);
        FRPForceRight = VecPosition(force);
    }
    // 设置左脚的另一个地面反作用力
    inline void setFRPLeft1(const VecPosition &centre, const VecPosition &force) {
        FRPCentreLeft1 = VecPosition(centre);
        FRPForceLeft1 = VecPosition(force);
    }
    // 设置右脚的另一个地面反作用力
    inline void setFRPRight1(const VecPosition &centre, const VecPosition &force) {
        FRPCentreRight1 = VecPosition(centre);
        FRPForceRight1 = VecPosition(force);
    }

    // 刷新身体模型的状态
    void refresh();

    // 判断指定腿部是否能够伸展到指定位置和偏航角
    bool canReachOutLeg(const int &legIndex, const VecPosition &xyz, const double &yaw) const;
    // 判断指定腿部是否能够伸展到指定位置和位姿
    bool canReachOutLeg(const int &legIndex, const VecPosition &xyz, const VecPosition &rpy = VecPosition(0.0, 0.0, 0.0)) const;
    // 让指定腿部伸展到指定位置和偏航角
    bool reachOutLeg(const int &legIndex, const VecPosition &xyz, const double &yaw);
    // 让指定腿部伸展到指定位置和位姿
    bool reachOutLeg(const int &legIndex, const VecPosition &xyz, const VecPosition &rpy = VecPosition(0.0, 0.0, 0.0));
    // 判断所有执行器是否都达到了目标角度
    bool targetsReached();

    // 获取指定腿部在给定角度下的脚部质心
    VecPosition getFootCG(const int &legIndex, const double &ang1, const double &ang2, const double &ang3, const double &ang4, const double &ang5, const double &ang6);
    // 获取指定腿部的脚部质心
    VecPosition getFootCG(const int &legIndex) const;

    // 获取指定腿部在给定角度下的脚部变换矩阵
    void getFootTransform(const int &legIndex, HCTMatrix &m, const double &ang1, const double &ang2, const double &ang3, const double &ang4, const double &ang5, const double &ang6) const;

    // 获取指定腿部的全局脚部质心和坐标轴
    void getFootGlobalCGAndAxes(const int &legIndex, VecPosition &CG, VecPosition &xAxis, VecPosition &yAxis, VecPosition &zAxis);
    // 获取指定腿部在给定角度下的全局脚部质心和坐标轴
    void getFootGlobalCGAndAxesAngles(const int &legIndex, const vector<double> &angles, VecPosition &origin, VecPosition &xAxis, VecPosition &yAxis, VecPosition &zAxis);

    // 获取球相对于左脚的位置
    VecPosition getBallWRTLeftFoot(WorldModel *worldModel);
    // 获取球相对于右脚的位置
    VecPosition getBallWRTRightFoot(WorldModel *worldModel);

    // 计算指定腿部的逆运动学，得到关节角度
    bool legInverseKinematics(const int &legIndex, const VecPosition &xyz, const VecPosition &rpy, double &a1, double &a2, double &a3, double &a4, double &a5, double &a6) const;
    // 稳定指定腿部的姿态
    bool stabilize(const int legIndex, const VecPosition zmp);
    // 判断指定腿部是否能够稳定在指定的零力矩点
    bool canStabilize(const int legIndex, const VecPosition zmp) const;
    // 应用稳定化策略到指定腿部
    bool applyStabilization(const int &legIndex, const VecPosition zmp, double &stab_ra1, double &stab_ra2, double &stab_la1, double &stab_la2, double &stab_pl2, double &stab_pl5, double &stab_pl6, double &stab_ol2, double &stab_ol5, double &stab_ol6) const;
    // 应用稳定化策略到指定手臂
    void applyStabilizationArm(const int armIndex, double &stab_shoulderY, double &stab_shoulderZ, VecPosition dirToMove, double bodyMass) const;

    // 获取跌倒角度
    inline double getFallAngle() {
        return fallAngle;
    }

    // 将相机坐标系下的向量转换到原点坐标系下
    VecPosition transformCameraToOrigin(const VecPosition &v);

    // 获取指定执行器的反射信息
    void getReflection( const int& effID,
                        const double& angle,
                        int& reflectedEffID,
                        double& reflectedAngle );

    // 获取指定腿部位姿的反射信息
    void getReflection( const int& legIDX,
                        const Pos6DOF& pos,
                        int& reflectedLegIDX,
                        Pos6DOF& reflectedPos );

    // 获取机器人的质心
    VecPosition getCenterOfMass() const;
    // 获取指定身体部分的质心和总质量
    VecPosition getCenterOfMass(int bodyPart, double &totalMass) const;

    // 从 NaoBehavior 迁移过来的函数，设置头部的初始姿态
    bool setInitialHead();
    // 设置指定手臂的初始姿态
    bool setInitialArm(const int &arm);
    // 设置指定腿部的初始姿态
    bool setInitialLeg(const int &leg);

    // 判断机器人是否有脚趾
    bool hasToe();
    // 判断是否在 Gazebo 环境中
    bool isGazebo();

protected:
    // 过滤掉指定腿部超出关节范围的逆运动学解
    void filterOutOfBounds(const int &legIndex, vector<joints_t> &solutions) const;
    // 从逆运动学解中选择最佳解
    const joints_t& pickBestSolution(const int &legIndex, vector<joints_t> &solutions) const;
    // 判断指定腿部的逆运动学解是否无效
    bool isInvalidSolution(const int &legIndex, const joints_t &solution) const;
};

#endif // BODY_MODEL_H