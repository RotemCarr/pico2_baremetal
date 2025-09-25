#include <stdint.h>

// --------------------------
// Memory-mapped register bases
// --------------------------
#define ATOMIC_SET     0x2000
#define ATOMIC_CLEAR   0x3000

#define RESETS_BASE      0x40020000
#define RESETS_RESET     (RESETS_BASE + 0x0)
#define RESETS_RESET_DONE (RESETS_BASE + 0x8)

#define IO_BANK0_BASE      0x40028000
#define PADS_BANK0_BASE    0x40038000

#define SIO_BASE           0xd0000000

#define TIMER0_BASE        0x400b0000
#define TIMER0_TIMELR      (TIMER0_BASE + 0x0c)
#define TIMER0_TIMEHR      (TIMER0_BASE + 0x08)
#define TIMER0_TIMERAWL    (TIMER0_BASE + 0x28)
#define TIMER0_TIMERAWH    (TIMER0_BASE + 0x24)

#define TICKS_BASE         0x40108000
#define TICKS_TIMER0_CTRL  (TICKS_BASE + 0x18)
#define TICKS_TIMER0_CYCLES (TICKS_BASE + 0x1c)

#define PLL_SYS_BASE       0x40050000
#define PLL_SYS_CS         (PLL_SYS_BASE + 0x00)
#define PLL_SYS_PWR        (PLL_SYS_BASE + 0x04)
#define PLL_SYS_FBDIV_INT  (PLL_SYS_BASE + 0x08)
#define PLL_SYS_PRIM       (PLL_SYS_BASE + 0x0c)

#define CLOCKS_BASE               0x40010000
#define CLOCKS_CLK_REF_CTRL       (CLOCKS_BASE + 0x30)
#define CLOCKS_CLK_REF_SELECTED   (CLOCKS_BASE + 0x38)
#define CLOCKS_CLK_SYS_CTRL       (CLOCKS_BASE + 0x3c)
#define CLOCKS_CLK_SYS_SELECTED   (CLOCKS_BASE + 0x44)

#define XOSC_BASE          0x40048000
#define XOSC_CTRL          (XOSC_BASE + 0x00)
#define XOSC_STATUS        (XOSC_BASE + 0x04)
#define XOSC_STARTUP       (XOSC_BASE + 0x0c)

#define BIT(n) (1u << (n))

// --------------------------
// Helpers
// --------------------------
static inline void reg_write(uint32_t addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
}

static inline uint32_t reg_read(uint32_t addr) {
    return *(volatile uint32_t *)addr;
}

// --------------------------
// Timer delay
// --------------------------
static void wait_ms(uint32_t ms) {
    uint32_t low_start = reg_read(TIMER0_TIMELR);
    uint32_t high_start = reg_read(TIMER0_TIMEHR);
    uint64_t start = ((uint64_t)high_start << 32) | low_start;
    uint64_t target = start + ((uint64_t)ms * 1000ULL);

    while (1) {
        uint32_t low  = reg_read(TIMER0_TIMERAWL);
        uint32_t high = reg_read(TIMER0_TIMERAWH);
        uint64_t now = ((uint64_t)high << 32) | low;
        if (now >= target) break;
    }
}

// --------------------------
// System init
// --------------------------
static void reset_peripherals(void) {
    reg_write(RESETS_RESET + ATOMIC_CLEAR, BIT(6)|BIT(9));
    while ((reg_read(RESETS_RESET_DONE) & (BIT(6)|BIT(9))) != (BIT(6)|BIT(9)));
}

static void start_timers(void) {
    // Reset TIMER0 counter
    reg_write(TIMER0_TIMERAWL, 0);
    reg_write(TIMER0_TIMERAWH, 0);

    // Set timer0 cycles small offset
    reg_write(TICKS_TIMER0_CYCLES, 12);

    // Enable timer0
    reg_write(TICKS_TIMER0_CTRL + ATOMIC_SET, 1);
}

// Placeholder functions to mimic prelude.s initialization
static void init_xosc(void) {
    reg_write(XOSC_STARTUP, (reg_read(XOSC_STARTUP) & ~0x3fff) | 469);
    reg_write(XOSC_CTRL, (reg_read(XOSC_CTRL) & ~0x00ffffff) | 0xfabaa0);
    while (!(reg_read(XOSC_STATUS) & (1 << 31))) {}
}

static void init_pll(void) {
    reg_write(RESETS_RESET + ATOMIC_CLEAR, BIT(14));
    while ((reg_read(RESETS_RESET_DONE) & BIT(14)) != BIT(14)) {}
    
    reg_write(PLL_SYS_FBDIV_INT, (reg_read(PLL_SYS_FBDIV_INT) & ~0xfff) | 125);
    reg_write(PLL_SYS_PWR + ATOMIC_CLEAR, BIT(5)|BIT(0));
    while (!(reg_read(PLL_SYS_CS) & (1 << 31))) {}
    reg_write(PLL_SYS_PRIM, (reg_read(PLL_SYS_PRIM) & ~0x77000) | ((5<<16)|(2<<12)));
    reg_write(PLL_SYS_PWR + ATOMIC_CLEAR, BIT(3));
}

static void configure_clk_ref(void) {
    // Select XOSC as the reference source
    reg_write(CLOCKS_CLK_REF_CTRL, (reg_read(CLOCKS_CLK_REF_CTRL) & ~0x3) | 0x2);

    // Wait until XOSC is selected
    while ((reg_read(CLOCKS_CLK_REF_SELECTED) & 0x0f) != 0x04) {}
}

static void configure_clk_sys(void) {
    // Select AUX (PLL_SYS) as the system clock source
    reg_write(CLOCKS_CLK_SYS_CTRL + ATOMIC_SET, 1);

    // Wait until AUX is selected
    while ((reg_read(CLOCKS_CLK_SYS_SELECTED) & 0x03) != 0x02) {}
}

// --------------------------
// Forward declaration
// --------------------------
void main(void);

// --------------------------
// Reset handler
// --------------------------
void Reset_Handler(void) {
    reset_peripherals();
    init_xosc();
    init_pll();
    configure_clk_ref();
    configure_clk_sys();
    start_timers();
    main();
    while(1);
}

// --------------------------
// Default interrupt handler
// --------------------------
void Default_Handler(void) {
    while(1);
}

// --------------------------
// Vector table
// --------------------------
__attribute__((section(".vectors")))
void (* const vector_table[])(void) = {
    (void(*)(void))0x20080000, // stack pointer
    Reset_Handler,
    Default_Handler, // NMI
    Default_Handler, // HardFault
};