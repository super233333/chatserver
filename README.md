# chatserver
使用muduo网络库搭建网络核心模块、Nginx实现聊天服务器的集群，提高并发能力、Redis作为消息中间件、MySQL作为数据存储、json序列化和反序列化作为通信协议的实时聊天服务器。
## 技术介绍
1、使用muduo网络库作为项目的网络核心模块，提供高并发网络IO服务，解耦网络和业务模块代码；

2、使用json序列化和反序列化消息作为私有通信协议；

3、配置nginx基于tcp的负载均衡，实现聊天服务器的集群功能，提高后端服务的并发能力；

4、基于redis的发布-订阅功能，实现跨服务器的消息通信；

5、使用mysql关系型数据库作为项目数据的落地存储；

6、使用连接池提高数据库的数据存取功能。
![image](https://github.com/user-attachments/assets/9d3ea4cf-84dd-4367-9d16-f946a4642ad8)

登录 login
{"msgid":1,"id":13,"password":"123456"}
{"msgid":1,"id":15,"password":"666666"}
{"msgid":1,"id":24,"password":"123"}

onechat
{"msgid":5,"id":13,"name":"zhang san","to":15,"msg":"hello!!!"}
{"msgid":5,"id":15,"name":"li si","to":13,"msg":"good!"}

{"msgid":6,"id":13,"friendid":15}
