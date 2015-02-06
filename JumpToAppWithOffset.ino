/*
  Jump to Application With Offset in Flash - based on Blink example
  Turns on an LED on for one second, off for one second, then on again
  for one more second before setting some peripherals back to their reset
  state and jumping to address 0x8080 in flash.

  Intended to be used with uTasker project to have a program capable of
  USB-MSD coexisting in the Teensy's flash with an Arduino program

  This example code is in the public domain.
 */

// Pin 13 has the LED on Teensy 3.1
// give it a name:
int led = 13;

// the assembly code must be run inside a C, not C++ function
#ifdef __cplusplus
extern "C" {
#endif
void jumpToApplicationAt0x8080() {
  /* Load stack pointer and program counter from start of new program */
  asm("movw r0, #0x8080");
  asm("ldr sp, [r0]");
  asm("ldr pc, [r0, #4]");
}
#ifdef __cplusplus
}
#endif

/*
 * These are the minimum peripherals that needed to be disabled to allow the
 * uTasker USB-MSD application to work.  You may need to reset more peripherals
 * depending on the application you are running, and what other peripherals
 * your sketch uses if you add more to this example than just blinking an LED
*/
void resetPeripherals() {
  /* set (some of) USB back to normal */
  NVIC_DISABLE_IRQ(IRQ_USBOTG);
  NVIC_CLEAR_PENDING(IRQ_USBOTG);
  SIM_SCGC4 &= ~(SIM_SCGC4_USBOTG);

  /* disable all GPIO interrupts */
  NVIC_DISABLE_IRQ(IRQ_PORTA);
  NVIC_DISABLE_IRQ(IRQ_PORTB);
  NVIC_DISABLE_IRQ(IRQ_PORTC);
  NVIC_DISABLE_IRQ(IRQ_PORTD);
  NVIC_DISABLE_IRQ(IRQ_PORTE);

  /* set (some of) ADC1 back to normal */
  // wait until calibration is complete
  while(ADC1_SC3 & ADC_SC3_CAL);

  // clear flag if calibration failed
  if(ADC1_SC3 & 1<<6)
    ADC1_SC3 |= 1<<6;

  // clear conversion complete flag (which could trigger ISR otherwise)
  if(ADC1_SC1A & 1<<7)
    ADC1_SC1A |= 1<<7;

  /* set some clocks back to default/reset settings */
  MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(4);
  SIM_CLKDIV1 = 0;
  SIM_CLKDIV2 = 0;
}

// the setup routine runs once when you press reset:
void setup() {
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second

  __disable_irq();
  resetPeripherals();
  jumpToApplicationAt0x8080();
}
