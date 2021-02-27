//Author: 张子辰
//This file is in public domain.
#include <iostream>
#include <global_variable.hpp>
using std::cout;using std::endl;

class test_class
{
public:
	test_class(int id)
		:id(id)
	{
		cout<<id<<" construct"<<endl;
	}
	int get_id()const{return id;}
private:
	int id;
};

global_variable(extern,test_class,a);
global_variable(test_class,b,a->get_id()+1);
global_variable(test_class,a,100);

extern test_class a0;
test_class b0=a0.get_id()+1;
test_class a0=100;

int main(int argc,char **argv)
{
	cout<<a->get_id()<<" "<<b->get_id()<<endl;
	cout<<a0.get_id()<<" "<<b0.get_id()<<endl;
	return 0;
}