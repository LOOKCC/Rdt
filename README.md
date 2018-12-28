# 华中科技大学计算机网络实验2
## 使用说明
 - 本程序是在Linux编译运行
 - 可执行文件在./bin，三个任务的可执行文件分别为GBN SR TCP，测试使用的文件为input.txt，输出为output.txt.
 - 缓冲区输出分分别为GBNOut.txt SRSendOut.txt SRRecOut.txt TCPOut.txt
## 编译说明
因不想去修改CMakeList，所以每次的main都是用的是StopWait.cpp里面的main函数，如需编译其他协议，首先编辑StopWait.cpp里面的include部分的两行和main开始的类的初始化两行，改为需要的协议。之后保存。cd到bulid, 执行make，则会在bin目录下找到名为stop_wait的可执行文件。
## 输入输出说明
input.txt需要放在和可执行文件同一个文件夹下，output.txt和相关缓冲区信息输出也会在此文件夹
