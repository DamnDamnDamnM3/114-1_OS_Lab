#include <stdio.h>
#include <unistd.h>

int main() {
    printf("[%d] execlp success\n", getpid());
    return 0;
}
