#include <sc.h>

#define _XTAL_FREQ 16000000UL		// �����16M�������Ϊ16000000UL


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

bit						bRcvDone;				// һ֡�������
bit						bAckMask;				// ������ս���ɹ���������Ӧ�����ŵ�����֡
bit						bBoostEn;				// Ӧ������ʹ�ܱ�־
bit						bKeyPress;			// ��������ʱ���־
bit						bIrDecodeOk;		// ������ս�����ȷ
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

unsigned int	u16EMTCount;		// �������Ӧ������ʱ�������

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


#define _DEBUG			// ���Գ�����

/**********************************************************
�������ƣ�AD_Sample
�������ܣ�AD���
��ڲ�����adch - ���ͨ����adldo-��׼��ѹ
			adldo = 0�� VDD ��ΪADC �ο�
			adldo = 4�� �ڲ�LDO 2.4V ��ΪADC �ο�
			adldo = 5�� �ڲ�LDO 2.0V ��ΪADC �ο�
���ڲ������� 
��    ע������ͨ������������Ϊģ���
	      ����10��,ȡ�м�˴ε�ƽ��ֵΪ�����������adresult��
**********************************************************/
void ADC_Sample(unsigned char adch,	unsigned char adldo)
{
	volatile unsigned int ad_temp;
	
	if(LDOEN^(adldo!=0)) 		//���AD�ο�ΪVDD���ڲ�LDO��������Ҫ��ʱ100US���� 
	{
		ADCON1 = adldo;			//�����,ADֵȡ12λ
		__delay_us(100);		//��ʱ100us 
	}	
	else
		ADCON1 = adldo;			//�����Ҫѡ������ADת��ʱ�ӣ����԰�ADCON1��BIT3��Ϊ1����TADSEL = 1; 
		
	ADCON0 = 0X41 | (adch << 2);	//16��Ƶ�������ƵΪ16M����TADSEL=1�������ѡ16��Ƶ������
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	GODONE = 1;						//��ʼת��

	unsigned char u8Bits = 0;
	while(GODONE)
	{
		if(0 == (--u8Bits))
			return;
	}
	
	ad_temp = (ADRESH << 4) + (ADRESL >> 4);	//����12λADֵ
	
	if(0 == admax)
	{
		admax = ad_temp;
		admin = ad_temp;
	}
	else if(ad_temp > admax)
		admax = ad_temp;				//AD�������ֵ
	else if(ad_temp < admin)
		admin = ad_temp;				//AD������Сֵ
	
	adsum += ad_temp;
	if(++adtimes >= 10)
	{
		adsum -= admax;
		if(adsum >= admin)	adsum -= admin;
			else	adsum = 0;
		
		adresult = (unsigned int)(adsum >> 3);		//8��ƽ��ֵ��Ϊ���ս��
		
		adsum = 0;
		admin = 0;
		admax = 0;
		adtimes = 0;
	}
}


//ADC���β���
unsigned int ADC_Result(unsigned char adch)
{
	unsigned int ad_result ;
	unsigned char u8Bits = 0;
	
	ADCON1 = 0;										// ����룬��VDD��AD�ο� 
	__delay_us(20);								// ��ʱ20us
	ADCON0 = 0X41 | (adch << 2);	// 16��Ƶ
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	
	GODONE = 1;										// ��ʼת��

	while(GODONE)
	{
		if(0 == (--u8Bits))
			return 0;									// ת����ʱ
	}
	

	ad_result = (ADRESH << 4) + (ADRESL >> 4);	//����12λADֵ;
	return ad_result;
}


/***********************************************************
�������ƣ�DelayXms
�������ܣ����뼶�Ǿ�׼��ʱ
��ڲ�����x - ��ʱʱ��
���ڲ�����
��    ע��
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
��ע��	  ��ʱʱ����㷽��
          ʱ������Ϊϵͳָ��ʱ�ӣ���ΪFosc/4��
          ��ʱʱ��T = {1/[(Fosc/4)*Ԥ��Ƶ��*���Ƶ��]}*(PR2+1)
					
          ���������ʾ����
					
          T = {1/[(16/4)*0.25*1]}*250 = 125 us
					
-------------------------------------------*/ 

