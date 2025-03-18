#include "worldmodel.h"
#include "../kalman/BallKF.h"
#include "../kalman/PlayerKF.h"

// 世界模型类的构造函数
WorldModel::WorldModel() {
    // 初始化周期数为 0
    cycle = 0;
    // 初始化左队得分
    scoreLeft = -1;
    // 初始化右队得分
    scoreRight = -1;
    // 初始化时间
    time = -1.0;
    // 初始化游戏时间
    gameTime = -1.0;
    // 初始化比赛模式为开球前
    playMode = PM_BEFORE_KICK_OFF;
    // 初始化上一次比赛模式为比赛结束
    lastPlayMode = PM_GAME_OVER;
    // 初始化上一次不同的比赛模式为比赛结束
    lastDifferentPlayMode = PM_GAME_OVER;

    // 初始化球员编号（占位值）
    uNum = 1; 
    // 标记球员编号是否已设置
    uNumSet = false;

    // 初始化球员所在边（占位值）
    side = SIDE_LEFT; 
    // 标记球员所在边是否已设置
    sideSet = false;

    // 初始化上一次技能动作（占位值）
    lastSkills.push_back( SKILL_STAND ); 
    lastSkills.push_back( SKILL_STAND ); 

    // 初始化上一次里程计位置
    lastOdometryPos = SIM::Point2D(0,0);
    // 初始化上一次里程计角度
    lastOdometryAngDeg = 0;

    // 初始化世界物体数组
    for (int i = 0; i < NUM_WORLD_OBJS; ++i) {
        WorldObject& obj = worldObjects[i];
        obj.id = i;
    }

    // 标记是否已定位
    fLocalized = false;
    // 标记是否摔倒
    fFallen = false;

    // TODO: 这样初始化不太正确，但定位模块应该会更新这些值
    // 有没有更好的初始化方法呢？
    // 初始化自身位置
    myPosition.setVecPosition( 0, 0, 0 );
    // 初始化上一次自身位置
    myLastPosition.setVecPosition( 0, 0, 0 );
    // 初始化自身角度
    myAngDegrees = 0;

#ifdef GROUND_TRUTH_SERVER
    // 初始化自身真实位置
    myPositionGroundTruth.setVecPosition(0, 0, 0);
    // 初始化自身真实角度
    myAngGroundTruth = 0;
    // 初始化球的真实位置
    ballGroundTruth.setVecPosition(0, 0, 0);
#endif

    // 初始化上一次看到球的时间
    lastBallSightingTime = -100;

    // 初始化上三次看到球的位置
    lastBallSeenPosition = vector<VecPosition>(3,VecPosition(0,0,0));
    // 初始化上三次看到球的时间
    lastBallSeenTime = vector<double>(3,0);

    // 初始化上一次看到线的时间
    lastLineSightingTime = -100;

    // 初始化局部到全局的变换矩阵
    localToGlobal = HCTMatrix();
    // 初始化全局到局部的变换矩阵
    globalToLocal = HCTMatrix();

    // 初始化队友摔倒状态数组
    fallenTeammate = vector<bool>(NUM_AGENTS);
    // 初始化对手摔倒状态数组
    fallenOpponent = vector<bool>(NUM_AGENTS);
    for (int i = 0; i < NUM_AGENTS; i++) {
        fallenTeammate[i] = false;
        fallenOpponent[i] = false;
    }

    // 创建 RVSender 对象
    rvsend = new RVSender();
    // 检查套接字文件描述符是否有效
    if (rvsend->getSockFD() == -1)
    {
        perror("WorldModel");
        delete rvsend;
        rvsend = NULL;
    }

    // 标记是否使用真实数据进行定位
    fUseGroundTruthDataForLocalization = false;

    // 初始化对手队名
    opponentTeamName = "";

    // 标记是否有信心
    confident = false;

    // 创建球的卡尔曼滤波器
    ballKalmanFilter = new BallKF(this);
    // 创建对手的卡尔曼滤波器
    opponentKalmanFilters = new PlayerKF(this);

    // 初始化队友最后听到消息的时间
    for (unsigned int i = 0; i < NUM_AGENTS; i++) {
        setTeammateLastHeardTime(i, -1);
    }
}

// 世界模型类的析构函数
WorldModel::~WorldModel() {
    if (rvsend != NULL)
        delete rvsend;

    delete ballKalmanFilter;
    delete opponentKalmanFilters;
}

