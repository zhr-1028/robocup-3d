#ifndef CURVE3D_H
#define CURVE3D_H

#include <vector>
#include "../math/vecposition.h"

// 3D曲线基类
class Curve3D {
public:
    // 构造函数，使用控制点向量初始化曲线
    Curve3D(const std::vector<VecPosition> &controlPoints);
    // 虚析构函数，确保派生类对象能正确释放
    virtual ~Curve3D();

    // 纯虚函数，用于获取曲线上参数t对应的点
    virtual VecPosition getPoint(float t) const = 0;

    // 获取曲线的控制点向量
    const std::vector<VecPosition> &getControlPoints() const {
        return _controlPoints;
    }

protected:
    // 存储曲线的控制点向量
    const std::vector<VecPosition> _controlPoints;
};

// 3D贝塞尔曲线类，继承自Curve3D
class Bezier3D : public Curve3D {
public:
    // 构造函数，使用控制点向量初始化贝塞尔曲线
    Bezier3D(const std::vector<VecPosition> &controlPoints);
    // 虚析构函数，确保对象能正确释放
    virtual ~Bezier3D();

    // 重写基类的getPoint函数，用于获取贝塞尔曲线上参数t对应的点
    virtual VecPosition getPoint(float t) const;

private:
    // 存储贝塞尔曲线计算所需的系数
    std::vector<long> _coeff;
};

// 3D埃尔米特样条曲线类，继承自Curve3D
class HermiteSpline3D : public Curve3D {
public:
    // 构造函数，使用控制点向量和切线缩放因子初始化埃尔米特样条曲线
    HermiteSpline3D(const std::vector<VecPosition> &controlPoints, float tangentScale);
    // 虚析构函数，确保对象能正确释放
    virtual ~HermiteSpline3D();
    // 重写基类的getPoint函数，用于获取埃尔米特样条曲线上参数t对应的点
    virtual VecPosition getPoint(float t) const;
private:
    // 切线缩放因子
    float scale;
    // 存储曲线各点的切线向量
    std::vector<VecPosition> tangents;
};

// 3D均匀B样条曲线类，继承自Curve3D
class UniformBSpline3D : public Curve3D {
public:
    // 构造函数，使用控制点向量初始化均匀B样条曲线
    UniformBSpline3D(const std::vector<VecPosition> &controlPoints);
    // 虚析构函数，确保对象能正确释放
    virtual ~UniformBSpline3D();

    // 重写基类的getPoint函数，用于获取均匀B样条曲线上参数t对应的点
    virtual VecPosition getPoint(float t) const;

protected:
    // 曲线的阶数
    const size_t _n;             
    // 节点数量
    const size_t _m;             
    // 节点向量
    std::vector<float> _T;      

    // 计算B样条基函数值的函数
    float b(int, int, float) const;

};

#endif