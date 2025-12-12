[Python|基于Bottle的SSTI注入 | TGlu'blog](https://tg1u.top/2025/04/17/Python-%E5%9F%BA%E4%BA%8EBottle%E7%9A%84SSTI%E6%B3%A8%E5%85%A5/)
[Bottle框架的模板引擎安全问题分析-先知社区](https://xz.aliyun.com/news/17430)
https://www.tremse.cn/2025/04/12/bottle%E6%A1%86%E6%9E%B6%E7%9A%84%E4%B8%80%E4%BA%9B%E7%89%B9%E6%80%A7/#cookie%E5%A4%84%E7%90%86%E6%9C%BA%E5%88%B6
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
- `<%`也行
- `%`用来放置单行代码
- `{{ }}`用来放置变量
但是最通用的是`{{ }}`,另外两个因为不会回显内容
# cookie处理机制
首先说个结论：
如果用bottle的get_cookie函数来解析cookie的话，是会触发pickle的反序列化的，后果就是有空可钻了。
