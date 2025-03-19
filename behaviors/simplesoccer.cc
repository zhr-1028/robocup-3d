#include "simplesoccer.h"

// 构造函数，初始化 SimpleSoccerBehavior 类的对象
SimpleSoccerBehavior::SimpleSoccerBehavior( const std::string teamName,
        int uNum,
        const map<string, string>& namedParams_,
        const string& rsg_)
    : NaoBehavior( teamName,
                   uNum,
                   namedParams_,
                   rsg_) {
}

// 此函数用于确定机器人重新定位（beam）时的位置和角度
void SimpleSoccerBehavior::beam( double& beamX, double& beamY, double& beamAngle ) {
    // 根据机器人的角色获取对应的位置
    VecPosition space = getSpaceForRole(static_cast<Role>(worldModel->getUNum()-1));
    beamX = space.getX();
    beamY = space.getY();
    beamAngle = 0;
}

// 该函数用于选择机器人当前要执行的技能
SkillType SimpleSoccerBehavior::selectSkill() {
    // 获取每个机器人的角色分配
    std::map<int, Role> agentRoles = getAgentRoles();
    /*
    // 绘制角色分配信息
    worldModel->getRVSender()->clear(); // 清除上一周期的绘图信息
    VecPosition space = getSpaceForRole(agentRoles[worldModel->getUNum()]);
    worldModel->getRVSender()->drawLine("me_to_space", me.getX(), me.getY(), space.getX(), space.getY());
    worldModel->getRVSender()->drawPoint("role_space", space.getX(), space.getY(), 10.0f);
    */

    // 如果当前机器人的角色是 ON_BALL（控球角色）
    if (agentRoles[worldModel->getUNum()] == ON_BALL) {
        // 如果机器人距离球大于 1 个单位
        if (me.getDistanceTo(ball) > 1) {
            // 走向球
            return goToTarget(ball);
        } else {
            // 否则，将球踢向对方球门
            return kickBall(KICK_FORWARD, VecPosition(HALF_FIELD_X, 0, 0));
        }
    }

    // 若不是控球角色，则前往对应角色的位置
    return goToSpace(getSpaceForRole(agentRoles[worldModel->getUNum()]));
}

// 此函数用于找出离球最近的机器人编号
int SimpleSoccerBehavior::getPlayerClosestToBall() {
    // 初始化离球最近的机器人编号为 -1
    int playerClosestToBall = -1;
    // 初始化离球的最近距离为一个较大的值
    double closestDistanceToBall = 10000;
    // 遍历所有队友
    for(int i = WO_TEAMMATE1; i < WO_TEAMMATE1+NUM_AGENTS; ++i) {
        VecPosition temp;
        // 计算当前遍历到的机器人编号
        int playerNum = i - WO_TEAMMATE1 + 1;
        if (worldModel->getUNum() == playerNum) {
            // 如果是当前机器人自身
            temp = worldModel->getMyPosition();
        } else {
            // 获取队友的信息
            WorldObject* teammate = worldModel->getWorldObject( i );
            if (teammate->validPosition) {
                // 如果队友位置信息有效
                temp = teammate->pos;
            } else {
                // 位置信息无效则跳过该队友
                continue;
            }
        }
        // 将位置的 z 坐标设为 0
        temp.setZ(0);

        // 计算该机器人到球的距离
        double distanceToBall = temp.getDistanceTo(ball);
        if (distanceToBall < closestDistanceToBall) {
            // 如果该距离小于当前记录的最近距离
            playerClosestToBall = playerNum;
            closestDistanceToBall = distanceToBall;
        }
    }
    return playerClosestToBall;
}

// 该函数用于为每个机器人分配角色
map<int, Role> SimpleSoccerBehavior::getAgentRoles() {
    // 存储每个机器人的角色分配
    map<int, Role> agentRoles;
    // 存储已经分配的角色
    set<Role> assignedRoles;
    // 将离球最近的机器人分配为 ON_BALL 角色
    agentRoles[getPlayerClosestToBall()] = ON_BALL;
    assignedRoles.insert(ON_BALL);
    if (!agentRoles.count(1)) {
        // 如果 1 号机器人还未分配角色，则将其分配为 GOALIE（守门员）角色
        agentRoles[1] = GOALIE;
        assignedRoles.insert(GOALIE);
    }
    // 简单的贪心算法为剩余未分配角色的机器人分配角色
    typedef std::list<std::pair<double, std::pair<int, Role>>> dist_to_roles_list;
    dist_to_roles_list agentsDistancesToRoles;
    for (int r = 0; r < NUM_ROLES; r++) {
        Role role = static_cast<Role>(r);
        if (assignedRoles.count(role)) {
            // 如果该角色已经分配过，则跳过
            continue;
        }
        for (int i = 1; i <= NUM_AGENTS; i++) {
            if (agentRoles.count(i)) {
                // 如果该机器人已经分配了角色，则跳过
                continue;
            }
            // 计算该机器人到该角色位置的距离，并存储到列表中
            agentsDistancesToRoles.push_back(make_pair(getAgentDistanceToRole(i, role), make_pair(i, role)));
        }
    }

    // 按距离从小到大排序
    agentsDistancesToRoles.sort();

    for (dist_to_roles_list::iterator it = agentsDistancesToRoles.begin(); it != agentsDistancesToRoles.end(); ++it) {
        pair<int, Role> assignment = it->second;
        int agent = assignment.first;
        Role role = assignment.second;
        if (agentRoles.count(agent) || assignedRoles.count(role)) {
            // 如果该机器人或该角色已经分配过，则跳过
            continue;
        }
        // 为机器人分配角色
        agentRoles[agent] = role;
        assignedRoles.insert(role);
        if (agentRoles.size() == NUM_AGENTS) {
            // 如果所有机器人都已分配角色，则停止
            break;
        }
    }

    return agentRoles;
}

