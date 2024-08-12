# chatserver
使用muduo网络库搭建网络核心模块、Nginx实现聊天服务器的集群，提高并发能力、Redis作为消息中间件、MySQL作为数据存储、json序列化和反序列化作为通信协议的实时聊天服务器。

登录 login
{"msgid":1,"id":13,"password":"123456"}
{"msgid":1,"id":15,"password":"666666"}
{"msgid":1,"id":24,"password":"123"}

onechat
{"msgid":5,"id":13,"name":"zhang san","to":15,"msg":"hello!!!"}
{"msgid":5,"id":15,"name":"li si","to":13,"msg":"good!"}

{"msgid":6,"id":13,"friendid":15}
