#define uchar unsigned char 
#include "STC15F2K60S2.H"
#include"sys.H"
#include"IR.h"
#include"DS1302.h"
#include"StepMotor.h"
#include"Beep.h"
#include"music.h"
#include"displayer.h"
#include"M24C02.h"
#include"EXT.h"

code unsigned long SysClock=11059200;    
/*ȫ�ֱ���
*/
code char decode_table[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x40}; 
code unsigned char song[]={0x21,0x10,0x21,0x10,0x25,0x10,0x25,0x10,0x26,0x10,0x26,0x10,0x25,0x10};
uchar code music1[] ={    //���ִ��룬����Ϊ��ͬһ�׸衷����ʽΪ: ����, ����, ����, ����,    
0x15,0x20,0x21,0x10,	 //������ʮλ�����ǵͰ˶ȣ��а˶Ȼ��Ǹ߰˶ȣ�1����Ͱ˶ȣ�2�����а˶ȣ�3����߰˶�
0x22,0x10,0x23,0x18,	 //��λ������׵�����������0x15����Ͱ˶ȵ�S0��0x21�����а˶ȵ�DO��
0x24,0x08,0x23,0x10,	 //�������Ǵ������������磺0x10����һ�ģ�0x20�������ģ�0x05����1/2��
0x21,0x10,0x22,0x20,
0x21,0x10,0x16,0x10,
0x21,0x40,0x15,0x20,
0x21,0x10,0x22,0x10,
0x23,0x10,0x23,0x08,
0x00,0x00};
uchar pd[]={1,2,3,4,5,6,7,8};
uchar table[10]={0x97,0xcf,0xe7,0x85,0xef,0xc7,0xa5,0xbd,0xb5,0xad};
uchar display=0;

char h1,h2,m1,m2,s1,s2;
struct_DS1302_RTC time;
struct_DS1302_RTC InitTime = {0,0,0,0,0,0,0};
uchar deadlock=0;


uchar IRarr[4];
uchar flag_wait_pd=0;
uchar pd_pos=0;
uchar pd_in[8];
uchar flag_pd_right=0;

uchar counter=0;

uchar timecounter=0;
uchar flag_pwm=0;

/*�ص�����
*/

void My1S_callback(){
	
	time = RTC_Read();
	
	h1 = time.hour>>4;		h2 = time.hour&0x0f;
	m1 = time.minute>>4;	m2 = time.minute&0x0f;
	s1 = time.second>>4;	s2 = time.second&0x0f;
    if(display==0){
        SetDisplayerArea(0,7);				       
	    Seg7Print(h1,h2,10,m1,m2,10,s1,s2);
    }
    else{
        if(pd_pos==0){
            SetDisplayerArea(0,1);
            Seg7Print(15,15,0,0,0,0,0,0);
        }
        SetDisplayerArea(0,pd_pos);
        Seg7Print(pd_in[0],pd_in[1],pd_in[2],pd_in[3],pd_in[4],pd_in[5],pd_in[6],pd_in[7]);
    }
    if(flag_pwm==1){
        SetPWM(0,30,0,0);
        flag_pwm=0;
    }



    //deadlock �߼�
    if(counter==3){
        counter=0;
        SetBeep(3500,100);
        deadlock=1;//����
    }
    if(deadlock){
        timecounter++;
    }
    if(timecounter==60){
        timecounter=0;
        deadlock=0;//����
        flag_wait_pd=1;//�ص���̬
    }
    if(timecounter==180){
        counter=0;
    }
}

void MyIR_CB(){
    unsigned char rxd=IRarr[3];
    SetBeep(1500,20);
    if(deadlock==0){
        if(flag_wait_pd==0){
            if(rxd==0x5d){
                //���������
                flag_wait_pd=1;
                display=1;//��ʱ��ʾ����
            }
        }
        else{
            if(rxd==0x5d){
                //�˳�����״̬
                pd_pos=0;
                flag_wait_pd=0;
                display=0;
                if(pd_pos==8)counter++;
            }
            //�������״̬
            if(pd_pos!=8){
                unsigned char temp=0;
                for(temp=0;temp<10;temp++){
                    if(table[temp]==rxd){
                        break;
                    }
                }
                if(temp!=10)
                pd_in[pd_pos++]=temp;
            }
            else{
                //��֤
                uchar flag_temp=1;
                uchar i;
                for(i=0;i<8;i++){
                    if(pd_in[i]!=pd[i]){flag_temp=0;break;}

                }
                flag_pd_right=flag_temp;
                if(flag_pd_right==0){
                    //����������
                    counter++;
					pd_pos=0;
                }else{
                    //���������ȷ
                    SetPWM(20,30,0,0);
                    flag_pwm=1;
                    SetMusic(100,0xFC,music1,sizeof(music1),enumMscNull);
                    display=0;//��ʾʱ��
                }
            }
        }
    }
}

/*
void Deadlock_CB(){
    if(counter==3){
        counter=0;
        SetBeep(3500,1000);
        deadlock=1;//����
    }
    if(deadlock){
        timecounter++;
    }
    if(timecounter==60){
        timecounter=0;
        deadlock=0;//����
        flag_wait_pd=1;//�ص���̬
    }
}
*/

int main(){
    /*��ʼ:��ʾ��ǰʱ��
    */
    DS1302Init(InitTime);
    BeepInit();
	DisplayerInit();
    MusicPlayerInit();
	SetPlayerMode(enumModePlay);
    IrInit(NEC_R05d);
    EXTInit(enumEXTPWM);

	SetDisplayerArea(0,7);
    SetIrRxd(IRarr,4);
    SetEventCallBack(enumEventSys1S, My1S_callback);
    // SetEventCallBack(enumEventSys1S, Deadlock_CB);
    SetEventCallBack(enumEventIrRxd,MyIR_CB);
    MySTC_Init();
    while(1){
        MySTC_OS();
    }
}

//�������û��ʲô�¼�,ֻ��ÿ��һ��ʱ����һ��,������ʹ���ж