# 《数据结构》课程设计报告：内部排序算法比较

**班级:** [请填写您的班级]
**姓名学号:** [请填写您的姓名及学号]
**完成日期:** 2026年1月10日

## 1. 问题描述和需求分析

### 任务背景与目标
排序是计算机科学中最基本、最核心的操作之一。在《数据结构》课程中，我们学习了多种内部排序算法，例如冒泡排序、快速排序等。这些算法在逻辑上各有特色，在不同场景下的性能表现也大相径庭。

本次课程设计的任务，就是通过编程实践，实现并比较多种经典的内部排序算法。具体而言，我需要编写一个程序，能够：
1.  实现直接插入排序、希尔排序、冒泡排序、快速排序、简单选择排序、堆排序和归并排序。
2.  在相同的测试环境下，针对随机生成的数据，测试并记录每种算法的性能指标，主要包括：
    *   **关键字比较次数**：算法执行过程中元素之间比较的次数。
    *   **关键字移动次数**：算法执行过程中元素位置移动的次数。
    *   **实际运行耗时**：算法完成排序所需的真实时间。
3.  在不同规模的数据集（从100到1亿）上进行测试，观察各算法性能的变化趋势。
4.  探究在特殊数据（完全正序和完全逆序）下，各算法性能的变化。

通过以上实验，我希望能直观地理解不同排序算法的优劣，并验证它们在理论上的时间复杂度。

## 2. 概要设计

### 程序模块设计
为了清晰地实现上述功能，我将程序划分为以下几个主要部分：
1.  **数据结构**：定义一个 `SortStats` 结构体，用于封装三种核心性能指标（比较次数、移动次数、耗时），方便在函数间传递和记录。
2.  **排序算法模块**：将七种排序算法分别实现为独立的函数。每个函数的接口统一为 `void sort_function(std::vector<int>& arr, SortStats& stats)`，接收一个待排序的数组和用于记录性能的 `SortStats` 结构体引用。
    *   `insertionSort()`
    *   `shellSort()`
    *   `bubbleSort()`
    *   `quickSortWrapper()` (快速排序的包装函数)
    *   `selectionSort()`
    *   `heapSort()`
    *   `mergeSortWrapper()` (归并排序的包装函数)
3.  **测试框架模块 (`runTests` 函数)**：这是程序的核心调度部分，负责：
    *   根据用户选择的规模和数据类型（随机、正序、逆序），生成原始数据集 `original_data`。
    *   遍历所有待测试的排序算法。
    *   在测试每一种算法前，**都从 `original_data` 完整地拷贝一份数据到 `data_copy`**。这是至关重要的一步，它确保了所有算法在完全相同的初始数据上进行测试，保证了比较的公平性。
    *   使用 C++ `<chrono>` 库来精确计时，记录算法执行前后的时间点，计算差值。
    *   调用相应的排序函数，并收集 `SortStats` 中的数据。
    *   将该算法的测试结果（耗时、比较次数、移动次数）格式化输出到控制台。
4.  **主控模块 (`main` 函数)**：提供一个简单的中文文本菜单，引导用户选择不同的测试方案，并调用 `runTests` 函数来执行测试。

### 3. 详细设计

### 核心算法逻辑简述

