<!--Copyright 2021 (C) 张子辰
Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright 
notice and this notice are preserved. This file is offered as-is, 
without any warranty.-->

# global variable

当前版本：2.0.1（未完成）			上一版本：2.0.0

C++ 语言版本要求：C++ 14、C++ 17 或 C++ 20 （对于大多数编译器，指定语言标准为 C++ 11 也能通过编译）

## 简介
C++ 程序的一个全局变量（指具有命名空间作用域的对象）的构造（指调用构造函数）可能依赖于另一个全局变量，而全局变量构造的顺序取决于目标文件中变量定义的顺序，而后者取决于连接顺序。于是，不恰当的连接顺序可导致构造一个全局变量时，它所依赖的全局变量尚未构造。

本库提供的类模板、宏可协助程序设计者安全地处理可能在构造前使用的全局变量。

## 注意！
本库的正确性基于 4 个假设，如果它们中的任何一个不成立，本库都会出错：

1. 在构造前，全局变量占用的主存空间被`0x00`填充；
2. 不使用初始化列表的情况下，构造函数不会对类中的整型变量进行任何修改；
3. `64`位操作系统的合法地址（包括物理地址和虚拟地址）的最高位为`0`；
4. 非`thread_local`型的全局变量的构造在主线程中完成，`thread_local`型的全局变量的构造不会依赖其他线程的未构造的`thread_local`型的全局变量。

第 3 条假设保证了`global_variable_t`可以使用 63 位储存指针，1 位储存`bool`型变量，这样做使`sizeof(global_variable_t)=8`，否则`sizeof(global_variable_t)=16`。第 4 条假设让`global_variable_t`无需考虑线程安全问题。

## 宏
### global_variable

- `global_variable(specifier,type,name,init)`	[宏]
- `global_variable(specifier,type,name)`	[宏]
- `global_variable(type,name,init)`	[宏]
- `global_variable(name,init)`	[宏]

声明/定义全局变量。`global_variable`声明/定义的全局变量在逻辑上等效于`type specifier name=init`，但使用这些全局变量的方法类型于使用指针。`global_variable`尽力给出与 C++ 编译器给出的关于其等效形式的诊断相近的诊断。

- `specifier`可以是`const`、`volatile`、`thread_local`、`static`、`extern`及它们的组合。例如，`static int`的`specifier`是`static`，`const int*`不含`specifier`，`extern int *const`的`specifier`是`extern const`。*`specifier`可以留空。[v2.0.0 中弃用]*

- `type`是变量的类型，如`uint32_t`、`std::thread`、`(std::map<std::string,std::stack<void*>>)`，如果类型中有逗号，请使用括号“`()`”包裹类型。`type`应满足*可析构(Destructible)* 要求。

- `name`是变量的名称，应满足 C++ 语言对变量名的要求。

- `init`是初始化方法，是一个返回值是（或可自动转化为）`type`的表达式， 如果表达式中有逗号，请使用括号“`()`”包裹表达式。

***注：本文中的“方法”只有自然语言中的含义，需要使用其在面向对象程序设计中的含义时，一律用“成员函数”或“函数”替代。***

#### 诊断信息
`global_variable`通过 static_assert 和 deprecated 属性（这是要求 C++ 14 的原因），分别对用户的不良用法给出错误和警告等诊断信息。
1. **企图在`specifier`中同时包含`static`和`extern`** ， 错误：conflicting specifiers in declaration of '<变量名>' (extern static)；
2. **在定义常量时不进行初始化**，错误：uninitialized 'const  <变量名> '；
3. **在声明非 const 的外部变量时初始化**，警告：'<变量名>' initialized and declared 'extern'。

#### 已知缺陷
1. 如果`specifier`参数中，相同的限定符出现了多次，宏展开将失败，且不产生由本库设计的诊断信息。
2. 外部常量（`extern const`）在定义时必须使用`global_variable(extern const,type,name,init);`，而不能使用`global_variable(const,type,name,init);`。

