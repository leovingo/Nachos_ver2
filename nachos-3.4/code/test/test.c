#include "syscall.h"
#include "copyright.h"
#define maxlen 32
int main()
{
	int i;
	i = ReadInt();
	PrintInt(i);
	Halt();
}
