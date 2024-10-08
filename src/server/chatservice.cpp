#include "chatservice.hpp"
#include "public.hpp"
#include <string>
#include <vector>
#include <muduo/base/Logging.h>
#include "user.hpp"
#include "group.hpp"

using namespace muduo;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEDN_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    // 群相关业务
    _msgHandlerMap.insert({CERATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if (_redis.connect())
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 处理服务器Ctrl + c退出,重置User状态信息
void ChatService::reset()
{
    _userModel.resetState();
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认处理器（空操作）
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid:" << msgid << " not find handler!"; // muduo库日志文件
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

// 处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "do login service!!!";
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _userModel.query(id); // 查询数据库id返回用户的相应信息（对象）
    if (id == user.getId() && pwd == user.getPassword())
    {
        // 如果id和password都正确，判断用户的登录状态
        if (user.getState() == "online")
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "该用户已经登录，不允许重复登录!";
            conn->send(response.dump());
        }
        else
        {
            {
                // 使用互斥锁，类似于智能指针 自动释放锁
                lock_guard<mutex> lock(_connMutex);
                // 登录成功，记录用户连接信息
                _userConnMap.insert({id, conn});
            }

            // id用户登录成功后 向redis订阅channe（id）
            _redis.subscribe(id);

            // 登录成功,更新用户状态 offline-》online
            LOG_INFO << "login success!!!";
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取该用户的消息离线消息后，把该用户的所有离线消息删除掉
                _offlineMsgModel.remove(id);
            }

            // 登陆成功 自动返回查询到的好友信息
            // 查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                // 转化为sting类型js数据
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();

                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            // 查询用户的群组信息并返回
            vector<Group> groupVec = _groupModel.queryGroups(id);
            if (!groupVec.empty())
            {
                vector<string> groupV;
                for (Group &group : groupVec)
                {
                    json grpjs;
                    grpjs["id"] = group.getId();
                    grpjs["groupname"] = group.getName();
                    grpjs["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjs["users"] = userV;
                    groupV.push_back(grpjs.dump());
                }
                response["groups"] = groupV;
            }
            conn->send(response.dump());
        }
    }
    else
    {
        // 登录失败，用户密码或者id输入错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户密码或者id输入错误!";
        conn->send(response.dump());
    }
}

// 处理注册业务 name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "do reg service!!";

    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPassword(pwd);

    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();

        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;

        conn->send(response.dump());
    }
}

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 用户注销，相当于下线 在redis中取消订阅
    _redis.unsubscribe(userid);

    // 更新用户的state信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

// 处理客户端异常
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if (it->second == conn)
            {
                // 从map表删除用户的链接信息
                _userConnMap.erase(it);
                user.setId(it->first);
            }
        }
    }

    // 用户注销，相当于下线 在redis中取消订阅
    _redis.unsubscribe(user.getId());

    // 更新用户的state信息
    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid在线,转发消息  服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    // 查询toid是否在线
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    // toid 不在线，存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    // 存储好友信息
    _friendModel.insert(userid, friendid);
}

// 创建群业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        // 存储群组创建人的信息到 groupuser表
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}
// 加入群业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}
// 群聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    vector<int> useridVec;
    useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end()) // 在线
        {
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                 // 离线,存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}

// 从redis消息队列获取订阅消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end())
    {
        it->second->send(msg);
    }
    else
    {
        _offlineMsgModel.insert(userid, msg);
    }
}