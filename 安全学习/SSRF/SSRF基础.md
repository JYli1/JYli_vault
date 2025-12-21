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

