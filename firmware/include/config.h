#pragma once

#include <Arduino.h>

// ==============================================================================
// HARDWARE PIN DEFINITIONS (ESP32-WROOM-32E)
// ==============================================================================

// BQ25504 Energy Harvester Supervisor Interface
#define PIN_VBAT_OK         GPIO_NUM_33   // RTC GPIO 8: Wakeup interrupt on VSTOR >= 3.6V

// Sensor Power Gating & Ultrasonic Interface
#define PIN_SENS_PWR_EN     GPIO_NUM_25   // Active-HIGH gate control for 5V boost & JSN-SR04T
#define PIN_TRIG            GPIO_NUM_32   // Ultrasonic Trigger output pulse
#define PIN_ECHO            GPIO_NUM_35   // Ultrasonic Echo input (via level shifter)

// Supercapacitor Voltage Monitoring ADC
#define PIN_VBAT_ADC        GPIO_NUM_34   // ADC1_CH6: 3:1 resistor divider monitoring supercap

// RFM95W LoRa Transceiver SPI & Control Interface
#define PIN_LORA_NSS        GPIO_NUM_5    // LoRa SPI Chip Select
#define PIN_LORA_SCK        GPIO_NUM_18   // LoRa SPI Clock
#define PIN_LORA_MISO       GPIO_NUM_19   // LoRa SPI MISO
#define PIN_LORA_MOSI       GPIO_NUM_23   // LoRa SPI MOSI
#define PIN_LORA_RST        GPIO_NUM_14   // LoRa Hardware Reset
#define PIN_LORA_DIO0       GPIO_NUM_26   // LoRa DIO0 (TxDone / RxDone)
#define PIN_LORA_DIO1       GPIO_NUM_27   // LoRa DIO1 (RxTimeout)

// Status LED (Only active during laboratory debug; compiled out in production)
#define PIN_DEBUG_LED       GPIO_NUM_2

// ==============================================================================
// TELEMETRY & RF PARAMETERS (AS923 / US915 / PH 915MHz)
// ==============================================================================
#define LORA_FREQUENCY      915.0E6       // 915 MHz ISM Band
#define LORA_SPREADING_FAC  10            // Spreading Factor SF10 (Max range in concrete canal NLOS)
#define LORA_BANDWIDTH      125.0E3       // 125 kHz Bandwidth
#define LORA_CODING_RATE    5             // 4/5 Coding Rate
#define LORA_TX_POWER       17            // +17 dBm (50 mW EIRP)
#define LORA_SYNC_WORD      0x12          // Private Sync Word (or 0x34 for Public LoRaWAN)

// ==============================================================================
// HYDROLOGICAL THRESHOLDS & POWER MANAGEMENT CONSTANTS
// ==============================================================================
#define NODE_ID             0x01          // Unique 8-bit Node Identifier
#define FLOOD_CRITICAL_CM   120           // Critical flood threshold (distance < 120cm triggers alert)
#define FLOOD_WARNING_CM    180           // Warning flood threshold

// Sleep Timing (in microseconds for esp_sleep_enable_timer_wakeup)
#define SLEEP_NOMINAL_US    (15ULL * 60ULL * 1000000ULL)  // 15 Minutes Nominal
#define SLEEP_FLOOD_ALERT_US (2ULL * 60ULL * 1000000ULL)  // 2 Minutes during Flood Threat
#define SLEEP_HIBERNATE_US  (60ULL * 60ULL * 1000000ULL)  // 60 Minutes if Energy Buffer Depleted

// Supercapacitor Voltage Calibration
#define ADC_VOLTAGE_DIVIDER_RATIO 3.00f   // (R1 + R2) / R2 = (2M + 1M) / 1M = 3.0
#define ADC_REF_VOLTAGE_MV        3300.0f
#define SUPERCAP_MAX_MV           4800    // 4.80V Maximum Operating Voltage
#define SUPERCAP_MIN_MV           3000    // 3.00V Drop-out Threshold
