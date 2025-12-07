# 【check_in】
```python
'''
I wish you a good head start.
flag is in file namely 'flag' in the same directory as this file.

Good luck!
'''

import re
import flask
import requests
import ipaddress
from urllib.parse import urlparse

GENERAL_WAF_REGEX = r'[a-zA-Z0-9_\[\]{}()<>,.!@#$^&*]{3}' # only two of these characters ;)

app = flask.Flask(__name__)

def general_waf(code):
    # Why do you need so many characters?
    if re.findall(GENERAL_WAF_REGEX, code):
        return True
    else:
        return False

def check_hostname(url):
    # must starts with vnctf.
    if not url.startswith('http://vnctf.'):
        return False

    hostname = urlparse(url).hostname
    query = urlparse(url).query

    # must only contain two of the restricted characters
    if general_waf(query):
        return False

    # must not be an ip address, so no 127.0.0.1 or ::1
    try:
        ipaddress.ip_address(hostname)
        return False
    except ValueError:
        pass

    return url

@app.route('/')
def index():
    return 'Welcome to MINI VNCTF 2025!'

@app.route('/fetch')
def fetch():
    url = flask.request.args.get('url')
    safe_url = check_hostname(url)
    if safe_url:
        try:
            response = requests.get(safe_url, allow_redirects=False) # no redirects
            return response.text
        except:
            return 'Error'
    else:
        return 'Invalid URL'

@app.route('/__internal/safe_eval')
def safe_eval():
    # check if the request is from the internal network
    if flask.request.remote_addr not in ['127.0.0.1', '::1']:
        return 'Forbidden'

    code = flask.request.args.get('hi')

    if len(code) >= 24 * 10 + 8 * 8:
        # Man! What can I say. 
        return 'Invalid code'

    # Ah, if you get here, then your final challenge is to break this jail.
    # Try it. Not as hard as it seems ;)
    blacklist = ['\\x','+','join', '"', "'", '[', ']', '2', '3', '4', '5', '6', '7', '8', '9']
    for i in blacklist:
        if i in code:
            return 'Invalid code'
    
    safe_globals = {'__builtins__':None, 'lit':list, 'dic':dict}

    return repr(eval(code, safe_globals))

if __name__ == '__main__':
    app.run(debug=False, host='0.0.0.0', port=8080)


```
给了源码，我们大致的看一下功能。
* 一个`/`路由就是一个欢迎页面。
* `/fetch`路由
```python
  @app.route('/fetch')
def fetch():
    url = flask.request.args.get('url')
    safe_url = check_hostname(url)
    if safe_url:
        try:
            response = requests.get(safe_url, allow_redirects=False) # no redirects
            return response.text
        except:
            return 'Error'
    else:
        return 'Invalid URL'
  ```
  接受`get`传的参数`url`，经过check函数之后会向后面指定的`url`发送`get`请求。
  * `check_hostname`函数
  ```python
  def check_hostname(url):
    # must starts with vnctf.
    if not url.startswith('http://vnctf.'):
        return False

    hostname = urlparse(url).hostname
    query = urlparse(url).query

    # must only contain two of the restricted characters
    if general_waf(query):
        return False

    # must not be an ip address, so no 127.0.0.1 or ::1
    try:
        ipaddress.ip_address(hostname)
        return False
    except ValueError:
        pass

    return url
  
  ```
  检查是否`http://vnctf.`开头，并分割`hostname`主机、`query`?后面的参数。
  所以这里？后面的参数不能含有黑名单中的字符。最后还会检查传入的是不是真正的IP，是的话就Flase了。
  通过上面的检查后服务器就会向合法的url发送请求。很明显应该是打ssrf。我们继续看。
  * `/__internal/safe_eval`路由
  ```python
  @app.route('/__internal/safe_eval')
def safe_eval():
    # check if the request is from the internal network
    if flask.request.remote_addr not in ['127.0.0.1', '::1']:
        return 'Forbidden'

    code = flask.request.args.get('hi')

    if len(code) >= 24 * 10 + 8 * 8:
        # Man! What can I say.
        return 'Invalid code'

    # Ah, if you get here, then your final challenge is to break this jail.
    # Try it. Not as hard as it seems ;)
    blacklist = ['\\x','+','join', '"', "'", '[', ']', '2', '3', '4', '5', '6', '7', '8', '9']
    for i in blacklist:
        if i in code:
            return 'Invalid code'

    safe_globals = {'__builtins__':None, 'lit':list, 'dic':dict}

    return repr(eval(code, safe_globals))
  ```
  首先请求必须来自本地才能访问这里，（更加确定是ssrf）
  然后长度不能大于`24 * 10 + 8 * 8=304`
  最后经过一个黑名单检测会进入到eval（很明显的沙箱逃逸）
  ```python
  safe_globals = {'__builtins__':None, 'lit':list, 'dic':dict}

    return repr(eval(code, safe_globals))
  ```
  去掉了`__builtins__`模块，给了我没`list`和`dict`对象
## 绕过
1. 获得本地身份，要走到沙箱逃逸要先打通ssrf，`http://vnctf.`开头且不能被`ipaddress.ip_address(hostname)`解析成IP。我们想到用@连接，`/fetch?url=http://vnctf.@localhost:8080`，这样就满足了，并且http是支持这样写的。我们发现访问8080端口是欢迎页面，源代码中正是开在8080，那这里就是ssrf打通。
2. 绕过两个黑名单沙箱逃逸。首先第一个对url字符的过滤我们可以用`两次url编码`，因为waf规定3次出现黑名单字符则过滤，但是没有过滤`%`，url编码刚好是`%xx`,每处现两次黑名单字符就会打断，完美绕过。这里是因为ssrf要代理发一个包，所以我们要两次url编码。
3. 最后一个黑名单过滤的沙箱逃逸了。首先数字只有`0,1`所以肯定不是常规的利用`subclasses[x]`。我们这里li