// 显示世界模型的信息
void WorldModel::display() {
    cout << "****************世界模型信息******************************\n";
    std::cout << "周期数: " << cycle << "\n";
    std::cout << "左队得分: " << scoreLeft << "\n";
    std::cout << "右队得分: " << scoreRight << "\n";
    std::cout << "时间: " << time << "\n";
    std::cout << "游戏时间: " << gameTime << "\n";
    std::cout << "比赛模式: " << playMode << "\n";

    std::cout << "球员编号是否设置: " << uNumSet << ", 球员编号: " << uNum << "\n";
    std::cout << "所在边是否设置: " << sideSet << ", 所在边: " << side << "\n";

    for (int i = WO_BALL; i < NUM_WORLD_OBJS; ++i) {
        WorldObject& obj = worldObjects[i];
        cout << "世界物体: " << WorldObjType2Str[i] << ":\n"
             << obj.pos << '\n'
             << " 当前是否可见: " << ( obj.currentlySeen ? "是" : "否" ) << '\n'
             << " 上次看到的周期: " << obj.cycleLastSeen
             << " 上次看到的时间: " << obj.timeLastSeen << endl;
    }

    cout << "*********************************************************\n";
}

// 更新球门柱和旗帜的位置
void WorldModel::updateGoalPostsAndFlags() {
    VecPosition fieldXPlusYPlus, fieldXPlusYMinus, fieldXMinusYPlus, fieldXMinusYMinus;
    if(side == SIDE_LEFT) {
        // 左侧时，设置旗帜位置
        worldObjects[FLAG_1_R].pos.setVecPosition( HALF_FIELD_X,
                HALF_FIELD_Y,
                0 );
        worldObjects[FLAG_2_R].pos.setVecPosition(  HALF_FIELD_X,
                -HALF_FIELD_Y,
                0 );
        worldObjects[FLAG_1_L].pos.setVecPosition( -HALF_FIELD_X,
                HALF_FIELD_Y,
                0 );
        worldObjects[FLAG_2_L].pos.setVecPosition( -HALF_FIELD_X,
                -HALF_FIELD_Y,
                0 );
        // 左侧时，设置球门柱位置
        worldObjects[GOALPOST_1_R].pos.setVecPosition( HALF_FIELD_X,
                HALF_GOAL_Y,
                GOAL_Z);
        worldObjects[GOALPOST_2_R].pos.setVecPosition(  HALF_FIELD_X,
                -HALF_GOAL_Y,
                GOAL_Z);
        worldObjects[GOALPOST_1_L].pos.setVecPosition( -HALF_FIELD_X,
                HALF_GOAL_Y,
                GOAL_Z);
        worldObjects[GOALPOST_2_L].pos.setVecPosition( -HALF_FIELD_X,
                -HALF_GOAL_Y,
                GOAL_Z);

    }
    else { // 右侧时
        // 右侧时，设置旗帜位置
        worldObjects[FLAG_1_R].pos.setVecPosition( -HALF_FIELD_X,
                -HALF_FIELD_Y,
                0 );
        worldObjects[FLAG_2_R].pos.setVecPosition( -HALF_FIELD_X,
                HALF_FIELD_Y,
                0 );
        worldObjects[FLAG_1_L].pos.setVecPosition(  HALF_FIELD_X,
                -HALF_FIELD_Y,
                0 );
        worldObjects[FLAG_2_L].pos.setVecPosition( HALF_FIELD_X,
                HALF_FIELD_Y,
                0 );
        // 右侧时，设置球门柱位置
        worldObjects[GOALPOST_1_R].pos.setVecPosition( -HALF_FIELD_X,
                -HALF_GOAL_Y,
                GOAL_Z );
        worldObjects[GOALPOST_2_R].pos.setVecPosition( -HALF_FIELD_X,
                HALF_GOAL_Y,
                GOAL_Z);
        worldObjects[GOALPOST_1_L].pos.setVecPosition(  HALF_FIELD_X,
                -HALF_GOAL_Y,
                GOAL_Z );
        worldObjects[GOALPOST_2_L].pos.setVecPosition( HALF_FIELD_X,
                HALF_GOAL_Y,
                GOAL_Z );
    }
}

