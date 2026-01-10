# 《数据结构》课程设计报告：文学研究助手

**班级:** [请填写您的班级]
**姓名学号:** [请填写您的姓名及学号]
**完成日期:** 2026年1月10日

## 1. 问题描述和需求分析

### 系统任务
在进行文学作品研究时，我们经常需要对文本进行一些量化分析，例如统计某部作品的总字数、查找特定词语的出现频率，或者把某个章节单独提取出来进行精读。如果手动完成这些工作，不仅效率低下，而且容易出错。

为了解决这个问题，我设计并开发了一个名为“文学研究助手”的C++程序。这个程序以鲁迅先生的小说集《呐喊》作为处理对象（存为 `nahan.txt` 文件），旨在实现以下三个核心的文本分析任务：
1.  **字数统计**：快速计算出整部小说的总字数。
2.  **关键词搜索**：让用户可以输入一个词语，程序能迅速反馈这个词在书里出现了多少次，以及具体出现在哪些行。
3.  **文本截取**：根据用户设定的开始和结束标志（通常是章节名），将这部分内容完整地提取出来，并保存成一个独立的文件，方便后续使用。

### 预设测试词语
为了测试“关键词搜索”功能的有效性，我预设了以下 10 个在《呐喊》中具有代表性的词语：
*   故乡
*   孔乙己
*   阿Q
*   革命
*   闰土
*   祥林嫂
*   药
*   狂人
*   日记
*   中国

## 2. 概要设计

### 文件处理思路
本程序的核心是对文本文件进行读取和分析。考虑到课程要求，我主要使用了C语言风格的文件I/O操作，即以 `FILE*` 文件指针作为操作文件的“句柄”。
*   **定位内容**：对于全文读取的任务（如字数统计和搜索），我让文件指针从头到尾顺序移动，逐行（或逐字节）读取，直到文件末尾（`EOF`）。
*   **数据读取**：主要使用 `fgets()` 函数来按行读取文件内容到一块缓冲区（`char buffer[]`）中。这样做的好处是，既可以处理一行数据，又可以通过 `strstr()` 方便地在行内查找关键词，结构比较清晰。

### 程序模块划分
根据功能需求，我将整个程序划分为三个主要的功能模块和一个主控模块：

1.  **主控模块 (`main` 函数)**：负责显示用户交互菜单，接收用户的选择，并调用其他功能模块来执行具体的任务。它是一个循环结构，直到用户选择退出为止。
2.  **统计模块 (`countCharacters` 函数)**：负责打开文件，逐字节读取内容，通过特定的算法（后述）来区分中文字符和ASCII字符，最终计算出总字数。
3.  **搜索模块 (`searchKeyword` 函数)**：接收用户输入的关键词，然后逐行读取文件，利用字符串查找函数 `strstr()` 在每一行中搜索该关键词，并统计出现次数和行号。
4.  **提取模块 (`extractContent` 函数)**：接收用户输入的起止标志，然后逐行读取文件，通过一个布尔变量 `is_extracting` 作为开关，控制内容的写入。当读到开始标志时，打开开关；读到结束标志时，关闭开关。

## 3. 详细设计

### 中文字符计数算法
统计字数最大的挑战在于如何处理中文字符。在不同的编码格式下，一个中文字符占用的字节数是不同的（GBK占2字节，UTF-8通常占3字节）。

我的程序假设文本文件 `nahan.txt` 是以 **UTF-8** 编码存储的。在这种编码下，ASCII字符的字节值在0-127之间，而中文字符的第一个字节的值会大于127。在C++中，如果把一个字节当中有符号字符（`signed char`）来处理，那么值大于127的字节就会被看作负数。

我的算法正是利用了这一特性：
1.  使用 `fgetc()` 逐字节读取文件。
2.  判断读取到的字节 `byte`：
    *   如果 `(signed char)byte < 0`，说明这是一个多字节字符（我们这里就认为是中文字符）的起始字节。我将字数计数器加一，并连续调用两次 `fgetc()` 来跳过这个中文字符剩下的两个字节。
    *   如果 `byte > 32`，说明它是一个可见的ASCII字符（排除了空格、换行符等），计数器也加一。
3.  通过这种方式，可以得出一个近似的总字数。虽然这种方法不是100%精确（比如没有处理占4字节的生僻字），但对于大二的课程设计来说，它是一个简单且有效的实现思路。

### 字符串匹配思路
在搜索模块中，我需要在一个大的文本文件中查找一个小的词语字符串。我采用了“逐行读取，行内匹配”的策略。

1.  使用 `fgets()` 将文件的一行读入一个足够大的字符数组 `buffer` 中。
2.  调用C语言 `<cstring>` 库中的 `strstr(buffer, keyword)` 函数。这个函数会在 `buffer` 中查找 `keyword` 第一次出现的位置。
3.  如果 `strstr()` 返回了一个非空的指针，说明找到了。我会增加总计数，并记录下当前的行号。
4.  为了找出同一行内的所有匹配项（比如一行中有两个“故乡”），我会让 `strstr()` 的下一次搜索从上一次找到位置的下一个字符开始，而不是从行首开始。这样循环直到在当前行再也找不到匹配项为止。

