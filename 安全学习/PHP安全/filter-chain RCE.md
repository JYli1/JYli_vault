php://filter是php伪协议中常用的，一般我们用来读取源代码
```php
include(php://filter/read=convert.base64-encode/resource=【文件名】);
```
![700](assets/filter-chain%20RCE/file-20251218131840603.png)
一直听过有一种filter链打RCE的方式但是一直都没去看过，感觉很难，这次VN面试也问到了，还是有些后悔，现在来学习下一下。

# 前置知识
首先我们要知道