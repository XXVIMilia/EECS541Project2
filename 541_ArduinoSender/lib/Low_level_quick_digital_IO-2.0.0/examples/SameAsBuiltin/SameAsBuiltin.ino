#include <LowLevelQuickDigitalIO.h>
//本示例比较了使用内置函数和本库函数的相同功能。从肉眼上看不出区别，证明本库能够实现和内置函数相同的基本功能。但是在高频读写情况下，本库将更胜一筹。

//可以改为任何你连接了可用于报告引脚状态的设备的引脚号
constexpr uint8_t BuiltinPin = 2;
constexpr uint8_t EfficientPin = 3;
using namespace LowLevelQuickDigitalIO;
void setup()
{
	//在setup阶段，BuiltinPin和EfficientPin的行为完全相同
	pinMode(BuiltinPin, OUTPUT);
	PinMode<EfficientPin, OUTPUT>();
	digitalWrite(BuiltinPin, HIGH);
	DigitalWrite<EfficientPin, HIGH>();
	delay(1000);
	digitalWrite(BuiltinPin, LOW);
	DigitalWrite<EfficientPin, LOW>();
}
void loop()
{
	//在loop阶段，EfficientPin持续闪烁
	delay(1000);
	DigitalToggle<EfficientPin>();
}