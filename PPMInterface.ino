/* 
 * PPM → USB Joystick (8 assi)
 * Arduino Pro Micro + Libreria Joystick
 */

#include <Joystick.h>

#define PPM_PIN 3
#define NUM_CHANNELS 8
#define PPM_FRAME_GAP 3000 // microsec

volatile uint16_t ppmRaw[NUM_CHANNELS];
volatile uint8_t ppmIndex = 0;
volatile uint32_t lastMicros = 0;

// ------------------ Interrupt lettura PPM ---------------------
void ppmISR() {
    uint32_t now = micros();
    uint32_t pulse = now - lastMicros;
    lastMicros = now;

    if (pulse > PPM_FRAME_GAP) {
        ppmIndex = 0;
    } else if (ppmIndex < NUM_CHANNELS) {
        ppmRaw[ppmIndex] = pulse;
        ppmIndex++;
    }
}

// ------------------ Joystick HID -----------------------------
Joystick_ Joystick(
    JOYSTICK_DEFAULT_REPORT_ID,
    JOYSTICK_TYPE_MULTI_AXIS,
    1,    // number of buttons
    0,    // hat switches
    true, // X axis
    true, // Y axis
    true, // Z axis
    true, // Rx
    true, // Ry
    true, // Rz
    true, // rudder
    true, // throttle
    false,// accelerator
    false,// brake
    false // steering
);

void setup() {
    attachInterrupt(digitalPinToInterrupt(PPM_PIN), ppmISR, RISING);

    for (int i = 0; i < NUM_CHANNELS; i++)
        ppmRaw[i] = 1500;

    // Inizializza joystick
    Joystick.begin();
    Joystick.setXAxisRange(0, 1023);
    Joystick.setYAxisRange(0, 1023);
    Joystick.setZAxisRange(0, 1023);
    Joystick.setRxAxisRange(0, 1023);
    Joystick.setRyAxisRange(0, 1023);
    Joystick.setRzAxisRange(0, 1023);
    Joystick.setRudderRange(0, 1023);
    Joystick.setThrottleRange(0, 1023);
}

// ------------------ Loop -----------------------------
void loop() {
    static uint32_t lastSend = 0;

    if (millis() - lastSend < 20) return; // 50 Hz
    lastSend = millis();

    uint16_t ch[NUM_CHANNELS];

    noInterrupts();
    for (uint8_t i = 0; i < NUM_CHANNELS; i++)
        ch[i] = ppmRaw[i];
    interrupts();

    // Converte PPM → 0–1023
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        ch[i] = constrain(ch[i], 1000, 2000);
        ch[i] = map(ch[i], 1000, 2000, 0, 1023);
    }

    // Le prime 8 uscite analogiche del joystick
    Joystick.setXAxis(ch[0]);
    Joystick.setYAxis(ch[1]);
    Joystick.setZAxis(ch[3]);
    Joystick.setRxAxis(ch[4]);
    Joystick.setRyAxis(ch[7]);
    Joystick.setRzAxis(ch[6]);
    //Joystick.setRudder(ch[5]);
    Joystick.setThrottle(ch[2]);

    // Canale 5 come pulsante on/off
    if (ch[5] >= 512) { // 512 corrisponde a 1500 µs mappati su 0-1023
        Joystick.pressButton(0); // pulsante 1 (indice 0)
    } else {
        Joystick.releaseButton(0);
    }

    Joystick.sendState();
}
