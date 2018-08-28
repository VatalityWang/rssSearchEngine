#include"MyLog.h"

namespace wd
{
        myLog logger;
        ostringstream logoss;
        myLog::myLog() : rootCategory(log4cpp::Category::getRoot().getInstance("rootCategory"))
        {
                //获取根Category，引用成员必须在成员初始化列表初始化

                //根Category下添加子Category
                /*
                log4cpp::OstreamAppender* osAppender = new log4cpp::OstreamAppender("osAppender",&cout);
                log4cpp::PatternLayout* pLayout1 = new log4cpp::PatternLayout();
                pLayout1->setConversionPattern("%d: %p %c %x: %m%n");
                osAppender->setLayout(pLayout1);
                rootCategory.addAppender(osAppender);
                rootCategory.setPriority(log4cpp::Priority::DEBUG);
                */

                //mylog.log会一直记录
                log4cpp::FileAppender *fileAppender = new log4cpp::FileAppender("fileAppender", "log.txt");
                log4cpp::PatternLayout *pLayout2 = new log4cpp::PatternLayout();
                pLayout2->setConversionPattern("%d: %p %c %x: %m%n");
                fileAppender->setLayout(pLayout2);
                rootCategory.addAppender(fileAppender);
                rootCategory.setPriority(log4cpp::Priority::DEBUG);

                //mylogrolling.log不超过指定大小，默认10M；这里我设置了5M,备份文件5个
                //如果想要用备份带回卷的日志文件记录方式，就把这里的注释去掉
                //        log4cpp::RollingFileAppender* rollingfileAppender = new log4cpp::RollingFileAppender(
                //                        "rollingfileAppender","logrolling.log",5*1024,5);
                //        log4cpp::PatternLayout* pLayout3 = new log4cpp::PatternLayout();
                //        pLayout3->setConversionPattern("%d: %p %c %x: %m%n");
                //        rollingfileAppender->setLayout(pLayout3);
                //        rootCategory.addAppender(rollingfileAppender);
                //        rootCategory.setPriority(log4cpp::Priority::DEBUG);

                rootCategory.info("myLog()");
        }
        myLog::~myLog()
        {
                rootCategory.info("~mylog()");
                rootCategory.shutdown();
        }
        void myLog::setPriority(myPriority priority)
        {
                switch (priority)
                {
                case FATAL:
                        rootCategory.setPriority(log4cpp::Priority::FATAL);
                        break;
                case ERROR:
                        rootCategory.setPriority(log4cpp::Priority::ERROR);
                        break;
                case WARN:
                        rootCategory.setPriority(log4cpp::Priority::WARN);
                        break;
                case INFO:
                        rootCategory.setPriority(log4cpp::Priority::INFO);
                        break;
                case DEBUG:
                        rootCategory.setPriority(log4cpp::Priority::DEBUG);
                        break;
                default:
                        rootCategory.setPriority(log4cpp::Priority::DEBUG);
                        break;
                }
        }
        void myLog::error(const char *msg)
        {
                rootCategory.error(msg);
        }
        void myLog::warn(const char *msg)
        {
                rootCategory.warn(msg);
        }
        void myLog::info(const char *msg)
        {
                rootCategory.info(msg);
        }
        void myLog::debug(const char *msg)
        {
                rootCategory.debug(msg);
        }
        void myLog::fatal(const char *msg)
        {
                rootCategory.fatal(msg);
        }
} // namespace wd