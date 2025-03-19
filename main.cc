#include <errno.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <fstream>
#include <rcssnet/tcpsocket.hpp>
//#include <rcssnet/udpsocket.hpp>
#include <rcssnet/exception.hpp>
#include <netinet/in.h>
#include "behaviors/behavior.h"
#include "behaviors/naobehavior.h"
#include "optimization/optimizationbehaviors.h"
#include "behaviors/pkbehaviors.h"
#include "behaviors/simplesoccer.h"
#include "behaviors/gazebobehavior.h"
#include "stats/recordstatsbehavior.h"

using namespace rcss::net;
using namespace std;

// 全局 TCP 套接字，用于与服务器通信
TCPSocket gSocket;
//UDPSocket gSocket;
// 服务器的主机地址，默认为本地主机
string gHost = "127.0.0.1";
// 服务器的端口号，默认为 3100
int gPort = 3100;

// 用于连接监控端口的变量
// 监控端口的 TCP 套接字
TCPSocket mSocket;
// 监控端口的主机地址，默认为本地主机
string mHost = "127.0.0.1";
// 监控端口号，默认为 -1 表示未使用
int mPort = -1;

// 一个用于某种标识的无符号长整型键值
unsigned long long key = 0;

// 布尔变量，用于指示是否继续智能体的主循环
static bool gLoop = true;

// 要导出的智能体身体类型的全局变量
// （可能应该避免使用全局变量，采用更好的方式存储该值）
int agentBodyType = 0;

// 是否使用胖代理的全局布尔变量
bool fFatProxy = false;

// SIGINT 信号处理函数原型
extern "C" void handler(int sig)
{
    if (sig == SIGINT)
        gLoop = false;
}

// 打印欢迎信息
void PrintGreeting()
{
    cout << "Robocup3D仿真代码\n";
}

// 打印帮助信息，展示程序的使用方法和可用选项
void PrintHelp()
{
    cout << "\n用法: agentspark [选项]" << endl;
    cout << "\n选项:" << endl;
    cout << " --help\t打印此消息。" << endl;
    cout << " --host=<IP>\t服务器的 IP 地址。" << endl;
    cout << " --port <端口号>\t服务器的端口号。" << endl;
    cout << " --type <类型编号>\t要使用的异构模型类型编号。" << endl;
    cout << " --rsg <rsg 文件>\t用于 NAO 模型的 rsg 文件。" << endl;
    cout << " --team <队名>\t团队的名称。" << endl;
    cout << " --unum <球员编号>\t球员的统一编号。" << endl;
    cout << " --paramsfile <文件名>\t要加载的参数文件的名称。" << endl;
    cout << " --pkgoalie\t点球大战中的守门员。" << endl;
    cout << " --pkshooter\t点球大战中的射手。" << endl;
    cout << " --gazebo\t用于 Gazebo RoboCup 3D 仿真插件的智能体。" << endl;
    cout << " --optimize <智能体类型>\t优化智能体的类型。" << endl;
    cout << " --recordstats\t记录比赛统计数据。" << endl;
    cout << " --mhost=<IP>\t用于发送绘图命令的监控器的 IP 地址。" << endl;
    cout << " --mport <端口号>\t用于训练命令解析器的监控器的端口号。" << endl;
    cout << " --fatproxy\t使用胖代理运行。" << endl;

    cout << "\n";
}

/*
 * 从输入文件中读取参数，该文件应格式化为一系列参数，
 * 以字符串到浮点数的键值对形式存在。
 * 参数名应与其值用制表符分隔，参数之间用单个换行符分隔。
 * 参数将被加载到 namedParams 映射中。
 */
map<string, string> namedParams;
void LoadParams(const string& inputsFile) {
    istream *input;
    ifstream infile;
    istringstream inString;

    // 打开参数文件
    infile.open(inputsFile.c_str(), ifstream::in);

    if(!infile) {
        cerr << "无法打开参数文件 " << inputsFile << endl;
        exit(1);
    }

    input = &(infile);

    string name;
    bool fBlockComment = false;
    while(!input->eof())
    {
        // 跳过注释和空行
        std::string str;
        std::getline(*input, str);
        if (str.length() >= 2 && str.substr(0,2) == "/*") {
            fBlockComment = true;
        } else if (str == "*/") {
            fBlockComment = false;
        }
        if(fBlockComment || str == "" || str[0] == '#' ) {
            continue;
        }

        // 解析字符串
        stringstream s(str);
        std::string key;
        std::string value;
        std::getline(s, key, '\t');      // 读取直到制表符
        std::getline(s, value);          // 读取直到换行符
        if(value.empty()) {
            continue;
        }
        namedParams[key] = value;
    }

    infile.close();
}

