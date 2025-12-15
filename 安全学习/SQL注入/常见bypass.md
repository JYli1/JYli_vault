# 宽字节绕过
`addslashes()、mysql_real_escape_string()、mysql_escape_string()、Magic_quotes_gpcaddslashes()`等函数会在`'`,`"`等符号前加上`\`,来转译，
此时我们可以在会被转译的字符前加上`%df`，与`\(%5c)`,组陈`%df%5d`，这在GBK编码中是一个汉字。这样就把`\`，给吃掉了
# 空格过滤
1. `+`代替空格
2. URL编码绕过
	%20 %09 %0a %0b %0c %0d %a0 %00
3. 注释绕过
	/**/ /!/