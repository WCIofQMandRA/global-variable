# global variable
```
Copyright 2021 (C) 张子辰
Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright 
notice and this notice are preserved. This file is offered as-is, 
without any warranty.
```
## 简介
C++ 程序的一个全局变量

[^1]: 指具有命名空间作用域的对象

的构造

[^2]: 指调用构造函数

可能依赖于另一个全局变量，而全局变量构造的顺序取决于目标文件中变量定义的顺序，而后者取决于连接顺序。于是，不恰当的连接顺序可导致构造一个全局变量时，它所依赖的全局变量尚未构造。

这可能会导致一系列难以解释的错误，以下是一个生动的虚构的例子：
一个类乙需要使用“当前运行环境的库甲的版本”这个量，并且在构造时甲的版本是否达到了某个要求，如果达到了，则构造一个使用了甲的新特征的对象，如果没有，则构造一个不使用新特征的对象。实现的方式可能是指向实现的指针（参见https://zh.cppreference.com/w/cpp/language/pimpl ）。

类乙的设计者断言，在同一台机器上（至少在同一个进程中），甲的版本是统一的。在设计时，他令基于甲的不同版本的乙的对象之间无法相互转换，甚至没有在转换时检查两个对象是否依赖了甲的同一个版本。设计者在类乙的头文件声明了全局变量`extern const uint32_t library_A_version`，并在源文件中用甲提供的函数`uint32_t get_library_A_version()`获得甲的版本。

乙的使用者定义了一个乙类型的全局变量`var1`，而且在连接时，`var1`出现在了`library_A_version`的前面。当`var1`构造时，`library_A_version`尚未构造，由于其为全局变量，其值等于`0`；当另一个乙的局部变量`var2`构造时，`library_A_version`已构造，其值达到了要求。在试图将`var2`赋值给`var1`时，程序出现了运行时错误。

本库提供的类模板、宏可协助程序设计者安全地处理可能在构造前使用的全局变量。

## 警告
本库的正确性基于 4 个假设，如果它们中的任何一个不成立，本库都会出错：

1. 在构造前，全局变量占用的主存空间被`0x00`填充；
2. 不使用初始化列表的情况下，构造函数不会对类中的整型变量进行任何修改；
3. `64`位操作系统的合法地址（包括物理地址和虚拟地址）的最高位为`0`；
4. 非`thread_local`型的全局变量的构造在主线程中完成，`thread_local`型的全局变量的构造不会	依赖其他线程的未构造的`thread_local`型的全局变量。

第 3 条假设保证了`global_variable_t`可以使用 63 位储存指针，1 位储存`bool`型变量，这样做使`sizeof(global_variable_t)=8`，否则`sizeof(global_variable_t)=16`。第 4 条假设让`global_variable_t`无需考虑线程安全问题。

## 推荐使用方法
***本文中的”方法“只有自然语言中的含义，需要使用其在面向对象程序设计中的含义时，一律用”成员函数“或”函数“替代。***

- [ ] **声明外部变量**：`extern_global_variable(specifier,type,name)`[宏]
- [ ] **声明其他全局变量**：`global_variable(specifier,type,name,init)`[宏]

- `specifier`可以是`const`、`volatile`、`thread_local`及它们的组合，如果不需要可以留空。
-  `type`是变量的类型，如`uint32_t`、`std::thread`、`(std::map<std::string,std::stack<void*>>)`，如果类型中有逗号，请使用括号“`()`”包裹类型。`type`应满足*可析构(Destructible)*要求。
- `name`是变量的名称，应满足 C++ 语言对变量名的要求。
- `init`是初始化方法，是一个返回值是（或可自动转化为）`type`的表达式，如果不需要初始化（<!--对于这种情况，你不应该使用本库，而应直接定义全局变量-->），可以使用“`{}`”或“`type()`”，  如果表达式中有逗号，请使用括号“`()`”包裹表达式。

## 具名要求
### *初始化器(Initalizer) *
类型`Tp`满足*初始化器*要求的充要条件是：

1. `Tp`满足*可默认构造(DefaultConstructible)*要求；
2. `Tp`满足*可析构(Destructible)}要求*；
3. `Tp`包含公有成员函数`T operator()()`，`T`被称为`Tp`的**初始化类型**。

## global_variable_t
`template <typename Tp,typename Init> class global_variable_t`[类模板]
包装一个可能在构造前使用的全局变量。`global_variable_t`保证，即使未构造也能正常使用，只要假设 1、2 成立。

- `Tp`: 包装的全局对象的类型，满足*可析构(Destructible)*要求和*可移动构造
	(MoveConstructible)*要求。
	
- `Init`: 初始化全局变量的方法，满足*初始化器(Initalizer)*要求，且初始化类型是`Tp`。

- | 成员函数 ：|   |
	| ---- | ---- |
	| (构造函数) | `global_variable_t()`                                |
	|            | `global_variable_t(const global_variable_t&)=delete` |
	| (析构函数) | `~global_variable_t()`                               |
	|`operator*()`|`Tp& operator*()&|
	|   |`const Tp& operator*()&const`|
	|`operator->()`|`Tp* opeartor->()|
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

## 保留字
所有以‘`____SGV_`’开头的标识符

## 高级用法

1. 不定义`extern_global_variable`和`global_variable`两个宏：在首次包含`global_variable.hpp`前定义宏`_LIB_SAFE_GLOBAL_VAR_NO_MACRO`
2. 更改extern_global_variable和global_variable两个宏的名字：在 1 的基础上，定义宏：
      - ` <extern_global_variable的新名字> ____SGV_extern_global_variable`
      -  `<global_variable的新名字> ____SGV_global_variable`
