#pragma once

// --- CABLAGE HX711 ---
#define LOADCELL_DOUT_PIN 32
#define LOADCELL_SCK_PIN 26

// --- CALIBRATION BALANCE ---
const float CALIBRATION_SCALE = 24.79f;

// --- SEUILS POIDS (kg) ---
const float SEUIL_ENTREE_KG = 2.0;
const float SEUIL_SORTIE_KG = 1.0;
const float SEUIL_NETTOYAGE_KG = -0.15;

// --- IDENTIFICATION CHATS (kg) ---
const float SEUIL_SULLY_MIN = 2.5;
const float SEUIL_SULLY_MAX = 5.5;
const float SEUIL_KROKMOU_MIN = 5.5;
const float SEUIL_KROKMOU_MAX = 9.0;

// --- SEUILS DIAGNOSTIC SULLY (g) ---
const float SULLY_VISITE_MAX = 10.0;
const float SULLY_PIPI_MAX = 35.0;

// --- SEUILS DIAGNOSTIC KROKMOU (g) ---
const float KROKMOU_VISITE_MAX = 20.0;
const float KROKMOU_PIPI_MAX = 70.0;

// --- SEUILS DIAGNOSTIC INCONNU (g) ---
const float INCONNU_VISITE_MAX = 15.0;
const float INCONNU_PIPI_MAX = 55.0;

// --- DURÉES SESSIONS (secondes) ---
const unsigned long DUREE_GRATTAGE_S = 90;
const unsigned long DUREE_VIGILANCE_S = 120;
const unsigned long DUREE_ALERTE_S = 240;

// --- DURÉES SYSTÈME (ms) ---
const unsigned long DUREE_SESSION_MAX_MS = 600000;  // 10 min anti-blocage
const unsigned long EXIT_STABILISATION_MS = 5000;   // attente après sortie
const unsigned long NETTOYAGE_ATTENTE_MS = 5000;    // attente après poids négatif
const unsigned long WIFI_CHECK_INTERVAL = 30000;    // vérif WiFi toutes les 30s
const unsigned long SANTE_CHECK_INTERVAL = 60000;   // vérif santé toutes les 60s
const unsigned long WDT_TIMEOUT_S = 30;             // watchdog timeout

// --- ALERTES SANTÉ ---
const unsigned long ALERTE_PIPI_MS = 24UL * 60 * 60 * 1000;  // 24h
const unsigned long ALERTE_CACA_MS = 48UL * 60 * 60 * 1000;  // 48h

// --- LOGS ---
const int MAX_LOG_LINES = 50;

// --- COULEURS LED ---
const uint32_t LED_BLEU = 0x0000ff;
const uint32_t LED_ROUGE = 0xff0000;
const uint32_t LED_VERT = 0x00ff00;
const uint32_t LED_ORANGE = 0xffa500;
const uint32_t LED_CYAN = 0x00ffff;
const uint32_t LED_ROUGE_SOMBRE = 0x330000;
