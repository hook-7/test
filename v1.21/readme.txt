版本号为 1.00 通过日志打印 sw_version = 1.00 进行判断版本号

1.dw21的CCO  包括单相网关、面板、三相CCO网关 
都是烧录同一个CCO固件，根据不同的串口数据来判断是面板还是网关

2.3921的CCO  仅包括单相网关、面板
都是烧录同一个CCO固件，根据不同的串口数据来判断是面板还是网关

3.STA兼容亮度和色温，根据CCO发送的PLC报文进行判断

4.CCO挂掉情况的处理，当CCO挂掉后 STA任然可以进行组网并通讯(重启后变为单灯状态)

5.防窜网机制(单灯状态) 当STA从来没组网时 STA不进行广播(不会影响其他已经组网的STA)
如果本身有感应头 跟随感应头状态 如果没有感应头就保持亮灯状态

6.防跳灯机制 发送亮广播或者灭广播 不会带相应的数值，其他STA收到后执行亮度亮度或灭灯亮度 


版本号为 1.01   // 2022/12/12/19.55
上一版本发现STA发送亮/灭广播 同组的STA收不到数据 为解决该问题 做如下修改
STA收到感应头的数据进行广播时，MAC地址填充为00FFFFFFFFFF 上一版本为FFFFFFFFFFFF

版本号为 1.02  //  2022/12/17/23.00
新增设置感应头灵敏度命令  可通过AT指令设置/查询感应头灵敏度

版本号为 1.03 //   2022/12/20/23.00
查询设备信息的时候将 感应头灵敏度给带上去

版本号为 1.04 //   2022/12/28/20.20
上一版本上电会给感应头发送设置灵敏度命令 导致感应头死机
本版本调整 发送设置灵敏度时间 将上电发送改为上电之后15s之后再进行发送

版本号为 1.05 //   2023/1/05/21.46
修改默认参数 组号00 有人亮度100 无人亮度20 感应头时间15 感应头永关 色温100 灵敏度等级为2

版本号为 QRY(之前命名没1.06) //   2023/2/21/16.30
基于1.05的参数 当PWM0输出非0时GPIO1输出高  当PWM输出为0时将GPIO1输出低
用于控制驱动器的电源

版本号为 1.06 //   2023/3/03/9.46
修改默认参数 组号00 有人亮度90 无人亮度10 感应头时间15 感应头永关 色温100 灵敏度等级为2
兼容QRY固件的GPIO控制逻辑
未组网的STA不接收CCO的报文 但是会保持单灯状态跟随感应头  保留CCO挂掉时的应急处理
应急处理逻辑为  当CCO挂掉后 STA任然可以正常运行(感应头的广播任然有效)  当STA重启后变为单灯状态 如果修复CCO或者换一个好的CCO 此时可恢复正常逻辑

版本号为 1.07 //   2023/3/17/9.46
参数配置和1.06版本一样
新增设备别名 功率 区号 版本号等信息
修改查询白名单列表 新增查询白名单数量

版本号为 1.08  // 2023/3/30
新增渐亮灭配置
上报info为老报文格式，加上一个字节的渐亮灭  其他逻辑和上面的版本保持一致
修改默认参数 
品名FF 组号00 有人亮度100 无人亮度10 感应头时间15 感应头永开 色温100 灵敏度等级为FF 渐亮灭02 额定功率FF
FF代表需要CCO来对该数据进行设置以上数据都放在工厂区 不会随着升级而改变参数
别名和版本号 别名固定为STA,版本号跟随固件的版本变化
预留OTA升级功能










------------------------------------------------------------------------------------------------------------------------
版本号为 1.20 // 2023/3/30
新增渐亮灭配置
上报info为新报文格式，新增区号、别名、功率等参数的上报  其他逻辑和上面的版本保持一致
修改默认参数 
品名FF 组号00 有人亮度100 无人亮度10 感应头时间15 感应头永开 色温100 灵敏度等级为FF 渐亮灭02 额定功率FF
FF代表需要CCO来对该数据进行设置以上数据都放在工厂区 不会随着升级而改变参数
别名和版本号 别名固定为STA,版本号跟随固件的版本变化
预留OTA升级功能

版本号为 1.21 // 2023/4/15
在1.20版本基础上完善
新增设置分组时 STA会闪烁  间隔为500ms   闪烁完毕后切换到亮灯亮度  此命令支持面板控制 AT控制


