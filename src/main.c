#include "stm32l053xx.h"	// Pulled from STM's HAL for the STM32L0-series

void main();
void setup_gpio();
void setup_tim6();

void main() {
	setup_gpio();
	setup_tim6();

	for (;;) {

	}
}

void setup_gpio() {
	RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

	// Setup PB9 as output.
	GPIOB->MODER &= ~(GPIO_MODER_MODE9_1);
}

void setup_tim6() {
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
	TIM6->PSC = 2097 - 1;
	TIM6->ARR = 300 - 1;
	TIM6->DIER |= TIM_DIER_UIE;
	TIM6->CR1 |= TIM_CR1_CEN;
	NVIC->ISER[0] |= (1 << TIM6_DAC_IRQn);
}

void TIM6_DAC_IRQHandler() {
	TIM6->SR &= ~TIM_SR_UIF;
	GPIOB->ODR = (GPIOB->ODR) ^ (GPIO_ODR_OD9);
}
