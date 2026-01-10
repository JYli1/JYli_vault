#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// C++ 标准库的使用是为了方便字符串和向量操作，核心文件操作使用C风格
// 这是一个符合大二学生水平的折中方案

// --- 常量定义 ---
const char* FILE_NAME = "nahan.txt"; // 定义源文件名
const int BUFFER_SIZE = 1024;        // 定义行缓冲区大小

// --- 函数声明 ---
void countCharacters();
void searchKeyword();
void extractContent();

// --- 主函数 ---
int main() {
    int choice;
    do {
        // 显示中文菜单
        std::cout << "\n--- 《呐喊》文学研究助手 ---\n";
        std::cout << "1. 统计总字数\n";
        std::cout << "2. 检索词语\n";
        std::cout << "3. 截选章节内容\n";
        std::cout << "0. 退出\n";
        std::cout << "请输入你的选择: ";
        
        std::cin >> choice;
        // 处理输入非数字的情况
        if (std::cin.fail()) {
            std::cout << "输入无效，请输入一个数字。\n";
            std::cin.clear(); // 清除错误状态
            std::cin.ignore(10000, '\n'); // 忽略错误输入
            continue;
        }

        switch (choice) {
            case 1:
                countCharacters();
                break;
            case 2:
                searchKeyword();
                break;
            case 3:
                extractContent();
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

// --- 功能函数实现 ---

/**
 * @brief 统计文件中的总字数
 * 
 * 中文字符处理逻辑：
 * 本函数假设文件编码为 UTF-8。在 UTF-8 编码中，
 * ASCII 字符（如英文字母、数字）占用1个字节，其值范围为 0-127 (0x00-0x7F)。
 * 中文字符占用3个字节（常见情况），第一个字节的范围是 0xE0-0xEF。
 * 为了简化，我们采用一种近似的判断方法：将字节作为有符号字符处理时，
 * 如果一个字节的值是负数，那么它就是一个多字节字符（如中文）的一部分。
 * 我们只统计每个多字节字符的起始字节，从而得到中文字符数。
 */
void countCharacters() {
    FILE* fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        std::cout << "错误: 无法打开文件 " << FILE_NAME << "。请确保文件存在于程序目录下。\n";
        return;
    }

    int char_count = 0;
    int byte;
    while ((byte = fgetc(fp)) != EOF) {
        // UTF-8 编码中，中文字符的第一个字节的范围通常是 1110xxxx (0xE0-0xEF)
        // 一个更简单的、近似的判断是检查最高位是否为1。
        // char类型中，负数表示最高位为1。
        if ((signed char)byte < 0) {
            // 这是一个多字节字符的开始，我们把它算作一个“字”
            // 然后跳过后面的字节（这里假设一个中文占3字节）
            fgetc(fp); 
            fgetc(fp);
            char_count++;
        } else if (byte > 32) { // 忽略空格、换行等控制字符
            // 这是一个ASCII字符
            char_count++;
        }
    }

    fclose(fp);
    std::cout << "《呐喊》的总字数（近似值）为: " << char_count << " 字\n";
}

/**
 * @brief 在文件中搜索用户输入的词语
 * 
 * 逐行读取文件，并使用 strstr 函数查找子字符串。
 * 记录出现次数和所在的行号。
 */
void searchKeyword() {
    std::string keyword;
    std::cout << "请输入要检索的词语: ";
    std::cin >> keyword;

    FILE* fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        std::cout << "错误: 无法打开文件 " << FILE_NAME << "。\n";
        return;
    }

    char buffer[BUFFER_SIZE];
    int line_number = 0;
    int count = 0;
    std::vector<int> found_lines; // 存储找到词语的行号

    while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
        line_number++;
        // 使用 strstr 在当前行中查找关键词
        char* occurrence = buffer;
        while ((occurrence = strstr(occurrence, keyword.c_str())) != NULL) {
            count++;
            // 记录首次在本行找到的位置
            if (found_lines.empty() || found_lines.back() != line_number) {
                 found_lines.push_back(line_number);
            }
            occurrence++; // 从下一个位置继续查找，避免死循环
        }
    }

    fclose(fp);

    if (count == 0) {
        std::cout << "在文中没有找到词语“" << keyword << "”。\n";
    } else {
        std::cout << "词语“" << keyword << "”在文中总共出现了 " << count << " 次。\n";
        std::cout << "出现位置（行号）: ";
        for (size_t i = 0; i < found_lines.size(); ++i) {
            std::cout << found_lines[i] << (i == found_lines.size() - 1 ? "" : ", ");
        }
        std::cout << "\n";
    }
}

/**
 * @brief 截取两个标志之间的文本内容
 * 
 * 逐行读取文件，设置一个标志位。当找到开始标志时，开始写入；
 * 当找到结束标志时，停止写入。
 */
void extractContent() {
    std::string start_marker, end_marker;
    const char* output_file = "extract.txt";

    // C++的cin不方便直接读带空格的字符串，这里我们用简单的cin
    std::cout << "请输入开始标志 (例如: 五、故乡): ";
    std::cin >> start_marker;
    std::cout << "请输入结束标志 (例如: 六、孔乙己): ";
    std::cin >> end_marker;

    FILE* fin = fopen(FILE_NAME, "r");
    if (fin == NULL) {
        std::cout << "错误: 无法打开源文件 " << FILE_NAME << "。\n";
        return;
    }

    FILE* fout = fopen(output_file, "w");
    if (fout == NULL) {
        std::cout << "错误: 无法创建输出文件 " << output_file << "。\n";
        fclose(fin);
        return;
    }

    char buffer[BUFFER_SIZE];
    bool is_extracting = false;
    bool success = false;

    while (fgets(buffer, BUFFER_SIZE, fin) != NULL) {
        // 如果当前不在提取状态，就检查是否是开始标志
        if (!is_extracting && strstr(buffer, start_marker.c_str()) != NULL) {
            is_extracting = true;
            success = true;
            // 找到了就开始写入，连同开始标志行一起写入
        }

        // 如果当前在提取状态，就检查是否是结束标志
        if (is_extracting && strstr(buffer, end_marker.c_str()) != NULL) {
            is_extracting = false;
            break; // 找到结束标志，停止提取
        }

        // 如果在提取状态，就将当前行写入到新文件
        if (is_extracting) {
            fprintf(fout, "%s", buffer);
        }
    }

    fclose(fin);
    fclose(fout);

    if(success) {
        std::cout << "内容已成功提取到文件 " << output_file << " 中。\n";
    } else {
        std::cout << "未在文中找到指定的开始标志“" << start_marker << "”，提取失败。\n";
        remove(output_file); // 如果没提取成功，删除空文件
    }
}
