#ifndef PUBLIC_H
#define PUBLIC_H
/*
server和client的公共文件
*/
enum EnMsgType
{
    LOGIN_MSG = 1, // 登录消息 1
    LOGIN_MSG_ACK, // 登录响应消息 2
    LOGINOUT_MSG,
    REG_MSG,        // 注册消息 
    REG_MSG_ACK,    // 注册响应消息 
    ONE_CHAT_MSG,   // 一对一聊天消息 
    ADD_FRIEDN_MSG, // 添加好友消息 

    CERATE_GROUP_MSG, // 创建群组
    ADD_GROUP_MSG,    // 加入群组
    GROUP_CHAT_MSG,   // 群聊天

};

#endif