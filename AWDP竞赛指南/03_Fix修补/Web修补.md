# AWDP Fix 修补篇 — Web 方向

## 核心原则

> Fix 的目标：**堵住漏洞，但不能破坏正常功能**。
> Checker 会同时测试功能和安全性，改太多容易功能挂掉，改太少漏洞堵不住。

**黄金法则**：
1. 只改最小范围 — 不要重构代码
2. 先理解正常功能 — 再动手修
3. 改完本地测一下 — 确保功能正常
4. 注意打包格式 — 路径和文件名不能变

---

## 一、SQL 注入修复

### 方法 1：参数化查询（最推荐）

```php
// 修复前（漏洞代码）
$id = $_GET['id'];
$sql = "SELECT * FROM users WHERE id = '$id'";
$result = mysqli_query($conn, $sql);

// 修复后（参数化查询）
$id = $_GET['id'];
$stmt = $conn->prepare("SELECT * FROM users WHERE id = ?");
$stmt->bind_param("s", $id);
$stmt->execute();
$result = $stmt->get_result();
```

### 方法 2：强制类型转换（简单场景）

```php
// 如果 id 一定是整数
$id = intval($_GET['id']);
$sql = "SELECT * FROM users WHERE id = $id";
```

### 方法 3：过滤/转义（应急方案）

```php
// PHP
$id = mysqli_real_escape_string($conn, $_GET['id']);

// Python
import re
if not re.match(r'^[a-zA-Z0-9_]+$', user_input):
    return "invalid input"
```

### Python (Flask + SQLAlchemy) 修复

```python
# 修复前
@app.route('/user')
def user():
    id = request.args.get('id')
    sql = f"SELECT * FROM users WHERE id = '{id}'"
    result = db.execute(sql)

# 修复后
@app.route('/user')
def user():
    id = request.args.get('id')
    result = db.execute("SELECT * FROM users WHERE id = :id", {"id": id})
```

---

## 二、命令注入修复

### 方法 1：白名单校验（最推荐）

```php
// 修复前
$ip = $_GET['ip'];
system("ping -c 1 " . $ip);

// 修复后 — 白名单校验
$ip = $_GET['ip'];
if (!preg_match('/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/', $ip)) {
    die("Invalid IP");
}
system("ping -c 1 " . $ip);
```

### 方法 2：escapeshellarg（PHP）

```php
$ip = $_GET['ip'];
system("ping -c 1 " . escapeshellarg($ip));
```

### 方法 3：过滤危险字符

```php
// 过滤所有 shell 元字符
$input = preg_replace('/[;&|`$(){}\'\"\\\\]/', '', $input);

// Python
import shlex
subprocess.run(['ping', '-c', '1', shlex.quote(ip)])
# 或者直接用列表形式（不经过 shell）
subprocess.run(['ping', '-c', '1', ip], shell=False)
```

---

## 三、文件上传修复

### 综合修复方案

```php
// 修复后
$allowed_ext = ['jpg', 'jpeg', 'png', 'gif'];
$allowed_mime = ['image/jpeg', 'image/png', 'image/gif'];

$file = $_FILES['upload'];
$ext = strtolower(pathinfo($file['name'], PATHINFO_EXTENSION));
$mime = mime_content_type($file['tmp_name']);

// 1. 检查后缀（白名单）
if (!in_array($ext, $allowed_ext)) {
    die("不允许的文件类型");
}

// 2. 检查 MIME 类型
if (!in_array($mime, $allowed_mime)) {
    die("不允许的文件类型");
}

// 3. 重命名文件（防止路径穿越）
$new_name = md5(uniqid()) . '.' . $ext;
move_uploaded_file($file['tmp_name'], '/uploads/' . $new_name);
```

### 关键点
- 用白名单而非黑名单
- 同时检查后缀和 MIME
- 重命名上传文件
- 如果有 `.htaccess` 上传漏洞，在上传目录加 `.htaccess` 禁止执行：
  ```
  php_flag engine off
  ```

---

## 四、SSTI 修复

### 方法 1：不要把用户输入直接渲染为模板

```python
# 修复前（漏洞代码）
@app.route('/hello')
def hello():
    name = request.args.get('name', '')
    return render_template_string(f"Hello {name}")

