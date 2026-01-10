#include <iostream>
#include <vector>
#include <string>
#include <chrono>    // ??????
#include <random>    // ?????????????
#include <algorithm> // ???? std::swap ?? std::sort (????¦Ï?)
#include <iomanip>   // ???????????

// ???????????????????
struct SortStats {
    long long comparisons = 0; // ??????????
    long long moves = 0;       // ????????????
    double time_ms = 0.0;      // ?????????
};

// --- ?????????? ---
void insertionSort(std::vector<int>& arr, SortStats& stats);
void shellSort(std::vector<int>& arr, SortStats& stats);
void bubbleSort(std::vector<int>& arr, SortStats& stats);
void quickSort(std::vector<int>& arr, int low, int high, SortStats& stats);
void selectionSort(std::vector<int>& arr, SortStats& stats);
void heapSort(std::vector<int>& arr, SortStats& stats);
void mergeSort(std::vector<int>& arr, int left, int right, SortStats& stats);

// --- ???????? ---
void printArray(const std::vector<int>& arr, const std::string& label);
void runTests(int size, const std::string& dataType);

// --- ?????? ---
int main() {
    int choice;
    do {
        std::cout << "\n--- ??????????????? ---" << std::endl;
        std::cout << "1. ????§³??????? (N=100)" << std::endl;
        std::cout << "2. ?????§Ö??????? (N=1,000)" << std::endl;
        std::cout << "3. ???????????? (N=100,000)" << std::endl;
        std::cout << "4. ??????????? (N=1,000,000)" << std::endl;
        std::cout << "5. ????????????? (N=100,000,000) - O(n^2)????????" << std::endl;
        std::cout << "6. ???????????? (N=100,000, ???????)" << std::endl;
        std::cout << "7. ???????????? (N=100,000, ???????)" << std::endl;
        std::cout << "0. ???" << std::endl;
        std::cout << "????????????: ";
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cout << "??????§¹?????????????\n";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }

        switch (choice) {
            case 1: runTests(100, "???"); break;
            case 2: runTests(1000, "???"); break;
            case 3: runTests(100000, "???"); break;
            case 4: runTests(1000000, "???"); break;
            case 5: runTests(100000000, "???"); break;
            case 6: runTests(100000, "????"); break;
            case 7: runTests(100000, "????"); break;
            case 0: std::cout << "?????????\n"; break;
            default: std::cout << "??§¹?????????????\n"; break;
        }
    } while (choice != 0);

    return 0;
}

// --- ????????? ---

// 1. ??????????
void insertionSort(std::vector<int>& arr, SortStats& stats) {
    stats = {0, 0, 0};
    for (size_t i = 1; i < arr.size(); ++i) {
        int key = arr[i];
        stats.moves++; // ?????ùzkey
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
        stats.moves++; // ??key??????
    }
}

// 2. ???????
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

// 3. ???????
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
                stats.moves += 3; // swap??3?????
                swapped = true;
            }
        }
        if (!swapped) break; // ????????§ß???????????????
    }
}

// 4. ????????
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

// 5. ?????????
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

// 6. ??????
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

// 7. ?öã????
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

// --- ?????? ---

void printArray(const std::vector<int>& arr, const std::string& label) {
    std::cout << label << " (?10?????): ";
    for (size_t i = 0; i < std::min((size_t)10, arr.size()); ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";
}

void runTests(int size, const std::string& dataType) {
    std::cout << "\n===== ???????: N=" << size << ", ????????: " << dataType << " =====\n";
    
    // 1. ????????
    std::vector<int> original_data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, size * 10);
    
    if (dataType == "???") {
        for(int i = 0; i < size; ++i) original_data[i] = distrib(gen);
    } else if (dataType == "????") {
        for(int i = 0; i < size; ++i) original_data[i] = i + 1;
    } else { // ????
        for(int i = 0; i < size; ++i) original_data[i] = size - i;
    }

    if (size <= 100) {
        printArray(original_data, "?????");
    }

    // 2. ????????????
    struct Algo {
        std::string name;
        void (*func)(std::vector<int>&, SortStats&);
    };
    std::vector<Algo> algorithms = {
        {"??????????", insertionSort},
        {"???????",     shellSort},
        {"???????",     bubbleSort},
        {"????????",     quickSortWrapper},
        {"?????????", selectionSort},
        {"??????",       heapSort},
        {"?öã????",     mergeSortWrapper}
    };
    
    // 3. ???§Ó???????
    std::cout << std::left << std::setw(15) << "??????" 
              << std::setw(15) << "??? (ms)" 
              << std::setw(20) << "??????" 
              << std::setw(20) << "???????" << "\n";
    std::cout << std::string(70, '-') << "\n";

    for (const auto& algo : algorithms) {
        // ???????????????? O(n^2) ??
        if (size >= 100000 && (algo.name == "??????????" || algo.name == "???????" || algo.name == "?????????")) {
            std::cout << std::left << std::setw(15) << algo.name << "??????????????\n";
            continue;
        }

        std::vector<int> data_copy = original_data;
        SortStats stats;
        
        auto start = std::chrono::high_resolution_clock::now();
        if (algo.name == "????????") {
            quickSortWrapper(data_copy, stats);
        } else if (algo.name == "?öã????") {
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
            printArray(data_copy, algo.name + " ?????");
        }
    }
    std::cout << "======================================\n";
}