// 团队名称
string teamName;
// 球员的统一编号
int uNum;
// 优化输出文件的名称
string outputFile(""); 
// 智能体类型，默认为 "naoagent"
string agentType("naoagent");
// rsg 文件的路径，默认为 "rsg/agent/nao/nao.rsg"
string rsg("rsg/agent/nao/nao.rsg");
// 读取命令行选项并设置相应的全局变量
void ReadOptions(int argc, char* argv[])
{
    // 默认团队名称
    teamName = "Ahjzu-3d";
    // 球员编号，值为 0 表示选择下一个可用编号
    uNum = 0; 

    for( int i = 0; i < argc; i++)
    {
        if ( strcmp( argv[i], "--help" ) == 0 )
        {
            PrintHelp();
            exit(0);
        }
        else if ( strncmp( argv[i], "--host", 6 ) == 0 )
        {
            string tmp=argv[i];

            if ( tmp.length() <= 7 ) // 最小合理性检查
            {
                PrintHelp();
                exit(0);
            }
            gHost = tmp.substr(7);
        }
        else if ( strncmp( argv[i], "--mhost", 7 ) == 0 )
        {
            string tmp=argv[i];

            if ( tmp.length() <= 8 ) // 最小合理性检查
            {
                PrintHelp();
                exit(0);
            }
            mHost = tmp.substr(8);
        }
        else if ( strncmp( argv[i], "--port", 6) == 0 ) {
            if (i == argc - 1) {
                PrintHelp();
                exit(0);
            }
            gPort = atoi(argv[i+1]);
        }
        else if ( strncmp( argv[i], "--mport", 7) == 0 ) {
            if (i == argc - 1) {
                PrintHelp();
                exit(0);
            }
            mPort = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "--team") == 0) {
            if(i == argc - 1) {
                PrintHelp();
                exit(0);
            }

            teamName = argv[i + 1];
        }
        else if(strcmp(argv[i], "--unum") == 0) {
            if(i == argc - 1) {
                PrintHelp();
                exit(0);
            }
            uNum = atoi(argv[i + 1]);
        }
        else if(strcmp(argv[i], "--paramsfile") == 0) {
            if(i == argc - 1) {
                PrintHelp();
                exit(0);
            }
            string inputsFile = argv[i+1];
            LoadParams(inputsFile);
        }
        else if (strcmp(argv[i], "--experimentout") == 0) {
            if(i == argc - 1) {
                PrintHelp();
                exit(0);
            }
            outputFile = argv[i+1];
        }
        else if (strcmp(argv[i], "--optimize") == 0) {
            if(i == argc - 1) {
                PrintHelp();
                exit(0);
            }
            agentType = argv[i+1];
        }
        else if (strcmp(argv[i], "--type") == 0) {
            if(i == argc - 1) {
                PrintHelp();
                exit(0);
            }
            rsg = "rsg/agent/nao/nao_hetero.rsg " + string(argv[i+1]);
            agentBodyType = atoi(argv[i+1]);
        }
        else if (strcmp(argv[i], "--rsg") == 0) {
            if(i == argc - 1) {
                PrintHelp();
                exit(0);
            }
            rsg = argv[i+1];
        }
        else if (strcmp(argv[i], "--pkgoalie") == 0) {
            agentType = "pkgoalie";
        }
        else if (strcmp(argv[i], "--pkshooter") == 0) {
            agentType = "pkshooter";
        }
        else if (strcmp(argv[i], "--simplesoccer") == 0) {
            agentType = "simplesoccer";
        }
        else if (strcmp(argv[i], "--gazebo") == 0) {
            agentType = "gazebo";
        }
        else if (strcmp(argv[i], "--recordstats") == 0) {
            agentType = "recordstats";
        }
        else if (strcmp(argv[i], "--fatproxy") == 0) {
            fFatProxy = true;
        }
    } // 结束 for 循环
}

