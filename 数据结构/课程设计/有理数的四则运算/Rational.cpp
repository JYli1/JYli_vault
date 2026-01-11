#include <iostream>
#include <string>
#include <limits> // 用于处理输入流错误
#include <cmath>   // 用于 abs 函数

// 定义有理数结构体
struct Rational {
    int numerator;    // 分子
    int denominator;  // 分母
};

// 计算最大公约数（辗转相除法）
int calculateGCD(int a, int b) {
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// 约分有理数
void simplify(Rational& r) {
    if (r.denominator == 0) {
        std::cout << "错误：分母不能为零！" << std::endl;
        return; // 在此返回，避免后续操作
    }
    if (r.numerator == 0) {
        r.denominator = 1;
        return;
    }

    int common_divisor = calculateGCD(r.numerator, r.denominator);
    r.numerator /= common_divisor;
    r.denominator /= common_divisor;

    // 保证分母为正
    if (r.denominator < 0) {
        r.numerator = -r.numerator;
        r.denominator = -r.denominator;
    }
}

// 从用户输入读取一个有理数
Rational readRational() {
    Rational r;
    char slash;
    while (true) {
        std::cout << "请输入一个有理数 (格式为 a/b): ";
        std::cin >> r.numerator >> slash >> r.denominator;
        if (std::cin.fail() || slash != '/') {
            std::cout << "输入格式错误，请重新输入。\n";
            std::cin.clear(); // 清除 cin 的错误状态
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 忽略当前行剩余的错误输入
            continue;
        }
        if (r.denominator == 0) {
            std::cout << "错误：分母不能为零，请重新输入。\n";
            continue;
        }
        break; // 输入合法，跳出循环
    }
    return r;
}

// 显示一个有理数
void displayRational(const Rational& r) {
    std::cout << r.numerator << "/" << r.denominator;
}

// 有理数加法
Rational add(const Rational& r1, const Rational& r2) {
    Rational result;
    result.numerator = r1.numerator * r2.denominator + r2.numerator * r1.denominator;
    result.denominator = r1.denominator * r2.denominator;
    simplify(result);
    return result;
}

// 有理数减法
Rational subtract(const Rational& r1, const Rational& r2) {
    Rational result;
    result.numerator = r1.numerator * r2.denominator - r2.numerator * r1.denominator;
    result.denominator = r1.denominator * r2.denominator;
    simplify(result);
    return result;
}

// 有理数乘法
Rational multiply(const Rational& r1, const Rational& r2) {
    Rational result;
    result.numerator = r1.numerator * r2.numerator;
    result.denominator = r1.denominator * r2.denominator;
    simplify(result);
    return result;
}

// 有理数除法
Rational divide(const Rational& r1, const Rational& r2) {
    Rational result;
    if (r2.numerator == 0) {
        // 作为错误标志，分母设为0
        result.denominator = 0;
        return result;
    }
    result.numerator = r1.numerator * r2.denominator;
    result.denominator = r1.denominator * r2.numerator;
    simplify(result);
    return result;
}

// 判断两个有理数是否相等
bool areEqual(Rational r1, Rational r2) {
    simplify(r1);
    simplify(r2);
    return (r1.numerator == r2.numerator) && (r1.denominator == r2.denominator);
}

// 主函数
int main() {
    int choice;
    Rational r1, r2, result;

    do {
        // 显示菜单
        std::cout << "\n--- 有理数四则运算计算器 ---\n";
        std::cout << "1. 加法\n";
        std::cout << "2. 减法\n";
        std::cout << "3. 乘法\n";
        std::cout << "4. 除法\n";
        std::cout << "5. 判断相等\n";
        std::cout << "6. 获取分子\n";
        std::cout << "7. 获取分母\n";
        std::cout << "0. 退出\n";
        std::cout << "请输入你的选择: ";
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cout << "输入无效，请输入一个数字。\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        // 处理需要输入有理数的操作
        if (choice >= 1 && choice <= 5) {
            std::cout << "请输入第一个有理数：\n";
            r1 = readRational();
            std::cout << "请输入第二个有理数：\n";
            r2 = readRational();
        } else if (choice == 6 || choice == 7) {
            std::cout << "请输入一个有理数：\n";
            r1 = readRational();
        }

        switch (choice) {
            case 1: // 加法
                result = add(r1, r2);
                displayRational(r1);
                std::cout << " + ";
                displayRational(r2);
                std::cout << " = ";
                displayRational(result);
                std::cout << "\n";
                break;
            case 2: // 减法
                result = subtract(r1, r2);
                displayRational(r1);
                std::cout << " - ";
                displayRational(r2);
                std::cout << " = ";
                displayRational(result);
                std::cout << "\n";
                break;
            case 3: // 乘法
                result = multiply(r1, r2);
                displayRational(r1);
                std::cout << " * ";
                displayRational(r2);
                std::cout << " = ";
                displayRational(result);
                std::cout << "\n";
                break;
            case 4: // 除法
                result = divide(r1, r2);
                if (result.denominator == 0) {
                    std::cout << "错误：除数不能为零！\n";
                } else {
                    displayRational(r1);
                    std::cout << " / ";
                    displayRational(r2);
                    std::cout << " = ";
                    displayRational(result);
                    std::cout << "\n";
                }
                break;
            case 5: // 判断相等
                displayRational(r1);
                if (areEqual(r1, r2)) {
                    std::cout << " 等于 ";
                } else {
                    std::cout << " 不等于 ";
                }
                displayRational(r2);
                std::cout << "\n";
                break;
            case 6: // 获取分子
                std::cout << "有理数 ";
                displayRational(r1);
                std::cout << " 的分子是: " << r1.numerator << "\n";
                break;
            case 7: // 获取分母
                std::cout << "有理数 ";
                displayRational(r1);
                std::cout << " 的分母是: " << r1.denominator << "\n";
                break;
            case 0:
                std::cout << "感谢使用，程序退出。\n";
                break;
            default:
                std::cout << "无效的选择，请重新输入。\n";
                break;
        }
    } while (choice != 0);

    return 0;
}