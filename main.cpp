#include <assert.h>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <signal.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
using namespace std;

int read_fd, write_fd;               // 定义读写管道
string user[2] = {"Lucy", "Petter"}; // 聊天双方姓名
vector<string> msg_history;          // 聊天消息记录
mutex msg_mutex; // 聊天线程互斥锁，主要用于解决在存储聊天记录时的线程安全问题
const int buff_size = 128;

void refresh_window() { // 刷新窗口，显示聊天记录
    system("clear");

    if (user[0] == "Lucy") { // 存储聊天记录交给一个进程进行
        int outfile = open("record.txt", O_WRONLY | O_CREAT | O_TRUNC,
                           0777); // 只写并且清空原文件内容
        for (auto s : msg_history) {
            write(outfile, s.data(), strlen(s.data())); // 往文件里写一份
        }
        close(outfile);
    }
    for (auto s : msg_history) {
        cout << s;
    }
    cout << "---------------------------------" << endl;
    cout << user[0] << ": " << flush;
}
void read_from_fifo() {
    char buff[buff_size] = {0};
    int n = 0;
    while ((n = read(read_fd, buff, buff_size - 1)) > 0) {
        // cout << user[1] << ": " << buff << endl;

        msg_mutex.lock();                                     // 上锁
        msg_history.push_back(user[1] + ": " + string(buff)); // 存储聊天记录
        msg_mutex.unlock();                                   // 解锁
        refresh_window();

        // 将 buff 中的数据清空
        memset(buff, 0, buff_size);
    }
    close(read_fd);
}

void write_to_fifo() {
    char buff[buff_size] = {0};
    while (true) {
        refresh_window();
        fgets(buff, buff_size, stdin);
        write(write_fd, buff, strlen(buff));

        msg_mutex.lock(); // 上锁
        msg_history.push_back(user[0] + ": " + string(buff));
        msg_mutex.unlock(); // 解锁
    }
    close(write_fd);
}

bool make_fifo(const char *name) { // 创建 fifo 文件
    // 如果 fifo 文件不存在，则创建它，权限为 777
    if (access(name, 0) && mkfifo(name, 0777)) {
        printf("mk %s error.", name);
        return false;
    }
    return true;
}
void init(int argc, char *argv[]) {
    if (argc != 2) {
        printf("please input the client name, Lucy or Petter.");
        exit(1);
    }
    if (make_fifo("fifo1") && make_fifo("fifo2")) {
        if (strcmp(argv[1], "Lucy") == 0) { // 分别创建两个管道
            read_fd = open("fifo1", O_RDONLY);
            write_fd = open("fifo2", O_WRONLY);
        } else if (strcmp(argv[1], "Petter") ==
                   0) { // 这两个的顺序不能反，因为 read 会阻塞
            write_fd = open("fifo1", O_WRONLY);
            read_fd = open("fifo2", O_RDONLY);
            swap(user[0], user[1]); // 交换两个用户的姓名
        } else {
            printf("client name error, Lucy or Petter.");
            exit(1);
        }
        assert(read_fd != -1);
        assert(write_fd != -1);
        printf("login successful!\n");
    } else {
        exit(1);
    }
}

void stop_fun(int signo) { // 结束信号中要做的事情
    close(read_fd);        // 关闭管道
    close(write_fd);
    exit(0);
}

int main(int argc, char *argv[]) { // 编译指令：g++ main.cpp -lpthread
    signal(SIGINT, stop_fun); // 注册 SIGINT 信号接收由 stop_fun 处理

    init(argc, argv);
    thread read_fun(read_from_fifo); // 创建一个读对方消息的线程
    read_fun.detach();               // 交付系统管理线程
    write_to_fifo();                 // 执行主线程中的写方法
    return 0;
}