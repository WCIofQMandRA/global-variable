//Author: 张子辰
//This file is in public domain.
#include <global_variable.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <cstdio>
using namespace std;

global_variable((map<string,std::pair<int,double>>),a,({{"Key1",{1,0.1}},{"Key2",{2,0.2}}}));
void fun1(std::vector<int>,const basic_string<char>&);

//指向函数的指针 pointer to a function
//void (*f_ptr1)(vector<int>,const string&)
global_variable((void(*)(vector<int>,const string&)),f_ptr1,fun1);
global_variable((function<void(vector<int>,const string&)>),f_ptr2,
	([](vector<int> v,const string &s)->void
	{
		cout<<"In lambda:";
		for(auto it=v.rbegin();it!=v.rend();++it)
		{
			putchar(s.at(*it));
		}
		putchar('\n');
	}));
int arr[5]={-1,-2,-3,-4,-5};

//指向含5个元素的int数组的指针 pointer to an array of int containing 5 elements
//int (*arr)[5]
global_variable(const,(int(*)[5]),b,&arr);

//指向函数的指针，该函数的返回值为指向含5个元素的int数组的指针
//pointer to a function, whose return value is a pointer to an array of int containing 5 elements
//typedef int(*int5_ptr)[5]
//int5_ptr (*f_ptr3)()
global_variable((int(*(*)())[5]),f_ptr3);

int(*fun2())[5] {return &arr;}

int main()
{
	a->at("Key1").second*=3.5;
	for(const auto &i:*a)
	{
		cout<<i.first<<"["<<i.second.first<<","<<i.second.second<<"]\n";
	}
	(*f_ptr1)({0,5},"ABCDEFG");
	(*f_ptr2)({0,5},"ABCDEFG");
	*f_ptr2=*f_ptr1;
	(*f_ptr2)({0,5},"ABCDEFG");
	for(size_t i=0;i<5;++i)
		cout<<(**b)[i]<<" ";
	cout<<"\n";
	cout<<"arr.size="<<sizeof(**b)/sizeof(int)<<endl;
	*f_ptr3=fun2;
	cout<<((*f_ptr3)()==&arr)<<endl;
	return 0;
}

void fun1(std::vector<int> v,const basic_string<char> &s)
{
	cout<<"In fun1:";
	for(auto it=v.begin();it!=v.end();++it)
	{
		putchar(s.at(*it));
	}
	putchar('\n');
}