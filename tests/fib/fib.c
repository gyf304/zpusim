int main() {
    int prev1 = 0;
    int prev2 = 1;
    while (1) {
        int new = prev1 + prev2;
        *((volatile int*)0x8000) = new;
        prev1 = prev2;
        prev2 = new;
    }
    return 0;
}
