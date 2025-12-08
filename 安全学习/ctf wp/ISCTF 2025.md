# 【include_upload】
之前其实是学过phar文件包含的，做了这题之后才发现之前白学了。
首先有一些我们要知道的基础知识
1. 首先我们得知道怎么包含phar文件中的php代码
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
* 直接include：可以直接触发stub中的php代码，但是这好像没啥用，因为文件内容并不会不可见什么的，也就是说该过滤文件内容还是过滤和直接上传没有差别。![](assets/ISCTF%202025/file-20251209013432455.png)
* 用phar伪协议：可以去触发里层文件的php代码，但是要加上路劲(这里的用处就是)![](assets/ISCTF%202025/file-20251209013800824.png)
好了，这上面的对题目其实没有用，单纯是我的个人疑惑。。。。
2. 我们绕过phar文件