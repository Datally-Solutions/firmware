#pragma once

// --- CABLAGE HX711 ---
#define LOADCELL_DOUT_PIN 32
#define LOADCELL_SCK_PIN 26

// --- CALIBRATION BALANCE ---
const float CALIBRATION_SCALE = 27.15f;

// --- SEUILS POIDS (kg) ---
const float SEUIL_ENTREE_KG = 2.0;
const float SEUIL_SORTIE_KG = 1.0;
const float SEUIL_NETTOYAGE_KG = -0.30;

// --- DURÉES SYSTÈME (ms) ---
const unsigned long DUREE_SESSION_MAX_MS = 600000;  // 10 min anti-blocage
const unsigned long EXIT_STABILISATION_MS = 5000;   // attente après sortie
const unsigned long NETTOYAGE_ATTENTE_MS = 5000;    // attente après poids négatif
const unsigned long WIFI_CHECK_INTERVAL = 30000;    // vérif WiFi toutes les 30s
const unsigned long WDT_TIMEOUT_S = 30;             // watchdog timeout

// --- LOGS ---
const int MAX_LOG_LINES = 50;

// --- COULEURS LED ---
const uint32_t LED_BLEU = 0x0000ff;
const uint32_t LED_ROUGE = 0xff0000;
const uint32_t LED_VERT = 0x00ff00;
const uint32_t LED_CYAN = 0x00ffff;
const uint32_t LED_ROUGE_SOMBRE = 0x330000;

const unsigned long NETTOYAGE_COOLDOWN_MS = 30000;  // 30s after exit before cleaning can trigger
