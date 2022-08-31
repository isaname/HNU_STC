#define uchar unsigned char 
#include "STC15F2K60S2.H"
#include"sys.H"
#include"IR.h"
#include"DS1302.h"
#include"StepMotor.h"
#include"Beep.h"
#include"music.h"
#include"displayer.h"
// #include"M24C02.h"
#include"EXT.h"
#include"uart1.h"

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
0x00,0x00};
uchar code table[10]={0x97,0xcf,0xe7,0x85,0xef,0xc7,0xa5,0xbd,0xb5,0xad};
uchar pd[]={1,2,3,4,5,6,7,8};//��ʼ������
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


uchar admin=0;
uchar admin_time=0;

uchar counter=0;

uchar timecounter=0;
uchar flag_pwm=0;

uchar ch_pos=0;
uchar flag_wait_ch=0;
uchar temp_pos=0;
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
    else if(display==1){
        if(pd_pos==0){
            SetDisplayerArea(0,1);
            Seg7Print(0x0a,0x0a,0,0,0,0,0,0);
        }else{
            SetDisplayerArea(0,pd_pos);
            Seg7Print(pd_in[0],pd_in[1],pd_in[2],pd_in[3],pd_in[4],pd_in[5],pd_in[6],pd_in[7]);
        }
    }else if(display==2){
        if(ch_pos==0){
            SetDisplayerArea(0,1);
            Seg7Print(0x0a,0x0a,0,0,0,0,0,0);
        }else{
            SetDisplayerArea(0,ch_pos);
            Seg7Print(pd_in[0],pd_in[1],pd_in[2],pd_in[3],pd_in[4],pd_in[5],pd_in[6],pd_in[7]);
        }
    }else{
        SetDisplayerArea(0,7);
        Seg7Print(10,10,10,10,10,10,10,10);//error
    }




    //deadlock �߼�
    if(counter==3){
        counter=0;
        SetBeep(3000,100);
        deadlock=1;//����
        display=3;
    }
    if(deadlock){
        timecounter++;
    }
    if(timecounter==60){
        timecounter=0;
        deadlock=0;//����
        flag_wait_pd=1;//�ص���̬
        display=0;
    }
    if(timecounter==180){
        counter=0;
    }


    //��֤�����߼�
    if(pd_pos==8){
        //��֤�����Ƿ���ȷ
        uchar flag_temp=1;
        uchar i;
        for(i=0;i<8;i++){
            if(pd_in[i]!=pd[i])flag_temp=0;
						pd_in[i]=0;
        }
        flag_pd_right=flag_temp;
        if(flag_pd_right==0){
            //����������
            counter++;
            pd_pos=0;
            SetBeep(2000,40);
            Uart1Print(&counter,1);
        }else{
            //���������ȷ
            SetPWM(50,30,0,0);

            display=0;//��ʾʱ��
            pd_pos=0;
            flag_wait_pd=0;//�ȴ�
            //����������,���������ʾ
            admin=1;//��־��Ȩλ
            admin_time=0;
        }
    }



    //�޸������߼�
    if(admin==1){
        admin_time++;
        if(admin_time==180){
            admin=0;
            admin_time=0;//ֻ��������ʱ���޸�����
            display=0;
            ch_pos=0;
            temp_pos=0;
        }
        if(admin_time==10){
            SetPWM(0,0,0,0);//ʮ���رյ��

        }
    }
    if(ch_pos==8){
        for(;temp_pos<10;temp_pos++){
        pd[temp_pos]=pd_in[temp_pos];
        NVM_Write(temp_pos,pd_in[temp_pos]);//������д��NVM
        pd_in[temp_pos]=0;
        }
        temp_pos=0;
        ch_pos=0;
        admin=0;
        display=0;//��ʾʱ��
        Uart1Print(pd,8);//��pd�е�ֵ����
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
        }
        if(admin==1){
            if(flag_wait_ch==0){
                if(rxd==0x1f){
                    flag_wait_ch=1;
                    display=2;//��ʱ��ʾ��������
                }
            }else{
                if(ch_pos!=8){
                    unsigned char temp=0;
                    for(temp=0;temp<10;temp++){
                        if(table[temp]==rxd){
                            break;
                        }
                    }
                    if(temp!=10)
                    pd_in[ch_pos++]=temp;
                }
            }
        }
    }
}


void PdInit(){
    for(temp_pos=0;temp_pos<8;temp_pos++){
        pd[temp_pos]=NVM_Read(temp_pos);
    }
    temp_pos=0;
    Uart1Print(pd,8);
}



int main(){
    /*��ʼ:��ʾ��ǰʱ��
    */
    DS1302Init(InitTime);
    Uart1Init(1200);
    BeepInit();
	DisplayerInit();
    MusicPlayerInit();
	SetPlayerMode(enumModePlay);
    IrInit(NEC_R05d);
    EXTInit(enumEXTPWM);
    PdInit();

    SetMusic(180,0xFA,music1,sizeof(music1),enumMscNull);

	SetDisplayerArea(0,7);
    SetIrRxd(IRarr,4);
    SetEventCallBack(enumEventSys1S, My1S_callback);
    SetEventCallBack(enumEventIrRxd,MyIR_CB);
    MySTC_Init();
    while(1){
        MySTC_OS();
    }
}

//�������û��ʲô�¼�,ֻ��ÿ��һ��ʱ����һ��,������ʹ���ж�
