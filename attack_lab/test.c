#include <stdio.h>
typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start, size_t len) {
    size_t i;
    for (i = 0; i < len; i++)
        printf("%p\t0x%.2x\n", &start[i], start[i]); 
    printf("\n");
}
void show_int(int x) {
    show_bytes((byte_pointer) &x, sizeof(int)); 
}


int main() {
    int v;
    scanf("%d",&v);
    show_int(v);
    return 0;
}