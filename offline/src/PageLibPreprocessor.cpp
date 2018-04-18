 ///
 /// @file    PageLibPreprocessor.cpp
 /// @author  lemon(haohb13@gmail.com)
 /// @date    2016-01-19 11:35:20
 ///


#include "PageLibPreprocessor.hpp"
#include "GlobalDefine.hpp"

#include <stdio.h>
#include <time.h>
#include <fstream>
#include <sstream>

using std::ifstream;
using std::ofstream;
using std::stringstream;

namespace wd
{

PageLibPreprocessor::PageLibPreprocessor(Configuration & conf)
: _conf(conf)
{}



void PageLibPreprocessor::doProcess()
{
	readInfoFromFile();
	time_t t1 = time(NULL);
	cutRedundantPages();//网页去重
	buildInvertIndexTable();//建立倒排索引
	time_t t2 = time(NULL);
	printf("preprocess time: %d min\n", (t2 - t1)/60);

	storeOnDisk();
	time_t t3 = time(NULL);
	printf("store time: %d min\n", (t3 - t2)/60);
}

void PageLibPreprocessor::readInfoFromFile()//根据配置信息读取网页库和偏移库的内容
{
	map<string, string> &configMap = _conf.getConfigMap();
	string pageLibPath = configMap[RIPEPAGELIB_KEY];//页面路径
	string offsetLibPath = configMap[OFFSETLIB_KEY];//偏移路径
	
	ifstream pageIfs(pageLibPath.c_str());
	ifstream offsetIfs(offsetLibPath.c_str());

	if((!pageIfs.good()) || (!offsetIfs.good()))
	{
		cout << "page or offset lib open error" << endl;
	}

	string line;
	int docId, docOffset, docLen;

	while(getline(offsetIfs, line))
	{
		stringstream ss(line);
		ss >> docId >> docOffset >> docLen;//分别获取文档ID，文档偏移量，文档长度

		string doc;
		doc.resize(docLen, ' ');//将该字符串变成doclen长度，并初始化为空
		pageIfs.seekg(docOffset, pageIfs.beg);//设置当前字符流的偏移量（相对于该字符流的起始位置）
		pageIfs.read(&*doc.begin(), docLen);//从网页库中读数据，doc的开始存doclen个字节的数据到doc中

		WebPage webPage(doc, _conf, _jieba);//将该网页分词后存成网页
		_pageLib.push_back(webPage);// 加入网页库中

		_offsetLib.insert(std::make_pair(docId, std::make_pair(docOffset, docLen)));//网页偏移库：文档ID,文档偏移量（相对于起始位置），文档长度
	}
#if 0
	for(auto mit : _offsetLib)
	{
		cout << mit.first << "\t" << mit.second.first << "\t" << mit.second.second << endl;
	}
#endif
}

void PageLibPreprocessor::cutRedundantPages()//_pageLib 类型为vector<WebPage>
{
	for(size_t i = 0; i != _pageLib.size() - 1; ++i)
	{
		for(size_t j = i + 1; j != _pageLib.size(); ++j)
		{
			if(_pageLib[i] == _pageLib[j])
			{
				_pageLib[j] = _pageLib[_pageLib.size() - 1];//将最后一个网页赋给当前与下标i重复的网页
				_pageLib.pop_back();//弹出最后的网页
				--j;//最后一个网页与当前网页开始比较
			}
		}
	}
}

void PageLibPreprocessor::buildInvertIndexTable()
{
	for(auto page : _pageLib)
	{
		map<string, int> & wordsMap = page.getWordsMap();
		for(auto wordFreq : wordsMap)
		{
			_invertIndexTable[wordFreq.first].push_back(std::make_pair(
					page.getDocId(), wordFreq.second));//vector<pair<文档ID，该词在该文档中的词频>>
		}
	}
	
	//计算每篇文档中的词的权重,并归一化
	map<int, double> weightSum;// 保存每一篇文档中所有词的权重平方和. int 代表docid

	int totalPageNum = _pageLib.size();
	for(auto & item : _invertIndexTable)
	{	
		int df = item.second.size();//vector的的大小
		//求关键词item.first的逆文档频率
		double idf = log(static_cast<double>(totalPageNum)/ df + 0.05) / log(2);

		for(auto & sitem : item.second)
		{
			double weight = sitem.second * idf;//权重=某词在文档中出现的次数*逆文档频率

			weightSum[sitem.first] += pow(weight, 2);
			sitem.second = weight;
		}
	}

	for(auto & item : _invertIndexTable)
	{	//归一化处理
		for(auto & sitem : item.second)
		{
			sitem.second = sitem.second / sqrt(weightSum[sitem.first]);
		}
	}


#if 0 // for debug
	for(auto item : _invertIndexTable)
	{
		cout << item.first << "\t";
		for(auto sitem : item.second)
		{
			cout << sitem.first << "\t" << sitem.second <<  "\t";
		}
		cout << endl;
	}
#endif
}

void PageLibPreprocessor::storeOnDisk()//将倒排索引和网页库 偏移库存盘
{

	sort(_pageLib.begin(), _pageLib.end());	

	ofstream ofsPageLib(_conf.getConfigMap()[NEWPAGELIB_KEY].c_str());
	ofstream ofsOffsetLib(_conf.getConfigMap()[NEWOFFSETLIB_KEY].c_str());

	if( !ofsPageLib.good() || !ofsOffsetLib.good())
	{	
		cout << "new page or offset lib ofstream open error!" << endl;
	}


	for(auto & page : _pageLib)
	{
		int id = page.getDocId();
		int length = page.getDoc().size();
		ofstream::pos_type offset = ofsPageLib.tellp();
		ofsPageLib << page.getDoc();

		ofsOffsetLib << id << '\t' << offset << '\t' << length << '\n';//文档ID 偏移量 长度
	}

	ofsPageLib.close();
	ofsOffsetLib.close();


	// invertIndexTable
	ofstream ofsInvertIndexTable(_conf.getConfigMap()[INVERTINDEX_KEY].c_str());
	if(!ofsInvertIndexTable.good())
	{
		cout << "invert index table ofstream open error!" << endl;
	}
	for(auto item : _invertIndexTable)
	{
		ofsInvertIndexTable << item.first << "\t";
		for(auto sitem : item.second)
		{
			ofsInvertIndexTable << sitem.first << "\t" << sitem.second <<  "\t";
		}
		ofsInvertIndexTable << endl;
	}
	ofsInvertIndexTable.close();
}

}// end of namespace wd
