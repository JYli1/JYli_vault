# GXYCTF2019【BabySQli】
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
   这里就是我们伪造了最后一条记录上去了，同样这里题目我们也可以伪造一条admin账户的查询结果，混淆正确密码