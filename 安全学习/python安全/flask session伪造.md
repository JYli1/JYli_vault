https://www.cnblogs.com/GTL-JU/p/16960460.html
## 0x01 session的作用

由于http协议是一个无状态的协议，也就是说同一个用户第一次请求和第二次请求是完全没有关系的，但是现在的网站基本上有登录使用的功能，这就要求必须实现有状态，而session机制实现的就是这个功能。  
用户第一次请求后，将产生的状态信息保存在session中，这时可以把session当做一个容器，它保存了正在使用的所有用户的状态信息；这段状态信息分配了一个唯一的标识符用来标识用户的身份，将其保存在响应对象的cookie中；当第二次请求时，解析cookie中的标识符，拿到标识符后去session找到对应的用户的信息

## 0x02 flask session的储存方式

1. 直接存在客户端的cookies中

2. 存储在服务端，如：redis,memcached,mysql，file,mongodb等等，存在flask-session第三方库

flask的session可以保存在客户端的cookie中，那么就会产生一定的安全问题。