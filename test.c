#include "cpu_reg.h"
#define Uchar unsigned char

Uchar tab1[] = {
    "      Timer0已中断响应过5000次          "
    "      Timer0中断响应将重新计数          "};

Uchar tab2[] = {
    " 庄淑云201765 郑庆喜201581"};

Uchar tab3[] = {
    "             等待中断响应               "};
void delay(int k)
{
    while (k--)
        ;
}
void mDelay1uS()
{
    unsigned char i;
    for (i = 2; i != 0; i--)
        ;
}

/* 延时2微秒,请根据单片机速度 */
void mDelay2uS()
{
    unsigned char i;
    for (i = 4; i != 0; i--)
        ;
}
Uint16 AA;
void lcd_datawrite(Uint16 wrdata)
{
    //写资料到DDRAM
    //while((LCD_BUSY & 0x01) == 0x01);
    mDelay2uS();
    LCD_DAT = wrdata & 0xff;
    NP = wrdata;
    mDelay2uS();
}
void lcd_regwrite(Uint16 regname, Uint16 regdata)
{
    //mDelay1uS( );
    LCD_COM = regname;
    mDelay1uS();
    LCD_COM = regdata;
    mDelay1uS();
}

Uint16 lcd_regread(Uint16 regname)
{
    //读缓存器
    Uint16 temp;
    mDelay1uS();
    LCD_COM = regname;
    mDelay1uS();
    temp = LCD_COM & 0xff;
    mDelay1uS();
    return (temp);
}
