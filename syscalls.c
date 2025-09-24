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