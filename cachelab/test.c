#include <stdio.h>

int main(int argc,char **argv) {
	printf("hello world!!!\n");
	printf("%d\n",argc);
	printf("%s\n%s\n%s\n",argv[0],argv[1],argv[2]);
	return 0;
}
