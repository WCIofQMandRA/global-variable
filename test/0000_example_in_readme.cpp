//Author: 张子辰
//This file is in public domain.
#include <iostream>
#include <utility>
#include <string>
#include <global_variable.hpp>
global_variable(int,a);	//equal to int a;
global_variable(extern const,(std::pair<int,double>),b);
global_variable(int,b_first,b->first);	//initalize before b
global_variable(extern const,(std::pair<int,double>),b,({2,3.01}));	
global_variable(extern,double,c);
global_variable(long long,d,0xFFFFFFFFFF);
//global_variable(const,int,e);//error: static assertion failed: uninitialized 'const e'
global_variable(extern,int,f,2);//warning: 'int ____SGV_helper_function_f()' is deprecated: 'f' initialized and declared 'extern'
std::string function(int x)
{
	return "void function("+std::to_string(x)+")";
}
global_variable(std::string(*)(int),ptr_function,function);

int main()
{
	std::cout<<*b_first<<" "<<b->second<<" "<<*d<<std::endl;
	//std::cout<<*c; //error: undefined reference to `c'
	++*f*=5;
	std::cout<<(*ptr_function)(*f)<<std::endl;
}