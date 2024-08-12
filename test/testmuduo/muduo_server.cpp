#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

// 1.组合TcpServer对象
// 2.创建EventLoop事件循环对象的指针
// 3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
// 4.在当前服务器类的构造函数中，注册处理连接的回调函数和处理读写事件回调函数
// 5.设置合适的服务端线程数量，muduo会自己划分I/O线程和worker线程
class ChatServer
{
public:
    ChatServer(EventLoop *loop,               // 事件循环
               const InetAddress &listenAddr, // IP + port
               const string &nameArg)
        : _Server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开回调
        _Server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        // 给服务器注册用户读写事件回调
        _Server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
        // 设置服务器线程数量
        _Server.setThreadNum(4);
    }

    // 开始事件循环
    void start()
    {
        _Server.start();
    }

private:
    // 专门处理用户的连接 创建 和 断开
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:online" << endl;
        }
        else
        {
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:offline" << endl;
            conn->shutdown();
            
        }
    }

    // 专门处理用户读写事件
    void onMessage(const TcpConnectionPtr &conn, // 连接
                   Buffer *buffer,               // 缓冲区
                   Timestamp time)               // 时间信息
    {
        string buf = buffer->retrieveAllAsString();
        cout << "recv data:" << buf << "time:" << time.toString() << endl;
        conn->send(buf);
    }
    TcpServer _Server;
    EventLoop *_loop;
};

int main()
{
    EventLoop loop;
    InetAddress addr("192.168.195.128", 6000);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop(); // 相当与 epoll_wait

    return 0;
}