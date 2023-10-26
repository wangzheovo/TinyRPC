#include<ctime>
#include<sys/time.h>
#include<iostream>
#include<string>
#include<sstream>
#include<stdio.h>




#include "log.h"
#include "util.h"

namespace RPC{

    


    std::string LogEvent::LogLevelToString(LogLevel level){
        switch (level)
        {
        case Debug:
            return "DEBUG";
        case Info:
            return "INFO";
        case Error:
            return "ERROR";
        default:
            return "UNKNOW";
        }
    }
    
    std::string LogEvent::toString(){

        //struct timeval适用于精确测量时间间隔，而struct tm适用于表示和操作日历时间的各个成分。

        struct timeval now_time; // 定义一个名为now_time的timeval结构体变量，用于存储当前时间

        gettimeofday(&now_time,nullptr); // 获取当前时间并存储在now_time变量中

        struct tm now_time_t; // 定义一个名为now_time_t的tm结构体变量，用于存储当前时间的解析结果

        localtime_r(&(now_time.tv_sec),&now_time_t); // 使用当前时间的秒数来解析出当前时间的年月日时分秒，并存储在now_time_t变量中

        char buf[128]; // 定义一个长度为128的字符数组，用于存储格式化后的时间字符串

        strftime(&buf[0],128,"%y-%m-%d %H:%M:%S",&now_time_t); // 使用now_time_t中的时间信息按照指定的格式"%y-%m-%d %H:%M:%S"将时间格式化成字符串，存储在buf数组中

        std::string time_str(buf); // 将buf数组中的字符串转换为std::string类型

        int32_t ms = now_time.tv_usec / 1000; // 获取当前时间的微秒数，并将其转换为毫秒数

        time_str = time_str + "."  + std::to_string(ms).substr(0,3); // 将毫秒数拼接到时间字符串的末尾

        m_pid = getPId();
        m_thread_id = getThreadId();

        
        std::stringstream ss;

        ss << "[" << LogLevelToString(m_level) << "]\t"
            << "[" << time_str << "]\t"
            << "[" << m_pid << ":" << m_thread_id << "]\t"
            << "[" << std::string(__FILE__) << ":" << __LINE__ <<"]";

        // 获取当前线程处理的请求的 msgid

        // std::string msgid = RunTime::GetRunTime()->m_msgid;
        // std::string method_name = RunTime::GetRunTime()->m_method_name;
        // if (!msgid.empty()) {
        //     ss << "[" << msgid << "]\t";
        // }

        // if (!method_name.empty()) {
        //     ss << "[" << method_name << "]\t";
        // }
        return ss.str();
    }
    static Logger* g_logger = nullptr;
    Logger* Logger::getGlobalLogger(){
        if(g_logger){
            return g_logger;
        }
        return new Logger();
    }

    void Logger::pushLog(const std::string& msg){
        m_buffer.push(msg);
    }

    void Logger::log(){
        while(!m_buffer.empty()){
            std::string msg = m_buffer.front();
            m_buffer.pop();
            printf(msg.c_str());
        }
    }


}
