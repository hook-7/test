#include <stdio.h>
unsigned int 	u16VinValue, u16VbatValue;

#include <stdio.h>
#include <string.h>

// 加密密钥
const char KEY = 'QLM';

void encrypt(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        str[i] ^= KEY;
    }
}

int jm() {
    char str[] = "AT+CLEAR_INFO=FFFFFFFFFFFF,FF";
    printf("原始字符串: %s\n", str);
    encrypt(str);
    printf("加密后的字符串: %s\n", str);
    encrypt(str);
    printf("解密后的字符串: %s\n", str);
    return 0;
}


int bVbatOV =1;

void main(){
    // jm();
    int *a;
    a = 7000;
    int ADCON0 = 0X41 | (11 << 2);
    printf("%d\r\n",ADCON0);
    // a = 100;

    if(a < 100){
        printf("%d\r\n",a);
    }
	if((u16VbatValue >= 4095))	// (1+ (20k/10k)) * 3.3 = 9.9V (最大电压); 4095 * 9.0V/9.9V = 3723
	{
        printf("过压");
		bVbatOV = 1; //过压 急闪
	}
	else if((u16VbatValue <= 3392)) 
	{
        printf("充电中");
		bVbatOV = 0; //正常 2000ms间隔闪烁
	}
    else{
        printf("充满");
        bVbatOV = 2;//常亮
    }
}