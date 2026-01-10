# 《数据结构》课程设计报告：长整数四则运算

**班级:** [请填写您的班级]
**姓名学号:** [请填写您的姓名及学号]
**完成日期:** 2026年1月10日

## 1. 问题描述和需求分析

### 程序任务
在C++中，像 `int` 或 `long long` 这样的基本数据类型都有其取值范围的上限。当我们需要处理的数字非常大，超过了 `long long` 的最大值（大约是 9x10^18）时，程序就无法正确存储和计算了。本次课程设计的任务就是解决这个问题，通过自己设计数据结构和算法，实现一个可以处理任意长度整数（长整数）的加法和减法计算器。

### 输入输出形式
*   **输入形式**: 用户通过键盘输入一个很长的数字字符串来表示一个长整数。例如，`12345678901234567890`。程序也支持输入负数，例如 `-987654321`。
*   **输出形式**: 程序的计算结果也以数字字符串的形式完整显示在屏幕上。

### 预设测试数据
为了验证程序的正确性，我计划使用以下 7 组测试数据：
1.  **大数加法**: `1234567890123456789 + 9876543210987654321`
2.  **加法进位**: `999999999 + 1`
3.  **大数减法**: `1000000000000000000 - 123456789`
4.  **减法借位**: `1000000000 - 1`
5.  **结果为负**: `12345 - 67890`
6.  **异号运算 (A + (-B))**: `1000000000 + (-1)`
7.  **异号运算 (A - (-B))**: `1000000000 - (-1)`

## 2. 概要设计

### 存储结构设计
直接用字符串来模拟运算是非常复杂的，因为需要逐个处理字符，效率不高。为了简化计算，特别是进位和借位，我采用了“换进制”的思想，将原本的十进制大数转换成一个更高进制的数来存储。

我选择的基数是 **10^9**（十亿）。选择它的原因有两个：
1.  **效率**: 10^9 足够大，可以将一个很长的十进制数压缩成一个长度短得多的数组，节约了存储空间，也减少了计算的循环次数。
2.  **安全性**: `int` 类型的最大值大约是 2x10^9。两个 10^9 以内的数相加，结果最大约为 2x10^9，不会超出 `int` 的范围（虽然为了更保险，我在计算时用了 `long long` 来存放中间结果）。

基于这个思想，我设计了如下的 `BigInt` 结构体：
```cpp
struct BigInt {
    std::vector<int> digits; // 用动态数组存储长整数的每一“位”
    bool is_negative;        // 存储正负号
};
```
`digits` 这个 `vector` 从低位到高位存储数字。例如，对于数字 `1234567890123`，它会被拆分成 `[456789012, 123]`。`digits[0]` 存的是最低位（`456789012`），`digits[1]` 存的是高位（`123`）。

### 主程序逻辑步骤
程序的主要工作流程如下：
1.  **显示菜单并获取选择**: 程序显示一个包含“加法”、“减法”和“退出”的菜单，等待用户输入。
2.  **读取输入字符串**: 提示用户输入两个长整数，并以字符串（`std::string`）的形式读入。
3.  **字符串转换为 BigInt**: 调用 `stringToBigInt` 函数，将两个输入字符串转换成我们设计的 `BigInt` 结构体。转换逻辑是：
    *   首先判断有无负号，设置 `is_negative` 标志。
    *   然后从字符串的末尾开始，每 9 位截取一次，用 `stoi` 函数将截取的子字符串转成 `int`，并存入 `digits` 向量中。
4.  **执行运算**: 根据用户的选择，调用 `add` 或 `subtract` 函数进行计算。
5.  **显示结果**: 调用 `displayBigInt` 函数，将计算结果（一个 `BigInt` 结构体）转换成十进制字符串并打印出来。
6.  **循环**: 重复以上步骤，直到用户选择退出。

## 3. 详细设计

### 加法进位逻辑
当两个长整数符号相同时，执行加法操作。其核心逻辑与我们小学时学的竖式加法非常相似。

