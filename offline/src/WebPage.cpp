 ///
 /// @file    WebPage.cpp
 /// @author  Damon(1225228598@qq.com)
 /// @date    2016-01-18 17:28:29
 ///

#include "WebPage.hpp"

#include <stdlib.h>
#include <iostream>
#include <queue>
#include <algorithm>

using std::cout;
using std::endl;
using std::priority_queue;
using std::pair;
using std::make_pair;


namespace wd
{

struct WordFreqCompare
{
	bool operator()(const pair<string, int> & left, const pair<string, int> & right)
	{
		if(left.second < right.second)
		{	return true;	}
		else if(left.second == right.second && left.first > right.first)
		{	return true;	}
		else
		{	return false;	}
	}
};


WebPage::WebPage(string & doc, Configuration & config, WordSegmentation & jieba)
: _doc(doc)
{
	//cout << "WebPage()" << endl;
	_topWords.reserve(10);
	processDoc(doc, config, jieba);

}


void WebPage::processDoc(const string & doc, Configuration & config, WordSegmentation & jieba)
{
	string docIdHead = "<docid>";
	string docIdTail = "</docid>";
	string docUrlHead = "<link>";
	string docUrlTail = "</link>";
	string docTitleHead = "<title>";
	string docTitleTail = "</title>";
	string docContentHead = "<content>";
	string docContentTail = "</content>";

	//提取文档的docid
	int bpos = doc.find(docIdHead);//
	int epos = doc.find(docIdTail);//
	string docId = doc.substr(bpos + docIdHead.size(), 
					epos - bpos - docIdHead.size());//
	_docId = atoi(docId.c_str());

	//title
	bpos = doc.find(docTitleHead);//
	epos = doc.find(docTitleTail);//
	_docTitle = doc.substr(bpos + docTitleHead.size(), 
					epos - bpos - docTitleHead.size());

	//cout << "========" << endl << _docTitle << endl;
	//content
	bpos = doc.find(docContentHead);//
	epos = doc.find(docContentTail);//
	_docContent = doc.substr(bpos + docContentHead.size(),
					epos - bpos - docContentHead.size());

	//cout << "========" << endl << _docContent << endl;


	//分词
	vector<string> wordsVec = jieba(_docContent.c_str());//将文档内容进行分词
	set<string> & stopWordList = config.getStopWordList();//读取停词词库
	calcTopK(wordsVec, TOPK_NUMBER, stopWordList);//调用TopK算法计算该网页的topK词集

}


void WebPage::calcTopK(vector<string> & wordsVec, int k, set<string> & stopWordList)
{
	for(auto iter = wordsVec.begin(); iter != wordsVec.end(); ++iter)
	{
		auto sit = stopWordList.find(*iter);
		if(sit == stopWordList.end())//在停用词词集中没有找到改词
		{
			++_wordsMap[*iter];//用来存储文档中的所用词（去停用词后），该词的词频
		}
	}

	priority_queue<pair<string, int>, vector<pair<string, int> >, WordFreqCompare>
		wordFreqQue(_wordsMap.begin(), _wordsMap.end());//创建特权队列


	while(!wordFreqQue.empty())
	{
		string top = wordFreqQue.top().first;
		wordFreqQue.pop();
		if(top.size() == 1 && (static_cast<unsigned int>(top[0]) == 10 ||
			static_cast<unsigned int>(top[0]) == 13))//强制类型转换为无符号整型
		{	continue;	}

		_topWords.push_back(top);//存词频最高的几个词
		if(_topWords.size() >= static_cast<size_t>(k))// 只取词频最高的k个词
		{
			break;
		}
	}


#if 0
	for(auto mit : _wordsMap)
	{
		cout << mit.first << "\t" << mit.second << std::endl;	
	}
	cout << endl;

	for(auto word : _topWords)
	{
		cout << word << "\t" << word.size() << "\t" << static_cast<unsigned int>(word[0]) << std::endl;
	}
#endif
}

// 判断两篇文档是否相同
bool operator == (const WebPage & lhs, const WebPage & rhs) 
{
	int commNum = 0;
	auto lIter = lhs._topWords.begin();
	for(;lIter != lhs._topWords.end(); ++lIter)
	{
		commNum += std::count(rhs._topWords.begin(), rhs._topWords.end(), *lIter);//计算两篇文章中高频单词相同的个数
	}
	int lhsNum = lhs._topWords.size();
	int rhsNum = rhs._topWords.size();
	int totalNum = lhsNum < rhsNum ? lhsNum : rhsNum;

	if( static_cast<double>(commNum) / totalNum > 0.75 )
	{	return true;	}
	else 
	{	return false;	}
}

//对文档按照docId进行排序
bool operator < (const WebPage & lhs, const WebPage & rhs)
{
	if(lhs._docId < rhs._docId)
		return true;
	else 
		return false;
}

}// end of namespace wd
