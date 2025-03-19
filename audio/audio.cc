/*
2015年RoboCup 3D仿真赛中的可选通信协议
用于临时球员挑战赛
*/
/*这段代码是RoboCup 3D仿真比赛中临时球员挑战赛的核心通信协议实现。它通过高效的编码和解码机制，
让球员在有限的通信能力下共享关键信息，从而实现团队协作和策略执行。这种通信机制在比赛中至关重要，能够显著提升团队的表现*/

#include <iostream>
#include "audio.h"

#include "../headers/Field.h"


using namespace std;

//#define HALF_FIELD_X 15.0
//#define HALF_FIELD_Y 10.0
//#define NUM_AGENTS 11

/*
  这些是球和球员的x和y坐标的限制范围。
*/
double minBallX = -HALF_FIELD_X - 2.0;  // 球的最小x坐标
double maxBallX = HALF_FIELD_X + 2.0;  // 球的最大x坐标
double minBallY = -HALF_FIELD_Y - 2.0;  // 球的最小y坐标
double maxBallY = HALF_FIELD_Y + 2.0;  // 球的最大y坐标

double minAgentX = -HALF_FIELD_X - 5.0;  // 球员的最小x坐标
double maxAgentX = HALF_FIELD_X + 5.0;  // 球员的最大x坐标
double minAgentY = -HALF_FIELD_Y - 5.0;  // 球员的最小y坐标
double maxAgentY = HALF_FIELD_Y + 5.0;  // 球员的最大y坐标

// ------ 接口方法（调用这些方法） ------

/**
 * 如果当前是球员发言的时间，创建发言消息。
 * uNum - 球员的编号（1-11）
 * currentServerTime - 当前服务器时间，不是比赛时间
 * ballLastSeenServerTime - 上次看到球的服务器时间，不是上次比赛时间
 * ballX - 球的全局X坐标
 * ballY - 球的全局Y坐标
 * myX - 我的全局X坐标
 * myY - 我的全局Y坐标
 * fFallen - 我是否当前处于倒地状态
 * message - 用于返回发言消息的字符串
 * RETURN - 如果消息创建成功返回true，否则返回false
 */
bool makeSayMessage(const int &uNum, const double &currentServerTime, const double &ballLastSeenServerTime, const double &ballX, const double &ballY, const double &myX, const double &myY, const bool &fFallen, string &message) {
    int cycles = int((currentServerTime * 50) + 0.1);
    if (cycles % (NUM_AGENTS*2) != (uNum-1)*2) {
        // 不是我们发言的时间段
        return false;
    }

    vector<int> bits;
    if(!(dataToBits(currentServerTime, ballLastSeenServerTime, ballX, ballY, myX, myY, fFallen, bits))) {
        return false;
    }

    if(!(bitsToString(bits, message))) {
        return false;
    }

    message = "(say " + message + ")";

    return true;
}

/**
 * 解析hear感知器的消息体（需要先去掉hear头部、时间和自我/方向信息）。
 * message - 要处理的消息
 * heardServerTime - 听到消息的服务器时间，不是比赛时间
 * uNum - 发言球员的编号
 * ballLastSeenServerTime - 发言球员看到球的服务器时间，不是比赛时间
 * ballX - 报告的球的全局X坐标
 * ballY - 报告的球的全局Y坐标
 * agentX - 发言球员报告的全局X坐标
 * agentY - 发言球员报告的全局Y坐标
 * fFallen - 发言球员是否当前处于倒地状态
 * time - 消息发出的时间
 * RETURN - 如果成功解析消息返回true，否则返回false
 */
bool processHearMessage(const string &message, const double &heardServerTime, int &uNum, double &ballLastSeenServerTime, double &ballX, double &ballY, double &agentX, double &agentY, bool &fFallen, double &time) {

    // 初始化值
    uNum = 0;
    ballLastSeenServerTime = 0;
    ballX = 0;
    ballY = 0;
    agentX = 0;
    agentY = 0;
    fFallen = false;
    time = 0;


    vector<int> bits;
    if(!(stringToBits(message, bits))) {
        return false;
    }

    if(!(bitsToData(bits, time, ballLastSeenServerTime, ballX, ballY, agentX, agentY, fFallen))) {
        return false;
    }

    time += double(int((heardServerTime-time)/1310.72))*1310.72;
    ballLastSeenServerTime +=  double(int((heardServerTime-ballLastSeenServerTime)/1310.72))*1310.72;

    if (heardServerTime-time >= .07 || heardServerTime-time < -.001) {
        return false;
    }

    int cycles = int((time * 50) + 0.1);
    uNum = (cycles%(NUM_AGENTS*2))/2+1;

    return true;
}


//------ 实现方法（由接口方法调用） ------

vector<int> intToBits(const int &n, const int &numBits) {

    vector<int> bits;

    if(n < 0) {
        bits.clear();
        return bits;
    }

    int m = n; // 复制

    bits.resize(numBits);
    for(int i = numBits - 1; i >= 0; i--) {
        bits[i] = m % 2;
        m /= 2;
    }

    return bits;
}

vector<int> intToBits(const unsigned long long &n, const int &numBits) {
    vector<int> bits;

    unsigned long long m = n; // 复制

    bits.resize(numBits);
    for(int i = numBits - 1; i >= 0; i--) {
        bits[i] = m % 2;
        m /= 2;
    }

    return bits;
}

int bitsToInt(const vector<int> &bits, const int &start, const int &end) {

    if(start < 0 || end >= (int)bits.size()) {
        return 0;// 错误
    }

    int n = 0;
    for(int i = start; i <= end; i++) {
        n *= 2;
        n += bits[i];
    }

    return n;
}