// 初始化与服务器的连接
bool Init()
{
    cout << "正在连接到 TCP " << gHost << ":" << gPort << "\n";
    //cout << "正在连接到 UDP " << gHost << ":" << gPort << "\n";

    try
    {
        // 绑定本地地址
        Addr local(INADDR_ANY,INADDR_ANY);
        gSocket.bind(local);
    }
    catch (const BindErr& error)
    {
        cerr << "绑定套接字失败，错误信息: '"
             << error.what() << "'" << endl;

        gSocket.close();
        return false;
    }

    try
    {
        // 连接到服务器
        Addr server(gPort,gHost);
        gSocket.connect(server);
    }
    catch (const ConnectErr& error)
    {
        cerr << "连接失败，错误信息: '"
             << error.what() << "'" << endl;
        gSocket.close();
        return false;
    }

    // 连接到监控端口，以便使用训练命令解析器
    if (mPort != -1) {
        try
        {
            Addr local(INADDR_ANY,INADDR_ANY);
            mSocket.bind(local);
        }
        catch (const BindErr& error)
        {
            cerr << "绑定监控套接字失败，错误信息: '"
                 << error.what() << "'" << endl;

            mSocket.close();
            return false;
        }

        try
        {
            Addr server(mPort,gHost);
            mSocket.connect(server);
        }
        catch (const ConnectErr& error)
        {
            cerr << "连接监控端口失败，错误信息: '"
                 << error.what() << "'" << endl;
            mSocket.close();
            return false;
        }
    }

    return true;
}

// 关闭与服务器和监控端口的连接
void Done()
{
    gSocket.close();
    cout << "已关闭与 " << gHost << ":" << gPort << " 的连接\n";
    if (mPort != -1) {
        mSocket.close();
    }
}

// 选择输入，等待套接字有数据可读
bool SelectInput()
{
    fd_set readfds;
    struct timeval tv = {60,0};
    FD_ZERO(&readfds);
    FD_SET(gSocket.getFD(),&readfds);

    while(1) {
        switch(select(gSocket.getFD()+1,&readfds, 0, 0, &tv)) {
        case 1:
            return 1;
        case 0:
            cerr << "(SelectInput) select 失败 " << strerror(errno) << endl;
            abort();
            return 0;
        default:
            if(errno == EINTR)
                continue;
            cerr << "(SelectInput) select 失败 " << strerror(errno) << endl;
            abort();
            return 0;
        }
    }
}

// 向服务器发送消息
void PutMessage(const string& msg)
{
    if (msg.empty())
    {
        return;
    }

    // 在消息前添加消息长度前缀
    unsigned int len = htonl(msg.size());
    string prefix((const char*)&len,sizeof(unsigned int));
    string str = prefix + msg;
    if ( static_cast<ssize_t>(str.size()) != write(gSocket.getFD(), str.data(), str.size())) {
        LOG_STR("无法发送完整消息: " + msg);
    }
}

// 向监控端口发送消息
void PutMonMessage(const string& msg)
{
    if (msg.empty())
    {
        return;
    }

    // 在消息前添加消息长度前缀
    unsigned int len = htonl(msg.size());
    string prefix((const char*)&len,sizeof(unsigned int));
    string str = prefix + msg;
    if ( static_cast<ssize_t>(str.size()) != write(mSocket.getFD(), str.data(), str.size())) {
        LOG_STR("无法发送完整监控消息: " + msg);
    }
}