# 修复后 — 用模板变量传递
@app.route('/hello')
def hello():
    name = request.args.get('name', '')
    return render_template_string("Hello {{ name }}", name=name)
```

### 方法 2：过滤危险字符

```python
# 如果必须拼接，过滤模板语法字符
import re
name = re.sub(r'[{}\[\]()_\'\"\\]', '', name)
```

---

## 五、PHP 反序列化修复

### 方法 1：替换为 json

```php
// 修复前
$data = unserialize($_GET['data']);

// 修复后
$data = json_decode($_GET['data'], true);
```

### 方法 2：限制反序列化的类

```php
// PHP 7.0+ 支持 allowed_classes
$data = unserialize($input, ['allowed_classes' => ['SafeClass']]);

// 或者完全禁止对象
$data = unserialize($input, ['allowed_classes' => false]);
```

### 方法 3：修复 POP 链中的危险操作

```php
// 找到链中执行命令的地方，加过滤
class Logger {
    public function __destruct() {
        // 修复前
        system($this->cmd);
        
        // 修复后 — 删除或注释掉危险操作
        // system($this->cmd);
        // 或者加白名单
        if (!in_array($this->cmd, ['ls', 'whoami'])) {
            return;
        }
    }
}
```

---

## 六、SSRF 修复

```php
// 修复方案：校验 URL，禁止内网访问
function is_safe_url($url) {
    $parsed = parse_url($url);
    $host = $parsed['host'] ?? '';
    
    // 禁止内网 IP
    $ip = gethostbyname($host);
    $forbidden = ['127.0.0.1', '0.0.0.0', 'localhost'];
    
    if (in_array($ip, $forbidden)) return false;
    
    // 禁止内网段
    if (preg_match('/^(10\.|172\.(1[6-9]|2[0-9]|3[01])\.|192\.168\.)/', $ip)) {
        return false;
    }
    
    // 只允许 http/https 协议
    $scheme = strtolower($parsed['scheme'] ?? '');
    if (!in_array($scheme, ['http', 'https'])) return false;
    
    return true;
}

// 使用
$url = $_GET['url'];
if (!is_safe_url($url)) {
    die("Forbidden");
}
```

```python
# Python 版
from urllib.parse import urlparse
import ipaddress
import socket

def is_safe_url(url):
    parsed = urlparse(url)
    if parsed.scheme not in ('http', 'https'):
        return False
    try:
        ip = socket.gethostbyname(parsed.hostname)
        return not ipaddress.ip_address(ip).is_private
    except:
        return False
```

---

## 七、文件包含修复

```php
// 修复前
include($_GET['file']);

// 修复方案 1：白名单
$allowed = ['home.php', 'about.php', 'contact.php'];
$file = $_GET['file'];
if (in_array($file, $allowed)) {
    include($file);
}

// 修复方案 2：过滤路径穿越
$file = basename($_GET['file']);  // 去掉路径，只保留文件名
include('./pages/' . $file);

// 修复方案 3：过滤协议
$file = $_GET['file'];
if (preg_match('/^(php|data|http|ftp|phar|zip|glob):/i', $file)) {
    die("Forbidden");
}
if (strpos($file, '..') !== false) {
    die("Forbidden");
}
```

---

## 八、XSS 修复

```php
// 输出时转义
echo htmlspecialchars($user_input, ENT_QUOTES, 'UTF-8');
```

```python
# Flask 默认会转义 Jinja2 模板中的变量
# 但如果用了 |safe 或 Markup()，需要去掉
# 修复前
return render_template('page.html', content=Markup(user_input))
# 修复后
return render_template('page.html', content=user_input)
```

---

## 九、Fix 打包提交速查

```bash
# 1. 确认修改了哪些文件
diff -r original_src/ fixed_src/

# 2. 进入源码目录打包
cd fixed_src/
tar -czvf ../fix.tar.gz ./*

# 3. 或者只打包修改的文件（推荐）
tar -czvf ../fix.tar.gz index.php config.php lib/db.php

# 4. 验证打包内容
tar -tzvf fix.tar.gz
```

### 常见踩坑
- 打包路径多了一层目录 → checker 找不到文件 → 修复失败
- 改了不该改的文件 → 功能测试不通过 → 修复失败
- 文件编码变了（UTF-8 BOM）→ PHP 报错 → 修复失败
- Windows 换行符 CRLF → Linux 上可能出问题
