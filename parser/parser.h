#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <boost/shared_ptr.hpp>
// 比赛模式常量定义 (参考SPL标准协议)
#define PM_BEFORE_KICK_OFF      0
#define PM_KICK_OFF_LEFT        1
#define PM_KICK_OFF_RIGHT       2
#define PM_PLAY_ON              3
// ... 其他已有模式
#define PM_PENALTY_SETUP       13  // 点球准备阶段
#define PM_PENALTY_READY       14  // 点球就绪状态
#define PM_PENALTY_KICK_LEFT   15  // 左侧点球
#define PM_PENALTY_KICK_RIGHT  16  // 右侧点球

class WorldModel;
class BodyModel;
class PFLocalization;
class VisionObject;
class VecPosition;

#include "../headers/headers.h"

// For UT Walk
class FrameInfoBlock;
class SensorBlock;
class JointBlock;

using namespace std;

class Parser {

private:

    WorldModel *worldModel;
    BodyModel *bodyModel;
    PFLocalization* particleFilter;

    int uNum;
    int side;

    string teamName;

    bool fProcessedVision;

    // For UT Walk
    SensorBlock* sensor_block_;
    JointBlock* joint_angles_block_;
    FrameInfoBlock* frame_info_;
    FrameInfoBlock* vision_frame_info_;

    vector< boost::shared_ptr<VisionObject> > visionObjs;

protected:

    vector<string> tokenise(const string &s);
    vector<string> tokeniseCommaDelim(const string &s);
    bool parseTime(const string &str);
    bool parseGameState(const string &str);
    bool parseGyro(const string &str);
    bool parseAccelerometer(const string &str);
    bool parseHear(const string &str);
    bool parseHingeJoint(const string &str);
    bool parseSee(const string &str);
    bool parseLine(const string &str, vector< boost::shared_ptr<VisionObject> >& visionObjs);
    bool parseGoalPost(const string &str, vector< boost::shared_ptr<VisionObject> >& visionObjs);
    bool parseFlag(const string &str, vector< boost::shared_ptr<VisionObject> >& visionObjs);
    bool parseBall(const string &str, vector< boost::shared_ptr<VisionObject> >& visionObjs);
    bool parsePlayer(const string &str, vector< boost::shared_ptr<VisionObject> >& visionObjs);
    bool parseFRP(const string &str);
    bool parseMyPos(const string &str);
    bool parseBallPos(const string &str);
    vector<string> segment(const string &str, const bool &omitEnds);
    void computeLocalCornerPositions( VecPosition& fieldXPlusYPlus,
                                      VecPosition& fieldXPlusYMinus,
                                      VecPosition& fieldXMinusYPlus,
                                      VecPosition& fieldXMinusYMinus );
public:

    Parser(WorldModel *worldModel, BodyModel *bodyModel, const string &teamName, PFLocalization* particleFilter, FrameInfoBlock* vision_frame_info, FrameInfoBlock* frame_info, JointBlock* joint_block, SensorBlock* sensor_block);

    ~Parser();

    bool parse(const string &input, bool &fParsedVision);
    void processVision();
    void processSightings(bool fIgnoreVision);
};

#endif // PARSER_H

