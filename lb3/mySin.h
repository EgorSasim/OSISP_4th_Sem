#ifndef MYSIN_H
#define MYSIN_H


#include <math.h>

double myFact(double n)
{
	double res =1;
	while(n > 0)
	{
		res *= n;
		--n;
	}
	return res;
}

double myPow(double n, double val)
{
	double res = 1;

	while(val > 0)
	{
		res *= n;
		val--;
	}
	return res;
}

double myAbs(double val)
{
	return (val >= 0) ? val : -val;
}

double mySin(double x, double lim)
{
	double sum = 0; // result
	double k   = 0;
	double tmp = 0;
	do
	{
		tmp = myPow(-1, k) * ( ( myPow(x, 2*k + 1) ) / ( myFact(2*k + 1) ) );
		sum += tmp;
		k++;
	}while ( myAbs(tmp) > lim );

	return sum;
}

#endif // MYSIN_H

