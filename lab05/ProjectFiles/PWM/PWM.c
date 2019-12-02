#include "PWM.h"

void initPWM(void){
	uint32_t ui32Gen;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0); // Enable the PWM0 peripheral
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); 
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)); // Wait for the PWM0 module to be ready
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 |GPIO_PIN_2); // Configure PIN for use by the PWM peripheral
	GPIOPinConfigure(GPIO_PF2_M0PWM2); // Configures the alternate function of a GPIO pin
	HWREG(PWM0_BASE + PWM_O_CC) = ((HWREG(PWM0_BASE + PWM_O_CC) & ~(PWM_CC_USEPWM | PWM_CC_PWMDIV_M)) | PWM_SYSCLK_DIV_64); // Set the PWM clock configuration into the PWM clock configuration register.	
	ui32Gen = PWM_GEN_BADDR(PWM0_BASE, PWM_GEN_1); // Compute the generator's base address.
	if((PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC) & PWM_X_CTL_MODE) //Set the individual PWM generator controls.
	{
			HWREG(ui32Gen + PWM_O_X_GENA) = (PWM_X_GENA_ACTCMPAU_ONE | PWM_X_GENA_ACTCMPAD_ZERO);
			HWREG(ui32Gen + PWM_O_X_GENB) = (PWM_X_GENB_ACTCMPBU_ONE | PWM_X_GENB_ACTCMPBD_ZERO);
	}
	else
	{
			HWREG(ui32Gen + PWM_O_X_GENA) = (PWM_X_GENA_ACTLOAD_ONE | PWM_X_GENA_ACTCMPAD_ZERO);
			HWREG(ui32Gen + PWM_O_X_GENB) = (PWM_X_GENB_ACTLOAD_ONE | PWM_X_GENB_ACTCMPBD_ZERO);
	}
	PWM_per_set(0xFFFF);  
	PWM_amplitude_set(0x7FFF);
	PWM_enable(false);
	PWMOutputInvert(PWM0_BASE, PWM_OUT_2_BIT, false);
	PWMGenEnable(PWM0_BASE, PWM_GEN_1);
}

void PWM_enable(bool estado){
	g_current_state = estado;
	if(estado == true)
	{
			HWREG(PWM0_BASE + PWM_O_ENABLE) |= PWM_OUT_2_BIT;
	}
	else
	{
			HWREG(PWM0_BASE + PWM_O_ENABLE) &= ~(PWM_OUT_2_BIT);
	}
}

void PWM_amplitude_set(uint16_t volume){
	uint16_t pulse_width;
	uint32_t period;
	uint32_t ui32Reg,ui32GenBase,ui32Gen;
	g_current_amp = volume;
	period = PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1);
	pulse_width = volume*period/0xFFFF;
	if(pulse_width >= period) 
		pulse_width--; 
	if(pulse_width <= period) 
		pulse_width++;
	if(pulse_width == 0) 
		pulse_width = 0;
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, pulse_width);
}

void PWM_per_set(uint16_t period){
	bool last_state = g_current_state;
	uint32_t ui32Gen;
	period *= 2;
	if(period < 3)
		period = 3;
	g_current_per = period;
	PWM_enable(false);
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, period);
	PWM_amplitude_set(g_current_amp);
	PWM_enable(last_state);
}
