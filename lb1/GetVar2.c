#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ulong ProduceString(char *str)
{
    int len = strlen(str);
    ulong sum = 0;

    for (int i = 0; i < len; i++)
    {
        u_char symb = str[i];
        u_char symb2 = str[(1+i)];

        uint tmp = (symb<<8)+symb2;
        sum += tmp*tmp;
        i++;
        printf("%x\n", tmp); 
    }

    return sum;
}

uint ParseDate(char *str)
{
    char* tokens = strtok(str, ".");
    int flag = 0, days, months, years;

    while (tokens != NULL)
    {
        switch (flag)
        {
        case 0:
            days = atoi(tokens);
            flag = 1;
            break;
        case 1:
            months = atoi(tokens);
            flag = 2;
            break;
        case 2:
            years = atoi(tokens);
            flag = 3;
            break;
        }
        tokens = strtok(NULL, ".");
    }
    printf("\nDate: %d\n", (days+months*30+years*365));
    return days + months * 30 + years * 365;
}

int main(int args, char *argv[])
{
    if (args != 6)
    {
        printf("Not enough args!!!\n");
        return -1;
    }

    int K = atoi(argv[5]);
    int len = 0;

    ulong TotalSum = 0;
    for (int i = 1; i < 4; i++)
        TotalSum += ProduceString(argv[i]);
    printf("\nS1: %lu", TotalSum);

    TotalSum *= ParseDate(argv[4]);


    ulong cpS = TotalSum;
    do
    {
        len++;
        cpS = cpS / 10;
    } while (cpS > 0);


    int ArrayOfDigits[len];
    int len2 = len;
    len2--;

    printf("Sum: %lu\n", TotalSum);

    do
    {
        ArrayOfDigits[len2--] = TotalSum % 10;
        TotalSum /= 10;
    } while (TotalSum > 0);


    printf("\n");
        for(int i =0;i<len; i++){
        printf("%3d",i+1);
    }
	putchar('\n');
	for(int i = 0; i < len; i++){
		printf("%3d",ArrayOfDigits[i]);
	}
	putchar('\n');
    printf("\nN1: %d\n", ArrayOfDigits[len/2-1]);
    printf("N2: %d\n", ArrayOfDigits[len/2]);
    printf("N3: %d\n", ArrayOfDigits[len/2+1]);

    int S1 = ArrayOfDigits[len / 2 -1] * 100 + ArrayOfDigits[len / 2] * 10 + ArrayOfDigits[len / 2+1];
    printf("??????????: %d\n", S1);
    int N = S1 % K + 1;
    printf("??????????????: %d\n", N);

    return 0;
}
