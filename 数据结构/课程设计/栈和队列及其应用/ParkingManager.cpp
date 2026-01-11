#include <iostream>
#include <stack>
#include <queue>
#include <string>
#include <iomanip>

// 车辆信息结构体
struct Car {
    std::string plateNumber; // 车牌号
    int arrivalTime;         // 到达时间
};

// 全局设定
const int PARKING_LOT_CAPACITY = 2; // 停车场容量
const double HOURLY_RATE = 5.0;     // 每小时费率

// 打印状态信息
void printStatus(const std::stack<Car>& parkingLot, const std::queue<Car>& waitingLane) {
    std::cout << "\n--- 当前状态 ---" << std::endl;
    std::cout << "停车场 (容量: " << parkingLot.size() << "/" << PARKING_LOT_CAPACITY << "):" << std::endl;
    if (parkingLot.empty()) {
        std::cout << "  停车场是空的。" << std::endl;
    } else {
        std::stack<Car> temp = parkingLot;
        while (!temp.empty()) {
            std::cout << "  车牌: " << temp.top().plateNumber << " (到达时间: " << temp.top().arrivalTime << ")" << std::endl;
            temp.pop();
        }
    }

    std::cout << "便道 (等候车辆: " << waitingLane.size() << "):" << std::endl;
    if (waitingLane.empty()) {
        std::cout << "  便道是空的。" << std::endl;
    } else {
        std::queue<Car> temp = waitingLane;
        while (!temp.empty()) {
            std::cout << "  车牌: " << temp.front().plateNumber << " (到达时间: " << temp.front().arrivalTime << ")" << std::endl;
            temp.pop();
        }
    }
    std::cout << "----------------\n" << std::endl;
}

int main() {
    // 设置本地化，以正确显示中文
    setlocale(LC_ALL, "");

    std::stack<Car> parkingLot; // 停车场 (栈)
    std::queue<Car> waitingLane;  // 便道 (队列)
    std::stack<Car> tempStack;    // 临时让路栈

    char command;
    std::string plate;
    int currentTime = 0;

    std::cout << "欢迎使用停车场管理系统！(停车场容量为 " << PARKING_LOT_CAPACITY << " 辆)" << std::endl;

    while (true) {
        std::cout << "请输入操作 (A: 到达, D: 离开, E: 退出) 和时间 (整数, 如: A 1): ";
        std::cin >> command >> currentTime;

        if (command == 'E' || command == 'e') {
            std::cout << "系统关闭。感谢使用！" << std::endl;
            break;
        }

        std::cout << "请输入车牌号: ";
        std::cin >> plate;

        switch (command) {
            case 'A':
            case 'a': {
                if (parkingLot.size() < PARKING_LOT_CAPACITY) {
                    // 停车场未满，直接进入
                    Car newCar = {plate, currentTime};
                    parkingLot.push(newCar);
                    std::cout << "车辆 " << plate << " 在时间 " << currentTime << " 进入停车场。" << std::endl;
                } else {
                    // 停车场已满，进入便道
                    Car newCar = {plate, currentTime};
                    waitingLane.push(newCar);
                    std::cout << "停车场已满，车辆 " << plate << " 在时间 " << currentTime << " 进入便道等候。" << std::endl;
                }
                break;
            }

            case 'D':
            case 'd': {
                Car targetCar;
                bool found = false;
                
                // 在停车场中寻找目标车辆
                while (!parkingLot.empty()) {
                    targetCar = parkingLot.top();
                    parkingLot.pop();
                    if (targetCar.plateNumber == plate) {
                        found = true;
                        break;
                    }
                    tempStack.push(targetCar); // 非目标车辆，临时移到让路栈
                }

                if (found) {
                    // 计算费用
                    int duration = currentTime - targetCar.arrivalTime;
                    double fee = duration * HOURLY_RATE;
                    std::cout << "车辆 " << plate << " 在时间 " << currentTime << " 离开停车场。" << std::endl;
                    std::cout << "停留时间: " << duration << " 小时, 费用: " << std::fixed << std::setprecision(2) << fee << " 元。" << std::endl;

                    // 将让路栈中的车辆移回停车场
                    while (!tempStack.empty()) {
                        parkingLot.push(tempStack.top());
                        tempStack.pop();
                    }
                    
                    // 如果便道有车，则便道第一辆车进入停车场
                    if (!waitingLane.empty()) {
                        Car carFromLane = waitingLane.front();
                        waitingLane.pop();
                        carFromLane.arrivalTime = currentTime; // 更新入场时间为当前时间
                        parkingLot.push(carFromLane);
                        std::cout << "便道车辆 " << carFromLane.plateNumber << " 在时间 " << currentTime << " 进入停车场。" << std::endl;
                    }

                } else {
                     // 如果在停车场未找到，则恢复让路车辆
                    while (!tempStack.empty()) {
                        parkingLot.push(tempStack.top());
                        tempStack.pop();
                    }
                    std::cout << "错误：停车场中未找到车牌号为 " << plate << " 的车辆。" << std::endl;
                }
                break;
            }

            default: {
                std::cout << "无效的命令，请输入 A, D, 或 E。" << std::endl;
                break;
            }
        }
        printStatus(parkingLot, waitingLane);
    }

    return 0;
}
