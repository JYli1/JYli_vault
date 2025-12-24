https://forum.butian.net/share/2262
http://freebuf.com/articles/web/264615.html
##  漏洞原原因
 在 Smarty 中，$smarty->display() 函数通常接收一个文件名作为参数（例如 index.tpl）。但是，Smarty 支持一种特殊的资源类型前缀 string:。  
当你使用 $smarty->display('string:' . $ip) 时，Smarty 不会去寻找文件，而是**直接把 $ip 后面的字符串当作模板源代码**进行解析和编译。
就像这样：
```php
<?php
require_once('./libs/' . 'Smarty.class.php');
$smarty = new Smarty();
$ip = $_POST['data'];
$smarty->display('string:'.$ip);
?>
```
这里把我们输入的`$ip`直接当作模板渲染。这意味着如果用户输入了 Smarty 的特殊标签（例如 {...}），Smarty 引擎会认为这是程序逻辑的一部分并执行它。

## 常见攻击方法 
### 文件读取 {include}标签
 
 {include file='\<filename>'}
### 访问类的静态成员或静态方法
在Smarty模板引擎中，`self`关键字代表当前类本身，通常用于访问类的静态成员或静态方法。

可利用的方法：
1. `getStreamVariable()`读文件:
   ```php
   public function getStreamVariable($variable)
{
        $_result = '';
        $fp = fopen($variable, 'r+');
        if ($fp) {
            while (!feof($fp) && ($current_line = fgets($fp)) !== false) {
                $_result .= $current_line;
            }
            fclose($fp);
            return $_result;
        }
        $smarty = isset($this->smarty) ? $this->smarty : $this;
        if ($smarty->error_unassigned) {
            throw new SmartyException('Undefined stream variable "' . $variable . '"');
        } else {
            return null;
        }
    }
//值得注意的是$variable就是我们要传递的文件的路径。
   ```
   这个方法之存在于`Smarty<=3.1.29`的版本，在Smarty 3.1.30版本中官方以及删除这个方法。
2. `writeFile()`方法:
   