bool dataToBits(const double &time, const double &ballLastSeenTime, const double &ballX, const double &ballY, const double &myX, const double &myY, const bool &fFallen, vector<int> &bits) {

    int cycles = (time * 50) + 0.1;
    cycles = cycles%(1<<16);
    vector<int> timeBits = intToBits(cycles, 16);

    int ballLastSeenCycle = (ballLastSeenTime * 50) + 0.1;
    ballLastSeenCycle = ballLastSeenCycle%(1<<16);
    vector<int> ballLastSeenTimeBits = intToBits(ballLastSeenCycle, 16);

    double clippedBallX = (ballX < minBallX)? minBallX : ((ballX > maxBallX)? maxBallX : ballX);
    int bx = (((clippedBallX - minBallX) * 1023) / (maxBallX - minBallX)) + 0.5;
    vector<int> ballXBits = intToBits(bx, 10);

    double clippedBallY = (ballY < minBallY)? minBallY : ((ballY > maxBallY)? maxBallY : ballY);
    int by = (((clippedBallY - minBallY) * 1023) / (maxBallY - minBallY)) + 0.5;
    vector<int> ballYBits = intToBits(by, 10);

    double clippedMyX = (myX < minAgentX)? minAgentX : ((myX > maxAgentX)? maxAgentX : myX);
    int mx = (((clippedMyX - minAgentX) * 1023) / (maxAgentX - minAgentX)) + 0.5;
    vector<int> myXBits = intToBits(mx, 10);

    double clippedMyY = (myY < minAgentY)? minAgentY : ((myY > maxAgentY)? maxAgentY : myY);
    int my = (((clippedMyY - minAgentY) * 1023) / (maxAgentY - minAgentY)) + 0.5;
    vector<int> myYBits = intToBits(my, 10);

    int fallenBit = (fFallen)? 1 : 0;

    bits.insert(bits.end(), timeBits.begin(), timeBits.end());//16
    bits.insert(bits.end(), ballLastSeenTimeBits.begin(), ballLastSeenTimeBits.end());//16
    bits.insert(bits.end(), ballXBits.begin(), ballXBits.end());//10
    bits.insert(bits.end(), ballYBits.begin(), ballYBits.end());//10
    bits.insert(bits.end(), myXBits.begin(), myXBits.end());//10
    bits.insert(bits.end(), myYBits.begin(), myYBits.end());//10
    bits.push_back(fallenBit);//1

    return true;

}

bool bitsToString(const vector<int> &bits, string &message) {

    message = "";
    if(commAlphabet.size() != 64) {
        cerr << "bitsToString: 字母表大小不是64!\n";
        return false;
    }

    vector<int> index;
    index.resize((bits.size() + 5) / 6);
    size_t ctr = 0;
    for(size_t i = 0; i < index.size(); i++) {

        index[i] = 0;
        for(int j = 0; j < 6; j++) {

            index[i] *= 2;

            if(ctr < bits.size()) {
                index[i] += bits[ctr];
                ctr++;
            }

        }
    }

    for(size_t i = 0; i < index.size(); i++) {
        message += commAlphabet.at(index[i]);
    }

    return true;
}


bool bitsToData(const vector<int> &bits, double &time, double &ballLastSeenTime, double &ballX, double &ballY, double &agentX, double &agentY, bool &fFallen) {
    if(bits.size() < (16 + 16 + 10 + 10 + 10 + 10 + 1)) {
        time = 0;
        ballLastSeenTime = 0,
        ballX = 0;
        ballY = 0;
        agentX = 0;
        agentY = 0;
        fFallen = false;
        return false;
    }

    int ctr = 0;

    int cycles = bitsToInt(bits, ctr, ctr + 15);
    time = cycles * 0.02;
    ctr += 16;

    int ballLastSeenCycles = bitsToInt(bits, ctr, ctr + 15);
    ballLastSeenTime = ballLastSeenCycles * 0.02;
    ctr += 16;

    int bx = bitsToInt(bits, ctr, ctr + 9);
    ballX = minBallX + ((maxBallX - minBallX) * (bx / 1023.0));
    ctr += 10;

    int by = bitsToInt(bits, ctr, ctr + 9);
    ballY = minBallY + ((maxBallY - minBallY) * (by / 1023.0));
    ctr += 10;

    int ax = bitsToInt(bits, ctr, ctr + 9);
    agentX = minAgentX + ((maxAgentX - minAgentX) * (ax / 1023.0));
    ctr += 10;

    int ay = bitsToInt(bits, ctr, ctr + 9);
    agentY = minAgentY + ((maxAgentY - minAgentY) * (ay / 1023.0));
    ctr += 10;

    fFallen = (bits[ctr] == 0)? false : true;
    ctr += 1;

    return true;
}


bool stringToBits(const string &message, vector<int> &bits) {

    if(commAlphabet.size() != 64) {
        cerr << "bits2String: 字母表大小不是64!\n";
        return false;
    }

    bits.resize(message.length() * 6);

    for(size_t i = 0; i < message.length(); i++) {

        // 确保消息中的每个字母都来自我们的字母表。创建一个映射。
        // 如果有任何违规，返回false；
        const char c = message.at(i);
        size_t n = commAlphabet.find(c);
        if(n == string::npos) {
            bits.clear();
            return false;
        }

        for(int j = 5; j >= 0; j--) {
            bits[(i * 6) + j] = n % 2;
            n /= 2;
        }
    }

    return true;
}