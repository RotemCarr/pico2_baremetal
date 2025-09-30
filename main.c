    // blink.c
    #include <stdint.h>

    // ---------- Memory-mapped register bases ----------
    #define ATOMIC_XOR     0x1000
    #define ATOMIC_SET     0x2000
    #define ATOMIC_CLEAR   0x3000

    #define RESETS_BASE            0x40020000
    #define RESETS_RESET           (RESETS_BASE + 0x0)
    #define RESETS_RESET_DONE      (RESETS_BASE + 0x8)

    #define IO_BANK0_BASE          0x40028000
    #define IO_BANK0_GPIO25_CTRL   (IO_BANK0_BASE + 0x0cc)

    #define PADS_BANK0_BASE        0x40038000
    #define PADS_BANK0_GPIO25      (PADS_BANK0_BASE + 0x68)

    #define SIO_BASE               0xd0000000
    #define SIO_GPIO_OE_SET        (SIO_BASE + 0x038)
    #define SIO_GPIO_OUT_SET       (SIO_BASE + 0x018)
    #define SIO_GPIO_OUT_CLR       (SIO_BASE + 0x020)

    #define TIMER0_BASE            0x400b0000
    #define TIMER0_TIMEHR          (TIMER0_BASE + 0x08)
    #define TIMER0_TIMELR          (TIMER0_BASE + 0x0c)
    #define TIMER0_TIMERAWH        (TIMER0_BASE + 0x24)
    #define TIMER0_TIMERAWL        (TIMER0_BASE + 0x28)

    #define BIT(n) (1u << (n))

    // ---------- Helpers ----------
    static inline void reg_write(uint32_t addr, uint32_t val) {
        *(volatile uint32_t *)addr = val;
    }

    static void wait_ms(uint32_t ms) {
        uint32_t high_start = *(volatile uint32_t *)TIMER0_TIMEHR;
        uint32_t low_start  = *(volatile uint32_t *)TIMER0_TIMELR;
        uint64_t start = ((uint64_t)high_start << 32) | low_start;
        uint64_t target = start + ((uint64_t)ms * 1000ULL);

        while (1) {
            uint32_t high = *(volatile uint32_t *)TIMER0_TIMERAWH;
            uint32_t low  = *(volatile uint32_t *)TIMER0_TIMERAWL;
            uint64_t now = ((uint64_t)high << 32) | low;
            if (now >= target) break;
        }
    }

    // ---------- Application entry ----------
    void main(void) {
        // Reset IO_BANK0 + PADS
        reg_write(RESETS_RESET + ATOMIC_CLEAR, BIT(6) | BIT(9));
        while ((*(volatile uint32_t *)RESETS_RESET_DONE & (BIT(6) | BIT(9))) != (BIT(6) | BIT(9)));

        // Configure GPIO25 for SIO (funcsel = 5)
        reg_write(IO_BANK0_GPIO25_CTRL + ATOMIC_CLEAR, 0x1f);
        reg_write(IO_BANK0_GPIO25_CTRL + ATOMIC_SET, 0x05);

        // Enable GPIO25 output
        reg_write(SIO_GPIO_OE_SET, BIT(25));

        // Clear input disable + pull up/down
        reg_write(PADS_BANK0_GPIO25 + ATOMIC_CLEAR, BIT(7) | BIT(8));

        while (1) {
            reg_write(SIO_GPIO_OUT_SET, BIT(25)); // LED on
            wait_ms(100);

            reg_write(SIO_GPIO_OUT_CLR, BIT(25)); // LED off
            wait_ms(100);
        }
    }