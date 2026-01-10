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
        std::cout << "\n--- 长整数四则运算 ---\n";
        std::cout << "1. 加法\n";
        std::cout << "2. 减法\n";
        std::cout << "0. 退出\n";
        std::cout << "请输入你的选择: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cout << "输入无效，请输入一个数字。\n";
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
            std::cout << "无效的选择，请重新输入。\n";
        }

    } while (choice != 0);

    std::cout << "感谢使用，程序退出。\n";
    return 0;
}
