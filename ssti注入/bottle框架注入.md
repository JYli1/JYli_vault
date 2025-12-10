# bottle框架的简介
Bottle 是一个非常轻量级的 Python Web 框架，适合用于构建简单的 Web 应用和RESTful API。Bottle 的最大特点之一是它的单文件设计，意味着你只需一个文件 bottle.py 即可使用整个框架，而不需要安装其他依赖。

最简示例：
```python
# 导入本地的 bottle.py 文件
from bottle import route, run

# 定义路由及处理函数
@route('/hello')
def hello():
    return "Hello, World!"

# 启动应用
run(host='localhost', port=8080)

```
