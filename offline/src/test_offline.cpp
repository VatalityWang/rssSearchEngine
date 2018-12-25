 ///
 /// @file    test_offline.cpp
 /// @author  Damon(1225228598@qq.com)
 /// @data    2017-11-12 11:21:17
 ///

#include "GlobalDefine.hpp"
#include "Configuration.hpp"
#include "DirScanner.hpp"
#include "PageLib.hpp"
#include "WordSegmentation.hpp"
#include "WebPage.hpp"
#include "PageLibPreprocessor.hpp"

#include <stdio.h>
#include <time.h>

int main(void)
{
	wd::Configuration conf("conf/my.conf");
	conf.debug();

	wd::DirScanner dirScanner(conf);//创建该对象(配置参数)
	dirScanner();
	dirScanner.debug();

	wd::PageLib pagelib(conf, dirScanner);

	time_t t1 = time(NULL);
	pagelib.create();
	pagelib.store();
	time_t t2 = time(NULL);
	printf("raw lib create and store time: %d min\n", (t2 - t1));

	wd::PageLibPreprocessor libPreprocessor(conf);//创建对象
	libPreprocessor.doProcess();

	return 0;
}