**文字描述：**
1.  创建一个 `result` 用于存放结果，并初始化一个进位变量 `carry = 0`。
2.  从最低位（`digits` 向量的索引0）开始，逐位向上计算。
3.  在每一位 `i`，新的值 `current` 等于 `A`的第`i`位 + `B`的第`i`位 + 来自上一位的进位`carry`。当然，要先判断 `A` 或 `B` 在这一位是否存在。
4.  这一位 `i` 最终存入 `result` 的值是 `current % BASE`。
5.  新的进位 `carry` 则是 `current / BASE`。
6.  循环直到两个数的最高位都处理完毕，并且 `carry` 也为0为止。

### 减法借位逻辑
当两个长整数符号相同，且被减数的绝对值大于等于减数时，执行减法。这对应了竖式减法中的借位操作。

**文字描述：**
1.  创建一个 `result` 用于存放结果，并初始化一个借位变量 `borrow = 0`。
2.  从最低位开始，逐位向上计算。
3.  在每一位 `i`，被减数的值是 `A`的第`i`位 - `borrow`。
4.  这个值再减去 `B` 在第 `i` 位的值（如果存在）。
5.  如果计算结果 `current` 是负数，说明需要向上位借位。我们将 `current` 加上 `BASE`，并将借位 `borrow` 设置为 `1`。否则，`borrow` 为 `0`。
6.  将 `current` 存入 `result` 的第 `i` 位。
7.  循环直到 `A` 的所有位都处理完毕。
8.  最后，需要清理结果中可能出现的前导0。比如 `123-120`，结果应该是 `3`，而不是 `003`。所以需要一个循环，从 `result` 的最高位向前检查，如果是0就把它删掉。

### 符号处理逻辑
加法和减法并非总是直接计算，还需要根据正负号来判断。我把这个逻辑放在了通用的 `add` 和 `subtract` 函数里，它们再调用上面只处理正数的 `internal_add` 和 `internal_subtract`。
*   **A + B**:
    *   如果 A, B 同号，直接相加，结果符号不变。
    *   如果 A, B 异号，则转换为减法。比如 `5 + (-3)` 变成 `5 - 3`；`3 + (-5)` 变成 `-(5 - 3)`。
*   **A - B**:
    *   如果 A, B 异号，则转换为加法。比如 `5 - (-3)` 变成 `5 + 3`。
    *   如果 A, B 同号，直接相减。比如 `5 - 3`；或者 `(-5) - (-3)` 变成 `-(5 - 3)`。
在转换之前，都需要通过 `compareAbsolute` 函数比较两个数的绝对值大小，来决定谁减谁，以及结果的正负号。

## 4. 调试分析

### 遇到的经典问题

1.  **问题：输出时中间的0丢失了**
    *   **描述**：这是一个非常典型的问题。例如，计算 `10^9`，我的结构体里存的是 `[0, 1]`。在输出时，如果我直接遍历并打印 `digits` 里的数，会得到 `10`，而不是 `1000000000`。同样，`10^9 + 1` 存的是 `[1, 1]`，直接输出会是 `11`，而不是 `1000000001`。
    *   **解决方法**：我使用了C++ `<iomanip>` 库里的 `setw(9)` 和 `setfill('0')`。在 `displayBigInt` 函数中，除了最高位（`digits.back()`）直接输出外，其余的每一位在输出时都强制设置为9位宽度，不足9位的在前面用0填充。这样就完美解决了补0问题。

2.  **问题：复杂的正负号判定**
    *   **描述**：一开始，我试图在同一个函数里用大量的 `if-else` 来处理所有正负号组合的加减法，逻辑非常混乱，很容易出错。
    *   **解决方法**：我采用了“分治”的思想。我把运算拆分成两层：
        *   **外层（`add`, `subtract`）**：只负责判断符号，将所有情况都转化为“两个正数的加法”或“大数减小数（正数）”这两种基本情况。
        *   **内层（`internal_add`, `internal_subtract`）**：只负责纯粹的、不考虑符号的计算。
        通过这种方式，逻辑变得非常清晰，每个函数只做一件事，调试和排错也方便了很多。

### 时间复杂度分析
假设长整数的长度（在10^9进制下）为 N。
*   **加法/减法**: 运算都是从低位到高位进行一次遍历，所以时间复杂度是 **O(N)**。
*   **字符串转换/输出**: 这两个过程也需要遍历整个字符串或 `digits` 向量，复杂度也与长度成正比，也是 **O(N)**。
这里的 N 与原始十进制数字的位数 M 的关系是 N ≈ M/9。所以，算法的效率与输入数字的长度成线性关系，性能表现良好。

