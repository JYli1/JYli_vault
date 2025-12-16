# 0x01 过滤 `引号
1. `str()`函数获取字符串，+索引
```bash
>>> ().__class__.__new__
<built-in method __new__ of type object at 0x9597e0>
>>> str(().__class__.__new__)
'<built-in method __new__ of type object at 0x9597e0>'
>>> str(().__class__.__new__)[21]
'w'
>>> str(().__class__.__new__)[21]+str(().__class__.__new__)[13]+str(().__class__.__new__)[14]+str(().__class__.__new__)[40]+str(().__class__.__new__)[10]+str(().__class__.__new__)[3]
'whoami'
```
2. `chr()`函数
```bash
>>> chr(56)
'8'
```
3. `list + dict `构造任意字符串
```python
list(dict(whoami=1))[0]
```
`dict`将式子转化为字典，`list`取出键位列表
4. `doc()`变量
__doc__ 变量可以获取到类的说明信息，从其中索引出想要的字符然后进行拼接就可以得到字符串：
```python
().__doc__.find('s')
().__doc__[19]+().__doc__[86]+().__doc__[19]
```
# 0x02 过滤`+`
过滤了 + 号主要影响到了构造字符串，假如题目过滤了引号和加号，构造字符串还可以使用 join 函数，初始的字符串可以通过 str() 进行获取.具体的字符串内容可以从 `__doc__` 中取，

```python
str().join(().__doc__[19],().__doc__[23])
```

# 0x03 绕过基于 sys.addaudithook 的 audit hook
## __loader__.load_module导入模块
