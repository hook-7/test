#include <sc.h>

#define _XTAL_FREQ 16000000UL		// 如果用16M晶振则改为16000000UL


#define	LED	RA0
#define ON	0
#define	OFF	1
// Variables declared
//typedef union
union frame_t
{
	struct
	{
		unsigned d0:1;
		unsigned d1:1;
		unsigned d2:1;
		unsigned d3:1;
		unsigned d4:1;
		unsigned d5:1;
		unsigned d6:1;
		unsigned c1:1;
		
		unsigned c2:1;
		unsigned st0:1;
		unsigned st1:1;
		unsigned st2:1;
		unsigned :1;
		unsigned :1;
		unsigned :1;
		unsigned :1;
	}Bit;
	
	unsigned int All;
};

union frame_t	Frame;

bit						bRcvDone;				// 一帧接收完成
bit						bAckMask;				// 红外接收解码成功后，屏蔽响应紧接着的数据帧
bit						bBoostEn;				// 应急开启使能标志
bit						bKeyPress;			// 按键按下时间标志
bit						bIrDecodeOk;		// 红外接收解码正确
bit						bAcOk;
bit						bLock;
bit						bWait;
bit						b1stGone;
bit						bRcvMask;
bit						bRcvStart;
bit						bGo;

bit						bVbatOV;
bit						bEMTGOing;
bit						bETDONE;
bit						bETMODE_2_GO;
bit						bETSTART;
bit						bEMTON;

bit						bEmergency;


unsigned int	u16KeyPressCount, u16KeyReleaseCount;

unsigned int	u16EMTCount;		// 红外接收应急测试时间计数器

unsigned char	u8Bits;
unsigned char	u8ETMode;
unsigned int 	u16VinValue, u16VbatValue;
unsigned long u32VinValue, u32VbatValue;
unsigned char u8SampleCount;
unsigned char u8DecodeOkHoldTimeCount, u8RcvMaskCount;
unsigned char	u8RcvBitCount;
unsigned int  u16RestartCount;
unsigned int  u16LedOvCount;
unsigned int  u16LedEmsCount;
unsigned long u32EMTestCount;

float			Vfly;

unsigned char	edge;


unsigned int u16IrData;

struct DATETIME_t
{
	struct 
	{
		unsigned int  timebase;
		unsigned char seconds;
		unsigned char minutes;		
		unsigned char hour;		
	}time;
	struct
	{
		unsigned int day;
		//unsigned char dayofweek;		
		unsigned char month;		
		unsigned char year;	
 	}date;
}; 
struct 	DATETIME_t 	rtcc_EMT,rtcc_48hr;

volatile unsigned int	adresult;
volatile unsigned long 	adsum;
volatile unsigned int 	admin, admax;
volatile unsigned char 	adtimes;

void ADC_Sample(unsigned char adch,unsigned char adldo);
void DelayXms(unsigned char x);

void AnalogSense(void);
void StatusCheck(void);
void StatusManager(void);


#define _DEBUG			// 调试程序用

/**********************************************************
函数名称：AD_Sample
函数功能：AD检测
入口参数：adch - 检测通道；adldo-基准电压
			adldo = 0， VDD 作为ADC 参考
			adldo = 4， 内部LDO 2.4V 作为ADC 参考
			adldo = 5， 内部LDO 2.0V 作为ADC 参考
出口参数：无 
备    注：采样通道需自行设置为模拟口
	      采样10次,取中间八次的平均值为采样结果存于adresult中
**********************************************************/
void ADC_Sample(unsigned char adch,	unsigned char adldo)
{
	volatile unsigned int ad_temp;
	
	if(LDOEN^(adldo!=0)) 		//如果AD参考为VDD和内部LDO互换，需要延时100US以上 
	{
		ADCON1 = adldo;			//左对齐,AD值取12位
		__delay_us(100);		//延时100us 
	}	
	else
		ADCON1 = adldo;			//如果需要选择更快的AD转换时钟，可以把ADCON1的BIT3设为1，即TADSEL = 1; 
		
	ADCON0 = 0X41 | (adch << 2);	//16分频，如果主频为16M，且TADSEL=1，则必须选16分频或以上
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	GODONE = 1;						//开始转换

	unsigned char u8Bits = 0;
	while(GODONE)
	{
		if(0 == (--u8Bits))
			return;
	}
	
	ad_temp = (ADRESH << 4) + (ADRESL >> 4);	//计算12位AD值
	
	if(0 == admax)
	{
		admax = ad_temp;
		admin = ad_temp;
	}
	else if(ad_temp > admax)
		admax = ad_temp;				//AD采样最大值
	else if(ad_temp < admin)
		admin = ad_temp;				//AD采样最小值
	
	adsum += ad_temp;
	if(++adtimes >= 10)
	{
		adsum -= admax;
		if(adsum >= admin)	adsum -= admin;
			else	adsum = 0;
		
		adresult = (unsigned int)(adsum >> 3);		//8次平均值作为最终结果
		
		adsum = 0;
		admin = 0;
		admax = 0;
		adtimes = 0;
	}
}


