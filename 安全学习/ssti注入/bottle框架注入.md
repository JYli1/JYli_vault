# bottle框架的简介
Bottle 是一个非常轻量级的 Python Web 框架，适合用于构建简单的 Web 应用和RESTful API。Bottle 的最大特点之一是它的单文件设计，意味着你只需一个文件 bottle.py （也可以pip下载官方的）即可使用整个框架，而不需要安装其他依赖。

最简示例：
```python

from bottle import route, run

# 定义路由及处理函数
@route('/')
def hello():
    return "Hello, World!"

# 启动应用
run(host='localhost', port=8080)

```

语法上和flask差不多，
默认模板语法使用语法符号为 `<% %> % {{ }}`

- `<% %>`用来放置多行代码
- `%`用来放置单行代码
- `{{ }}`用来放置变量
但是最通用的是`{{ }}`,另外两个因为不会回显内容