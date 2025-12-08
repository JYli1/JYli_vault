# 【include_upload】
之前其实是学过phar文件包含的，做了这题之后才发现之前白学了。

1. 首先我们得知道怎么包含phar文件中的php代码，phar文件其实有3个部分都可以写php代码，并且会原样输出的![](assets/ISCTF%202025/file-20251209014648966.png)但是3个地方的php代码并不是都可以被解析，（我之前一直以为一个文件里面只要有php代码，被include之后就可以被解析）
实验文件：phar.php include.php
```php
<?php
class TestObject {
    public $a="<?php echo '触发matedata中的php代码'; ?>";
}

@unlink("phar.phar");                  // 删除旧的 phar 文件（如果存在）

$phar = new Phar("phar.phar");         // 创建一个新的 phar 对象，文件名必须以 .phar 结尾
$phar->startBuffering();               // 开始缓冲（准备写入 phar 内容）

$phar->setStub("<?php echo '触发stub中的php代码';  __HALT_COMPILER(); ?>");  // 设置 stub（文件头部的启动代码）

$o = new TestObject();
// 实例化一个对象
$phar->setMetadata($o);                // 把对象放到 phar 的 metadata 区域（存入 manifest）

$phar->addFromString("test.txt", "<?php echo '触发phar中test.txt中的php代码'; ?>"); // 向 phar 添加一个文件 test.txt，内容为 "test"

// phar 内部会自动计算签名（默认是 SHA1）
$phar->stopBuffering();                // 停止缓冲并写入文件
?>

```
```php
<?php
highlight_file(__FILE__);
include($_GET['file']);
?>
```
* 直接include：可以直接触发stub中的php代码，但是这好像没啥用，因为文件内容并不会不可见什么的，也就是说该过滤文件内容还是过滤和直接上传php文件没有差别。![](assets/ISCTF%202025/file-20251209013432455.png)
* 用phar伪协议：可以去触发里层文件的php代码，但是要加上路劲，（这里有一个用法，就是phar伪协议会自动去解压压缩包并且得到里面的文件，这样就可以被include了![](assets/ISCTF%202025/file-20251209013800824.png)
好了，这上面的对题目其实没有用，单纯是我的个人疑惑。。。。
2. 我们绕过phar文件中stub部分`__HALT_COMPILER`过滤的时候,学过用gzip压缩过滤。说是一样可以触发反序列化的。至于为什么可以看文章：
 https://www.anquanke.com/post/id/240007#h2-5 。

3. 对于include函数而言，识别一个文件是不是phar文件名中有`.phar`则认为他是phar文件。并且对于phar文件中压缩过的内容会自动解压。并判断有没有`<?php __HALT_COMPILER();?>`文件头,没有则报错，所以如果解压出来有php代码的话，自然也会解析了。
![](assets/ISCTF%202025/file-20251209020656891.png)![](assets/ISCTF%202025/file-20251209020707761.png)
4.  但是至于为什么反序列化的时候会解析压缩后的matedata就不是这个原因了，看上面的文章，大概是phar伪协议的原因，他的底层实现会先解压再反序列化，而且几种压缩都是可以的，`主要记住的就是这个是需要pahr伪协议的`![](assets/ISCTF%202025/file-20251209021043109.png)
总结一下就是，我终于搞清楚了
phar反序列化用gzip时是需要phar伪协议的，因为phar伪协议的底层实现会先解压再反序列化
phar文件包含用gzip绕过时是不需要伪协议的，因为那个是include的底层会对phar文件进行先解压再包含，但是要注意的是php代码要写在stub才会执行。然后小技巧就是`文件中出现了.phar即可认为是phar文件`


## wp
最后回归题目，
```php
<?php
highlight_file(__FILE__);
error_reporting(0);
$file = $_GET['file'];
if(isset($file) && strtolower(substr($file, -4)) == ".png"){
    include'./upload/' . basename($_GET['file']);
    exit;
}
?>

```
文件包含，还有一个文件上传的点。
分析源码：
* 使用了`basename()`,这个函数会取出输入的地址中最后一个文件名（意思是abc/acs/ascsc/casc/flag.txt会被识别为flag.txt,前面省略），所以我们就不考虑目录穿越读文件了。
* `if(isset($file) && strtolower(substr($file, -4)) == ".png")`  这句话是说`$file`参数只有取出最后4个之后为`.png`才能包含，`strtolower()`函数是转化为小写。
* 总的就是只能包含`.png`后缀的文件
这里开始想到有include，那直接再png的body中写php代码也能解析的，但是也有waf。
对文件内容过滤了`php`,`<?`,总之换标签是行不通了。这里就卡住了，学了什么的小技巧才知道，对于`include()`函数只要文件名中包含了`.phar`就会当作是在include一个phar文件。并且如果文件内容经过压缩的话还能自动解压，这样phar中的可见php代码也没有了。完美绕过。

实践：
首先通过脚本生成一个phar文件，并且通过
`gzip -c phar.phar >1.phar.png`命令把压缩后的phar文件直接写入一个png文件，然后上传就好了![](assets/ISCTF%202025/file-20251209023903748.png)
已经成功绕过
