高性能引脚读写

内置的引脚读写函数digitalRead和digitalWrite需要每次将引脚转换成寄存器地址再进行读写，而且还需要检查PWM计时器设定，虽然增加了可靠性，减少了内存占用，但是性能较低，无法满足超高频读写的需求。本库仅在第一次调用时将引脚转换成内存地址，以后直接进行位运算操纵引脚，性能大幅提升，但无法处理PWM情况，适用于简单的高低电平操纵。

库中所有函数都和对应的内置函数大致同名。但是首字母大写，且所有参数为模板参数。由于每个函数实际上仅仅只有一条位运算赋值语句（条件判断分支在编译期就优化掉了），完全可以对每次调用直接内联，牺牲少许存储空间换取更高性能。

pinMode函数无法优化，因为其寄存器地址会在运行时发生变化，不是常量，必须每次临时计算。请仍使用内置pinMode。

More efficient digital IO than builtin functions.

Builtin functions (digitalRead & digitalWrite) translate pin code into memory address for each call, and will check PWM settings on the Pin. This is more robust but less efficient. My library calculate corresponding memory address only at the first call, store that address for following calls, and don't bother on PWM stuff. Each call is thus simplified into a single bit operation.

All functions have almost the same name as their builtin counterparts. However, the first letter is is capped, and all arguments are template arguments.

The pinMode function cannot be optimized because its register address will change during runtime; i.e., it is not a constant and must be calculated every time. Please still use the built-in pinMode.

# API参考
```C++
#include <LowLevelQuickDigitalIO>
using namespace LowLevelQuickDigitalIO;
/*
内置digitalRead函数的高性能替代
Efficient substitute of builtin digitalRead
*/
template <uint8_t PinCode>
bool DigitalRead();
/*
内置digitalWrite函数的高性能替代
Efficient substitute of builtin digitalWrite
*/
template <uint8_t PinCode, bool HighOrLow>
void DigitalWrite();
/*
这个函数无内置函数对应，是我附赠的一个小礼物，可以翻转当前电平，即HIGH变成LOW，LOW变成HIGH，而无需额外代码判断当前电平然后写入。
This is a gift function, not corresponding to any builtin function. It toggles the current state, i.e., HIGH to LOW, or LOW to HIGH, without extra code to determine the current state.
*/
template <uint8_t PinCode>
void DigitalToggle();
```