// 根据场地的四个点更新局部到全局和全局到局部的变换矩阵，并更新移动物体的坐标
void WorldModel::
updateMatricesAndMovingObjs( VecPosition& fieldXPlusYPlus,
                             VecPosition& fieldXPlusYMinus,
                             VecPosition& fieldXMinusYPlus,
                             VecPosition& fieldXMinusYMinus ) {
    // 计算局部场地中心位置
    VecPosition localFieldCentre = (fieldXPlusYPlus + fieldXMinusYMinus) * 0.5;
    // 计算局部场地 X 轴方向向量
    VecPosition localFieldXDirection = (fieldXPlusYPlus - fieldXMinusYPlus).normalize();
    // 计算局部场地 Y 轴方向向量
    VecPosition localFieldYDirection = (fieldXPlusYPlus - fieldXPlusYMinus).normalize();
    // 计算局部场地 Z 轴方向向量
    VecPosition localFieldZDirection = (localFieldXDirection.crossProduct(localFieldYDirection)).normalize();

    // 设置局部到全局变换矩阵
    setLocalToGlobal(0, 0, localFieldXDirection.getX());
    setLocalToGlobal(0, 1, localFieldXDirection.getY());
    setLocalToGlobal(0, 2, localFieldXDirection.getZ());
    setLocalToGlobal(0, 3, -(localFieldCentre.dotProduct(localFieldXDirection)));

    setLocalToGlobal(1, 0, localFieldYDirection.getX());
    setLocalToGlobal(1, 1, localFieldYDirection.getY());
    setLocalToGlobal(1, 2, localFieldYDirection.getZ());
    setLocalToGlobal(1, 3, -(localFieldCentre.dotProduct(localFieldYDirection)));

    setLocalToGlobal(2, 0, localFieldZDirection.getX());
    setLocalToGlobal(2, 1, localFieldZDirection.getY());
    setLocalToGlobal(2, 2, localFieldZDirection.getZ());
    setLocalToGlobal(2, 3, -(localFieldCentre.dotProduct(localFieldZDirection)));

    setLocalToGlobal(3, 0, 0);
    setLocalToGlobal(3, 1, 0);
    setLocalToGlobal(3, 2, 0);
    setLocalToGlobal(3, 3, 1.0);

    // 定义局部身体中心位置
    VecPosition localBodyCentre = VecPosition(0, 0, 0);
    // 定义局部身体 X 轴方向向量
    VecPosition localBodyXDirection = VecPosition(1.0, 0, 0);
    // 定义局部身体 Y 轴方向向量
    VecPosition localBodyYDirection = VecPosition(0, 1.0, 0);
    // 定义局部身体 Z 轴方向向量
    VecPosition localBodyZDirection = VecPosition(0, 0, 1.0);

    // 计算全局身体中心位置
    VecPosition bodyCentre = l2g(localBodyCentre);
    // 计算全局身体 X 轴方向向量
    VecPosition bodyXDirection = (l2g(localBodyXDirection) - bodyCentre).normalize();
    // 计算全局身体 Y 轴方向向量
    VecPosition bodyYDirection = (l2g(localBodyYDirection) - bodyCentre).normalize();
    // 计算全局身体 Z 轴方向向量
    VecPosition bodyZDirection = (l2g(localBodyZDirection) - bodyCentre).normalize();

    // 设置全局到局部变换矩阵
    setGlobalToLocal(0, 0, bodyXDirection.getX());
    setGlobalToLocal(0, 1, bodyXDirection.getY());
    setGlobalToLocal(0, 2, bodyXDirection.getZ());
    setGlobalToLocal(0, 3, -(bodyCentre.dotProduct(bodyXDirection)));

    setGlobalToLocal(1, 0, bodyYDirection.getX());
    setGlobalToLocal(1, 1, bodyYDirection.getY());
    setGlobalToLocal(1, 2, bodyYDirection.getZ());
    setGlobalToLocal(1, 3, -(bodyCentre.dotProduct(bodyYDirection)));

    setGlobalToLocal(2, 0, bodyZDirection.getX());
    setGlobalToLocal(2, 1, bodyZDirection.getY());
    setGlobalToLocal(2, 2, bodyZDirection.getZ());
    setGlobalToLocal(2, 3, -(bodyCentre.dotProduct(bodyZDirection)));

    setGlobalToLocal(3, 0, 0);
    setGlobalToLocal(3, 1, 0);
    setGlobalToLocal(3, 2, 0);
    setGlobalToLocal(3, 3, 1.0);

    // 设置移动物体的坐标
    for( int i = WO_BALL; i <= WO_OPPONENT_FOOT_R11; ++i ) {
        WorldObject* pObj = getWorldObject(i);
        if( pObj->currentlySeen ) {
            // 计算物体的局部原点位置
            VecPosition objLocalOrigin = pObj->vision.polar.getCartesianFromPolar();
            // 计算物体的全局位置
            VecPosition objGlobal = l2g( objLocalOrigin );

            if( pObj->id == WO_BALL ) {
                // 如果是球，设置球的位置
                setBall( objGlobal );
            } else if( WO_OPPONENT1 <= pObj->id && pObj->id <= WO_OPPONENT11 ) {
                // 如果是对手，设置对手的位置
                setOpponent( pObj->id, objGlobal );
            } else if( WO_TEAMMATE1 <= pObj->id && pObj->id <= WO_TEAMMATE11 ) {
                // 如果是队友，设置队友的位置
                setTeammate( pObj->id, objGlobal );
            } else if( WO_TEAMMATE_HEAD1 <= pObj->id && pObj->id <= WO_OPPONENT_FOOT_R11 ) {
                // 如果是其他物体，设置物体的位置
                setObjectPosition( pObj->id, objGlobal );
            }
        }
    }
}