### 心得体会
这次课程设计让我对“抽象”和“转换”思想有了更深的体会。面对一个用基本类型无法解决的大数问题，我没有陷入对每一位的繁琐操作，而是通过“进制转换”的思想，把一个大问题（十进制的运算）转换成了一个更简单、更结构化的问题（10^9进制的数组运算）。

这让我明白，在编程中，选择一个合适的数据结构和算法模型，远比埋头苦干要重要。`BigInt` 结构体的设计就是这次任务成功的关键。此外，对边界条件（如0，负数）和特殊情况（如进位，借位）的细致处理，也锻炼了我的逻辑思维能力。虽然调试过程有些痛苦，但最终解决问题时带来的成就感是无法比拟的。

## 5. 用户使用说明

1.  **编译与运行**：


2.  **输入长整数**：
    *   程序会提示你输入两个长整数。
    *   请直接输入一串纯数字，**不要包含逗号或其他分隔符**。
    *   如果想输入负数，请在数字串的最前面加上一个减号 `-`，例如 `-123456789`。

3.  **选择操作**：
    *   输入完两个数字后，程序会返回主菜单。
    *   根据提示输入 `1` 进行加法，或 `2` 进行减法，然后按回车。
    *   程序会显示详细的运算过程和结果。
    *   若想退出，输入 `0` 即可。

