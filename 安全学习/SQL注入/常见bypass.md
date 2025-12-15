# 宽字节绕过
`addslashes()、mysql_real_escape_string()、mysql_escape_string()、Magic_quotes_gpcaddslashes()`等函数会在`'`,`"`等符号前加上`\`,来转译，
此时我们可以在会被转译的字符前加上`%df`，与`\(%5c)`,组陈`%df%5d`，这在GBK编码中是一个汉字。这样就把`\`，给吃掉了
# 空格过滤
1. `+`代替空格
2. URL编码绕过
	%20 %09 %0a %0b %0c %0d %a0 %00
3. 注释绕过
	`/**/` `/!/`详情见[安全狗各版本 绕过](安全狗各版本%20绕过.md)
4. 括号绕过
	在MySQL数据库中，任何查询中都可以使用括号嵌套SQL语句，可以利用括号绕过空格的过滤
	`select(user())from dual where(1=1)and(2=2)

# 绕过引号
这个时候如果引号被过滤了，那么上面的where子句就无法使用了。那么遇到这样的问题就要使用**十六进制**来处理这个问题了。users的十六进制的字符串是7573657273。那么最后的sql语句就变为了：

`select column_name from information_schema.tables where table_name=0x7573657273

# 逗号rao