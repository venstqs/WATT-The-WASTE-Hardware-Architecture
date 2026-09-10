/**
 * ============================================================================
 * Estero-Volt: Off-Grid BMFC IoT Telemetry Firmware
 * Target Controller: ESP32-WROOM-32E
 * Framework: Arduino-ESP32 / ESP-IDF
 * 
 * Description:
 * Ultra-low power firmware operating under a strict energy-harvested budget.
 * Orchestrates cold wakeups, high-side sensor power switching, ultrasonic acoustic
 * ranging, LoRa packet transmission, dynamic duty cycling, and deep sleep entry.
 * ============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "config.h"

// ============================================================================
// PERSISTENT RTC MEMORY STORAGE (Survives Deep Sleep)
// ============================================================================
RTC_DATA_ATTR uint32_t bootCount = 0;
RTC_DATA_ATTR uint16_t lastDistanceMm = 0;
RTC_DATA_ATTR uint16_t lastVbatMv = 0;
RTC_DATA_ATTR uint8_t  consecutiveAlerts = 0;

// ============================================================================
// TELEMETRY PAYLOAD STRUCTURE (Compact 10-byte Binary Format)
// ============================================================================
#pragma pack(push, 1)
struct TelemetryPayload {
    uint8_t  nodeId;            // 1 Byte: Unique node address
    uint32_t messageSequence;   // 4 Bytes: Rolling transmission counter
    uint16_t distanceMm;        // 2 Bytes: Measured water level distance (mm)
    uint16_t vbatMv;            // 2 Bytes: Supercapacitor voltage (mV)
    uint8_t  socPercent;        // 1 Byte: Supercapacitor State-of-Charge (0 - 100%)
    uint8_t  statusFlags;       // 1 Byte: Bitmask [0: Alert, 1: Low-E, 2: SensorErr, 3: RFSuccess]
};
#pragma pack(pop)

// ============================================================================
// LOW-POWER HARDWARE CONFIGURATION ROUTINES
// ============================================================================

/**
 * Configure floating / unbonded GPIOs to eliminate parasitic leakage current.
 * A floating CMOS digital input pin oscillates with noise and dissipates microamps.
 */
static void configureUnusedPinsToLowPower() {
    // Array of unused GPIO pins on ESP32-WROOM-32E module
    const gpio_num_t unusedPins[] = {
        GPIO_NUM_0,  GPIO_NUM_4,  GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_15,
        GPIO_NUM_16, GPIO_NUM_17, GPIO_NUM_21, GPIO_NUM_22
    };

    for (size_t i = 0; i < sizeof(unusedPins) / sizeof(unusedPins[0]); i++) {
        gpio_reset_pin(unusedPins[i]);
        pinMode(unusedPins[i], INPUT_PULLDOWN); // Tie internally to ground
    }
}

/**
 * Measure the supercapacitor bank voltage using ESP32 ADC1 with factory calibration.
 * @return Calibrated voltage in millivolts (mV).
 */
static uint16_t readSupercapacitorVoltage() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db); // 0 - 3.3V range

    // Take 16-sample average to filter out high-frequency switching noise
    uint32_t rawSum = 0;
    for (int i = 0; i < 16; i++) {
        rawSum += analogRead(PIN_VBAT_ADC);
        delayMicroseconds(200);
    }
    uint32_t rawAvg = rawSum / 16;

    // Convert raw ADC reading to millivolts at pin
    // Using ESP32 characterized transfer curve (nominal 12-bit full-scale ~ 3100-3300mV)
    float pinMv = ((float)rawAvg / 4095.0f) * ADC_REF_VOLTAGE_MV;

    // Scale back up through the 3:1 resistor divider (2M / 1M)
    float supercapMv = pinMv * ADC_VOLTAGE_DIVIDER_RATIO;

    if (supercapMv < 0.0f) supercapMv = 0.0f;
    return (uint16_t)supercapMv;
}

/**
 * Calculate the State-of-Charge (SOC %) based on electrostatic energy retention.
 * Supercapacitor energy is proportional to V^2: E = 0.5 * C * V^2.
 */
