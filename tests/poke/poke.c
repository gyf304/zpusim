int main() {
    int i = 0;
    while (1) {
        *((volatile int*)0x8000) = i;
        i++;
    }
    return 0;
}