void main(void)
{
	asm("nop");
	asm("clrwdt");
	
	bWait = 1;
	
	
	INTCON = 0;							// ��ֹ�ж�
	OSCCON = 0x70;					// 8M
	OPTION_REG = 0B00000011;	// Timer0ʹ���ڲ�ʱ�ӣ�Ԥ��ƵΪ1:16;
	
	ANSEL = 0x00;
	ANSELH = 0x00;
	ANS1 = 1;								// RA1ѡ��ģ������
	ANS11 = 1;							// RB3ѡ��ģ������
	
	
	WPUA = 0B00000011;			// ����������1Ϊʹ������
	WPUB = 0B00000000; 
	
	TRISA = 0xFF;						// PORTA �����������
	TRISA0 = 0;
	
	//IOCA = 0;								// ��ֹPOARTA��IO�ڵ�ƽ�仯�ж�
	//IOCA0 = 1;							// ����RA0��IO��ƽ�仯�ж�
	
	PORTB = 0;
	TRISB = 0xFF;						// PORTB �����������
	//TRISB0 = 0;							// RB1 ��Ϊ���
	TRISB1 = 0;							// RB1 ��Ϊ���
	IOCB2 = 1;							// ��ƽ�仯�ж�
	
	PR2 = 194;							// �趨Timer��ʼֵ����ʱ������ 780us
	//TMR2IF = 0;
	//TMR2IE = 1;							// ʹ��Timer2����ж�
	
	T2CON = 0B00000011;			//����Timer2,����TMR2�ķ�Ƶ��Ϊ1:16
	

	
	DelayXms(100);
	
	//INTCON = 0XC0;				// �������ж�
	RBIE = 1;								// ����PORTB ��ƽ�仯�ж�
	PORTB;									// ��ȡPORTB�����棬��ƽ�仯�ж�ʹ�ܱ���ִ�д����
	RBIF = 0;
	
	TMR0 = 4;							// �趨Timer��ʼֵ����ʱʱ��Ϊ��256 - 4+2��* 4 * 16/16M = 1000 us
	T0IF = 0;
	T0IE = 1;								// ʹ��Timer0����ж�
	
	TMR2IF = 0;
	TMR2IE = 1;							// ʹ��Timer2����ж�
	//TMR2ON = 1;
	
	PEIE  = 1;
	GIE = 1;
	

	while(1)
	{
		AnalogSense();	// ��ѹ����
		
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

	if((u16VinValue >= 1800) && (bAcOk == 0))	// 4095 ��Ӧ VDD ��2000/4095��*VDD   ��1800/4095��*3.3=1.45V
	{
		bAcOk = 1;
	}
	else if((u16VinValue <= 1250) && (bAcOk == 1)) // (1300/4095) * VDD  ��1250/4095��*3.3=1V
	{
		bAcOk = 0;
	}
	
	if((u16VbatValue >= 3723) && (bVbatOV == 0))	// (1+ (20k/10k)) * 3.3 = 9.9V; 4095 * 9.0V/9.9V = 3723
	{
		bVbatOV = 1; //��ѹ
	}
	else if((u16VbatValue <= 3516) && (bVbatOV == 1)) 
	{
		bVbatOV = 0; //����
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
				if(u16KeyPressCount >= 13400)			// ~ 3000 ms Ӧ��ʱ��ס���԰���3���ر�Ӧ������
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
�������ƣ�ISR
�������ܣ��жϷ���
��ڲ�������
���ڲ�������
��ע��
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

		PORTB;				// ��ȡPORTB״̬����ƽ�仯�ж���Ҫ
		RBIF = 0;			// ���жϱ�־
	}
	
	
	if(TMR2IF)
	{
		//if(bAckMask == 0)
		{
			Frame.All |= ((!RB2) << (11 - u8Bits++));		// ����
			
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
	
	
	if(T0IF)	// 1 ms ��ʱ
	{
		if(bRcvStart == 1)
		{
			if(u8RcvBitCount++ >= 18)	// ��������£�18ms֮��Ӧ�ý��յ���11λ����
			{
				u8RcvBitCount = 0;
				
				if(u8Bits <= 10)
				{
					IOCB = 0;								// ��ֹPOARTB��IO�ڵ�ƽ�仯�ж�
					RBIE = 0;								// ��ֹPORTB ��ƽ�仯�ж�
					PORTB;									// ��ȡPORTB״̬����ƽ�仯�ж���Ҫ
					RBIF = 0;								// ���жϱ�־
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
				
				IOCB2 = 1;								// ����POARTA��IO�ڵ�ƽ�仯�ж�
				RBIE = 1;								// ����PORTA ��ƽ�仯�ж�
				PORTB;									// ��ȡPORTA״̬����ƽ�仯�ж���Ҫ
				RBIF = 0;								// ���жϱ�־
			}
		}
		
		
		// �������Ӧ������ʱ��
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
		
		
		// LED ��˸���� 
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
					if(u32EMTestCount >= 30000)			// 30,000 ms = 30s �¼�Ӧ��ʱ��
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
		
		
		
		
		TMR0 += 4;			// TMR0 �����Զ���ֵ������TIM0��ʱ��,TIMER�ǲ������ģ�һ��Ϊ�������ڣ����Լӣ�4 + 2��= 6 = (256 - 250)
		T0IF = 0;
	}
	
}
