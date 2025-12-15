# 宽字节绕过
`addslashes()、mysql_real_escape_string()、mysql_escape_string()、Magic_quotes_gpcaddslashes()`等函数会在`'`,`"`等符号前加上`\`,来转译，
此时我们可以在会被转译的字符前加上`%df`，与`\(%5c)`,组陈`%df%5d`，这在GBK编码中是一个汉字。这样就把`\`，给吃掉了