// 从服务器接收消息
bool GetMessage(string& msg)
{
    static char buffer[16 * 1024];

    unsigned int bytesRead = 0;
    // 读取消息长度前缀
    while(bytesRead < sizeof(unsigned int))
    {
        SelectInput();
        int readResult = read(gSocket.getFD(), buffer + bytesRead, sizeof(unsigned int) - bytesRead);
        if(readResult < 0)
            continue;
        if (readResult == 0) {
            // [patmac] 如果与服务器断开连接则终止程序
            // 例如服务器被关闭时。这有助于防止失控的智能体。
            cerr << "与服务器失去连接" << endl;
            Done();
            exit(1);
        }
        bytesRead += readResult;
    }

    // 解析消息长度
    union int_char_t {
        char *c;
        unsigned int *i;
    };
    int_char_t size;
    size.c = buffer;
    unsigned int msgLen = ntohl(*(size.i));
    if(sizeof(unsigned int) + msgLen > sizeof(buffer)) {
        cerr << "消息过长，终止程序" << endl;
        abort();
    }

    // 读取剩余的消息段
    unsigned int msgRead = bytesRead - sizeof(unsigned int);
    char *offset = buffer + bytesRead;

    while (msgRead < msgLen)
    {
        if (! SelectInput())
        {
            return false;
        }

        unsigned readLen = sizeof(buffer) - msgRead;
        if(readLen > msgLen - msgRead)
            readLen = msgLen - msgRead;

        int readResult = read(gSocket.getFD(), offset, readLen);
        if(readResult < 0)
            continue;
        msgRead += readResult;
        offset += readResult;
    }

    // 零终止接收到的数据
    (*offset) = 0;

    msg = string(buffer+sizeof(unsigned int));

    // 调试信息
    //cout << msg << endl;

    static string lastMsg = "";
    if (msg.compare(lastMsg) == 0) {
        cerr << "从服务器接收到重复消息 -- 服务器是否已终止我们的连接？\n";
        Done();
        exit(1);
    }
    lastMsg = msg;

    return true;
}

// 运行智能体的主循环
void Run()
{
    Behavior *behavior;
    if (agentType == "naoagent") {
        behavior = new NaoBehavior(teamName, uNum, namedParams, rsg);
    }
    else if (agentType == "pkgoalie") {
        behavior = new PKGoalieBehavior(teamName, uNum, namedParams, rsg);
    }
    else if (agentType == "pkshooter") {
        behavior = new PKShooterBehavior(teamName, uNum, namedParams, rsg);
    }
    else if (agentType == "simplesoccer") {
        behavior = new SimpleSoccerBehavior(teamName, uNum, namedParams, rsg);
    }
    else if (agentType == "gazebo") {
        agentBodyType = GAZEBO_AGENT_TYPE;
        behavior = new GazeboBehavior(teamName, uNum, namedParams, rsg);
    }
    else if (agentType == "fixedKickAgent") {
        cerr << "正在创建 OptimizationBehaviorFixedKick" << endl;
        behavior = new OptimizationBehaviorFixedKick(  teamName,
                uNum,
                namedParams,
                rsg,
                outputFile);
    }
    else if (agentType == "walkForwardAgent") {
        cerr << "正在创建 OptimizationBehaviorWalkForward" << endl;
        behavior = new OptimizationBehaviorWalkForward(  teamName,
                uNum,
                namedParams,
                rsg,
                outputFile);
    }
    else if ( agentType == "recordstats") {
        behavior = new RecordStatsBehavior(teamName, uNum, namedParams, rsg,
                                           outputFile);
    }
    else {
        throw "未知的智能体类型";
    }

    // 发送初始化消息
    PutMessage(behavior->Init()+"(syn)");

    string msg;
    while (gLoop)
    {
        // 接收服务器消息
        GetMessage(msg);
        // 智能体思考并生成响应消息
        string msgToServer = behavior->Think(msg);
        // 支持智能体同步模式
        msgToServer.append("(syn)");
        // 发送响应消息到服务器
        PutMessage(msgToServer);
        if (mPort != -1) {
            // 发送监控消息到监控端口
            PutMonMessage(behavior->getMonMessage());
        }
    }
}

int
main(int argc, char* argv[])
{
    // 注册信号处理函数，捕获 SIGINT 信号
    signal(SIGINT, handler);

    // 捕获并输出抛出的错误信息
    try
    {
        PrintGreeting();
        ReadOptions(argc,argv);

        if (! Init())
        {
            return 1;
        }

        Run();
        Done();
    }
    catch (char const* c)
    {
        cerr << "-------------错误------------" << endl;
        cerr << c << endl;
        cerr << "-----------错误结束----------" << endl;
    }
    catch (string s)
    {
        cerr << "-------------错误------------" << endl;
        cerr << s << endl;
        cerr << "-----------错误结束----------" << endl;
    }
}