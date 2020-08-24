#include <stdio.h>
int main()
{
    int x1 = 100, y1 = 200, x2 = 300, y2 = 100, x3 = 500, y3 = 500, temp, x = 1000,y=500;
    if (y1 >= y2)
    {
        temp = x1;
        x1 = x2;
        x2 = temp;
        temp = y1;
        y1 = y2;
        y2 = temp;
    }
    if (y1 >= y3)
    {
        temp = x1;
        x1 = x3;
        x3 = temp;
        temp = y1;
        y1 = y3;
        y3 = temp;
    }
    if (y2 >= y3)
    {
        temp = x2;
        x2 = x3;
        x3 = temp;
        temp = y2;
        y2 = y3;
        y3 = temp;
    }

    printf("%d", ((x1 - x3) * y - (x1 - x3) * y1 > (y1 - y3) * x - (y1 - y3) * x1));
    return 0;
}