#include <stdio.h>

double atan();

main()
{
	double pi;

	pffinit();
	printf("I can print floating-point numbers! %f\n", 2.718281828);
	pi = 4.0 * atan(1.0);
	printf("I think pi = %f\n", pi);
}