// 此函数用于根据角色获取对应的位置
VecPosition SimpleSoccerBehavior::getSpaceForRole(Role role) {
    // 获取球的位置
    VecPosition ball = worldModel->getBall();
    if (beamablePlayMode()) {
        // 如果处于可重新定位的比赛模式，将球的位置设为原点
        ball = VecPosition(0, 0, 0);
    }
    // 将球的 z 坐标设为 0
    ball.setZ(0);

    // 确保球的位置在球场范围内
    ball.setX(max(min(ball.getX(), HALF_FIELD_X), -HALF_FIELD_X));
    ball.setY(max(min(ball.getY(), HALF_FIELD_Y), -HALF_FIELD_Y));

    // 初始化位置为原点
    VecPosition space = VecPosition(0,0,0);

    // 根据不同的角色确定对应的位置
    switch(role) {
    case ON_BALL:
        space = ball;
        break;
    case GOALIE:
        space = VecPosition(-HALF_FIELD_X+0.5, 0, 0);
        break;
    case SUPPORTER:
        space = ball + VecPosition(-2,0,0);
        break;
    case BACK_LEFT:
        space = ball + VecPosition(-10,3,0);
        break;
    case BACK_RIGHT:
        space = ball + VecPosition(-10,-3,0);
        break;
    case MID_LEFT:
        space = ball + VecPosition(-1,2,0);
        break;
    case MID_RIGHT:
        space = ball + VecPosition(-1,-2,0);
        break;
    case WING_LEFT:
        space = ball + VecPosition(0,7,0);
        break;
    case WING_RIGHT:
        space = ball + VecPosition(0,-7,0);
        break;
    case FORWARD_LEFT:
        space = ball + VecPosition(5,3,0);
        break;
    case FORWARD_RIGHT:
        space = ball + VecPosition(5,-3,0);
        break;
    default:
        cerr << "Unknown role: " << role << "\n";
    }

    // 确保计算出的位置在球场范围内
    space.setX(max(-HALF_FIELD_X, min(HALF_FIELD_X, space.getX())));
    space.setY(max(-HALF_FIELD_Y, min(HALF_FIELD_Y, space.getY())));

    if (beamablePlayMode()) {
        // 在开球等可重新定位的模式下，确保位置在自己半场
        space.setX(min(-0.5, space.getX()));
    }

    return space;
}

// 该函数用于计算指定机器人到指定角色位置的距离
double SimpleSoccerBehavior::getAgentDistanceToRole(int uNum, Role role) {
    VecPosition temp;
    if (worldModel->getUNum() == uNum) {
        // 如果是当前机器人自身
        temp = worldModel->getMyPosition();
    } else {
        // 获取队友的位置信息
        WorldObject* teammate = worldModel->getWorldObject( WO_TEAMMATE1 + uNum - 1 );
        temp = teammate->pos;
    }
    // 将位置的 z 坐标设为 0
    temp.setZ(0);

    // 计算该机器人到指定角色位置的距离
    double distanceToRole = temp.getDistanceTo(getSpaceForRole(role));
    return distanceToRole;
}

// 此函数用于让机器人前往指定位置
SkillType SimpleSoccerBehavior::goToSpace(VecPosition space) {
    // 定义到达位置的阈值
    const double SPACE_THRESH = 0.5;
    if (me.getDistanceTo(space) < SPACE_THRESH ) {
        // 如果距离小于阈值，则监视球的位置
        return watchPosition(ball);
    }

    // 调整目标位置，避免离队友或球太近
    VecPosition target = collisionAvoidance(true /*teammate*/, false/*opponent*/, true/*ball*/, 1/*proximity thresh*/, .5/*collision thresh*/, space, true/*keepDistance*/);
    // 前往调整后的目标位置
    return goToTarget(target);
}

// 该函数用于让机器人监视指定位置
SkillType SimpleSoccerBehavior::watchPosition(VecPosition pos) {
    // 定义位置中心的阈值
    const double POSITION_CENTER_THRESH = 10;
    // 将全局位置转换为局部位置
    VecPosition localPos = worldModel->g2l(pos);

    // 计算局部位置的角度
    SIM::AngDeg localPosAngle = atan2Deg(localPos.getY(), localPos.getX());
    if (abs(localPosAngle) > POSITION_CENTER_THRESH) {
        // 如果角度超过阈值，则转向该位置
        return goToTargetRelative(VecPosition(), localPosAngle);
    }

    // 否则，保持站立
    return SKILL_STAND;
}






