#ifndef __MYLOG_H__
#define __MYLOG_H__

// #include <log4cpp/Category.hh>
// #include <log4cpp/FileAppender.hh>
// #include <log4cpp/SimpleLayout.hh>
// #include <log4cpp/PatternLayout.hh>

#include<iostream>
#include<log4cpp/Category.hh>
#include<log4cpp/Appender.hh>
#include<log4cpp/OstreamAppender.hh>
#include<log4cpp/FileAppender.hh>
#include<log4cpp/RollingFileAppender.hh>
#include<log4cpp/PatternLayout.hh>
#include<log4cpp/Priority.hh>
#include<string>
#include<sstream>
using namespace std;
//using namespace log4cpp;   //为了程序可读性好,下面都自己加上作用域限定
//单例类封装log4cpp
namespace wd
{
        enum myPriority{EMERG,FATAL,ALERT,CRIT,ERROR,WARN,NOTICE,INFO,DEBUG};
        class myLog
        {
                public:
                        //static myLog* getInstance();  
                        //static void destroy();
                        void setPriority(myPriority priority);   //myPriority 我把它定义为一种enum类型
                        void fatal(const char* msg);
                        void error(const char* msg);
                        void warn(const char* msg);
                        void info(const char* msg);
                        void debug(const char* msg);
                private:
                        //static myLog* _plog;
                        log4cpp::Category& rootCategory;
                //private:
                public:
                        myLog();
                        ~myLog();
        };

        extern myLog logger;
        extern ostringstream logoss;
        /***********************实现(方便引用，就不单独放到一个文件中了)***********************/
        //myLog* myLog::_plog = NULL;  //静态成员初始化
        //构造函数


        // inline void myLog::destroy()
        // {
        //         _plog->rootCategory.info("myLog destroy");   //静态函数内部没有隐士传递的this指针，所以必须要用_plog对象指针来调用
        //         delete _plog;   //delete的处理方式是：先调用析构函数，然后在释放空间；
        //         //所以可以在析构函数里再向日志里打印信息，然后再关掉rootCategory
        // }
        // inline myLog* myLog::getInstance()
        // {
        //         if(NULL==_plog)
        //         {
        //                 _plog = new myLog();
        //         }
        //         return _plog;
        // }
        //类对象调用,重新设置rootCategory优先级


        /***********************myLog类定义完毕***********************/
        /***********************定义一些宏，引用的时候直接展开，让myLog使用更方便***********************/
        //把要输出的消息拼接上文件名，所在函数名，所在行号;msg是const char*，所以最后用c_str()函数
        //__FILE__ __func__ __LINE 这几个宏定义是编译器识别的，__LINE__是int形
        //分别是文件名，函数名，所在行号
        //防止在函数中调用，声明成内联函数
        inline string int2string(int lineNumber)
        {
                ostringstream oss;
                oss<<lineNumber;
                return oss.str();
        }

        // #define catMsg(msg) string(msg).append(" {fileName:")\
        //         .append(__FILE__).append(" functionName:")\
        //         .append(__func__).append(" lineNumber:")\
        //         .append(int2string(__LINE__)).append("}").c_str()

        /*
        #define catMsg(msg) string(msg).append("{functionName:")\
                .append(__func__).append(" lineNumber:")\
                .append(int2string(__LINE__)).append("}").c_str()

        */
        /*
        #define catMsg(msg) string(" functionName:")\
                .append(__func__).append(" lineNumber:")\
                .append(int2string(__LINE__)).append("}").append(msg).c_str()
        */
        #define catMsg(msg) string(__func__).append(":")\
                .append(string(msg)).c_str()


        //单例类，只有一个对象
        //
        #define logSetpriority(priority) log->setPriority(priority)
        #define logError(msg) log->error(catMsg(msg))
        #define logWarn(msg) log->warn(catMsg(msg))
        #define logInfo(msg) logger.info(catMsg(msg))
        #define logDebug(msg) log->debug(catMsg(msg))
        #define logFatal(msg) log->fatal(catMsg(msg))
        #define logDestroy() myLog::destroy()
}

#endif


/*
#include"myLog.h"
#include<iostream>
using namespace std;
void func()
{
        logInfo("this is a function");
}
int main()
{
        logInfo("system is running");
        logWarn("system has a warning");
        logError("system has a error");
        logFatal("system has a fatal");
        logSetpriority(ERROR);  //重新设置优先级
        logWarn("warn again");  //Warn的优先级小于ERROR,这条信息将不会被记录
        logSetpriority(DEBUG);  //为了记录下面代码的日志信息,INFO的优先级也是小于ERROR的
        func();
        logDestroy();    //程序结束，销毁myLog对象
        return 0;
}
*/

