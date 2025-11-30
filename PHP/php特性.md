## php整数溢出

**PHP** **数组的索引限制PHP 中数组的最大索引值为PHP_INT_MAX（64 位系统中通常是9223372036854775807），当尝试使用大于这个值的索引时，数组赋值会失败，且赋值表达式返回false。**

代码中$ c的递增过程关键逻辑涉及两次++$c操作：

第一次：$ count[++$c] = 1

先将$c递增 1，再作为数组索引

第二次：若进入if分支，会执行$count[++$c] = 1

再次将$c递增 1

两种参数的执行差异

当c=9223372036854775806时：第一次++$c后，$c变为9223372036854775807（等于PHP_INT_MAX），此时$count[9223372036854775807] = 1是合法的。接下来执行$count[] = 1时，PHP 会尝试将索引设为9223372036854775808（超出最大值），赋值失败，返回false，因此进入else分支执行cat /flag。

当c=9223372036854775807时：第一次++$c后，$c变为9223372036854775808（超出PHP_INT_MAX），此时$ count[9223372036854775808] = 1直接失败。后续执行$count[] = 1时，索引会从 0 开始（因为前序赋值失败导致数组状态异常），赋值成功返回true，进入if分支，无法获取 flag。

  

# md5和sha1

**以下值在****md5****加密后以0E开头：

- QNKCDZO
    
- 240610708
    
- s155964671a
    
- s214587387a
    
- s214587387a
    

**还有****MD5****和双MD5以后的值都是0e开头的

- CbDLytmyGm2xQyaLNhWn
    
- CbDLytmyGm2xQyaLNhWn
    
- 7r4lGXCH2Ksu2JNT3BYM
    

**以下值在sha1加密后以0E开头：**

- sha1(‘aaroZmOk’)
    
- sha1(‘aaK1STfY’)
    
- sha1(‘aaO8zKZF’)
    
- sha1(‘aa3OFF9m’)
    

  

# PHP伪协议

## 一、**file://**

### 作用

用于访问文件（绝对路径、相对路径、网络路径）

### 示例

**http://www.xx.comfile****=file:///etc/passsd**

## 二、**php****://

### 作用

访问输入输出流

### 举例

#### `1.`**`php://filter`**

作用：**读取****源代码**并进行base64编码输出

示例：http://127.0.0.1/cmd.php?cmd=php://filter/read=convert.base64-encode/resource=****[文件名]（针对php文件需要base64编码）**

**php****://filter/read=string.rot13/resource=flag.txt（不能base64时可以用这个）**

**php****://filter/convert.iconv.UTF-8.UTF-7/resource=/flag(另外一种用法，把内容解释为****UTF-8****后以UTF-7输出)**

参数：resource=<要过滤的数据流> 这个参数是必须的。它指定了你要筛选过滤的数据流 read=<读链的筛选列表> 该参数可选。可以设定一个或多个过滤器名称，以管道符（|）分隔。 write=<写链的筛选列表> 该参数可选。可以设定一个或多个过滤器名称，以管道符（|）分隔。 <；两个链的筛选列表> 任何没有以 read= 或 write= 作前缀 的筛选器列表会视情况应用于读或写链。

#### `2.`**`php://input`**

作用：**执行**POST数据中的php代码

示例： **http://127.0.0.1/cmd.phpcmd****=php://input**

psot传参传入php代码即可

注意：**`enctype="multipart/form-data"`** 的候 **`php://input`** 是无效的

## 三、**data://**

作用：自PHP>=5.2.0起，可以使用data://数据流装器，以传递相应格式的数据。通常可以用来执行PHP代码。一般需要用到`base64编码`传输

示例：http://127.0.0.1/include.php?file=data://text/plain;base64,****PD9waHAgcGhwaW5mbygpOy8vPz4=**