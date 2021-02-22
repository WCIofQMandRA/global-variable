//Author: 张子辰
//This file is in public domain.
#include <iostream>
#include <global_variable.hpp>

global_variable(int,a,6);
global_variable(const,int,b,2);
global_variable(int,c);
global_variable(static,int,d);

int main(int argc,char **argv)
{
	std::cout<<*a<<" "<<*b<<std::endl;
	*c=*b-*a,*d=*b* *a-*c;
	std::cout<<*c<<" "<<*d<<std::endl;
	--(*a=*b-2);
	std::cout<<*a<<std::endl;
	std::cin>>*d;
	std::cout<<*d/ *b<<std::endl;
	return 0;
}