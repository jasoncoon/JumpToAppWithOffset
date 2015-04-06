/*
  Jump to Application With Offset in Flash - based on Blink example
  Blinks an LED twice fast, then sets up a jump to an application stored at an
  offset of 0x8080 in Flash.

  Combine with the Blink example compiled to run at offset 0x8080 for a
  full example of running two applications from the Teensy 3.1's flash.

  The jump is accomplished by setting a flag that is persistent through reset,
  resetting the Teensy to get the peripherals back to a known state, then
  performing the jump before this application starts.  The check is done
  using startup_late_hook() so EEPROM can be used for storing the flag.  A
  few peripherals are started by the time startup_late_hook() is called, so
  those are reset before jumping to the offset in flash.

  The resetPeripherals() function resets the minimum peripherals that needed
  to be disabled to allow a few tested applications to work.  You may need to
  reset more peripherals depending on the application you jump to from here.

  For complete instructions please see README.md included with sketch or here:
  https://github.com/pixelmatix/JumpToAppWithOffset

  This example code is in the public domain.
 */

// Pin 13 has the LED on Teensy 3.1
// give it a name:
int led = 13;

// default flag is EEPROM Address 0x00 set to 0xAE
// another (untested) possibility is setting a byte in an uninitialized section of RAM (e.g. DMAMEM)
#define IS_JUMP_TO_OFFSET_FLAG_SET()  (eeprom_read_byte(0x00) == 0xAE)
#define CLEAR_JUMP_FLAG()             eeprom_write_byte(0x00, 0)
#define SET_JUMP_FLAG()               eeprom_write_byte(0x00, 0xAE)

// the assembly code hook must be run inside a C, not C++ function
#ifdef __cplusplus
extern "C" {
#endif
// TODO: make a generic jump function that takes the address as a variable
void jumpToApplicationAt0x8080() {
  /* Load stack pointer and program counter from start of new program */
  asm("movw r0, #0x8080");
  asm("ldr sp, [r0]");
  asm("ldr pc, [r0, #4]");
}

// if jumping to an address at 0x10000 or higher, need to load 32-bits into r0
void jumpToApplicationAt0x38080() {
  /* Load stack pointer and program counter from start of new program */
  asm("movw r0, #0x8080");
  asm("movt r0, #0x0003");
  asm("ldr sp, [r0]");
  asm("ldr pc, [r0, #4]");
}

// TODO: set more peripherals back to reset conditions
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

void startup_late_hook(void) {
  // look for the condition that indicates we want to jump to the application with offset
  if(IS_JUMP_TO_OFFSET_FLAG_SET()) {

    // clear the condition
    CLEAR_JUMP_FLAG();

    // set peripherals (mostly) back to normal then jump
    __disable_irq();
    resetPeripherals();
    jumpToApplicationAt0x8080();
  }
}
#ifdef __cplusplus
}
#endif

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART() (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

void restartAndJumpToApp(void) {
  SET_JUMP_FLAG();
  CPU_RESTART();
}

// the setup routine runs once when you press reset:
void setup() {
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
}

// the loop blinks the LED twice then prepares to start the application with offset:
void loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                // wait for a half second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                // wait for a half second
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                // wait for a half second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                // wait for a half second

  restartAndJumpToApp();
}
