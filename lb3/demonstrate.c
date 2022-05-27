#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <gmp.h>
#define _USE_MATH_DEFINES

void calcTailor(long double*, int, int );
long long calcFactorial(int);


int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		printf("Not enough parameters\n");
		return -1;
	}
	int range = atoi(argv[1]);
	int intermid_members_amount = atoi(argv[2]);
	long double results[range];
	calcTailor(results, range, intermid_members_amount);

	//for(int i = 0; i < range; ++i)
	//{
	//	printf("Result of i = %d is:\t %.20Lf\n", i, results[i]);
	//}
	return 0;
}


long long calcFactorial(int num)
{
	int fact = 1;
	for(int i = 1; i <= num; ++i)
		fact *= i;
	return fact;
}

void calcTailor(long double* resStorage, int range, int interm_memb_amount)
{

	signed char	sign;
	int 		degree;
	long long 	factorial;
	long double 	intermidResult;
	long double	result;

	for(int i = 0; i < range; ++i)
	{
		sign	       = 1;
		degree	       = 1;
		factorial      = 1;
		intermidResult = 0;
		result 	       = 0;

		for(int j = 0; j  < interm_memb_amount; ++j)
		{
			intermidResult = sign *( powl( 2*M_PI*((long double)i/range), degree) / calcFactorial(factorial) );
			sign 	       = -sign;
			degree 	      += 2;
			factorial     += 2;
			result += intermidResult;
			printf("\t M_PI*(i/range): %Lf\n", M_PI*((long double)i/range));
			printf("\tIntermidRes: %Lf\n", intermidResult);
		}
		printf("%i_Result: %Lf\n", i, result);

		resStorage[i] = result;
	}

}
