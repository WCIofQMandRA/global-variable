<!--Copyright 2021 (C) 张子辰
Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright 
notice and this notice are preserved. This file is offered as-is, 
without any warranty.-->

# global variable

当前版本：2.0.0 (未完成)		上一版本：1.1.0

C++ 语言版本要求：C++ 11、C++ 14、C++ 17 或 C++ 20

## 简介
C++ 程序的一个全局变量(指具有命名空间作用域的对象)的构造(指调用构造函数)可能依赖于另一个全局变量，而全局变量构造的顺序取决于目标文件中变量定义的顺序，而后者取决于连接顺序。于是，不恰当的连接顺序可导致构造一个全局变量时，它所依赖的全局变量尚未构造。

本库提供的类模板、宏可协助程序设计者安全地处理可能在构造前使用的全局变量。

## 注意！
本库的正确性基于 4 个假设，如果它们中的任何一个不成立，本库都会出错：

1. 在构造前，全局变量占用的主存空间被`0x00`填充；
2. 不使用初始化列表的情况下，构造函数不会对类中的整型变量进行任何修改；
3. `64`位操作系统的合法地址（包括物理地址和虚拟地址）的最高位为`0`；
4. 非`thread_local`型的全局变量的构造在主线程中完成，`thread_local`型的全局变量的构造不会依赖其他线程的未构造的`thread_local`型的全局变量。

第 3 条假设保证了`global_variable_t`可以使用 63 位储存指针，1 位储存`bool`型变量，这样做使`sizeof(global_variable_t)=8`，否则`sizeof(global_variable_t)=16`。第 4 条假设让`global_variable_t`无需考虑线程安全问题。

## 推荐使用方法
***本文中的“方法”只有自然语言中的含义，需要使用其在面向对象程序设计中的含义时，一律用“成员函数”或“函数”替代。***

- [ ] **声明/定义全局变量**：
  1. `global_variable(specifier,type,name,init)`[宏]
  2. `global_variable(specifier,type,name)`[宏]
  
- `specifier`可以是`const`、`volatile`、`thread_local`、`static`、`extern`及它们的组合，如果不需要可以留空。

- `type`是变量的类型，如`uint32_t`、`std::thread`、`(std::map<std::string,std::stack<void*>>)`，如果类型中有逗号，请使用括号“`()`”包裹类型。`type`应满足*可析构(Destructible)* 要求。

- `name`是变量的名称，应满足 C++ 语言对变量名的要求。

- `init`是初始化方法，是一个返回值是（或可自动转化为）`type`的表达式， 如果表达式中有逗号，请使用括号“`()`”包裹表达式。

## 类
### global_variable_t
- [ ] `template <typename Tp,Tp (&Init)(),const char*const* Name=nullptr> class global_variable_t`[类模板] *[C++ 17前]*
- [ ] `template <typename Tp,const auto &Init,const char*const* Name=nullptr> class global_variable_t`[类模板] *[C++ 17起]*

包装一个可能在构造前使用的全局变量。`global_variable_t`保证，即使未构造也能正常使用，只要假设 1、2 成立。

- `Tp`: 包装的全局对象的类型，满足*可析构(Destructible)* 要求和*可移动构造
	(MoveConstructible)* 要求。
	
- `Init`: 初始化全局变量的方法，满足*可调用 (Callable)* 要求，更进一步，`Init`接受`0`个参数，返回值的类型是`Tp`。

- `Name`: 变量的名称，是一个`char*`变量的地址。

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

## 异常
当全局变量的构造间接或直接依赖自身时，`global_variable_t`对象将抛出`circular_initialization`型异常，`what()`函数可查询初始化依赖环中的全局变量。
例如：

```cpp
extern_global_variable(,int,a);
global_variable(,int,b,*a);
global_variable(,int,a,*b);
```
输出为：
```
terminate called after throwing an instance of 'circular_initialization'
what():  the circle is `b' <-- `a' <-- `b'
```

注：全局变量的名称为模板的`Name`参数。

## 保留字
所有以‘`____SGV_`’开头的标识符

## 高级用法

1. 不定义宏`global_variable`：在首次包含`global_variable.hpp`前定义宏`_LIB_SAFE_GLOBAL_VAR_NO_MACRO`
2. 更改宏global_variable的名字：在 1 的基础上，定义宏：
      -  `<global_variable的新名字> ____SGV_global_variable`

## 版本检测

```cpp
#define ____SGV_VERS_MAJOR 2ull
#define ____SGV_VERS_MINOR 0ull
#define ____SGV_VERS_PATCHLEVEL 0ull
```

## 更新与兼容性

### v1.0.0 -> v1.1.0
1. 弃用了`extern_global_variable`宏，在`global_variable`，的`specifier`中加入`extern `即可声明外部变量；
2. 允许去掉`global_variable`的`init`参数而不初始化变量，注意：
   - 是 **去掉** 而非 **留空** ，
   - 不得初始化外部变量；
3. 修改文档的错误：`global_variable`的`specifier`中可以含有`static`。

### v1.1.0 -> v2.0.0
1. 修复了使用`extern`时出现编译错误的问题；
2. 移除了具名要求 *初始化器* ；
3. 改变了类模板`global_variable_t`的定义，现在它使用*可调用* 对象的左值引用初始化变量；
4. 在声明外部变量时初始化将导致警告，而非错误。

本版本（2.0.0）与 v1.x 源代码兼容，但**不**二进制兼容。