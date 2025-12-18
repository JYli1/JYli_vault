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
相当于使用`iconv()`函数处理数据，但是这个过滤器不支持参数，但可使用输入/输出的编码名称，组成过滤器名称，比如 `convert.iconv.<input-encoding>.<output-encoding>` 或 `convert.iconv.<input-encoding>/<output-encoding>` （两种写法的语义都相同）
表示把读取的内容从一种编码转化成例外一种编码

`iconv` 函数用于将字符串从一种编码转换为另一种编码
```php
iconv(string $from_encoding, string $to_encoding, string $string): string|false
```


## base64decode的垃圾处理
==base64的有效字符包含大小写字母(a-z,A-Z)，数字(0-9)，两个额外字符(+，/)，另外还有一个填充字符(=)

`base64_decode()`：
函数在解码时，不在意字符串中的非法字符，也就是你在哪里添加一些非法字符他会自动忽略
`convert.base64-decode`：
过滤器，在这一点上也是很相似，但是有一个地方就是对于`=`的处理上有点区别，如果出现`=`或者其他原因，会无法正常解码。为了解决这个问题，我们想到：
filter过滤器阔以设置多个，是否阔以利用`convert.iconv.*`过滤器把字符串编码后再base64解码，这样就没有`=`了
