 ///
 /// @file    DirScanner.hpp
 /// @author  Damon(1225228598@qq.com)
 /// @data    2017-11-10 09:26:00
 ///


#ifndef _WD_DIRSCAN_H_
#define _WD_DIRSCAN_H_

#include <string>
#include <vector>

namespace wd
{

const int kFileNo = 10000;

class Configuration;

class DirScanner  //建立连接时扫描目录
{
public:
	DirScanner(Configuration & conf);

	void operator()();//重载括号运算符

	std::vector<std::string> & files();//读取文件

	void debug();//创建对象并调试
private:
	void traverse(const std::string & dirName);

private:
	Configuration & conf_;
	std::vector<std::string> vecFiles_;
};

} // end of namespace wd
#endif
