//global_variable.hpp
//version 1.0.0
//Copyright (C) 2021 张子辰
//安全地处理可能在构造前使用的全局变量

//	Copying and distribution of this file, with or without modification,
//	are permitted in any medium without royalty provided the copyright
//	notice and this notice are preserved.  This file is offered as-is,
//	without any warranty.

#include <stdexcept>
#include <typeinfo>
#include <string>
#include <cstring>
#if defined(__GNUC__)||defined(__clang__)
#include <cxxabi.h>
#endif

class circular_initialization:public std::logic_error
{
public:
	circular_initialization(const std::string &what_arg)
		:std::logic_error(what_arg){}
	circular_initialization(const char* what_arg)
		:std::logic_error(what_arg){}
	circular_initialization(const circular_initialization& other)noexcept
		:std::logic_error(other){}
};

template <typename Tp,typename Init>
class global_variable_t
{
private:
#if defined(__amd64)||defined(__x86_64)||defined(_WIN64)||(defined(__SIZEOF_SIZE_T__)&&__SIZEOF_SIZE_T__==8)
	mutable size_t d:63;
	mutable bool initizing:1;
#else
	mutable Tp* d;
	mutable bool initizing;
#endif
	std::string variable_name()const
	{
		using namespace std;
		string name;
#if defined(__GNUC__)||defined(__clang__)
		char *realname;
		int status;
		realname=abi::__cxa_demangle(typeid(Init).name(),nullptr,nullptr,&status);
		name=realname;
		free(realname);	
#else
		name=typeid(Init).name();
#endif
		if(name.length()>22&&strncmp(name.data(),"_____SGV_",8)==0)
		{
			name=name.substr(9,name.length()-22);
		}
		else name="<unknown variable:"+name+">";
		return name;
	}
	void init()const
	{
		if(d==0)
		{
			if(!initizing)initizing=true;
			else
				throw circular_initialization("the circle is `"+variable_name()+"'");
			try
			{
				d=reinterpret_cast<size_t>(new Tp(Init()()));
			}
			catch(circular_initialization &err)
			{
				using namespace std;
				throw circular_initialization(err.what()+" <-- `"s+variable_name()+"'");
			}
			initizing=false;
		}
	}
	global_variable_t(const global_variable_t&)=delete;
public:
	global_variable_t()
	{
		init();
	}
	typedef Tp type;
	Tp& operator*()&
	{
		init();
		return *reinterpret_cast<Tp*>(d);
	}
	const Tp& operator*()const&
	{
		init();
		return *reinterpret_cast<Tp*>(d);
	}
	Tp* operator->()
	{
		init();
		return reinterpret_cast<Tp*>(d);
	}
	const Tp* operator->()const
	{
		init();
		return reinterpret_cast<Tp*>(d);
	}
	~global_variable_t()
	{
		delete reinterpret_cast<Tp*>(d);
	}
};
//延迟拼接
#define ____SGV_CONCAT(x,y) ____SGV_CONCAT_IMPL(x,y)
#define ____SGV_CONCAT_IMPL(x,y) x##y

//条件
#define ____SGV_IF(STA,THEN,ELSE) ____SGV_CONCAT(____SGV_IF_,STA) (THEN,ELSE)
#define ____SGV_IF_0(THEN,ELSE) ELSE
#define ____SGV_IF_1(THEN,ELSE) THEN

//获取参数
#define ____SGV_GET_2(_0,_1,_2,...) _2

//判断是否有括号
#define ____SGV_COMMA_V(...) ,
#define ____SGV_IS_PARENS(x) ____SGV_IS_PARENS_IMPL(____SGV_COMMA_V x)
#define ____SGV_IS_PARENS_IMPL(...) ____SGV_GET_2(__VA_ARGS__,1,0,0)

//移除括号
#define ____SGV_ECHO(x) x
#define ____SGV_REMOVE_PARENS(x) ____SGV_REMOVE_PARENS_IMPL x
#define ____SGV_REMOVE_PARENS_IMPL(...) __VA_ARGS__
#define ____SGV_TRY_REMOVE_PARENS(x)\
	____SGV_IF(____SGV_IS_PARENS(x),____SGV_REMOVE_PARENS,____SGV_ECHO)(x)

#define ____SGV_extern_global_variable(specifier,type,name)\
	class _____SGV_##name##_helper_class;\
	extern specifier global_variable_t<____SGV_TRY_REMOVE_PARENS(type),_____SGV_##name##_helper_class> name

#define ____SGV_global_variable(specifier,type,name,init)\
	class _____SGV_##name##_helper_class\
	{public:____SGV_TRY_REMOVE_PARENS(type) operator()(){return ____SGV_TRY_REMOVE_PARENS(init);}};\
	specifier global_variable_t<____SGV_TRY_REMOVE_PARENS(type),_____SGV_##name##_helper_class> name
	
#ifndef _LIB_SAFE_GLOBAL_VAR_NO_MACRO
#	define extern_global_variable ____SGV_extern_global_variable
#	define global_variable ____SGV_global_variable
#endif
