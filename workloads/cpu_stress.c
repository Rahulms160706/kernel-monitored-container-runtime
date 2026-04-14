#include <stdio.h>

int main() {
    while (1) {
        for (volatile int i = 0; i < 100000000; i++);
        printf("Working...\n");
    }
    return 0;
}
