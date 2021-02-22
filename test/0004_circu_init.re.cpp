//Author: 张子辰
//This file is in public domain.
#include <iostream>
#include <global_variable.hpp>
using std::cout;using std::endl;

global_variable(extern,int,i0);
global_variable(extern,int,i1);
global_variable(extern,int,i2);
global_variable(extern,int,i3);
global_variable(extern,int,i4);
global_variable(extern,int,i5);
global_variable(extern,int,i6);
global_variable(extern,int,i7);
global_variable(extern,int,i8);
global_variable(extern,int,i9);
global_variable(extern,int,iA);
global_variable(extern,int,iB);
global_variable(extern,int,iC);
global_variable(extern,int,iD);
global_variable(extern,int,iE);
global_variable(extern,int,iF);

int main()
{
	cout<<"this string will never be output"<<endl;
	return 0;
}

global_variable(int,i0,*i1+0x1);
global_variable(int,i1,*i2+0x2);
global_variable(int,i2,*i3+0x3);
global_variable(int,i3,*i4+0x4);
global_variable(int,i4,*i5+0x5);
global_variable(int,i5,*i6+0x6);
global_variable(int,i6,*i7+0x7);
global_variable(int,i7,*i8+0x8);
global_variable(int,i8,*i9+0x9);
global_variable(int,i9,*iA+0xA);
global_variable(int,iA,*iB+0xB);
global_variable(int,iB,*iC+0xC);
global_variable(int,iC,*iD+0xD);
global_variable(int,iD,*iE+0xE);
global_variable(int,iE,*iF+0xF);
global_variable(int,iF,*i0+0x0);