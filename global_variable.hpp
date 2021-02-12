//global_variable.hpp
//version 2.0.0
//Copyright (C) 2021 张子辰
//安全地处理可能在构造前使用的全局变量

//	Copying and distribution of this file, with or without modification,
//	are permitted in any medium without royalty provided the copyright
//	notice and this notice are preserved.  This file is offered as-is,
//	without any warranty.

#ifndef ____SGV_global_variable
#include <stdexcept>

#define ____SGV_VERS_MAJOR 2ull
#define ____SGV_VERS_MINOR 0ull
#define ____SGV_VERS_PATCHLEVEL 0ull

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

#if defined(__cpp_nontype_template_parameter_auto) || __cplusplus>=201703L
template <typename Tp,const auto &Init,const char*const*const name=nullptr>
#else
template <typename Tp,Tp (&Init)(),const char*const*const name=nullptr>
#endif
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
		if(name!=nullptr&&*name!=nullptr)return *name;
		else return "<unknown variable>";
	}
	void init()const
	{
		if(d==0)
		{
			if(!initizing)initizing=true;
			else
				throw circular_initialization("the circle is '"+variable_name()+"'");
			try
			{
				d=reinterpret_cast<size_t>(new Tp(Init()));
			}
			catch(circular_initialization &err)
			{
				using namespace std;
				throw circular_initialization(err.what()+string(" <-- '")+variable_name()+"'");
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
#define ____SGV_CONCAT3(x,y,z) ____SGV_CONCAT3_IMPL(x,y,z)
#define ____SGV_CONCAT3_IMPL(x,y,z) x##y##z
#define ____SGV_CONCAT5(x,y,z,w,v) ____SGV_CONCAT5_IMPL(x,y,z,w,v)
#define ____SGV_CONCAT5_IMPL(x,y,z,w,v) x##y##z##w##v

//条件
#define ____SGV_IF(STA,THEN,ELSE) ____SGV_CONCAT(____SGV_IF_,STA) (THEN,ELSE)
#define ____SGV_IF_0(THEN,ELSE) ELSE
#define ____SGV_IF_1(THEN,ELSE) THEN

//获取参数
#define ____SGV_GET_2(_0,_1,_2,...) _2
#define ____SGV_GET_7(_0,_1,_2,_3,_4,_5,_6,_7,...) _7

//判断是否有括号
#define ____SGV_IS_PARENS(x) ____SGV_IS_PARENS_IMPL(____SGV_COMMA_V x)
#define ____SGV_IS_PARENS_IMPL(...) ____SGV_GET_2(__VA_ARGS__,1,0,0)
#define ____SGV_COMMA_V(...) ,

//判断是否为空，假定x不是元组
#define ____SGV_EMPTY_1(x) ____SGV_EMPTY_1_IMPL(____SGV_COMMA_V x(),1,0)
#define ____SGV_EMPTY_1_IMPL(...) ____SGV_GET_2(__VA_ARGS__)

//移除括号
#define ____SGV_REMOVE_PARENS(x) ____SGV_REMOVE_PARENS_IMPL x
#define ____SGV_REMOVE_PARENS_IMPL(...) __VA_ARGS__
//在有括号时移除括号
#define ____SGV_TRY_REMOVE_PARENS(x)\
	____SGV_IF(____SGV_IS_PARENS(x),____SGV_REMOVE_PARENS,____SGV_ECHO)(x)
#define ____SGV_ECHO(x) x//回显参数

//检查x是否是合法的限定符
#define ____SGV_IS_SPECIFIER(x)\
	____SGV_IF(____SGV_IS_PARENS(x),____SGV_IS_SPECIFIER_TUPLE,____SGV_IS_SPECIFIER_NTUPLE)(x)
#define ____SGV_IS_SPECIFIER_TUPLE(x) 0
//检查非元组x是否是合法的限定符
#define ____SGV_IS_SPECIFIER_NTUPLE(x) ____SGV_EMPTY_1(____SGV_IS_SPECIFIER_IMPL0(x))
//检查限定符是否合法，合法时展开为空，否则为一个非空、非元组的宏参数
#define ____SGV_IS_SPECIFIER_IMPL0(x) ____SGV_IS_SPECIFIER_IMPL1(____SGV_REMOVE_RIGHT_PAR(____SGV_CHECKv(x)))
//移除未配对的右括号，其中x在展开前括号配对，展开后有游离的右括号
#define ____SGV_REMOVE_RIGHT_PAR(x) ____SGV_ECHO(x
//当且仅当x为____SGV_HASv_时，展开为空
#define ____SGV_IS_SPECIFIER_IMPL1(x) ____SGV_CONCAT(____SGV_IS_SPECIFIER_IMPL2,x)
#define ____SGV_IS_SPECIFIER_IMPL2____SGV_HASv_
#define ____SGV_CHECKv(...) ____SGV_HASv_##__VA_ARGS__)
#define ____SGV_CHECKv1(...) ____SGV_HASv_##__VA_ARGS__)
#define ____SGV_CHECKv2(...) ____SGV_HASv_##__VA_ARGS__)
#define ____SGV_CHECKv3(...) ____SGV_HASv_##__VA_ARGS__)
#define ____SGV_CHECKv4(...) ____SGV_HASv_##__VA_ARGS__)
#define ____SGV_CHECKv5(...) ____SGV_HASv_##__VA_ARGS__)
#define ____SGV_HASv_static ____SGV_CHECKv1(
#define ____SGV_HASv_const ____SGV_CHECKv2(
#define ____SGV_HASv_thread_local ____SGV_CHECKv3(
#define ____SGV_HASv_volatile ____SGV_CHECKv4(
#define ____SGV_HASv_extern ____SGV_CHECKv5(

//检测限定符中是否有extern
#define ____SGV_CHECK0(...) ____SGV_HAS0_##__VA_ARGS__)
#define ____SGV_CHECK01(...) ____SGV_HAS0_##__VA_ARGS__)
#define ____SGV_CHECK02(...) ____SGV_HAS0_##__VA_ARGS__)
#define ____SGV_CHECK03(...) ____SGV_HAS0_##__VA_ARGS__)
#define ____SGV_CHECK04(...) ____SGV_HAS0_##__VA_ARGS__)
#define ____SGV_HAS0_static ____SGV_CHECK01(
#define ____SGV_HAS0_const ____SGV_CHECK02(
#define ____SGV_HAS0_thread_local ____SGV_CHECK03(
#define ____SGV_HAS0_volatile ____SGV_CHECK04(
#define ____SGV_HAS0_extern ____SGV_HAS0_extern_IMPL(
#define ____SGV_HAS0_ ____SGV_HAS0_IMPL(
#define ____SGV_HAS0_IMPL(...) 0
#define ____SGV_HAS0_extern_IMPL(...) 1

//检测限定符中是否有static
#define ____SGV_CHECK1(...) ____SGV_HAS1_##__VA_ARGS__)
#define ____SGV_CHECK11(...) ____SGV_HAS1_##__VA_ARGS__)
#define ____SGV_CHECK12(...) ____SGV_HAS1_##__VA_ARGS__)
#define ____SGV_CHECK13(...) ____SGV_HAS1_##__VA_ARGS__)
#define ____SGV_CHECK14(...) ____SGV_HAS1_##__VA_ARGS__)
#define ____SGV_HAS1_extern ____SGV_CHECK11(
#define ____SGV_HAS1_const ____SGV_CHECK12(
#define ____SGV_HAS1_thread_local ____SGV_CHECK13(
#define ____SGV_HAS1_volatile ____SGV_CHECK14(
#define ____SGV_HAS1_static ____SGV_HAS1_static_IMPL(
#define ____SGV_HAS1_ ____SGV_HAS1_IMPL(
#define ____SGV_HAS1_IMPL(...) 0
#define ____SGV_HAS1_static_IMPL(...) 1

//检测限定符中是否有const
#define ____SGV_CHECK2(...) ____SGV_HAS2_##__VA_ARGS__)
#define ____SGV_CHECK21(...) ____SGV_HAS2_##__VA_ARGS__)
#define ____SGV_CHECK22(...) ____SGV_HAS2_##__VA_ARGS__)
#define ____SGV_CHECK23(...) ____SGV_HAS2_##__VA_ARGS__)
#define ____SGV_CHECK24(...) ____SGV_HAS2_##__VA_ARGS__)
#define ____SGV_HAS2_extern ____SGV_CHECK21(
#define ____SGV_HAS2_static ____SGV_CHECK22(
#define ____SGV_HAS2_thread_local ____SGV_CHECK23(
#define ____SGV_HAS2_volatile ____SGV_CHECK24(
#define ____SGV_HAS2_const ____SGV_HAS2_const_IMPL(
#define ____SGV_HAS2_ ____SGV_HAS2_IMPL(
#define ____SGV_HAS2_IMPL(...) 0
#define ____SGV_HAS2_const_IMPL(...) 1

//检测限定符中是否有volatile
#define ____SGV_CHECK3(...) ____SGV_HAS3_##__VA_ARGS__)
#define ____SGV_CHECK31(...) ____SGV_HAS3_##__VA_ARGS__)
#define ____SGV_CHECK32(...) ____SGV_HAS3_##__VA_ARGS__)
#define ____SGV_CHECK33(...) ____SGV_HAS3_##__VA_ARGS__)
#define ____SGV_CHECK34(...) ____SGV_HAS3_##__VA_ARGS__)
#define ____SGV_HAS3_extern ____SGV_CHECK31(
#define ____SGV_HAS3_static ____SGV_CHECK32(
#define ____SGV_HAS3_thread_local ____SGV_CHECK33(
#define ____SGV_HAS3_const ____SGV_CHECK34(
#define ____SGV_HAS3_volatile ____SGV_HAS3_volatile_IMPL(
#define ____SGV_HAS3_ ____SGV_HAS3_IMPL(
#define ____SGV_HAS3_IMPL(...) 0
#define ____SGV_HAS3_volatile_IMPL(...) 1

//检测限定符中是否有thread_local
#define ____SGV_CHECK4(...) ____SGV_HAS4_##__VA_ARGS__)
#define ____SGV_CHECK41(...) ____SGV_HAS4_##__VA_ARGS__)
#define ____SGV_CHECK42(...) ____SGV_HAS4_##__VA_ARGS__)
#define ____SGV_CHECK43(...) ____SGV_HAS4_##__VA_ARGS__)
#define ____SGV_CHECK44(...) ____SGV_HAS4_##__VA_ARGS__)
#define ____SGV_HAS4_extern ____SGV_CHECK41(
#define ____SGV_HAS4_static ____SGV_CHECK42(
#define ____SGV_HAS4_volatile ____SGV_CHECK43(
#define ____SGV_HAS4_const ____SGV_CHECK44(
#define ____SGV_HAS4_thread_local ____SGV_HAS4_thread_local_IMPL(
#define ____SGV_HAS4_ ____SGV_HAS4_IMPL(
#define ____SGV_HAS4_IMPL(...) 0
#define ____SGV_HAS4_thread_local_IMPL(...) 1

//匹配限定符：extern static const volatile thread_local
//展开结果：
//对于合法的限定符，为由0/1组成的5位数，第i位表示是否有上一行的第i个限定符
//对于非法的限定符，为“invalid”
#define ____SGV_SPECIFIERS(spec)\
	____SGV_IF(____SGV_IS_SPECIFIER(spec),\
		____SGV_SPECIFIERS_VALID,____SGV_SPECIFIERS_INVALID)(spec)
#define ____SGV_SPECIFIERS_INVALID(spec) invalid
#define ____SGV_SPECIFIERS_VALID(spec)\
	____SGV_CONCAT5(____SGV_CHECK0(spec),____SGV_CHECK1(spec),\
	____SGV_CHECK2(spec),____SGV_CHECK3(spec),____SGV_CHECK4(spec))

//当参数传递到这些宏时，已经过至少一次转发，已完全展开，所以无需使用延迟拼接
#if 1
//
#define ____SGV_GV00000_ARG2(type,name) ____SGV_GV00000_ARG3(type,name,{})
#define ____SGV_GV00000_ARG3(type,name,init)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name


//extern
#define ____SGV_GV10000_ARG2(type,name)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name();\
	extern const char *____SGV_variable_name_##name;\
	extern global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name
#define ____SGV_GV10000_ARG3(type,name,init)\
	[[deprecated("'" #name "' initialized and declared 'extern'")]]\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//static
#define ____SGV_GV01000_ARG2(type,name) ____SGV_GV01000_ARG3(type,name,{})
#define ____SGV_GV01000_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	static global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern static
#define ____SGV_GV11000_ARG2(type,name)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")
#define ____SGV_GV11000_ARG3(type,name,init)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")

//const
#define ____SGV_GV00100_ARG2(type,name)\
	static_assert(false,"uninitialized 'const " #name "'")
#define ____SGV_GV00100_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	const global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern const
#define ____SGV_GV10100_ARG2(type,name)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name();\
	extern const char *____SGV_variable_name_##name;\
	extern const global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name
#define ____SGV_GV10100_ARG3(type,name,init)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	extern const global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name;\
	const global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//static const
#define ____SGV_GV01100_ARG2(type,name)\
	static_assert(false,"uninitialized 'const " #name "'")
#define ____SGV_GV01100_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	const global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern static const
#define ____SGV_GV11100_ARG2(type,name)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")
#define ____SGV_GV11100_ARG3(type,name,init)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")

//volatile
#define ____SGV_GV00010_ARG2(type,name) ____SGV_GV00010_ARG3(type,name,{})
#define ____SGV_GV00010_ARG3(type,name,init)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	volatile global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern volatile
#define ____SGV_GV10010_ARG2(type,name)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name();\
	extern const char *____SGV_variable_name_##name;\
	extern volatile global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name
#define ____SGV_GV10010_ARG3(type,name,init)\
	[[deprecated("'" #name "' initialized and declared 'extern'")]]\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	volatile global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//static volatile
#define ____SGV_GV01010_ARG2(type,name) ____SGV_GV01010_ARG3(type,name,{})
#define ____SGV_GV01010_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	static volatile global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern static volatile
#define ____SGV_GV11010_ARG2(type,name)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")
#define ____SGV_GV11010_ARG3(type,name,init)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")

//const volatile
#define ____SGV_GV00110_ARG2(type,name)\
	static_assert(false,"uninitialized 'const " #name "'")
#define ____SGV_GV00110_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	const volatile global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern const volatile
#define ____SGV_GV10110_ARG2(type,name)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name();\
	extern const char *____SGV_variable_name_##name;\
	extern const volatile global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name
#define ____SGV_GV10110_ARG3(type,name,init)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	extern const volatile global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name;\
	const volatile global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//static const volatile
#define ____SGV_GV01110_ARG2(type,name)\
	static_assert(false,"uninitialized 'const " #name "'")
#define ____SGV_GV01110_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	const volatile global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern static const volatile
#define ____SGV_GV11110_ARG2(type,name)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")
#define ____SGV_GV11110_ARG3(type,name,init)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")

//thread_local
#define ____SGV_GV00001_ARG2(type,name) ____SGV_GV00001_ARG2(type,name,())
#define ____SGV_GV00001_ARG3(type,name,init)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern thread_local
#define ____SGV_GV10001_ARG2(type,name)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name();\
	extern const char *____SGV_variable_name_##name;\
	extern thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name
#define ____SGV_GV10001_ARG3(type,name,init)\
	[[deprecated("'" #name "' initialized and declared 'extern'")]]\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//static thread_local
#define ____SGV_GV01001_ARG2(type,name) ____SGV_GV01001_ARG3(type,name,{})
#define ____SGV_GV01001_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	static thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern static thread_local
#define ____SGV_GV11001_ARG2(type,name)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")
#define ____SGV_GV11001_ARG3(type,name,init)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")

//const thread_local
#define ____SGV_GV00101_ARG2(type,name)\
	static_assert(false,"uninitialized 'const " #name "'")
#define ____SGV_GV00101_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	const thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern const thread_local
#define ____SGV_GV10101_ARG2(type,name)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name();\
	extern const char *____SGV_variable_name_##name;\
	extern const thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name
#define ____SGV_GV10101_ARG3(type,name,init)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	extern const thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name;\
	const thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//static const thread_local
#define ____SGV_GV01101_ARG2(type,name)\
	static_assert(false,"uninitialized 'const " #name "'")
#define ____SGV_GV01101_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	const thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern static const thread_local
#define ____SGV_GV11101_ARG2(type,name)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")
#define ____SGV_GV11101_ARG3(type,name,init)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")

//volatile thread_local
#define ____SGV_GV00011_ARG2(type,name) ____SGV_GV00011_ARG3(type,name,{})
#define ____SGV_GV00011_ARG3(type,name,init)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	volatile thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern volatile thread_local
#define ____SGV_GV10011_ARG2(type,name)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name();\
	extern const char *____SGV_variable_name_##name;\
	extern volatile thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name
#define ____SGV_GV10011_ARG3(type,name,init)\
	[[deprecated("'" #name "' initialized and declared 'extern'")]]\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	volatile thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//static volatile thread_local
#define ____SGV_GV01011_ARG2(type,name) ____SGV_GV01011_ARG3(type,name,{})
#define ____SGV_GV01011_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	static volatile thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern static volatile thread_local
#define ____SGV_GV11011_ARG2(type,name)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")
#define ____SGV_GV11011_ARG3(type,name,init)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")

//const volatile thread_local
#define ____SGV_GV00111_ARG2(type,name)\
	static_assert(false,"uninitialized 'const " #name "'")
#define ____SGV_GV00111_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	const volatile thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name
	
//extern const volatile thread_local
#define ____SGV_GV10111_ARG2(type,name)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name();\
	extern const char *____SGV_variable_name_##name;\
	extern const volatile thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name
#define ____SGV_GV10111_ARG3(type,name,init)\
	____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	const char *____SGV_variable_name_##name=#name;\
	extern const volatile thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name;\
	const volatile thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//static const volatile thread_local
#define ____SGV_GV01111_ARG2(type,name)\
	static_assert(false,"uninitialized 'const " #name "'")
#define ____SGV_GV01111_ARG3(type,name,init)\
	static ____SGV_TRY_REMOVE_PARENS(type) ____SGV_helper_function_##name()\
		{return ____SGV_TRY_REMOVE_PARENS(init);}\
	static const char *____SGV_variable_name_##name=#name;\
	const volatile thread_local global_variable_t<____SGV_TRY_REMOVE_PARENS(type),\
		____SGV_helper_function_##name,&____SGV_variable_name_##name> name

//extern static const volatile thread_local
#define ____SGV_GV11111_ARG2(type,name)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")
#define ____SGV_GV11111_ARG3(type,name,init)\
	static_assert(false,"conflicting specifiers in declaration of '" #name "' (extern static)")
#endif// if 0/1

//4重载
#define ____SGV_global_variable(...) ____SGV_CONCAT(____SGV_GV_ARG,____SGV_GET_7(__VA_ARGS__,7,6,5,4,3,2,1))(__VA_ARGS__)
//global_variable(type,name)
#define ____SGV_GV_ARG2(type,name)\
	____SGV_CONCAT3(____SGV_GV,00000,_ARG2)(type,name)
//
#define ____SGV_GV_ARG3(x,y,z)\
	____SGV_IF(____SGV_IS_SPECIFIER(x),____SGV_GV_ARG30,____SGV_GV_ARG31)(x,y,z)
//global_variable(specifier,type,name)
#define ____SGV_GV_ARG30(specifier,type,name)\
	____SGV_CONCAT3(____SGV_GV,____SGV_SPECIFIERS_VALID(specifier),_ARG2)(type,name)
//global_variable(type,name,init)
#define ____SGV_GV_ARG31(type,name,init)\
	____SGV_CONCAT3(____SGV_GV,00000,_ARG3)(type,name,init)
//global_variable(specifier,type,name,init)
#define ____SGV_GV_ARG4(specifier,type,name,init)\
	____SGV_CONCAT3(____SGV_GV,____SGV_SPECIFIERS(specifier),_ARG3)(type,name,init)

//源代码兼容v1.0.0
#define ____SGV_extern_global_variable ____SGV_global_variable

#ifndef _LIB_SAFE_GLOBAL_VAR_NO_MACRO
#	//源代码兼容v1.0.0
#	define extern_global_variable ____SGV_extern_global_variable
#	define global_variable ____SGV_global_variable
#endif

#endif// !defined(____SGV_global_variable)
