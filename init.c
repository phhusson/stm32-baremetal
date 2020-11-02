#include <stm32f4xx.h>

void init(void) {
	//Wait 7 cpu cycles for every flash access
	//(could be pessimist, but safe.)
	FLASH->ACR|=7;

	//Copy data segment from rom to ram
	extern char _sdata,_edata,_sldata;
	char *b=&_sdata;
	char *a=&_sldata;
	while( b < &_edata)
		*a++ = *b++;

	//Initialize bss in ram to 0
	extern char _sbss,_ebss;
	b=&_sbss;
	while( b < &_ebss)
		*b++ = 0;

	//HSEON&~HSEBYP
	RCC->CR = (RCC->CR | RCC_CR_HSEON) & ~RCC_CR_HSEBYP;
	//Wait for HSERDY
	while( !(RCC->CR&RCC_CR_HSERDY) );

	//cf p125 for clocks derivation
	//We want main clock @168MHz, and USG OTG FS @48MHz
	// USB OTG FS = 2/7 * Main Clock(168MHz) = 48MHz 

	// PLLP = 2, PLLQ = 7, fVCO = 168*2=336MHz
	// Clock source = HSE
	// HSE = 8MHz crystal; 336=24*42
	// PLLN = 42, PLLM = 1
	// PLLM must be >=2
	// PLLN = 84, PLLM=2,

	// real registers values:
	// PLLQ=7, PLLSRC=1, PLLP=0, PLLN=28, PLLM=2
	RCC->PLLCFGR =
		/* PLLQ */ 7 << 24 |
		/* PLLSRC */ 1 << 22 |
		/* PLLP */ 0 << 16 |
		/* PLLN */ 84 << 6 |
		/* PLLM */ 2 << 0;

	//PLLON
	RCC->CR = RCC->CR | RCC_CR_PLLON;
	//Wait for PLLRDY
	while( !(RCC->CR & RCC_CR_PLLRDY) );
	
	//Now let's use PLL's output as main clock
	//First setup dividors for AHB/APB1/APB2
	//Max AHB=168MHz, APB1=42MHz, APB2=84MHz
	//AHB divisor = 1, HPRE = 0
	//APB1 divisor = 4, PPRE1=b101=5
	//APB2 divisor = 2, PPRE2=b100=4
	RCC->CFGR = ( RCC->CFGR & ~RCC_CFGR_HPRE & ~RCC_CFGR_PPRE1 & ~RCC_CFGR_PPRE2 ) |
		/* HPRE */ 0 << 4 |
		/* PPRE1 */ 5 << 10 |
		/* PPRE2 */ 4 << 13;

	//Then switch to apropriate clock source
	//SW=2
	RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | 2 << 0;
	//Wait for SWS=2
	while( (RCC->CFGR & RCC_CFGR_SWS) != (2<<2) );

	extern uint32_t g_pfnVectors;
	SCB->VTOR = (uint32_t)&g_pfnVectors;

	//Set priority mode 4.4
	SCB->AIRCR = (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) | 3 << SCB_AIRCR_PRIGROUP_Pos;

    //Enable GPIO peripheral on AHB bus
    RCC->AHB1ENR |= 1 << 3;

    //Blinking Orange LED (GPIOD 13)
    //Set as GPIO Output
#define LED_GPIO_N 13
    GPIOD->MODER = (GPIOD->MODER & ~(3<<(2*LED_GPIO_N))) | 1 << (2*LED_GPIO_N);
    //Set as push-pull
    GPIOD->OTYPER &= ~(1<<LED_GPIO_N);
    //Set as 50MHz output
    GPIOD->OSPEEDR = (GPIOD->OSPEEDR & ~(3<<(2*LED_GPIO_N))) | 2 << (2*LED_GPIO_N);

    for(;;) {
        for(volatile int i=0; i<1000*1000; i++);
        //Set pin
        GPIOD->BSRRL = 1 << LED_GPIO_N;
        for(volatile int i=0; i<1000*1000; i++);
        //Reset pin
        GPIOD->BSRRH = 1 << LED_GPIO_N;
    }
}
