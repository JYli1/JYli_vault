# AWDP Fix 修补篇 — Pwn 方向

## 核心原则

> Pwn 的 Fix 本质是 **patch 二进制文件**，不是改源码。
> 你拿到的是编译好的 ELF，需要直接修改机器码。

**关键约束**：
1. 不能改变文件大小（大多数情况）— 只能原地替换指令
2. 不能破坏正常功能 — checker 会测试正常输入输出
3. 修改后重新打包为 tar.gz 上传

---

## 一、Patch 工具选择

| 工具 | 适用场景 | 优势 |
|------|----------|------|
| **IDA Pro + Keypatch** | 通用 patch | 可视化，直接改汇编 |
| **Ghidra** | 免费替代 IDA | 自带 patch 功能 |
| **patchelf** | 修改 ELF 头信息 | 改 interpreter、RPATH |
| **LIEF (Python)** | 脚本化 patch | 自动化批量修改 |
| **hexedit / 010 Editor** | 直接改字节 | 精确控制 |
| **radare2 (r2)** | 命令行 patch | 快速，适合脚本化 |

---

## 二、IDA Keypatch 操作流程

这是比赛中最常用的方式：

```
1. IDA 打开二进制文件
2. 定位到漏洞函数
3. Edit → Patch program → Assemble（或用 Keypatch 插件）
4. 修改汇编指令
5. Edit → Patch program → Apply patches to input file
6. 保存，打包上传
```

### Keypatch 安装
- IDA 插件：https://github.com/keystone-engine/keypatch
- 安装后按 `Ctrl+Alt+K` 即可在当前地址写汇编

---

## 三、常见漏洞的 Patch 方法

### 3.1 栈溢出 — 限制读入长度

**漏洞**：`read(0, buf, 0x100)` 但 buf 只有 0x20 字节

```nasm
; 修复前（读 0x100 字节）
mov     edx, 100h       ; size = 0x100
lea     rsi, [rbp-20h]  ; buf
mov     edi, 0           ; fd = stdin
call    read

; 修复后（改 size 为 0x20）
mov     edx, 20h        ; size = 0x20 ← 只改这一条指令
lea     rsi, [rbp-20h]
mov     edi, 0
call    read
```

**对应的字节修改**：
```
修复前: BA 00 01 00 00    (mov edx, 0x100)
修复后: BA 20 00 00 00    (mov edx, 0x20)
```

**gets() 替换为安全函数**：
```nasm
; gets(buf) 没有长度限制，非常危险
; 方案：把 call gets 改为 call fgets 或直接 nop 掉

; 如果程序中有 fgets@plt，可以替换
; 但通常更简单的做法是：在 gets 前加长度检查
; 或者直接 patch 调用参数
```

### 3.2 格式化字符串 — 替换为安全调用

**漏洞**：`printf(buf)` → 用户可控格式化字符串

```nasm
; 修复前
lea     rdi, [rbp-40h]   ; rdi = buf（用户输入）
call    printf

; 修复后 — 改为 puts（自动加换行，不解析格式化）
lea     rdi, [rbp-40h]
call    puts              ; 替换 printf 为 puts

; 或者改为 printf("%s", buf)
; 需要找一个 "%s" 字符串的地址
lea     rsi, [rbp-40h]   ; rsi = buf（第二个参数）
lea     rdi, [fmt_s]     ; rdi = "%s"（第一个参数）
call    printf
```

**puts 替换的字节操作**：
```
找到 printf@plt 和 puts@plt 的地址
修改 call 指令的偏移量即可

call 指令格式: E8 xx xx xx xx
偏移量 = 目标地址 - (当前地址 + 5)
```

### 3.3 堆漏洞 — UAF 修复

**漏洞**：free 后指针未清零

```nasm
; 修复思路：在 free 之后，将指针置零
; 找到 free(ptr) 调用后面，加入 mov [存储指针的地址], 0

; 如果空间不够插入新指令，可以：
; 1. 找到附近的 nop sled 或无用代码区域
; 2. 用 jmp 跳过去执行清零操作再跳回来（trampoline）
```

### 3.4 整数溢出修复

```nasm
; 修复前：没有检查 size 参数
mov     edi, [rbp-4]     ; size（用户输入，可能为负数）
call    malloc

; 修复后：加范围检查
cmp     dword [rbp-4], 0
jle     error_handler     ; size <= 0 则跳转到错误处理
cmp     dword [rbp-4], 1000h
jge     error_handler     ; size >= 0x1000 则跳转
mov     edi, [rbp-4]
call    malloc
```

---

## 四、用 LIEF (Python) 自动化 Patch