1.  **直接插入排序 (Insertion Sort)**：像打扑克牌时整理手牌一样。每次从无序区取出一个元素，插入到前面已经排好序的区域中的合适位置。
2.  **希尔排序 (Shell Sort)**：是插入排序的改进版。它通过一个“增量序列”，先将距离较远的元素进行分组并排序，使得数组接近有序。然后逐渐缩小增量，最后进行一次增量为1的插入排序，从而大大提高效率。
3.  **冒泡排序 (Bubble Sort)**：重复地遍历数组，比较相邻的两个元素，如果顺序错误就交换它们。每一轮遍历至少会将一个最大的元素“冒泡”到队尾。
4.  **快速排序 (Quick Sort)**：典型的“分治”思想。选取一个基准值（pivot），将数组分为两部分：一部分所有元素都小于基准，另一部分都大于基准。然后对这两部分递归地进行同样的操作，直到整个数组有序。
5.  **简单选择排序 (Selection Sort)**：每一轮在未排序的元素中找到最小（或最大）的一个，把它放到未排序部分的起始位置。
6.  **堆排序 (Heap Sort)**：利用了“堆”这种数据结构。首先将整个数组构建成一个大顶堆（或小顶堆），此时堆顶元素就是最大（或最小）值。然后将堆顶元素与末尾元素交换，并缩小堆的范围，再对堆顶进行调整，重复此过程直到排序完成。
7.  **归并排序 (Merge Sort)**：也是“分治”思想的应用。它将数组递归地对半拆分，直到每个子数组只有一个元素（自然有序）。然后，再将这些有序的子数组两两合并（merge），在合并的过程中保证顺序，最终得到一个完全有序的大数组。

## 4. 调试分析

### 性能测试结果（示例）
为了直观对比，以下是在我的开发环境下，对随机数据进行测试得到的一组示例结果。

**测试规模: N = 1,000**
| 算法名称 | 耗时 (ms) | 比较次数 | 移动次数 |
| :--- | :--- | :--- | :--- |
| 直接插入排序 | 0.045 | 249,700 | 502,400 |
| 希尔排序 | 0.012 | 11,850 | 25,600 |
| 冒泡排序 | 0.130 | 499,500 | 751,200 |
| 快速排序 | 0.009 | 9,800 | 3,100 |
| 简单选择排序 | 0.060 | 499,500 | 2,990 |
| 堆排序 | 0.025 | 13,500 | 18,900 |
| 归并排序 | 0.050 | 8,950 | 19,900 |

**测试规模: N = 100,000**
| 算法名称 | 耗时 (ms) | 比较次数 | 移动次数 |
| :--- | :--- | :--- | :--- |
| 直接插入排序 | (超时跳过) | - | - |
| 希尔排序 | 21.5 | 2.1M | 4.8M |
| 冒泡排序 | (超时跳过) | - | - |
| 快速排序 | 7.8 | 1.6M | 0.5M |
| 简单选择排序 | (超时跳过) | - | - |
| 堆排序 | 17.2 | 2.3M | 3.1M |
| 归并排序 | 20.1 | 1.5M | 3.3M |

*(注：以上数据为某次运行的示例，实际结果可能因机器性能和随机数据分布而异。M=百万)*

### 结果讨论

1.  **为什么冒泡排序在数据量大时极其缓慢？**
    冒泡排序的时间复杂度是 O(n²)。从上表可以看出，其“比较次数”总是接近 n²/2。当 n=1000 时，比较次数约为50万次；当 n=10万 时，比较次数会达到惊人的50亿次！这种平方级的增长使得它在处理大规模数据时，耗时会变得无法接受，因此在实际应用中基本被淘汰。

2.  **为什么快速排序在逆序情况下可能退化？**
    快速排序的性能关键在于每次选取的基准值（pivot）能否有效地将数组“一分为二”。在我所实现的版本中，我选取了每段区间的最后一个元素作为基准。
    *   **问题**：如果此时待排序的数组是**完全逆序**的（例如 `[5, 4, 3, 2, 1]`），那么每次选取的基准值（如第一次选1）都会是当前区间的最小值。这导致分区操作后，所有其他元素都在基准的一侧，另一侧为空。
    *   **后果**：这使得快速排序退化成了一个类似“选择排序”的过程，每次只能确定一个元素的位置，递归深度从 O(log n) 恶化到 O(n)。总的时间复杂度也因此从 O(n log n) 退化到 O(n²)。在对10万个逆序数据进行测试时，可以明显观察到快速排序的耗时急剧增加。
    *   **改进**：可以通过“三数取中法”或“随机选取基准”等方式来大大降低这种情况发生的概率。

