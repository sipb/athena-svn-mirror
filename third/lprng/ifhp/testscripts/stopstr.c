#include <unistd.h>
#include <stdlib.h>
int main()
{
	write(1,"\031\001", 2);
	return(0);
}
