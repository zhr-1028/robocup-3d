/*
2015年RoboCup 3D仿真赛中的可选通信协议
用于临时球员挑战赛
*/
/*这段代码是RoboCup 3D仿真比赛中临时球员挑战赛的核心通信协议实现。它通过高效的编码和解码机制，
让球员在有限的通信能力下共享关键信息，从而实现团队协作和策略执行。这种通信机制在比赛中至关重要，能够显著提升团队的表现*/

#ifndef _AUDIO_H
#define _AUDIO_H

#include <string>
#include <vector>

// 通信字母表必须包含64个符号。
const std::string commAlphabet =  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789*#";

/*-------------------------------工具函数---------------------------------------------*/
std::vector<int> intToBits(const int &n, const int &numBits);  // 将整数转换为二进制位
std::vector<int> intToBits(const unsigned long long &n, const int &numBits);  // 将长整数转换为二进制位
int bitsToInt(const std::vector<int> &bits, const int &start, const int &end);  // 将二进制位转换为整数

/*-------------------------------编码---------------------------------------------*/
bool makeSayMessage(const int &uNum, const double &currentServerTime, const double &ballLastSeenServerTime, const double &ballX, const double &ballY, const double &myX, const double &myY, const bool &fFallen, std::string &message);  // 生成发言消息
bool dataToBits(const double &time, const double &ballLastSeenTime, const double &ballX, const double &ballY, const double &myX, const double &myY, const bool &fFallen, std::vector<int> &bits);  // 将数据转换为二进制位
bool bitsToString(const std::vector<int> &bits, std::string &message);  // 将二进制位转换为字符串

/*-------------------------------解码---------------------------------------------*/
bool processHearMessage(const std::string &message, const double &heardServerTime, int &uNum, double &ballLastSeenServerTime, double &ballX, double &ballY, double &agentX, double &agentY, bool &fFallen, double &time);  // 解析听到的消息
bool bitsToData(const std::vector<int> &bits, double &time, double &ballLastSeenTime, double &ballX, double &ballY, double &agentX, double &agentY, bool &fFallen);  // 将二进制位转换为数据
bool stringToBits(const std::string &message, std::vector<int> &bits);  // 将字符串转换为二进制位

#endif