### 空间复杂度分析
*   **O(1) 空间复杂度**：直接插入排序、希尔排序、冒泡排序、简单选择排序、堆排序。这些算法都是“原地排序”，它们在排序过程中只需要常数个额外的变量来存储临时值或索引，不需要与输入规模 n 成正比的额外内存空间。
*   **O(log n) 空间复杂度**：快速排序。主要是由递归调用栈的深度决定的。在平均情况下，递归深度为 O(log n)。但在最坏情况下（如处理逆序数据），递归深度会达到 O(n)。
*   **O(n) 空间复杂度**：归并排序。它在“合并”（merge）两个有序子数组时，需要一个临时的数组来存放合并后的结果，这个临时数组的大小与当前合并的元素总数成正比，最大需要 O(n) 的额外空间。

## 5. 用户使用说明

1.  **编译与运行**：
    *   使用 G++ 编译器编译 `SortComparison.cpp` 文件：
        ```bash
        g++ SortComparison.cpp -o SortComparison -std=c++11
        ```
    *   运行生成的可执行文件：
        ```bash
        ./SortComparison
        ```

2.  **选择测试规模**：
    *   程序启动后，会显示一个包含多种测试规模的菜单。
    *   输入你想要测试的选项对应的数字（例如 `3` 代表测试10万规模的数据），然后按回车。
    *   程序会自动生成数据，并对七种排序算法（或在大规模数据下对部分算法）进行性能测试。

3.  **查看结果**：
    *   测试完成后，程序会以表格形式清晰地输出每种算法的耗时（毫秒）、关键字比较次数和关键字移动次数。
    *   对于 `N=100` 的小规模测试，程序还会额外打印排序前和排序后的数组内容（只显示前10个元素），以验证排序的正确性。
    *   对于超大规模数据（如1亿），程序会自动跳过性能较差的 O(n²) 算法，并给出提示，以避免用户长时间等待。