static uint8_t calculateSupercapSOC(uint16_t vbatMv) {
    if (vbatMv <= SUPERCAP_MIN_MV) return 0;
    if (vbatMv >= SUPERCAP_MAX_MV) return 100;

    float vCurrent = (float)vbatMv;
    float vMin = (float)SUPERCAP_MIN_MV;
    float vMax = (float)SUPERCAP_MAX_MV;

    // Electrostatic SOC formula: (V_act^2 - V_min^2) / (V_max^2 - V_min^2)
    float energyCurrent = (vCurrent * vCurrent) - (vMin * vMin);
    float energyMax     = (vMax * vMax) - (vMin * vMin);

    float soc = (energyCurrent / energyMax) * 100.0f;
    return (uint8_t)constrain((int)soc, 0, 100);
}

/**
 * Trigger and acquire acoustic time-of-flight distance from JSN-SR04T sensor.
 * @return Distance in millimeters (mm), or 0 on timeout/error.
 */
static uint16_t measureAcousticDistanceMm() {
    // 1. Enable 5V boost converter & load switch
    digitalWrite(PIN_SENS_PWR_EN, HIGH);
    
    // Allow boost regulator output capacitors and sensor PLL to stabilize
    delay(15); 

    // 2. Clear trigger pin and emit a 12-microsecond excitation pulse
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(4);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(12);
    digitalWrite(PIN_TRIG, LOW);

    // 3. Measure return echo pulse width (Timeout set to 30ms ~ 5.1 meters max range)
    unsigned long pulseDurationUs = pulseIn(PIN_ECHO, HIGH, 30000UL);

    // 4. Power down sensor rail immediately to eliminate 30mA standby drain
    digitalWrite(PIN_SENS_PWR_EN, LOW);

    if (pulseDurationUs == 0) {
        // Acoustic echo lost (surface reflection attenuation or out of range)
        return 0;
    }

    // Distance calculation: Speed of sound at 28°C ~ 348 m/s = 0.348 mm/us
    // Distance = (Duration * 0.348 mm/us) / 2
    float distanceMm = ((float)pulseDurationUs * 0.348f) / 2.0f;
    return (uint16_t)distanceMm;
}

/**
 * Transmit formatted binary packet over LoRa at 915 MHz (Chirp Spread Spectrum).
 * @return True if transmission completed successfully.
 */
static bool transmitLoRaTelemetry(const TelemetryPayload& payload) {
    // Initialize SPI bus pins for RFM95W
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);
    LoRa.setPins(PIN_LORA_NSS, PIN_LORA_RST, PIN_LORA_DIO0);

    if (!LoRa.begin(LORA_FREQUENCY)) {
        return false;
    }

    // Configure RF parameters for maximum urban penetration
    LoRa.setSpreadingFactor(LORA_SPREADING_FAC);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE);
    LoRa.setTxPower(LORA_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    LoRa.enableCrc();

    // Broadcast binary telemetry frame
    LoRa.beginPacket();
    LoRa.write((const uint8_t*)&payload, sizeof(TelemetryPayload));
    LoRa.endPacket(false); // Synchronous wait for TxDone

    // Put LoRa silicon into ultra-low current sleep mode (0.2uA)
    LoRa.sleep();
    SPI.end();
    return true;
}

/**
 * Adaptive Duty Cycling Engine:
 * Computes optimal deep sleep interval based on real-time flood proximity
 * and available supercapacitor energy buffer.
 */
static uint64_t computeAdaptiveSleepIntervalUs(uint16_t distanceMm, uint8_t socPercent) {
    uint16_t distanceCm = distanceMm / 10;

    // STATE 1: SEVERE CRITICAL FLOOD DETECTED (Water level < 120cm from sensor)
    if (distanceMm > 0 && distanceCm < FLOOD_CRITICAL_CM) {
        consecutiveAlerts++;
        // If energy allows, increase telemetry frequency to 2-minute cadence
        if (socPercent >= 30) {
            return SLEEP_FLOOD_ALERT_US;
        } else {
            // Buffer stressed: transmit at 5-minute cadence
            return (5ULL * 60ULL * 1000000ULL);
        }
    }

    // STATE 2: MODERATE FLOOD WARNING (120cm - 180cm)
    if (distanceMm > 0 && distanceCm < FLOOD_WARNING_CM) {
        consecutiveAlerts = 0;
        return (7ULL * 60ULL * 1000000ULL); // 7-minute cadence
    }

    // STATE 3: LOW HARVESTING BUFFER (SOC < 25%)
    // Estero biofilm current throttled due to chemical shock or cold; preserve buffer
    if (socPercent < 25) {
        consecutiveAlerts = 0;
        return SLEEP_HIBERNATE_US; // 60-minute sleep
    }

    // STATE 4: NOMINAL CONDITIONS (Distance normal, SOC healthy)
    consecutiveAlerts = 0;
    return SLEEP_NOMINAL_US; // 15-minute nominal cadence
}

