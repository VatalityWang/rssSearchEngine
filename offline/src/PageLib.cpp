 ///
 /// @file    PageLib.cpp
 /// @author  Damon(1225228598@qq.com)
 /// @data    2017-11-10 16:55:10
 ///


#include "Configuration.hpp"
#include "PageLib.hpp"
#include "DirScanner.hpp"
#include "RssReader.hpp"
#include "GlobalDefine.hpp"

#include <iostream>
#include <fstream>

namespace wd
{
PageLib::PageLib(Configuration & conf,
				 DirScanner & dirScanner)
	: _conf(conf),
	  _dirScanner(dirScanner)
{}

void PageLib::create()
{
	std::vector<std::string> & vecFiles = _dirScanner.files();//xml文件的路径
	RssReader reader(vecFiles);
	reader.loadFeedFiles();
	reader.makePages(_vecPages);//将存放网页的vector传递给makePages形成网页库
}

void PageLib::store()
{
	//需要读取配置信息//???
	std::map<std::string, std::string> & confMap = _conf.getConfigMap();//获得配置文件的map容器
	std::string pageLibPath = confMap[RIPEPAGELIB_KEY];//网页库路径
	std::string offsetLibPath = confMap[OFFSETLIB_KEY];//网页偏移库路径

	std::ofstream ofsPage(pageLibPath.c_str());//
	std::ofstream ofsOffset(offsetLibPath.c_str());//
	if(!(ofsPage.good() && ofsOffset.good()))//
	{
		std::cout << "ofstream open error!" << std::endl;
		return;
	}
	for(size_t idx = 0; idx != _vecPages.size(); ++idx)
	{
		int id = idx + 1;
		int length = _vecPages[idx].size();//第idx篇网页的长度
		std::ofstream::pos_type offset = ofsPage.tellp();//当前网页相对于网页库起始位置的偏移量
		ofsPage << _vecPages[idx];

		ofsOffset << id << '\t' << offset << '\t' << length << '\n';
	}
	ofsPage.close();
	ofsOffset.close();
}

}//end of namespace wd