//ADC单次采样
unsigned int ADC_Result(unsigned char adch)
{
	unsigned int ad_result ;
	unsigned char u8Bits = 0;
	
	ADCON1 = 0;										// 左对齐，用VDD做AD参考 
	__delay_us(20);								// 延时20us
	ADCON0 = 0X41 | (adch << 2);	// 16分频
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	
	GODONE = 1;										// 开始转换

	while(GODONE)
	{
		if(0 == (--u8Bits))
			return 0;									// 转换超时
	}
	

	ad_result = (ADRESH << 4) + (ADRESL >> 4);	//计算12位AD值;
	return ad_result;
}


/***********************************************************
函数名称：DelayXms
函数功能：毫秒级非精准延时
入口参数：x - 延时时间
出口参数：
备    注：
***********************************************************/
void DelayXms(unsigned char x)
{
	unsigned char u8Bits,j;
	
	for(u8Bits = x;	u8Bits > 0;	u8Bits--)
	{
		for(j = 153;	j>0;	j--);
	}	
}






/*-------------------------------------------
备注：	  定时时间计算方法
          时钟输入为系统指令时钟（即为Fosc/4）
          定时时间T = {1/[(Fosc/4)*预分频比*后分频比]}*(PR2+1)
					
          本程序计算示例：
					
          T = {1/[(16/4)*0.25*1]}*250 = 125 us
					
-------------------------------------------*/ 

void main(void)
{
	asm("nop");
	asm("clrwdt");
	
	bWait = 1;
	
	
	INTCON = 0;							// 禁止中断
	OSCCON = 0x70;					// 8M
	OPTION_REG = 0B00000011;	// Timer0使用内部时钟，预分频为1:16;
	
	ANSEL = 0x00;
	ANSELH = 0x00;
	ANS1 = 1;								// RA1选择模拟输入
	ANS11 = 1;							// RB3选择模拟输入
	
	
	WPUA = 0B00000011;			// 配置上拉，1为使能上拉
	WPUB = 0B00000000; 
	
	TRISA = 0xFF;						// PORTA 口先作输出口
	TRISA0 = 0;
	
	//IOCA = 0;								// 禁止POARTA的IO口电平变化中断
	//IOCA0 = 1;							// 运行RA0的IO电平变化中断
	
	PORTB = 0;
	TRISB = 0xFF;						// PORTB 口先作输入口
	//TRISB0 = 0;							// RB1 作为输出
	TRISB1 = 0;							// RB1 作为输出
	IOCB2 = 1;							// 电平变化中断
	
	PR2 = 194;							// 设定Timer初始值，定时周期是 780us
	//TMR2IF = 0;
	//TMR2IE = 1;							// 使能Timer2溢出中断
	
	T2CON = 0B00000011;			//开启Timer2,设置TMR2的分频比为1:16
	

	
	DelayXms(100);
	
	//INTCON = 0XC0;				// 开启总中断
	RBIE = 1;								// 允许PORTB 电平变化中断
	PORTB;									// 读取PORTB并锁存，电平变化中断使能必须执行此语句
	RBIF = 0;
	
	TMR0 = 4;							// 设定Timer初始值，计时时间为（256 - 4+2）* 4 * 16/16M = 1000 us
	T0IF = 0;
	T0IE = 1;								// 使能Timer0溢出中断
	
	TMR2IF = 0;
	TMR2IE = 1;							// 使能Timer2溢出中断
	//TMR2ON = 1;
	
	PEIE  = 1;
	GIE = 1;
	

	while(1)
	{
		AnalogSense();	// 电压采样
		
		if((bAcOk == 1) && (bGo == 0))
		{
			bGo = 1;
			bEMTGOing = 1;
		}
		
		if(bGo == 1)
		{
			StatusCheck();
			StatusManager();
		}
	
		asm("clrwdt");
	}
}

void AnalogSense(void)
{
	u32VinValue += ADC_Result(11);
	u32VbatValue += ADC_Result(1);

	u8SampleCount++;
	if(u8SampleCount >= 64)
	{
		u8SampleCount = 0;
		
		u16VinValue = u32VinValue >> 6;
		u16VbatValue = u32VbatValue >> 6;
		u32VinValue = 0;
		u32VbatValue = 0;
	}

	if((u16VinValue >= 1800) && (bAcOk == 0))	// 4095 对应 VDD （2000/4095）*VDD   （1800/4095）*3.3=1.45V
	{
		bAcOk = 1;
	}
	else if((u16VinValue <= 1250) && (bAcOk == 1)) // (1300/4095) * VDD  （1250/4095）*3.3=1V
	{
		bAcOk = 0;
	}
	
	if((u16VbatValue >= 3723) && (bVbatOV == 0))	// (1+ (20k/10k)) * 3.3 = 9.9V; 4095 * 9.0V/9.9V = 3723
	{
		bVbatOV = 1; //过压
	}
	else if((u16VbatValue <= 3516) && (bVbatOV == 1)) 
	{
		bVbatOV = 0; //正常
	}
}

