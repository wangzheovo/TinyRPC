#include<sys/types.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<sys/time.h>
#include<string.h>
#include<arpa/inet.h>
#include<fstream>
#include<iostream>
#include<string>
#include<sstream>

#include "util.h"

namespace RPC{
    static int32_t g_p_id = 0;
    static thread_local int32_t t_thread_id = 0;
    
    pid_t getPId(){
        if(g_p_id == 0){
            g_p_id = getpid();
        }
        
        return g_p_id;
    }

    pid_t getThreadId(){
        if(t_thread_id == 0){
            t_thread_id = syscall(SYS_gettid);
        }
        return t_thread_id;
    }
    
    // 返回当前毫秒数
    int64_t getNowMs(){
        timeval val;
        gettimeofday(&val,NULL);
        return val.tv_sec*1000 + val.tv_usec/1000;
    }

    int32_t getInt32FromNetByte(const char* buf){
        int32_t re;
        memcpy(&re,buf,sizeof(re));
        return ntohl(re);
    }

    double getCPUUtilization() {
        std::ifstream statFile("/proc/stat");  // 打开/proc/stat文件
        // 第一行为总的CPU利用情况，以"cpu"开头
        std::string line;
        std::getline(statFile, line);
        statFile.close();

        // 提取各个CPU时间片
        unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
        sscanf(line.c_str(), "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq,
            &steal, &guest, &guest_nice);

        unsigned long totalIdle = idle + iowait;  // 总的闲置时间
        unsigned long totalNonIdle = user + nice + system + irq + softirq + steal;  // 总的非闲置时间
        unsigned long total = totalIdle + totalNonIdle;  // 总的CPU时间

        double utilization = (double)(totalNonIdle * 100) / total;  // 计算CPU利用率

        return utilization;
    }

    // 获取平均负载
    double getAverageLoad() {
        std::ifstream loadFile("/proc/loadavg");  // 打开/proc/loadavg文件
        std::string line;
        std::getline(loadFile, line);  // 读取一行
        loadFile.close();

        std::istringstream iss(line);
        double load1, load5, load15;
        iss >> load1 >> load5 >> load15;  // 解析平均负载信息

        return load1;  // 返回1分钟的平均负载
    }
    
}
