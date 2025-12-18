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
```bash
root@2406768acb7a:/var/www/html# cat test1.txt                                                                                                                                    
YmFzZTY0==                                                                                                                                                         

root@2406768acb7a:/var/www/html# php -r "echo file_get_contents('php://filter/read=convert.iconv.UTF8.UTF7/convert.base64-decode/resource=test1.txt');"                           

base64���
```
看到成功解码了，这里时先从UTF8编码为UTF7，这样`=`就消除了，然后再base64解码

## 字符序列标志
```
字节顺序标记（BOM）
   Unicode标准和ISO 10646定义了字符“ZERO WIDTH
   非中断空间”（0xFEFF），也被非正式地称为“字节
   订单标记”（缩写为“BOM”）。后一个名字暗示了第二个
   字符的可能用法，除了作为
   真正的“零宽度非中断空间”内的文字。这种用法，
   根据Unicode第2.4节和ISO 10646附录F（资料性）的建议，
   是将0xFEFF字符前置到Unicode字符流，
   “签名”;这样的串行化流的接收器然后可以使用
   初始字符作为流由以下内容组成的提示
   Unicode字符和作为识别序列化顺序的一种方法。
   在序列化的UTF-16中，前缀有这样的签名，顺序是
   big-endian，如果前两个八位字节是0xFE后跟0xFF;如果它们
   是0xFF后面跟着0xFE，顺序是little-endian。注意
   0xFFFE不是Unicode字符，正是为了保留
   0xFEFF作为字节顺序标记的有用性。
```