void StatusCheck(void)
{
	
	if(RB0 == 0)
	{
		u16KeyReleaseCount = 0;
		
		if(bKeyPress == 0)
		{
			u16KeyPressCount++;
			
			if(bEmergency == 1)
			{
				if(u16KeyPressCount >= 13400)			// ~ 3000 ms 应急时按住测试按键3秒后关闭应急功能
				{
					u16KeyPressCount = 0;
					bKeyPress = 1;
				}
			}
			else
			{
				if(u16KeyPressCount >= 250)			// ~ 33 ms
				{
					u16KeyPressCount = 0;
					bKeyPress = 1;
				}
			}
		}
	}
	else
	{
		u16KeyPressCount = 0;
		u16KeyReleaseCount++;
		
		if(u16KeyReleaseCount >= 250)		// 33 ms
		{
			u16KeyReleaseCount = 0;
			bKeyPress = 0;
		}
	}
	
}


void StatusManager(void)
{
	
	if(bAcOk == 1)
	{
		bLock = 0;
		bEmergency = 0;
		
		if((bKeyPress == 1) || (bIrDecodeOk == 1))
		{
			if(bIrDecodeOk == 1)
			{
				bAckMask = 1;
			}
			
			bBoostEn = 1;
		}
		else
		{
			if(bAckMask == 0)
			{
				bBoostEn = 0;
			}
		}
	}
	else
	{
		if(bLock == 0)
		{
			if((bKeyPress == 1) || (bIrDecodeOk == 1))
			{
				bBoostEn = 0;
				bLock = 1;
				bIrDecodeOk = 0;
			}
			else
			{
				bBoostEn = 1;
				bEmergency = 1;
			}
		}
	}
	

	if((bBoostEn == 1) || (u8ETMode != 0))
	{
		RB1 = 1;
	}
	else
	{
		RB1 = 0;
	}

}