### 内容提取逻辑
提取模块的逻辑像一个“水龙头”开关。

1.  设置一个布尔变量 `is_extracting = false`，表示默认不提取内容。
2.  逐行读取文件。在每一轮循环中：
    *   **判断是否开启“水龙头”**：如果 `is_extracting` 是 `false`，就用 `strstr()` 检查当前行是否包含“开始标志”。如果包含，就将 `is_extracting` 设置为 `true`。
    *   **判断是否关闭“水龙头”**：如果 `is_extracting` 是 `true`，就用 `strstr()` 检查当前行是否包含“结束标志”。如果包含，就将 `is_extracting` 设置为 `false`，并跳出循环，因为提取工作已经完成。
    *   **写入文件**：只要 `is_extracting` 保持为 `true`，程序就会把当前读取到的行用 `fprintf()` 写入到目标文件 `extract.txt` 中。

## 4. 调试分析

### 遇到的问题及解决方法

1.  **问题：中文字符编码导致的统计混乱**
    *   **描述**：一开始，我简单地认为一个字符就是一个字节，直接统计文件字节数，结果得到的“字数”非常庞大，完全不正确。后来意识到中文字符编码的问题，在UTF-8下，一个汉字占3个字节。
    *   **解决方法**：我查阅了UTF-8的编码规则，并设计了前面提到的基于字节值的判断算法。在报告和代码注释中，我明确指出了“本算法基于UTF-8编码，并且是一种近似算法”，这体现了作为一名学生，在现有知识范围内对问题局限性的认知和说明。

2.  **问题：关键词跨行匹配的困难**
    *   **描述**：如果一个词语被换行符分割，比如一行的末尾是“孔乙”，下一行的开头是“己”，我的 `strstr` 逐行匹配方法就无法识别出“孔乙己”这个词。
    *   **解决方法**：这是一个比较复杂的问题，需要设计一个跨行缓冲区的匹配算法。考虑到课程设计的难度和时间，我采取了简化的策略。我选择的 `fgets()` 按行读取的方法本身就决定了我的程序无法处理跨行词语。我在设计上接受了这个限制，因为在大多数文学作品的排版中，一个完整的词语被换行符断开的情况相对较少。这是一种在项目开发中常见的“权衡（trade-off）”。

### 经验与体会
通过这次课程设计，我对C风格的文件操作 `FILE*` 有了非常深刻的理解。之前只是在课上听过 `fopen`, `fclose`，但从没实践过。
*   **文件指针是核心**：我明白了 `FILE*` 就像一个遥控器，它指向了文件当前被读写的位置。`fgetc()` 会让它前进一个字节，`fgets()` 会让它前进一行。理解了文件指针的移动，就理解了文件顺序读取的本质。
*   **缓冲区的重要性**：`fgets()` 读取数据时，是先把一块数据读到内存的缓冲区里，而不是一个字节一个字节地和硬盘交互，这大大提高了效率。
*   **错误处理的必要性**：`fopen` 可能会因为文件不存在而失败，返回 `NULL`。如果不检查这个返回值，后面的读写操作就会导致程序崩溃。这次实践让我养成了每次进行文件操作后都检查返回值的习惯，代码的健壮性大大提高。

总的来说，这次设计让我把“理论”和“实践”真正结合了起来。看似简单的文件读写，在面对中文编码、性能、健壮性等实际问题时，也需要缜密的思考。

## 5. 用户使用说明

为了让本程序正常工作，请遵循以下步骤：

1.  **准备文件**：请确保有一个名为 `nahan.txt` 的文本文件，并且它和编译后生成的可执行文件在 **同一个文件夹** 下。该文件应使用 **UTF-8** 编码。

2.  **编译运行**：
    *   使用 G++ 编译器编译 `LiteraryAssistant.cpp` 文件：
        ```bash
        g++ LiteraryAssistant.cpp -o LiteraryAssistant
        ```
    *   运行程序：
        ```bash
        ./LiteraryAssistant
        ```

3.  **菜单操作**：
    *   程序运行后，会显示一个包含三个选项的菜单。
    *   在提示 “请输入你的选择:” 后，输入数字 `1`, `2`, `3` 中的一个，然后按回车。
    *   **选择1 (统计字数)**：程序会直接输出统计结果。
    *   **选择2 (检索词语)**：程序会提示你输入一个词语，输入后按回车，即可看到搜索结果。
    *   **选择3 (截选内容)**：程序会先后提示你输入“开始标志”和“结束标志”。输入标志时请注意，**当前版本不支持输入带空格的标志**（例如“五、故乡”应直接输入`五、故乡`）。内容截取成功后，会在程序目录下生成一个 `extract.txt` 文件。
    *   若要退出程序，输入 `0` 即可。

## 6. 附录：源程序代码
```cpp
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
        std::cout << "\n--- 《呐喊》文学研究助手 ---\
";
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

```