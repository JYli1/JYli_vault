# GXYCTF2019【BabySQli】
sql注入，先随便输一个，返回密码错误，但是发现源码中有一段加密字符
![](assets/Day%205/file-20251223124539623.png)
base32+base64解密：
```sql
select * from user where username = '$name'
```
是sql语句，尝试了group by爆出列数,3列，但是union注入的时候，只会反回`wrong user
本来想尝试报错注入，但是小括号被过滤了。
想盲注，但是and这些也被过滤了
