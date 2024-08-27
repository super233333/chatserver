# chatserver
使用muduo网络库搭建网络核心模块、Nginx实现聊天服务器的集群，提高并发能力、Redis作为消息中间件、MySQL作为数据存储、json序列化和反序列化作为通信协议的实时聊天服务器。
## 技术介绍
1、使用muduo网络库作为项目的网络核心模块，提供高并发网络IO服务，解耦网络和业务模块代码；

2、使用json序列化和反序列化消息作为私有通信协议；

3、配置nginx基于tcp的负载均衡，实现聊天服务器的集群功能，提高后端服务的并发能力；

4、基于redis的发布-订阅功能，实现跨服务器的消息通信；

5、使用mysql关系型数据库作为项目数据的落地存储；

6、使用连接池提高数据库的数据存取功能。


## 项目架构
![image](https://github.com/user-attachments/assets/13933448-3717-4eef-8355-d0c69e948972)

## 项目目录
![image](https://github.com/user-attachments/assets/5ea82c5e-5bf2-4903-8e33-20c0c6b486ec)

bin：存放服务器和客户端的可执行程序

build：cmake编译生成的临时文件

CMakeLists.txt：项目使用cmake作为编译环境

include：项目的头文件

src：项目的源文件

test：muduo网络库使用的demo

thirdparty：存放json.hpp

## 数据库设计
![image](https://github.com/user-attachments/assets/c39934ff-13b5-4b04-bc9a-8a963034c47a)

![image](https://github.com/user-attachments/assets/6bd84d5d-1fe9-46a5-962a-7993727f9cd0)

![image](https://github.com/user-attachments/assets/12c254a4-e44d-41c1-b928-d01af3123227)

## 项目介绍
功能分为：登录账号、注册账号、退出账号、一对一聊天业务、创建群聊业务、加入群聊业务、群内发消息业务、添加好友业务、离线消息存储业务。

特别说明：以下示例已经启动Nginx和redis-server，Nginx集群了两台服务器，端口分别为6000、6002。
![image](https://github.com/user-attachments/assets/99d9a429-62f2-4e1c-807f-4c9567483771)


## 启动服务器
这里启动两台服务器，因为Nginx配置tcp负载均衡时配置了两台。

![image](https://github.com/user-attachments/assets/82363bd5-0c09-4b5f-8e80-57b1aff31fc0)
![image](https://github.com/user-attachments/assets/dda5b5e5-7d58-40e1-b76b-97a2ca04dffd)

![image](https://github.com/user-attachments/assets/849e2ffd-8ca7-4dfc-b336-5edc8d9ea06d)

## 启动客户端
同样启动两台客户端进行聊天业务。

![image](https://github.com/user-attachments/assets/b81aa0e0-0d38-46bf-9ff6-9c1644b6128d)

由于Nginx，此时client01和client02分别连接到不同的server端

![image](https://github.com/user-attachments/assets/c9d0422a-1066-4f6e-be51-afdf3e963af0)


## 注册账号
![image](https://github.com/user-attachments/assets/6c682d21-2e24-4666-a3a5-7057964304bf)

Tom 用户注册成功，他的用户id为25，登录时通过用户id去登录。

## 登录成功
![image](https://github.com/user-attachments/assets/e6943952-e151-4d47-bd5c-825d6953246c)

![image](https://github.com/user-attachments/assets/cf44bb82-31c8-4795-b6ba-6159ae515f61)

两台客户端分别登录了两个用户，用户id分别为25和26，可以看到用户25,26的用户还没有添加任何好友和群聊。

![image](https://github.com/user-attachments/assets/f284b6cd-9f3b-4692-a85a-c8daea292aac)

## 一对一聊天业务
现在进行id=24和id=25之间聊天，不是好友也可以互相聊天。

一对一聊天的命令行输入为：chat:用户id:内容

![image](https://github.com/user-attachments/assets/77ae6c50-bbd5-476e-9063-8a55ef10f6c0)

## 创建群聊业务
创建群聊的命令行：creategroup:群聊名:群聊描述

创建群聊的人权限为创建者。

![image](https://github.com/user-attachments/assets/e7c97a3e-3e6d-4447-84b9-4cc2d5866aa1)

![image](https://github.com/user-attachments/assets/6eae6966-3451-4c8f-a8ce-28d76f612f34)

![image](https://github.com/user-attachments/assets/f41fcc92-ccf8-467a-97ff-e1f5b2bebf36)

可以看到，数据库里群聊已经创建成功，群聊id为3，userid=25的用户在groupid=3的群里的权限为创建者。

## 加入群聊业务
![image](https://github.com/user-attachments/assets/bff2b57f-aa40-472f-b976-f5c080008c9c)

![image](https://github.com/user-attachments/assets/98ba1698-d804-4d56-ab31-325a281d880d)

userid=26的用户加入groupid=3的群，他的权限为普通权限。

## 群聊业务
![image](https://github.com/user-attachments/assets/1aa5f9d9-911e-48d9-a8de-3e2ba714a90e)

![image](https://github.com/user-attachments/assets/17de795b-c741-4b9d-a238-c24be8f1752f)

userid=25的用户在groupid=3的群里发了一条消息，群内成员收到了这条消息。

## 添加好友业务
![image](https://github.com/user-attachments/assets/329cf5fc-3c65-437f-92d3-d3036ae387a8)


![image](https://github.com/user-attachments/assets/aba52fd2-5b72-49ab-94db-988cbb39685c)


可以看到，userid=26的用户有一个好友，好友id=25，即userid=25的用户。

## 离线消息存储业务
现在我们让userid=26的用户退出。

![image](https://github.com/user-attachments/assets/5674b787-59a4-47bf-9801-e7cbf33035fb)
userid=25的用户持续向userid=26发单聊消息，向groupid=3发送群聊消息。

![image](https://github.com/user-attachments/assets/d780b00a-8d83-47fd-990a-a27713827696)

数据库里已经存了两条消息。

![image](https://github.com/user-attachments/assets/84297d74-8d4f-4326-ab51-99cb4d72574d)

现在我们让userid=26的用户重新登录。

![image](https://github.com/user-attachments/assets/4600d390-2611-450c-8d50-e69329180785)

刚刚添加的好友、群聊、离线消息在重新登录之后都成功显示出来了。

## 特殊说明
要使程序成功跑起来，需要在db.cpp的数据库ip和密码改成自己本机的ip和密码。

![image](https://github.com/user-attachments/assets/32b2e876-f06f-4e46-b02b-e6819a154d4a)
