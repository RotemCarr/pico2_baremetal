// Minimal implementations for startup
void *memcpy(void *dst, const void *src, unsigned long n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    for (unsigned long i = 0; i < n; i++)
        d[i] = s[i];
    return dst;
}

void *memset(void *dst, int val, unsigned long n) {
    unsigned char *d = dst;
    for (unsigned long i = 0; i < n; i++)
        d[i] = val;
    return dst;
}

// TODO! use ROM Hard-float functions instead.
float __aeabi_ui2f(unsigned int x) { return (float)x; }
float __aeabi_fdiv(float a, float b) { return a / b; }
float __aeabi_fmul(float a, float b) { return a * b; }
float __aeabi_fadd(float a, float b) { return a + b; }
float __aeabi_fsub(float a, float b) {return a - b;  }
unsigned int __aeabi_f2uiz(float a) { return (unsigned int)a; }