// ============================================================================
// MAIN EXECUTION CYCLE (One-shot Deep Sleep Architecture)
// ============================================================================

void setup() {
    // Increment persistent lifecycle counter
    bootCount++;

    // 1. Initialize sensor gating pin (Active LOW during boot to guarantee 0 current)
    pinMode(PIN_SENS_PWR_EN, OUTPUT);
    digitalWrite(PIN_SENS_PWR_EN, LOW);

    // Initialize ultrasonic trigger and echo pins
    pinMode(PIN_TRIG, OUTPUT);
    digitalWrite(PIN_TRIG, LOW);
    pinMode(PIN_ECHO, INPUT);

    // 2. Prevent floating pin leakage
    configureUnusedPinsToLowPower();

    // 3. Query Wakeup Cause
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

    // 4. Measure Supercapacitor Storage & State-of-Charge
    uint16_t currentVbat = readSupercapacitorVoltage();
    uint8_t  currentSoc  = calculateSupercapSOC(currentVbat);

    // 5. Measure Canal Water Distance via Ultrasonic Ping
    uint16_t currentDistanceMm = measureAcousticDistanceMm();
    if (currentDistanceMm == 0 && lastDistanceMm > 0) {
        // Sensor retry fallback on single frame acoustic glitch
        delay(20);
        currentDistanceMm = measureAcousticDistanceMm();
    }
    lastDistanceMm = currentDistanceMm;
    lastVbatMv     = currentVbat;

    // 6. Build Compact Binary Telemetry Payload
    TelemetryPayload payload;
    payload.nodeId          = NODE_ID;
    payload.messageSequence = bootCount;
    payload.distanceMm      = currentDistanceMm;
    payload.vbatMv          = currentVbat;
    payload.socPercent      = currentSoc;
    payload.statusFlags     = 0;

    // Set bitmask flags
    if (currentDistanceMm > 0 && (currentDistanceMm / 10) < FLOOD_CRITICAL_CM) {
        payload.statusFlags |= (1 << 0); // Bit 0: Critical Flood Alert
    }
    if (currentSoc < 20) {
        payload.statusFlags |= (1 << 1); // Bit 1: Energy Starvation
    }
    if (currentDistanceMm == 0) {
        payload.statusFlags |= (1 << 2); // Bit 2: Acoustic Echo Timeout
    }

    // 7. LoRa Broadcast
    bool txOk = transmitLoRaTelemetry(payload);
    if (txOk) {
        payload.statusFlags |= (1 << 3); // Bit 3: RF Ack/Done
    }

    // 8. Adaptive Duty Cycling Calculation
    uint64_t sleepDurationUs = computeAdaptiveSleepIntervalUs(currentDistanceMm, currentSoc);

    // 9. Configure Wakeup Sources:
    // Source A: RTC Timer (Interval dynamically assigned by Adaptive Duty Cycling)
    esp_sleep_enable_timer_wakeup(sleepDurationUs);

    // Source B: BQ25504 Supervisor Wakeup (VBAT_OK pin goes HIGH when storage hits 3.6V)
    // If the system woke prematurely from brownout, it sleeps until BQ25504 affirms energy ready.
    esp_sleep_enable_ext0_wakeup(PIN_VBAT_OK, 1);

    // 10. Isolate RTC GPIOs to eradicate pin leakage during sleep
    rtc_gpio_isolate(PIN_VBAT_OK);
    rtc_gpio_isolate(PIN_SENS_PWR_EN);

    // 11. Enter Deep Sleep Mode (< 15 uA total quiescent draw)
    esp_deep_sleep_start();
}

void loop() {
    // Unreachable: Execution halts in esp_deep_sleep_start()
}
