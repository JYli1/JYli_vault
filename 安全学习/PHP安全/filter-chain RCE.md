php://filter是php伪协议中常用的，一般我们用来读取源代码
```php
include(php://filter/read=convert.base64-encode/resource=【文件名】);
```
![700](assets/filter-chain%20RCE/file-20251218131840603.png)
一直听过有一种filter链打RCE的方式但是一直都没去看过，感觉很难，这次VN面试也问到了，还是有些后悔，现在来学习下一下。

# 前置知识
首先我们要知道一些过滤器的知识
现在可用过滤器列表有四种：
- 字符串过滤器
- 转换过滤器
- 压缩过滤器
- 加密过滤器
下面我们了解一下我们要用到的：
## 转换过滤器
这就是我们常用的`convert.base64-encode` 和 `convert.base64-decode`
分别表示对读取的内容进行 base64加解密。
## 压缩过滤器
`convert.iconv.*
相当于使用iconv()函数处理数据，但是这个过滤器不支持参数，但可使用输入/输出的编码名称，组成过滤器名称，比如 `convert.iconv.<input-encoding>.<output-encoding>` 或 `convert.iconv.<input-encoding>/<output-encoding>` （两种写法的语义都相同）
表示把读取的nei'r