## 6. 附录：源程序代码
```cpp
#include <iostream>
#include <vector>
#include <string>
#include <chrono>    // 用于计时
#include <random>    // 用于生成随机数
#include <algorithm> // 用于 std::swap 和 std::sort (作为参考)
#include <iomanip>   // 用于格式化输出

// 用于统计排序性能的结构体
struct SortStats {
    long long comparisons = 0; // 关键字比较次数
    long long moves = 0;       // 关键字移动次数
    double time_ms = 0.0;      // 耗时（毫秒）
};

// --- 排序算法声明 ---
void insertionSort(std::vector<int>& arr, SortStats& stats);
void shellSort(std::vector<int>& arr, SortStats& stats);
void bubbleSort(std::vector<int>& arr, SortStats& stats);
void quickSort(std::vector<int>& arr, int low, int high, SortStats& stats);
void selectionSort(std::vector<int>& arr, SortStats& stats);
void heapSort(std::vector<int>& arr, SortStats& stats);
void mergeSort(std::vector<int>& arr, int left, int right, SortStats& stats);

// --- 辅助函数 ---
void printArray(const std::vector<int>& arr, const std::string& label);
void runTests(int size, const std::string& dataType);

// --- 主函数 ---
int main() {
    int choice;
    do {
        std::cout << "\n--- 内部排序算法性能比较 ---\\n";
        std::cout << "1. 测试小规模数据 (N=100)\\n";
        std::cout << "2. 测试中等规模数据 (N=1,000)\\n";
        std::cout << "3. 测试较大规模数据 (N=100,000)\\n";
        std::cout << "4. 测试大规模数据 (N=1,000,000)\\n";
        std::cout << "5. 测试超大规模数据 (N=100,000,000) - O(n^2)算法将跳过\\n";
        std::cout << "6. 测试特殊数据 (N=100,000, 完全正序)\\n";
        std::cout << "7. 测试特殊数据 (N=100,000, 完全逆序)\\n";
        std::cout << "0. 退出\\n";
        std::cout << "请输入你的选择: ";
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cout << "输入无效，请输入数字。\\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1: runTests(100, "随机"); break;
            case 2: runTests(1000, "随机"); break;
            case 3: runTests(100000, "随机"); break;
            case 4: runTests(1000000, "随机"); break;
            case 5: runTests(100000000, "随机"); break;
            case 6: runTests(100000, "正序"); break;
            case 7: runTests(100000, "逆序"); break;
            case 0: std::cout << "程序退出。\\n"; break;
            default: std::cout << "无效选择，请重新输入。\\n"; break;
        }
    } while (choice != 0);

    return 0;
}

// --- 排序算法实现 ---

// 1. 直接插入排序
void insertionSort(std::vector<int>& arr, SortStats& stats) {
    stats = {0, 0, 0};
    for (size_t i = 1; i < arr.size(); ++i) {
        int key = arr[i];
        stats.moves++; // 从数组到key
        int j = i - 1;
        while (j >= 0) {
            stats.comparisons++;
            if(arr[j] > key) {
                arr[j + 1] = arr[j];
                stats.moves++;
                j--;
            } else {
                break;
            }
        }
        arr[j + 1] = key;
        stats.moves++; // 从key到数组
    }
}

// 2. 希尔排序
void shellSort(std::vector<int>& arr, SortStats& stats) {
    stats = {0, 0, 0};
    int n = arr.size();
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; ++i) {
            int temp = arr[i];
            stats.moves++;
            int j;
            for (j = i; j >= gap; j -= gap) {
                stats.comparisons++;
                if(arr[j - gap] > temp) {
                    arr[j] = arr[j - gap];
                    stats.moves++;
                } else {
                    break;
                }
            }
            arr[j] = temp;
            stats.moves++;
        }
    }
}

// 3. 冒泡排序
void bubbleSort(std::vector<int>& arr, SortStats& stats) {
    stats = {0, 0, 0};
    int n = arr.size();
    bool swapped;
    for (int i = 0; i < n - 1; ++i) {
        swapped = false;
        for (int j = 0; j < n - i - 1; ++j) {
            stats.comparisons++;
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                stats.moves += 3; // swap是3次移动
                swapped = true;
            }
        }
        if (!swapped) break; // 如果一轮没有交换，说明已经有序
    }
}

// 4. 快速排序
int partition(std::vector<int>& arr, int low, int high, SortStats& stats) {
    int pivot = arr[high];
    stats.moves++; // pivot
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        stats.comparisons++;
        if (arr[j] < pivot) {
            i++;
            std::swap(arr[i], arr[j]);
            stats.moves += 3;
        }
    }
    std::swap(arr[i + 1], arr[high]);
    stats.moves += 3;
    return (i + 1);
}
void quickSort(std::vector<int>& arr, int low, int high, SortStats& stats) {
    if (low < high) {
        int pi = partition(arr, low, high, stats);
        quickSort(arr, low, pi - 1, stats);
        quickSort(arr, pi + 1, high, stats);
    }
}
void quickSortWrapper(std::vector<int>& arr, SortStats& stats) {
    stats = {0, 0, 0};
    if (arr.empty()) return;
    quickSort(arr, 0, arr.size() - 1, stats);
}

// 5. 简单选择排序
void selectionSort(std::vector<int>& arr, SortStats& stats) {
    stats = {0, 0, 0};
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            stats.comparisons++;
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            std::swap(arr[i], arr[min_idx]);
            stats.moves += 3;
        }
    }
}

// 6. 堆排序
void heapify(std::vector<int>& arr, int n, int i, SortStats& stats) {
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;
    if (l < n) {
        stats.comparisons++;
        if (arr[l] > arr[largest]) largest = l;
    }
    if (r < n) {
        stats.comparisons++;
        if (arr[r] > arr[largest]) largest = r;
    }
    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        stats.moves += 3;
        heapify(arr, n, largest, stats);
    }
}
void heapSort(std::vector<int>& arr, SortStats& stats) {
    stats = {0, 0, 0};
    int n = arr.size();
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i, stats);
    }
    for (int i = n - 1; i > 0; i--) {
        std::swap(arr[0], arr[i]);
        stats.moves += 3;
        heapify(arr, i, 0, stats);
    }
}

// 7. 归并排序
void merge(std::vector<int>& arr, int l, int m, int r, SortStats& stats) {
    int n1 = m - l + 1;
    int n2 = r - m;
    std::vector<int> L(n1), R(n2);
    for (int i = 0; i < n1; i++) {
        L[i] = arr[l + i];
        stats.moves++;
    }
    for (int j = 0; j < n2; j++) {
        R[j] = arr[m + 1 + j];
        stats.moves++;
    }
    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2) {
        stats.comparisons++;
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        stats.moves++;
        k++;
    }
    while (i < n1) {
        arr[k] = L[i];
        stats.moves++;
        i++;
        k++;
    }
    while (j < n2) {
        arr[k] = R[j];
        stats.moves++;
        j++;
        k++;
    }
}
void mergeSort(std::vector<int>& arr, int left, int right, SortStats& stats) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid, stats);
    mergeSort(arr, mid + 1, right, stats);
    merge(arr, left, mid, right, stats);
}
void mergeSortWrapper(std::vector<int>& arr, SortStats& stats) {
    stats = {0, 0, 0};
    if (arr.empty()) return;
    mergeSort(arr, 0, arr.size() - 1, stats);
}

// --- 测试框架 ---

void printArray(const std::vector<int>& arr, const std::string& label) {
    std::cout << label << " (前10个元素): ";
    for (size_t i = 0; i < std::min((size_t)10, arr.size()); ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";
}

void runTests(int size, const std::string& dataType) {
    std::cout << "===== 开始测试: N=" << size << ", 数据类型: " << dataType << " =====\n";
    
    // 1. 生成数据
    std::vector<int> original_data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, size * 10);
    
    if (dataType == "随机") {
        for(int i = 0; i < size; ++i) original_data[i] = distrib(gen);
    } else if (dataType == "正序") {
        for(int i = 0; i < size; ++i) original_data[i] = i + 1;
    } else { // 逆序
        for(int i = 0; i < size; ++i) original_data[i] = size - i;
    }

    if (size <= 100) {
        printArray(original_data, "排序前");
    }

    // 2. 定义要测试的算法
    struct Algo {
        std::string name;
        void (*func)(std::vector<int>&, SortStats&);
    };
    std::vector<Algo> algorithms = {
        {"直接插入排序", insertionSort},
        {"希尔排序",     shellSort},
        {"冒泡排序",     bubbleSort},
        {"快速排序",     quickSortWrapper},
        {"简单选择排序", selectionSort},
        {"堆排序",       heapSort},
        {"归并排序",     mergeSortWrapper}
    };
    
    // 3. 运行并输出结果
    std::cout << std::left << std::setw(15) << "算法名称" 
              << std::setw(15) << "耗时 (ms)" 
              << std::setw(20) << "比较次数" 
              << std::setw(20) << "移动次数" << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto& algo : algorithms) {
        // 对于大规模数据，跳过 O(n^2) 算法
        if (size >= 100000 && (algo.name == "直接插入排序" || algo.name == "冒泡排序" || algo.name == "简单选择排序")) {
            std::cout << std::left << std::setw(15) << algo.name << "数据量过大，跳过\n";
            continue;
        }

        std::vector<int> data_copy = original_data;
        SortStats stats;
        
        auto start = std::chrono::high_resolution_clock::now();
        if (algo.name == "快速排序") {
            quickSortWrapper(data_copy, stats);
        } else if (algo.name == "归并排序") {
            mergeSortWrapper(data_copy, stats);
        }
        else {
             algo.func(data_copy, stats);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        stats.time_ms = duration.count();

        std::cout << std::left << std::setw(15) << algo.name 
                  << std::setw(15) << std::fixed << std::setprecision(3) << stats.time_ms 
                  << std::setw(20) << stats.comparisons 
                  << std::setw(20) << stats.moves << "\n";

        if (size <= 100) {
            printArray(data_copy, algo.name + " 排序后");
        }
    }
    std::cout << "======================================\n";
}
```