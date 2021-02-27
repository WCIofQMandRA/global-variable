//Author: 张子辰
//This file is in public domain.

#include <iostream>
#include <global_variable.hpp>
using std::cout;using std::endl;

namespace test_ns1
{
	global_variable(int,a,-1);
	global_variable(extern,int,b);
	namespace inner
	{
		global_variable(int,a,100);
	}
}

namespace test_ns2
{
	global_variable(int,a,-100);
	global_variable(int,b,*test_ns1::b>>1);
}

namespace test_ns3
{
	global_variable(int,c,10);
}

namespace test_ns4
{
	global_variable(int,c,-10);
}

int main()
{
	{
		using namespace test_ns3;
		cout<<*test_ns1::a<<" "<<*test_ns2::a<<" "<<*test_ns1::inner::a<<" "<<*test_ns1::b<<" "<<*c<<endl;
	}
	{
		using test_ns4::c;
		using namespace test_ns1;
		cout<<*a<<" "<<*test_ns2::b<<" "<<*inner::a<<" "<<*b<<" "<<*c<<endl;
	}
	{
		using namespace test_ns2;
		cout<<*a<<" "<<*b<<endl;
	}
}

namespace test_ns1
{
	global_variable(int,b,1024);
}
