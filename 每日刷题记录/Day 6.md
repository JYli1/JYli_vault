## [网鼎杯 2018]Comment
进来是一个留言板，需要登录才能写，要爆破后三位密码。
我们先扫目录：
![](assets/Day%206/file-20251224142401994.png)

发现有很多git文件，这里学习了一下githacker使用
```powershell
  githacker --url http://2b661b6a-7be0-408c-84e4-0aa48b856bee.node4.buuoj.cn:81/ --folder ./result
```
可以下载下来git仓库，还有一份源码：
![](assets/Day%206/file-20251224142342607.png)
```php
<?php
include "mysql.php";
session_start();
if($_SESSION['login'] != 'yes'){
    header("Location: ./login.php");
    die();
}
if(isset($_GET['do'])){
switch ($_GET['do'])
{
case 'write':
    break;
case 'comment':
    break;
default:
    header("Location: ./index.php");
}
}
else{
    header("Location: ./index.php");
}
?>

```
我们发现这份源码好像也实现不了网站功能呀，怀疑是不完整源码，又去学习了一下恢复git文件

我们先通过git命令找到历史记录：
```powershell
PS D:\webtool\GitHacker\result\6bc57b1d5399283fce7fbb0c289a505f> git log --reflog
```
然后找到完整源码的文件恢复：
```powershell
PS D:\webtool\GitHacker\result\6bc57b1d5399283fce7fbb0c289a505f> git reset --hard e5b2a2443c2b6d395d06960123142bc91123148c
```
![](assets/Day%206/file-20251224142721761.png)
具体怎么判断的哪个是完整的我也不清楚，大概就是看`refs/stash 标记`
然后我们就得到了完整的源代码了：

```php
<?php
include "mysql.php";
session_start();
if($_SESSION['login'] != 'yes'){
    header("Location: ./login.php");
    die();
}
if(isset($_GET['do'])){
switch ($_GET['do'])
{
case 'write':
    $category = addslashes($_POST['category']);
    $title = addslashes($_POST['title']);
    $content = addslashes($_POST['content']);
    $sql = "insert into board
            set category = '$category',
                title = '$title',
                content = '$content'";
    $result = mysql_query($sql);
    header("Location: ./index.php");
    break;
case 'comment':
    $bo_id = addslashes($_POST['bo_id']);
    $sql = "select category from board where id='$bo_id'";
    $result = mysql_query($sql);
    $num = mysql_num_rows($result);
    if($num>0){
    $category = mysql_fetch_array($result)['category'];
    $content = addslashes($_POST['content']);
    $sql = "insert into comment
            set category = '$category',
                content = '$content',
                bo_id = '$bo_id'";
    $result = mysql_query($sql);
    }
    header("Location: ./comment.php?id=$bo_id");
    break;
default:
    header("Location: ./index.php");
}
}
else{
    header("Location: ./index.php");
}
?>
```
很明显发现一个`wirte`模式，执行`insert`命令，`comment`模式执行查询，这是一个二次注入的特征。
首先我们尝试爆破一个密码，这里猜测是整数：
![](assets/Day%206/file-20251224161904911.png)
注意到只有pauload为`666`时，响应大小最大且存在302跳转，那这应该就是正确密码了。
下面进行二次注入，这里查询的是`category`