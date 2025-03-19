#ifndef RVDRAW_H
#define RVDRAW_H

/*
 *  Copyright (C) 2011 Justin Stoecker
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// 头文件保护，防止头文件被重复包含
// 若 RVDRAW_H 未定义，则执行下面的代码并定义 RVDRAW_H
// 这样可以避免因多次包含该头文件而导致的重复定义错误

// 包含必要的标准库头文件
#include <cstdio>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <math.h>
#include <map>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <iomanip>

// 定义 RoboViz 服务使用的端口号
#define ROBOVIS_PORT "32769"

// 使用标准命名空间，方便后续使用标准库中的类和函数
using namespace std;

// 以下是一些内联函数，用于将不同类型的数据写入缓冲区
// 将一个无符号字符值写入缓冲区，并返回写入的字节数（这里固定为 1）
inline int writeCharToBuf(unsigned char* buf, unsigned char value) {
    *buf = value;
    return 1;
}

// 将一个浮点数写入缓冲区，先将浮点数转换为字符串，再复制到缓冲区
// 返回写入的字节数（这里固定为 6）
inline int writeFloatToBuf(unsigned char* buf, float value) {
    char temp[20];
    sprintf(temp, "%6f", value);
    memcpy(buf, temp, 6);
    return 6;
}

// 将颜色值（一组浮点数）写入缓冲区，每个颜色通道值乘以 255 后转换为无符号字符
// 返回写入的字节数，即颜色通道数
inline int writeColorToBuf(unsigned char* buf, const float* color, int channels) {
    int i;
    for (i = 0; i < channels; i++)
        writeCharToBuf(buf+i, (unsigned char)(color[i]*255));
    return i;
}

// 将字符串写入缓冲区，若字符串不为空则复制字符串内容，最后添加一个空字符 '\0'
// 返回写入的字节数
inline int writeStringToBuf(unsigned char* buf, const string* text) {
    long i = 0;
    if (text != NULL)
        i += text->copy((char*)buf+i, text->length(), 0);
    i += writeCharToBuf(buf+i, 0);
    return i;
}

// 定义一个结构体 DrawObject，用于存储绘图对象的信息
// id 表示绘图对象的标识符，buf 表示绘图对象的缓冲区内容
struct DrawObject {
    string id;
    string buf;
};

// 定义 RVSender 类，用于向 RoboViz 发送绘图命令
class RVSender
{
private:
    // 静态成员变量，记录队伍的方向（左或右）
    static int side; 
    // 静态成员变量，记录球员的统一编号
    static int uNum; 
    // 静态成员变量，用于生成唯一的标识符编号
    static long uniqueIdNum; 
    // 存储绘图对象的映射，键为绘图对象的名称，值为绘图对象的缓冲区内容
    map<string, string> drawings; 
    // 存储队伍方向映射的映射，键为方向编号，值为方向名称
    map<int, string> sideMap; 
    // 根据球员编号和队伍方向获取队伍代理字符
    char getTeamAgent(int uNum, int side); 
    // 根据球员编号和队伍方向获取对应的颜色值
    void getColor(int uNum, int side, float &r, float &g, float &b); 
    // 获取自身的标识符
    string getMyId(); 
    // 根据绘图名称获取绘图对象的标识符
    string getDrawingId(const string* name); 
    // 根据唯一编号获取唯一标识符
    string getUniqueId(long unique); 
    // 生成一个新的唯一标识符
    string getUniqueId(); 
    // 翻转多边形的顶点顺序，用于处理右侧队伍的情况
    void flipPolygon(float *v, const int numVerts); 
    // 更新绘图对象的信息
    void updateDrawings(string id, string buf); 

    // 套接字文件描述符，用于网络通信
    int sockfd; 
    // 地址信息结构体，存储目标地址和端口等信息
    struct addrinfo p; 
    // 标志位，指示套接字是否已创建
    bool socketCreated; 

    // 以下是一系列创建不同绘图对象缓冲区的私有成员函数
    // 创建一个交换缓冲区的绘图对象缓冲区
    unsigned char* newBufferSwap(const string* name, int* bufSize); 
    // 创建一个圆形绘图对象缓冲区
    unsigned char* newCircle(const float* center,
                             float radius, float thickness,
                             const float* color,
                             const string* setName,
                             int *bufSize); 
    // 创建一个线段绘图对象缓冲区
    unsigned char* newLine(const float* a,
                           const float* b,
                           float thickness,
                           const float* color,
                           const string* setName,
                           int* bufSize); 
    // 创建一个点绘图对象缓冲区
    unsigned char* newPoint(const float* p,
                            float size,
                            const float* color,
                            const string* setName,
                            int* bufSize); 
    // 创建一个球体绘图对象缓冲区
    unsigned char* newSphere(const float* p,
                             float radius,
                             const float* color,
                             const string* setName,
                             int* bufSize); 
    // 创建一个多边形绘图对象缓冲区
    unsigned char* newPolygon(const float* v,
                              int numVerts,
                              const float* color,
                              const string* setName,
                              int* bufSize); 
    // 创建一个注释绘图对象缓冲区
    unsigned char* newAnnotation(const string *txt,
                                 const float *p,
                                 const float* color,
                                 const string* setName,
                                 int* bufSize); 
    // 创建一个球员注释绘图对象缓冲区
    unsigned char* newAgentAnnotation(const string *txt,
                                      const char teamAgent,
                                      const float* color,
                                      int* bufSize); 
    // 创建一个移除球员注释绘图对象缓冲区
    unsigned char* newRemoveAgentAnnotation(const char teamAgent,
                                            int* bufSize); 
    // 创建一个选择球员绘图对象缓冲区
    unsigned char* newSelectAgent(const char teamAgent,
                                  int* bufSize); 

public:
    // 默认构造函数
    RVSender(); 
    // 带参数的构造函数，初始化套接字文件描述符和地址信息结构体
    RVSender(int sockfd_, struct addrinfo p); 
    // 析构函数
    ~RVSender(); 

    // 获取套接字文件描述符
    inline int getSockFD() {
        return sockfd;
    }
    // 获取地址信息结构体
    inline struct addrinfo getP() {
        return p;
    }

    // 设置球员的统一编号
    inline void setUNum(const int &uNum) {
        this->uNum = uNum;
    }
    // 设置队伍的方向
    inline void setSide(const int &side) {
        this->side = side;
    }

    // 判断是否已经初始化（队伍方向和球员编号是否都已设置）
    inline bool isInit() {
        return side != -1 && uNum != -1;
    }

    // 定义颜色枚举类型，包含多种颜色选项
    // 该枚举类型必须与 getColor 函数中的 switch 语句匹配
    enum Color {
        RED       = 1, ORANGE      =  2, YELLOW    =  3, GREEN     = 4,
        BLUEGREEN = 5, LIGHTBLUE   =  6, BLUE      =  7, DARKBLUE  = 8,
        VIOLET    = 9, PINK        = 10, MAGENTA   = 11
    };

    // 定义默认颜色，这里选择红色
    static const Color DEFAULT_COLOR = RED;

    /*
     *  以下是支持动画的绘图命令
     *
     *  每次调用绘图命令时，会更新一个映射，键为绘图对象的名称（每个命令的第一个参数），
     *  值为绘图命令的缓冲区内容（无需了解具体细节）。
     *
     *  调用 refresh() 函数将这些缓冲区内容写入屏幕，建议将 refresh() 放在 NaoBehavior::Think() 中。
     *
     *  调用 clear() 函数擦除缓冲区中的内容，这样可以移除旧的图形，避免屏幕杂乱。
     *  建议将 clear() 放在 NaoBehavior::selectSkill() 中。
     *
     *  为图形提供唯一的 'string name' 参数很重要，使用相同 'string name' 参数创建的图形会覆盖旧的图形，
     *  这就是动画的实现方式。
     *
     *  可以使用 RVSender::Color 枚举类型指定颜色，也可以指定自己的 r, g, b 值。
     *  如果不指定颜色，将默认使用与球员 uNum 对应的颜色（非常有用）。
     */

    // 将绘图映射中的所有元素绘制到屏幕上
    void refresh(); 
    // 擦除绘图映射中的所有元素
    void clear(); 

    // 绘制圆形，指定名称、圆心坐标、半径和颜色（默认使用默认颜色）
    void drawCircle(string name, double x, double y, double radius,
                    RVSender::Color c = DEFAULT_COLOR);
    // 绘制圆形，指定名称、圆心坐标、半径和自定义的 r, g, b 颜色值
    void drawCircle(string name, double x, double y, double radius, float r, float g, float b);

    // 绘制线段，指定名称、起点和终点坐标、颜色（默认使用默认颜色）
    void drawLine(string name, double x1, double y1, double x2, double y2,
                  RVSender::Color c = DEFAULT_COLOR);
    // 绘制线段，指定名称、起点和终点坐标、自定义的 r, g, b 颜色值
    void drawLine(string name, double x1, double y1, double x2, double y2, float r, float g, float b);

    // 绘制文本，指定名称、文本内容、坐标和颜色（默认使用默认颜色）
    void drawText(string name, string text, double x, double y,
                  RVSender::Color c = DEFAULT_COLOR);
    // 绘制文本，指定名称、文本内容、坐标和自定义的 r, g, b 颜色值
    void drawText(string name, string text, double x, double y, float r, float g, float b);

    // 绘制点，指定名称、坐标、半径和颜色（默认使用默认颜色）
    void drawPoint(string name, double x, double y, double radius,
                   RVSender::Color c = DEFAULT_COLOR);
    // 绘制点，指定名称、坐标、半径和自定义的 r, g, b 颜色值
    void drawPoint(string name, double x, double y, double radius, float r, float g, float b);

    // 绘制球体，指定名称、坐标、半径和颜色（默认使用默认颜色）
    void drawSphere(string name, double x, double y, double z, double radius,
                    RVSender::Color c = DEFAULT_COLOR);
    // 绘制球体，指定名称、坐标、半径和自定义的 r, g, b 颜色值
    void drawSphere(string name, double x, double y, double z, double radius,
                    float r, float g, float b);

    // 绘制多边形，指定名称、顶点数组和顶点数量
    void drawPolygon(string name, float *v, int numVerts);
    // 绘制多边形，指定名称、顶点数组、顶点数量和透明度
    void drawPolygon(string name, float *v, int numVerts, float a);
    // 绘制多边形，指定名称、顶点数组、顶点数量、颜色（默认使用默认颜色）和透明度
    void drawPolygon(string name, float *v, int numVerts, RVSender::Color c = DEFAULT_COLOR,
                     float a=1.0f);
    // 绘制多边形，指定名称、顶点数组、顶点数量、自定义的 r, g, b 颜色值和透明度
    void drawPolygon(string name, float *v, int numVerts, float r, float g, float b, float a=1.0f);

    /*
     *  以下是用于绘制静态图形的命令，这些图形会一直显示在屏幕上，
     *  直到调用 clearStaticDrawings() 函数移除所有图形。可以使用定时器调用该函数，
     *  以保持屏幕上只显示最近的绘图历史。
     */

    // 清除所有静态图形
    void clearStaticDrawings();

    // 绘制圆形，指定圆心坐标、半径和颜色（默认使用默认颜色）
    void drawCircle(double x, double y, double radius,
                    RVSender::Color c = DEFAULT_COLOR);
    // 绘制圆形，指定圆心坐标、半径和自定义的 r, g, b 颜色值
    void drawCircle(double x, double y, double radius, float r, float g, float b);

    // 绘制线段，指定起点和终点坐标、颜色（默认使用默认颜色）
    void drawLine(double x1, double y1, double x2, double y2,
                  RVSender::Color c = DEFAULT_COLOR);
    // 绘制线段，指定起点和终点坐标、自定义的 r, g, b 颜色值
    void drawLine(double x1, double y1, double x2, double y2, float r, float g, float b);

    // 这两个函数在 RoboViz 中会导致奇怪的错误，暂时注释掉
    /*void drawText(string text, double x, double y,
                    RVSender::Color c=(RVSender::Color)uNum);
    void drawText(string text, double x, double y, float r, float g, float b);*/

    // 绘制点，指定坐标、半径和颜色（默认使用默认颜色）
    void drawPoint(double x, double y, double radius,
                   RVSender::Color c = DEFAULT_COLOR);
    // 绘制点，指定坐标、半径和自定义的 r, g, b 颜色值
    void drawPoint(double x, double y, double radius, float r, float g, float b);

    // 绘制球体，指定坐标、半径和颜色（默认使用默认颜色）
    void drawSphere(double x, double y, double z, double radius,
                    RVSender::Color c = DEFAULT_COLOR);
    // 绘制球体，指定坐标、半径和自定义的 r, g, b 颜色值
    void drawSphere(double x, double y, double z, double radius,
                    float r, float g, float b);

    // 绘制多边形，指定顶点数组和顶点数量
    void drawPolygon(float *v, int numVerts);
    // 绘制多边形，指定顶点数组、顶点数量和透明度
    void drawPolygon(float *v, int numVerts, float a);
    // 绘制多边形，指定顶点数组、顶点数量、颜色和透明度
    void drawPolygon(float *v, int numVerts, RVSender::Color c, float a=1.0f);
    // 绘制多边形，指定顶点数组、顶点数量、自定义的 r, g, b 颜色值和透明度
    void drawPolygon(float *v, int numVerts, float r, float g, float b, float a=1.0f);

    /*
     *  以下是绘制球员文本的命令，这些命令的工作方式不同，不涉及动画，
     *  不需要使用 refresh() 和 clear() 命令，也不需要唯一的 'string name' 参数。
     */

    // 绘制文本，指定文本内容、坐标和颜色（默认使用默认颜色）
    void drawText(string text, double x, double y,
                  RVSender::Color c = DEFAULT_COLOR);
    // 绘制文本，指定文本内容、坐标和自定义的 r, g, b 颜色值
    void drawText(string text, double x, double y, float r, float g, float b);

    // 向自身绘制文本，指定文本内容和颜色（默认使用默认颜色）
    void drawAgentText(string text, RVSender::Color c = DEFAULT_COLOR);
    // 向自身绘制文本，指定文本内容和自定义的 r, g, b 颜色值
    void drawAgentText(string text, float r, float g, float b);

    // 向指定统一编号的队友绘制文本，指定文本内容、球员编号和颜色（默认使用默认颜色）
    void drawAgentText(string text, int uNum, RVSender::Color c = DEFAULT_COLOR);
    // 向指定统一编号的队友绘制文本，指定文本内容、球员编号和自定义的 r, g, b 颜色值
    void drawAgentText(string text, int uNum, float r, float g, float b);

    // 向指定队伍方向和统一编号的球员绘制文本，指定文本内容、球员编号、队伍方向和颜色（默认使用默认颜色）
    void drawAgentText(string text, int uNum, int side, RVSender::Color c = DEFAULT_COLOR);
    // 向指定队伍方向和统一编号的球员绘制文本，指定文本内容、球员编号、队伍方向和自定义的 r, g, b 颜色值
    void drawAgentText(string text, int uNum, int side, float r, float g, float b);

    // 移除球员文本，默认移除自身的文本，也可以指定队友或场上的任何球员
    void removeAgentText(int u = -1, int s = -1);

    // 选择指定球员，默认选择自身，也可以指定队友或场上的任何球员
    void selectAgent(int u = -1, int s = -1);

    /* 旧的绘图命令，不支持动画 */
    // 交换缓冲区
    void swapBuffers(const string* setName);
    // 绘制线段，指定起点和终点坐标、线宽、颜色和绘图集合名称
    void drawLine(float x1, float y1, float z1,
                  float x2, float y2, float z2,
                  float thickness,
                  float r, float g, float b,
                  const string* setName);
    // 绘制圆形，指定圆心坐标、半径、线宽、颜色和绘图集合名称
    void drawCircle(float x, float y, float radius,
                    float thickness,
                    float r, float g, float b,
                    const string* setName);
    // 绘制球体，指定坐标、半径、颜色和绘图集合名称
    void drawSphere(float x, float y, float z,
                    float radius,
                    float r, float g, float b,
                    const string* setName);
    // 绘制点，指定坐标、点大小、颜色和绘图集合名称
    void drawPoint(float x, float y, float z,
                   float size,
                   float r, float g, float b,
                   const string *setName);
    // 绘制多边形，指定顶点数组、顶点数量、颜色、透明度和绘图集合名称
    void drawPolygon(const float *v, int numVerts,
                     float r, float g, float b,
                     float a,
                     const string *setName);
    // 绘制注释，指定注释文本、坐标、颜色和绘图集合名称
    void drawAnnotation(const string *txt,
                        float x, float y, float z,
                        float r, float g, float b,
                        const string *setName);
    // 绘制球员注释，指定注释文本、队伍代理字符和颜色
    void drawAgentAnnotation(const string *txt,
                             char teamAgent,
                             float r, float g, float b);
    // 移除球员注释，指定队伍代理字符
    void removeAgentAnnotation(char teamAgent);
    // 选择球员，指定队伍代理字符
    void selectAgent(char teamAgent);
};

// 结束头文件保护
#endif // !RVDRAW_H