/***********************************************
函数名称：ISR
函数功能：中断服务
入口参数：无
出口参数：无
备注：
************************************************/
void interrupt ISR(void)
{
	if(RBIF)
	{
		if(edge == 0)
		{
			TMR2IF = 0;
			TMR2ON = 1;
			bRcvStart = 1;
		}
	
		edge++;
		if(edge++ >= 2)
		{
			edge = 0;
		}

		PORTB;				// 读取PORTB状态，电平变化中断需要
		RBIF = 0;			// 清中断标志
	}
	
	
	if(TMR2IF)
	{
		//if(bAckMask == 0)
		{
			Frame.All |= ((!RB2) << (11 - u8Bits++));		// 解码
			
			if(u8Bits >= 12)
			{
				u8Bits = 0;
				
				if(Frame.All == 0x0D88)
				{
					bIrDecodeOk = 1;
					bAckMask = 1;
				}
				else
				{
					bIrDecodeOk = 0;
				}
			}
		}

		TMR2IF = 0;
		TMR2ON = 0;
	}
	
	
	if(T0IF)	// 1 ms 定时
	{
		if(bRcvStart == 1)
		{
			if(u8RcvBitCount++ >= 18)	// 正常情况下，18ms之后应该接收到第11位数据
			{
				u8RcvBitCount = 0;
				
				if(u8Bits <= 10)
				{
					IOCB = 0;								// 禁止POARTB的IO口电平变化中断
					RBIE = 0;								// 禁止PORTB 电平变化中断
					PORTB;									// 读取PORTB状态，电平变化中断需要
					RBIF = 0;								// 清中断标志
				}
			}
		}
		else
		{
			u8RcvBitCount = 0;
		}
		
		if((IOCB2 == 0) && (RBIE == 0))
		{
			if(u16RestartCount++ >= 50)
			{
				u16RestartCount = 0;
				
				edge = 0;
				u8Bits = 0;
				bRcvStart = 0;
				Frame.All = 0;
				bIrDecodeOk = 0;
				
				IOCB2 = 1;								// 允许POARTA的IO口电平变化中断
				RBIE = 1;								// 允许PORTA 电平变化中断
				PORTB;									// 读取PORTA状态，电平变化中断需要
				RBIF = 0;								// 清中断标志
			}
		}
		
		
		// 红外接收应急测试时间
		if(bAckMask == 1)
		{
			u16EMTCount++;
			if(u16EMTCount >= 5000)
			{
				u16EMTCount = 0;
				
				bBoostEn = 0;
				bAckMask = 0;
			}
		}
		else
		{
			u16EMTCount = 0;
		}
		
		if(bIrDecodeOk == 1)
		{
			u8DecodeOkHoldTimeCount++;
			if(u8DecodeOkHoldTimeCount >= 1)
			{
				u8DecodeOkHoldTimeCount = 0;
				bIrDecodeOk = 0;
			}
		}
		else
		{
			u8DecodeOkHoldTimeCount = 0;
		}
		
		
		if(bRcvMask == 1)
		{
			u8RcvMaskCount++;
			if(u8RcvMaskCount >= 100)
			{
				u8RcvMaskCount = 0;
				bRcvMask = 0;
			}
		}
		else
		{
			u8RcvMaskCount = 0;
		}
		
		
		// LED 闪烁控制 
		if(bVbatOV == 1)
		{
			u16LedEmsCount = 0;
			u16LedOvCount++;
			if(u16LedOvCount >= 50)
			{
				u16LedOvCount = 0;
				
				LED ^= 1;
			}
		}
		else if(bBoostEn == 1)
		{
			u16LedOvCount = 0;
			u16LedEmsCount = 0;
			LED = ON;
		}
		else if(bAcOk == 1) 
		{
			u16LedOvCount = 0;
			u16LedEmsCount++;
			if(u16LedEmsCount >= 2000)
			{
				u16LedEmsCount = 0;
				
				LED ^= 1;
			}
		}
		else
		{
			u16LedOvCount = 0;
			u16LedEmsCount = 0;
			LED = OFF;
		}
		
		
		
		if(bEMTGOing == 1)																	// 48 Hours counting
		{
			rtcc_48hr.time.timebase++;
			if(rtcc_48hr.time.timebase >= 1000)					// 1000, 1 s
			{
				rtcc_48hr.time.timebase = 0;
				rtcc_48hr.time.seconds++;
				if(rtcc_48hr.time.seconds >= 60)				// 60, 60 s
				{
					rtcc_48hr.time.seconds = 0;
					rtcc_48hr.time.minutes++;
					if(rtcc_48hr.time.minutes >= 60)			// 60, 60 Minutes
					{
						rtcc_48hr.time.minutes = 0;
						rtcc_48hr.time.hour++;
						if(rtcc_48hr.time.hour >= 48)			// 48, 48 Hours
						{
							rtcc_48hr.time.hour = 0;
							bEMTGOing = 0;
							bEMTON = 1;
						}
					}
				}
			}
		}

		//pms.EMTON = 1; //Debug
		if(bEMTON == 1)
		{
			rtcc_EMT.time.timebase++;
			if(rtcc_EMT.time.timebase >= 1000)						// 1000, 1s
			{
				rtcc_EMT.time.timebase = 0;

				rtcc_EMT.time.seconds++;
				if(rtcc_EMT.time.seconds >= 60)						// 60, 60s
				{
					rtcc_EMT.time.seconds = 0;
					rtcc_EMT.time.minutes++;
					if(rtcc_EMT.time.minutes >= 60)					// 60, 60 min
					{
						rtcc_EMT.time.minutes = 0;
						rtcc_EMT.time.hour++;
						if(rtcc_EMT.time.hour >= 24)					// 24, 1 day
						{
							rtcc_EMT.time.hour = 0;
							rtcc_EMT.date.day++;

							if((rtcc_EMT.date.day % 365) == 0)		// 365, 365 days
							{
								rtcc_EMT.date.day = 0;
								u8ETMode = 2;
								bETSTART = 1;
							}
							
							if((rtcc_EMT.date.day % 30) == 0)  		// 30, 30 days
							{
								if(u8ETMode != 2)
								{
									u8ETMode = 1;
									bETSTART = 1;
								}
							}	
						}
					}
				}
			}	
			
			if(bETSTART)
			{
				u32EMTestCount ++;
				
				if(u8ETMode == 1)
				{
					if(u32EMTestCount >= 30000)			// 30,000 ms = 30s 月检应急时间
					{
						u32EMTestCount = 0;
						u8ETMode = 0;
						bETSTART = 0;
						bETDONE = 1;
					}
				}
				/*
				else if(u8ETMode == 2)
				{
					bETMODE_2_GO = 1;
				}
				*/
			}
			else
			{
				u32EMTestCount = 0;
			}
		}
		
		
		
		
		TMR0 += 4;			// TMR0 不能自动赋值，操作TIM0的时候,TIMER是不计数的，一般为两个周期，所以加（4 + 2）= 6 = (256 - 250)
		T0IF = 0;
	}
	
}