#### 示例
```cpp
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
```
输出：
```
2 3.01 1099511627775
void function(15)
```
## 类
### global_variable_t
- `template <typename Tp,Tp (&Init)(),const char*const*const Name=nullptr> class global_variable_t`	[类模板] *[C++ 17 前]*
- `template <typename Tp,const auto &Init,const char*const*const Name=nullptr> class global_variable_t`	[类模板] *[C++ 17 起]*

包装一个可能在构造前使用的全局变量。`global_variable_t`保证，即使未构造也能正常使用，只要假设 1、2 成立。

- `Tp`: 包装的全局变量的类型，满足*可析构(Destructible)* 要求和*可移动构造
	(MoveConstructible)* 要求。
	
- `Init`: 初始化全局变量的方法，<ins>是函数指针</ins>[C++ 14]　<ins>满足*可调用 (Callable)* 要求</ins> [C++ 17 起]，更进一步，`Init`接受`0`个参数，返回值的类型是`Tp`。

- `Name`: 变量的名称，是一个`const char*`变量的地址。

- | 成员函数 ：|   |
	| ---- | ---- |
	| (构造函数) | `global_variable_t()`                                |
	|            | `global_variable_t(const global_variable_t&)=delete` |
	| (析构函数) | `~global_variable_t()`                               |
	|`operator*()`|`Tp& operator*()&`|
	|   |`const Tp& operator*()&const`|
	|`operator->()`|`Tp* opeartor->()`|
	|   |`const Tp* operator->()const`|
	
- |成员类型：|   |
	| ---- | ---- |
	| `type` | `typedef Tp type;` |

#### 异常
当全局变量的构造间接或直接依赖自身时，`global_variable_t`对象将抛出`circular_initialization`型异常，`what()`函数可查询初始化依赖环中的全局变量。
例如：

```cpp
global_variable(extern,int,a);
global_variable(int,b,*a);
global_variable(int,a,*b);
```
输出为：
```
terminate called after throwing an instance of 'circular_initialization'
what():  the circle is 'b' <-- 'a' <-- 'b'
```

## 保留字
所有以‘`____SGV_`’（`_`×4 + `SGV_`）开头的标识符

## 抑制宏污染
1. 阻止`global_variable.hpp`定义宏`global_variable`：在首次包含`global_variable.hpp`前定义宏~~`_LIB_SAFE_GLOBAL_VAR_NO_MACRO`~~  *[弃用]* 或`____SGV_NO_global_variable`
2. 更改宏global_variable的名字：在 1 的基础上，定义宏：
      -  `<global_variable的新名字> ____SGV_global_variable`

## 内部
**本节提到的特征并非本库的接口，它们很可能会在未来的版本中改变，且这些变更不一定会被文档提到。这些特征不一定带有描述，且带有的描述不一定容易理解。使用者应尽量避免使用它们。在每个特征后，会附上一个介于 0-2 的数字，表示这个特征在后续版本中被保持的可能性，数字越大，该特征越可能被保持，2 表示该特征几乎不可能改变，0 表示作者不试图保持该特征的稳定。本节并没有列出所有的内部特征。**

