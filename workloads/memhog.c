#include <stdlib.h>
#include <unistd.h>

int main() {
    size_t size = 200 * 1024 * 1024; // 200 MB
    char *p = malloc(size);

    if (!p) {
        write(1, "malloc failed\n", 14);
        while (1);
    }

    for (size_t i = 0; i < size; i++) {
        p[i] = 'A';
    }

    sleep(10);
    return 0;
}
