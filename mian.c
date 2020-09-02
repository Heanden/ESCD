#include "cpu_reg.h"
int count = 0;
void Delay_MS(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 10000; j++)
        {
            ;
        }
    }
}
void main()
{
    asm("  STM #0000h,CLKMD ");
    while (*CLKMD & 0x01)
        ;
    asm("  STM #10c7h,CLKMD ");
    *SWCR = 0x0001;
    asm("  stm   #7240h, SWWSR ");
    //2 wait except for on-chip program 1
    asm("  stm   #00A0h, PMST  "); //MP/MC = 0, IPTR = 001,ovly=0
    asm("  stm   #0802h, BSCR  "); //802
    asm("  STM #0h,IMR ");

    asm("  STM #0010h,TCR  ");  //关定时器
    asm("  STM #0186ah,PRD  "); //1ms
    asm("  STM #0C2fh,TCR  ");  //TCR=最后四位
    asm("  STM #0008h,IFR  ");
    asm("  ORM #0008h,*(IMR) "); /*开时间中断*/
    asm("  ORM #0100h,*(IMR) "); /*开INT3中断*/
    asm("  RSBX  INTM ");        /*开中断*/

    lcd_init();
    lcd_clear();
    lcd_regwrite(0x10, 0x3b);
    lcd_regwrite(0x00, 0xcd);

    while (1)
    {
        Delay_MS(10);
        gotoxy(0x00, 0x40);
        lcd_character(tab3, 40);
        Delay_MS(500);
        if (count >= 5000)
        {
            Delay_MS(10);
            lcd_clear();
            Delay_MS(1);
            gotoxy(0x00, 0x40);
            lcd_character(tab1, 80);
            Delay_MS(2000);
            lcd_clear();
            Delay_MS(1);
            count = 0;
        }
    }
}

interrupt void timer0()
{
    count++;
}
interrupt void int3()
{
    Delay_MS(10);
    lcd_clear();
    Delay_MS(1);
    gotoxy(0x00, 0x40);
    lcd_character(tab2, 40);
    Delay_MS(500);
    lcd_clear();
    Delay_MS(1);
}