- `____SGV_CONCAT(x,y)`	[宏, **2**]	将`x`、`y`的展开拼接
- `____SGV_CONCAT3(x,y,z)` [宏, **0**]
- `____SGV_CONCAT5(x,y,z,w,v)` [宏, **0**]
- `____SGV_IF(STA,THEN,ELSE)` [宏, **2**]	如果`STA`展开后为`1`，则展开为`THEN`；如果`STA`展开为`0`，则展开为`ELSE`
- `____SGV_GET_2(...)` [宏, **1**]	获取变参数宏的第 2 个参数
- `____SGV_GET_7(...)` [宏, **0**]
- `____SGV_IS_PARENS(x)` [宏, **1**]	判断`x`是否为元组
- `____SGV_EMPTY_1(x)` [宏, **0**]	判断`x`是否为空，要求`x`不是元组
- `____SGV_REMOVE_PARENS(x)` [宏, **2**]	将元组`x`解包
- `____SGV_ECHO(x)` [宏, **1**]	回显参数
- `____SGV_IS_SPECIFIER(x)` [宏, **1**]	判断`x`是否为合法的限定符
- `____SGV_REMOVE_RIGHT_PAR(x)` [宏, **0**]	移除游离的右括号
- `____SGV_CHECK0(...)` [宏, **0**]	检查限定符中是否有`extern`
- `____SGV_CHECK1(...)` [宏, **0**]	检查限定符中是否有`static`
- `____SGV_CHECK2(...)` [宏, **0**]	检查限定符中是否有`const`
- `____SGV_CHECK3(...)` [宏, **0**]	检查限定符中是否有`volatile`
- `____SGV_CHECK4(...)` [宏, **0**]	检查限定符中是否有`thread_local`
- `____SGV_SPECIFIERS(x)` [宏, **0**]	匹配限定符，详细功能见代码注释
- `____SGV_SPECIFIERS_VALID(x)` [宏, **0**]
- `____SGV_GVbbbbb_ARG2(type,name)` (`b`为`0`/`1`) [宏, **0**] 共 32 个
- `____SGV_GVbbbbb_ARG3(type,name,init)` (`b`为`0`/`1`) [宏, **0**] 共 32 个
- `____SGV_global_variable(...)` [宏, **2**]
- `____SGV_GV_ARG2(type,name)` [宏, **1**]
- `____SGV_GV_ARG3(x,y,z)` [宏, **1**]
- `____SGV_GV_ARG30(specifier,type,name)` [宏, **1**]
- `____SGV_GV_ARG31(type,name,init)` [宏, **1**]
- `____SGV_GV_ARG4(specifier,type,name,init)` [宏, **1**]

## 版本检测

```cpp
#define ____SGV_VERS_MAJOR 2ull
#define ____SGV_VERS_MINOR 0ull
#define ____SGV_VERS_PATCHLEVEL 1ull
```

## 更新与兼容性

### v2.0.0 -> v2.0.1
1. 现在支持 MSVC 的“传统预处理器”；
2. 现在`type`可以是任何类型，如`void(*)()`；
3. 宏`_LIB_SAFE_GLOBAL_VAR_NO_MACRO`被弃用，现在，阻止定义宏`global_variable`应该在首次包含`global_variable.hpp`前定义`____SGV_NO_global_variable`，未来（不早于v3.0.0）定义宏`_LIB_SAFE_GLOBAL_VAR_NO_MACRO`将无任何效果。

### v1.1.0 -> v2.0.0
1. 修复了使用`extern`时，因使用不完整类型而出现编译错误的问题；
2. 移除了具名要求 *初始化器* ；
3. 改变了类模板`global_variable_t`的定义，现在它使用*可调用* 对象的左值引用初始化变量；
4. 宏`global_variable`包含4次重载，在`global_variable(specifier,type,name,init)`的基础上，可以去掉`specifier`或/和`init`参数；
5. 在声明外部变量时初始化将导致警告，而非错误；
6. 在`specifier`发生冲突时，尝试通过`static_assert`和`deprecated`属性给出诊断信息。

### v1.0.0 -> v1.1.0
1. 弃用了`extern_global_variable`宏，在`global_variable`，的`specifier`中加入`extern `即可声明外部变量；
2. 允许去掉`global_variable`的`init`参数而不初始化变量，注意：
   - 是 **去掉** 而非 **留空** ，
   - 不得初始化外部变量；
3. 修改文档的错误：`global_variable`的`specifier`中可以含有`static`。

本版本（2.0.1）与v2.0.0源代码兼容，且二进制兼容；与 v1.x 源代码兼容，但**不**二进制兼容。后续的版本可能会移除与v1.0.0的源代码兼容性，毕竟v1.0.0并未发布过。为了解决一个已知问题，下一个版本可能与本版本**不**二进制兼容。
