## [XYCTF 2025] Now you see me 1
太阴了，真正的源代码藏起来了
```python
# -*- encoding: utf-8 -*-
'''
@File    :   app.py
@Time    :   2024/12/27 18:27:15
@Author  :   LamentXU 

运行，然后你会发现启动了一个flask服务。这是怎么做到的呢？
注：本题为彻底的白盒题，服务端代码与附件中的代码一模一样。不用怀疑附件的真实性。
'''
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            ;exec(__import__("base64").b64decode('IyBZT1UgRk9VTkQgTUUgOykKIyAtKi0gZW5jb2Rpbmc6IHV0Zi04IC0qLQonJycKQEZpbGUgICAgOiAgIHNyYy5weQpAVGltZSAgICA6ICAgMjAyNS8wMy8yOSAwMToxMDozNwpAQXV0aG9yICA6ICAgTGFtZW50WFUgCicnJwppbXBvcnQgZmxhc2sKaW1wb3J0IHN5cwplbmFibGVfaG9vayA9ICBGYWxzZQpjb3VudGVyID0gMApkZWYgYXVkaXRfY2hlY2tlcihldmVudCxhcmdzKToKICAgIGdsb2JhbCBjb3VudGVyCiAgICBpZiBlbmFibGVfaG9vazoKICAgICAgICBpZiBldmVudCBpbiBbImV4ZWMiLCAiY29tcGlsZSJdOgogICAgICAgICAgICBjb3VudGVyICs9IDEKICAgICAgICAgICAgaWYgY291bnRlciA+IDQ6CiAgICAgICAgICAgICAgICByYWlzZSBSdW50aW1lRXJyb3IoZXZlbnQpCgpsb2NrX3dpdGhpbiA9IFsKICAgICJkZWJ1ZyIsICJmb3JtIiwgImFyZ3MiLCAidmFsdWVzIiwgCiAgICAiaGVhZGVycyIsICJqc29uIiwgInN0cmVhbSIsICJlbnZpcm9uIiwKICAgICJmaWxlcyIsICJtZXRob2QiLCAiY29va2llcyIsICJhcHBsaWNhdGlvbiIsIAogICAgJ2RhdGEnLCAndXJsJyAsJ1wnJywgJyInLCAKICAgICJnZXRhdHRyIiwgIl8iLCAie3siLCAifX0iLCAKICAgICJbIiwgIl0iLCAiXFwiLCAiLyIsInNlbGYiLCAKICAgICJsaXBzdW0iLCAiY3ljbGVyIiwgImpvaW5lciIsICJuYW1lc3BhY2UiLCAKICAgICJpbml0IiwgImRpciIsICJqb2luIiwgImRlY29kZSIsIAogICAgImJhdGNoIiwgImZpcnN0IiwgImxhc3QiICwgCiAgICAiICIsImRpY3QiLCJsaXN0IiwiZy4iLAogICAgIm9zIiwgInN1YnByb2Nlc3MiLAogICAgImd8YSIsICJHTE9CQUxTIiwgImxvd2VyIiwgInVwcGVyIiwKICAgICJCVUlMVElOUyIsICJzZWxlY3QiLCAiV0hPQU1JIiwgInBhdGgiLAogICAgIm9zIiwgInBvcGVuIiwgImNhdCIsICJubCIsICJhcHAiLCAic2V0YXR0ciIsICJ0cmFuc2xhdGUiLAogICAgInNvcnQiLCAiYmFzZTY0IiwgImVuY29kZSIsICJcXHUiLCAicG9wIiwgInJlZmVyZXIiLAogICAgIlRoZSBjbG9zZXIgeW91IHNlZSwgdGhlIGxlc3NlciB5b3UgZmluZC4iXSAKICAgICAgICAjIEkgaGF0ZSBhbGwgdGhlc2UuCmFwcCA9IGZsYXNrLkZsYXNrKF9fbmFtZV9fKQpAYXBwLnJvdXRlKCcvJykKZGVmIGluZGV4KCk6CiAgICByZXR1cm4gJ3RyeSAvSDNkZGVuX3JvdXRlJwpAYXBwLnJvdXRlKCcvSDNkZGVuX3JvdXRlJykKZGVmIHIzYWxfaW5zMWRlX3RoMHVnaHQoKToKICAgIGdsb2JhbCBlbmFibGVfaG9vaywgY291bnRlcgogICAgbmFtZSA9IGZsYXNrLnJlcXVlc3QuYXJncy5nZXQoJ015X2luczFkZV93MHIxZCcpCiAgICBpZiBuYW1lOgogICAgICAgIHRyeToKICAgICAgICAgICAgaWYgbmFtZS5zdGFydHN3aXRoKCJGb2xsb3cteW91ci1oZWFydC0iKToKICAgICAgICAgICAgICAgIGZvciBpIGluIGxvY2tfd2l0aGluOgogICAgICAgICAgICAgICAgICAgIGlmIGkgaW4gbmFtZToKICAgICAgICAgICAgICAgICAgICAgICAgcmV0dXJuICdOT1BFLicKICAgICAgICAgICAgICAgIGVuYWJsZV9ob29rID0gVHJ1ZQogICAgICAgICAgICAgICAgYSA9IGZsYXNrLnJlbmRlcl90ZW1wbGF0ZV9zdHJpbmcoJ3sjJytmJ3tuYW1lfScrJyN9JykKICAgICAgICAgICAgICAgIGVuYWJsZV9ob29rID0gRmFsc2UKICAgICAgICAgICAgICAgIGNvdW50ZXIgPSAwCiAgICAgICAgICAgICAgICByZXR1cm4gYQogICAgICAgICAgICBlbHNlOgogICAgICAgICAgICAgICAgcmV0dXJuICdNeSBpbnNpZGUgd29ybGQgaXMgYWx3YXlzIGhpZGRlbi4nCiAgICAgICAgZXhjZXB0IFJ1bnRpbWVFcnJvciBhcyBlOgogICAgICAgICAgICBjb3VudGVyID0gMAogICAgICAgICAgICByZXR1cm4gJ05PLicKICAgICAgICBleGNlcHQgRXhjZXB0aW9uIGFzIGU6CiAgICAgICAgICAgIHJldHVybiAnRXJyb3InCiAgICBlbHNlOgogICAgICAgIHJldHVybiAnV2VsY29tZSB0byBIaWRkZW5fcm91dGUhJwoKaWYgX19uYW1lX18gPT0gJ19fbWFpbl9fJzoKICAgIGltcG9ydCBvcwogICAgdHJ5OgogICAgICAgIGltcG9ydCBfcG9zaXhzdWJwcm9jZXNzCiAgICAgICAgZGVsIF9wb3NpeHN1YnByb2Nlc3MuZm9ya19leGVjCiAgICBleGNlcHQ6CiAgICAgICAgcGFzcwogICAgaW1wb3J0IHN1YnByb2Nlc3MKICAgIGRlbCBvcy5wb3BlbgogICAgZGVsIG9zLnN5c3RlbQogICAgZGVsIHN1YnByb2Nlc3MuUG9wZW4KICAgIGRlbCBzdWJwcm9jZXNzLmNhbGwKICAgIGRlbCBzdWJwcm9jZXNzLnJ1bgogICAgZGVsIHN1YnByb2Nlc3MuY2hlY2tfb3V0cHV0CiAgICBkZWwgc3VicHJvY2Vzcy5nZXRvdXRwdXQKICAgIGRlbCBzdWJwcm9jZXNzLmNoZWNrX2NhbGwKICAgIGRlbCBzdWJwcm9jZXNzLmdldHN0YXR1c291dHB1dAogICAgZGVsIHN1YnByb2Nlc3MuUElQRQogICAgZGVsIHN1YnByb2Nlc3MuU1RET1VUCiAgICBkZWwgc3VicHJvY2Vzcy5DYWxsZWRQcm9jZXNzRXJyb3IKICAgIGRlbCBzdWJwcm9jZXNzLlRpbWVvdXRFeHBpcmVkCiAgICBkZWwgc3VicHJvY2Vzcy5TdWJwcm9jZXNzRXJyb3IKICAgIHN5cy5hZGRhdWRpdGhvb2soYXVkaXRfY2hlY2tlcikKICAgIGFwcC5ydW4oZGVidWc9RmFsc2UsIGhvc3Q9JzAuMC4wLjAnLCBwb3J0PTUwMDApCg=='))                                                                 
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")                                                                 
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
print("Hello, world!")
```
这里看起来就是全是print但是，运行确实会启动一个flask服务，原来在后面，base64解码：
```python
# YOU FOUND ME ;)
# -*- encoding: utf-8 -*-
'''
@File    :   src.py
@Time    :   2025/03/29 01:10:37
@Author  :   LamentXU 
'''
import flask
import sys
enable_hook =  False
counter = 0
def audit_checker(event,args):
    global counter
    if enable_hook:
        if event in ["exec", "compile"]:
            counter += 1
            if counter > 4:
                raise RuntimeError(event)

lock_within = [
    "debug", "form", "args", "values", 
    "headers", "json", "stream", "environ",
    "files", "method", "cookies", "application", 
    'data', 'url' ,'\'', '"', 
    "getattr", "_", "{{", "}}", 
    "[", "]", "\\", "/","self", 
    "lipsum", "cycler", "joiner", "namespace", 
    "init", "dir", "join", "decode", 
    "batch", "first", "last" , 
    " ","dict","list","g.",
    "os", "subprocess",
    "g|a", "GLOBALS", "lower", "upper",
    "BUILTINS", "select", "WHOAMI", "path",
    "os", "popen", "cat", "nl", "app", "setattr", "translate",
    "sort", "base64", "encode", "\\u", "pop", "referer",
    "The closer you see, the lesser you find."] 
        # I hate all these.
app = flask.Flask(__name__)
@app.route('/')
def index():
    return 'try /H3dden_route'
@app.route('/H3dden_route')
def r3al_ins1de_th0ught():
    global enable_hook, counter
    name = flask.request.args.get('My_ins1de_w0r1d')
    if name:
        try:
            if name.startswith("Follow-your-heart-"):
                for i in lock_within:
                    if i in name:
                        return 'NOPE.'
                enable_hook = True
                a = flask.render_template_string('{#'+f'{name}'+'#}')
                enable_hook = False
                counter = 0
                return a
            else:
                return 'My inside world is always hidden.'
        except RuntimeError as e:
            counter = 0
            return 'NO.'
        except Exception as e:
            return 'Error'
    else:
        return 'Welcome to Hidden_route!'

if __name__ == '__main__':
    import os
    try:
        import _posixsubprocess
        del _posixsubprocess.fork_exec
    except:
        pass
    import subprocess
    del os.popen
    del os.system
    del subprocess.Popen
    del subprocess.call
    del subprocess.run
    del subprocess.check_output
    del subprocess.getoutput
    del subprocess.check_call
    del subprocess.getstatusoutput
    del subprocess.PIPE
    del subprocess.STDOUT
    del subprocess.CalledProcessError
    del subprocess.TimeoutExpired
    del subprocess.SubprocessError
    sys.addaudithook(audit_checker)
    app.run(debug=False, host='0.0.0.0', port=5000)

```
