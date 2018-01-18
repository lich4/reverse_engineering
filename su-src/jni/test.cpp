#include <stdio.h>
#include <sys/ptrace.h>
#include <stdlib.h>

int main(int argc ,char** argv)
{
	int pid = atoi(argv[1]);
	int ret = ptrace(PTRACE_ATTACH, pid, 0, 0);
	printf("pid=%d ret=%d\n", pid, ret);
	return 0;
}