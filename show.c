#include"sys.H"
#include"DS1302.h"
#include"uart1.h"
code unsigned long SysClock=11059200;    

unsigned char i=0;
unsigned char arr[8];

struct_DS1302_RTC time;
struct_DS1302_RTC InitTime = {0,0,0,0,0,0,0};
void show(){
    for(i=0;i<8;i++){
        arr[i]=NVM_Read(i);
    }
    Uart1Print(arr,8);
}
int main(){
    Uart1Init(1200);
    DS1302Init(InitTime);

    SetEventCallBack(enumEventSys1S,show);
    MySTC_Init();
    while (1)
    {
        MySTC_OS();
    }
    
}