## 6. 附录：源程序代码
```cpp
#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // 用于 reverse
#include <iomanip>   // 用于 setw 和 setfill

// 使用 vector<int> 来存储长整数，并设置一个基数
// 10^9 可以放入 int，并且方便按9位进行字符串转换
const int BASE = 1e9;

// 长整数结构体
struct BigInt {
    std::vector<int> digits; // 从低位到高位存储
    bool is_negative = false;    // 符号位，false为正，true为负
};

// --- 函数声明 ---
BigInt stringToBigInt(const std::string& s);
void displayBigInt(const BigInt& n);
int compareAbsolute(const BigInt& a, const BigInt& b); // 比较绝对值大小
BigInt add(BigInt a, BigInt b);
BigInt subtract(BigInt a, BigInt b);

// --- 核心实现 ---

// 字符串转长整数
BigInt stringToBigInt(const std::string& s) {
    BigInt n;
    std::string temp_s = s;

    // 处理符号
    if (temp_s[0] == '-') {
        n.is_negative = true;
        temp_s = temp_s.substr(1);
    }
    
    // 如果是0，特殊处理
    if (temp_s == "0") {
        n.digits.push_back(0);
        n.is_negative = false; // 0 是非负数
        return n;
    }

    // 从后往前，每9位截取一段
    int len = temp_s.length();
    for (int i = len; i > 0; i -= 9) {
        int start = std::max(0, i - 9);
        n.digits.push_back(std::stoi(temp_s.substr(start, i - start)));
    }
    return n;
}

// 显示长整数
void displayBigInt(const BigInt& n) {
    // 处理0
    if (n.digits.empty() || (n.digits.size() == 1 && n.digits[0] == 0)) {
        std::cout << 0;
        return;
    }

    if (n.is_negative) {
        std::cout << "-";
    }

    // 直接输出最高位
    std::cout << n.digits.back();

    // 输出其他位，不足9位的前面补0
    for (int i = n.digits.size() - 2; i >= 0; --i) {
        std::cout << std::setw(9) << std::setfill('0') << n.digits[i];
    }
}

// 比较两个长整数的绝对值
// 返回: 1 (a>b), -1 (a<b), 0 (a=b)
int compareAbsolute(const BigInt& a, const BigInt& b) {
    if (a.digits.size() > b.digits.size()) return 1;
    if (a.digits.size() < b.digits.size()) return -1;

    // 位数相同，从高位开始比较
    for (int i = a.digits.size() - 1; i >= 0; --i) {
        if (a.digits[i] > b.digits[i]) return 1;
        if (a.digits[i] < b.digits[i]) return -1;
    }
    return 0; // 相等
}

// 内部使用的加法实现（假设两个数都是正数）
BigInt internal_add(const BigInt& a, const BigInt& b) {
    BigInt result;
    result.is_negative = a.is_negative;
    long long carry = 0; // 进位
    int max_len = std::max(a.digits.size(), b.digits.size());

    for (int i = 0; i < max_len || carry; ++i) {
        long long current = carry;
        if (i < a.digits.size()) current += a.digits[i];
        if (i < b.digits.size()) current += b.digits[i];
        result.digits.push_back(current % BASE);
        carry = current / BASE;
    }
    return result;
}

// 内部使用的减法实现（假设 a 的绝对值 >= b 的绝对值，且都为正）
BigInt internal_subtract(const BigInt& a, const BigInt& b) {
    BigInt result;
    result.is_negative = a.is_negative;
    long long borrow = 0; // 借位

    for (int i = 0; i < a.digits.size(); ++i) {
        long long current = a.digits[i] - borrow;
        if (i < b.digits.size()) {
            current -= b.digits[i];
        }
        
        if (current < 0) {
            current += BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result.digits.push_back(current);
    }
    
    // 去除结果中前导的0，例如 123-120=3，而不是003
    while (result.digits.size() > 1 && result.digits.back() == 0) {
        result.digits.pop_back();
    }
    return result;
}


// 通用加法，处理符号
BigInt add(BigInt a, BigInt b) {
    if (a.is_negative == b.is_negative) {
        // 同号相加
        return internal_add(a, b);
    } else {
        // 异号相加，转换为减法
        if (compareAbsolute(a, b) >= 0) {
            // |a| >= |b|, 结果符号与a相同
            BigInt result = internal_subtract(a, b);
            result.is_negative = a.is_negative;
            return result;
        } else {
            // |a| < |b|, 结果符号与b相同
            BigInt result = internal_subtract(b, a);
            result.is_negative = b.is_negative;
            return result;
        }
    }
}

// 通用减法，处理符号
BigInt subtract(BigInt a, BigInt b) {
    if (a.is_negative != b.is_negative) {
        // 异号相减，转换为同号相加
        // a - (-b)  -> a + b
        // (-a) - b  -> -(a + b)
        BigInt result = internal_add(a, b);
        result.is_negative = a.is_negative;
        return result;
    } else {
        // 同号相减
        // a - b 或 (-a) - (-b) -> b - a
        if (compareAbsolute(a, b) >= 0) {
            // |a| >= |b|, 结果符号与a相同（如果a,b都为负，则(-a)-(-b)=b-a，符号与b-a相同）
            BigInt result = internal_subtract(a, b);
            result.is_negative = a.is_negative;
            return result;
        } else {
            // |a| < |b|, 结果符号与b的相反符号相同
            BigInt result = internal_subtract(b, a);
            result.is_negative = !b.is_negative;
            return result;
        }
    }
}


// 主函数
int main() {
    int choice;
    std::string s1, s2;

    do {
        std::cout << "\n--- 长整数四则运算 ---\\n";
        std::cout << "1. 加法\\n";
        std::cout << "2. 减法\\n";
        std::cout << "0. 退出\\n";
        std::cout << "请输入你的选择: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cout << "输入无效，请输入一个数字。\\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 1 || choice == 2) {
            std::cout << "请输入第一个长整数: ";
            std::cin >> s1;
            std::cout << "请输入第二个长整数: ";
            std::cin >> s2;

            BigInt n1 = stringToBigInt(s1);
            BigInt n2 = stringToBigInt(s2);
            BigInt result;

            if (choice == 1) {
                result = add(n1, n2);
                std::cout << "计算结果: ";
                displayBigInt(n1);
                std::cout << " + ";
                displayBigInt(n2);
                std::cout << " = ";
                displayBigInt(result);
                std::cout << "\n";
            } else {
                result = subtract(n1, n2);
                std::cout << "计算结果: ";
                displayBigInt(n1);
                std::cout << " - ";
                displayBigInt(n2);
                std::cout << " = ";
                displayBigInt(result);
                std::cout << "\n";
            }
        } else if (choice != 0) {
            std::cout << "无效的选择，请重新输入。\\n";
        }

    } while (choice != 0);

    std::cout << "感谢使用，程序退出。\\n";
    return 0;
}
```