# ChatClient-based-FIFO（基于 FIFO 的单机聊天程序）

## 环境配置

**系统环境：**Ubuntu 18.04

**编译指令：**`g++ -lpthread -o main.o main.cpp`

**执行方式：**

- Lucy: `./main.o Lucy`
- Petter: `./main.o Petter`

**说明：**

- 当前目录下的 `fifo1` 与 `fifo2` 为管道所需创建的文件
- 当前目录下的 `Lucy` 与 `Petter` 分别为两个用户进程保存的完整聊天记录



## 演示视频

[【C++】基于 FIFO 的聊天客户端](https://www.bilibili.com/video/BV14K411N7aR/)：https://www.bilibili.com/video/BV14K411N7aR/