```python
import lief

binary = lief.parse("./pwn1")

# 方法 1：直接修改字节
# 将地址 0x401234 处的字节改为 nop (0x90)
binary.patch_address(0x401234, [0x90, 0x90, 0x90, 0x90, 0x90])

# 方法 2：修改指令（需要配合 keystone）
from keystone import Ks, KS_ARCH_X86, KS_MODE_64
ks = Ks(KS_ARCH_X86, KS_MODE_64)

# 汇编新指令
code, _ = ks.asm("mov edx, 0x20")
binary.patch_address(0x401234, list(code))

# 保存
binary.write("pwn1_patched")
```

```bash
# 别忘了加执行权限
chmod +x pwn1_patched
```

---

## 五、用 radare2 快速 Patch

```bash
# 打开文件（写模式）
r2 -w ./pwn1

# 定位到漏洞地址
s 0x401234

# 写入汇编指令
wa mov edx, 0x20

# 写入原始字节
wx 90909090

# 用 nop 填充
wao nop

# 保存退出
q
```

---

## 六、常用 Patch 技巧速查表

| 漏洞类型 | Patch 方法 | 具体操作 |
|----------|-----------|---------|
| 栈溢出（read/gets） | 限制读入长度 | 修改 `mov edx, size` 中的 size 值 |
| 栈溢出（strcpy） | 替换为 strncpy | 修改 call 目标 + 加 size 参数 |
| 格式化字符串 | printf→puts | 修改 call 偏移，指向 puts@plt |
| UAF | free 后置零指针 | 在 free 后插入 `mov [ptr], 0` |
| Double Free | 加 free 前检查 | 检查指针是否为 0 再 free |
| 整数溢出 | 加范围检查 | 插入 cmp + jmp 指令 |
| 后门函数 | 直接 nop 掉 | 用 0x90 覆盖整个函数调用 |
| 危险函数调用 | nop 或替换 | 用 nop 填充或改 call 目标 |

---

## 七、Patch 空间不够时的 Trampoline 技巧

当需要插入的指令比原指令长时，没有足够空间：

```
原理：
1. 在原位置写一个 jmp 到空闲区域（5 字节）
2. 在空闲区域写完整的 patch 代码
3. 最后 jmp 回原来的下一条指令

寻找空闲区域：
- .eh_frame 段（通常不影响运行）
- 函数末尾的 padding（nop sled）
- 未使用的函数体
```

```nasm
; 原位置（假设在 0x401200）
jmp     0x402000          ; 跳到空闲区域（5 字节）
nop                       ; 填充剩余空间

; 空闲区域（0x402000）
cmp     dword [rbp-4], 0  ; 你的 patch 代码
jle     error_handler
mov     edi, [rbp-4]
call    malloc
jmp     0x401210          ; 跳回原来的下一条指令
```

---

## 八、Pwn Fix 打包提交

```bash
# 打包修改后的二进制
tar -czvf fix.tar.gz pwn1

# 验证
tar -tzvf fix.tar.gz
# 应该只有一个文件：pwn1

# 确认文件可执行
chmod +x pwn1
./pwn1  # 本地测试正常功能
```

### 常见踩坑
- 忘记 `chmod +x` → 程序无法执行
- patch 改变了文件大小 → 某些 checker 会校验
- patch 破坏了正常逻辑 → 功能测试不通过
- 打包时多了目录层级 → checker 找不到文件

---

## 九、glibc 2.34+ 的 Patch 注意事项

glibc 2.34 移除了 `__malloc_hook` 和 `__free_hook`，这意味着：
- 如果题目的漏洞利用依赖 hook 劫持，patch 时不需要特别处理 hook
- 但如果题目 glibc 较老（2.27-2.31），攻击者可能通过 hook 来 getshell
- **Fix 思路**：直接修复根本漏洞（UAF/溢出），而不是试图保护 hook

### 针对堆题的通用 Patch 策略

```
1. UAF → 在 free 后将指针置零（最关键）
2. Double Free → 在 free 前检查指针是否为 NULL
3. 堆溢出 → 限制写入长度（修改 read/memcpy 的 size 参数）
4. 如果空间不够 → 用 trampoline 跳到 .eh_frame
5. 如果实在改不动 → 考虑直接 nop 掉 free 调用（牺牲内存换安全）
   注意：nop 掉 free 可能导致内存泄漏，但短时间内 checker 通常不会检测这个
```

---

## 十、seccomp 相关的 Patch

如果题目本身没有 seccomp 但你想加固：

```nasm
; 在程序入口处加 seccomp 规则（高级技巧，空间要求大）
; 通常不推荐在 AWDP 中使用，因为可能破坏功能
; 更好的做法是直接修复漏洞本身
```

如果题目有 seccomp 但规则不够严格：
```
; 可以考虑收紧 seccomp 规则
; 但这需要较大的 patch 空间，通常不现实
; 还是优先修复漏洞本身
```
