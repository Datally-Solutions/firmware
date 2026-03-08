#include <ElegantOTA.h>
#include <M5Atom.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <esp_ota_ops.h>
#include <esp_task_wdt.h>

#include "HX711.h"
#include "config.h"
#include "diagnostic.h"
#include "logger.h"
#include "notifier.h"
#include "provisioning.h"
#include "secrets.h"

// --- BALANCE ---
HX711 scale;

// --- SERVEUR WEB ---
WebServer server(80);

// --- ÉTAT SYSTÈME ---
float poidsEntree = 0;
bool occupe = false;
bool exitPending = false;
bool otaInProgress = false;
unsigned long tempsEntree = 0;
unsigned long exitDetectedAt = 0;
unsigned long dernierCheckWifi = 0;

// --- SUIVI DERNIÈRE ACTIVITÉ ---
unsigned long sullyDernierPipi = 0;
unsigned long sullyDernierCaca = 0;
unsigned long krokmouDernierPipi = 0;
unsigned long krokmouDernierCaca = 0;

// --- LOGS ---
String logBuffer = "";
int logLineCount = 0;

// ---------------------------------------------------------------------------
// FORWARD DECLARATIONS
// ---------------------------------------------------------------------------
void detecterEntree();
void traiterSortieChat();
void detecterNettoyage();

Preferences prefs;

String deviceId;

// --- WIFI CREDENTIALS (loaded from NVS) ---
String wifiSsid;
String wifiPassword;

// ---------------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);
    btStop();
    M5.begin(false, false, true);

    uint64_t chipid = ESP.getEfuseMac();
    char id[18];
    snprintf(id, sizeof(id), "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
    deviceId = String(id);

    addLog("Device ID: " + deviceId);

    // ── WiFi ──────────────────────────────────────────────────────────────────
    M5.dis.fillpix(LED_BLEU);

    bool hasCredentials = chargerCredentialsWifi(wifiSsid, wifiPassword);

    if (hasCredentials) {
        bool connected = connecterWifi(wifiSsid, wifiPassword);
        if (!connected) {
            addLog("Credentials invalides. Démarrage mode provisioning...");
            demarrerModeProvisionning(deviceId);
        }
    } else {
        addLog("Première utilisation. Démarrage mode provisioning...");
        demarrerModeProvisionning(deviceId);
    }

    // If we reach here WiFi is connected ✅
    addLog("Wi-Fi Connecté ! IP : " + WiFi.localIP().toString());

    // NVS — load "how long ago" and convert to current millis() base
    prefs.begin("litiere", false);
    unsigned long now = millis();
    unsigned long agoSPipi = prefs.getULong("s_pipi", 0);
    unsigned long agoSCaca = prefs.getULong("s_caca", 0);
    unsigned long agoKPipi = prefs.getULong("k_pipi", 0);
    unsigned long agoKCaca = prefs.getULong("k_caca", 0);

    sullyDernierPipi = agoSPipi > 0 ? now - agoSPipi : 0;
    sullyDernierCaca = agoSCaca > 0 ? now - agoSCaca : 0;
    krokmouDernierPipi = agoKPipi > 0 ? now - agoKPipi : 0;
    krokmouDernierCaca = agoKCaca > 0 ? now - agoKCaca : 0;

    addLog("NVS chargé — s_pipi=" + String(agoSPipi / 3600000) + "h ago" + " s_caca=" +
           String(agoSCaca / 3600000) + "h ago" + " k_pipi=" + String(agoKPipi / 3600000) +
           "h ago" + " k_caca=" + String(agoKCaca / 3600000) + "h ago");

    // Watchdog
    esp_task_wdt_init(WDT_TIMEOUT_S, true);
    esp_task_wdt_add(NULL);

    // Routes web
    server.on("/", []() {
        String html = "<html><head><meta charset='UTF-8'></head><body>";
        html += "<h1>🐱 Litière</h1>";
        html += "<p>État : " + String(occupe ? "🔴 Occupée" : "🟢 Libre") + "</p>";
        html += "<p>Dernier pipi Sully : " + String((millis() - sullyDernierPipi) / 3600000) +
                "h ago</p>";
        html += "<p>Dernier pipi Krokmou : " + String((millis() - krokmouDernierPipi) / 3600000) +
                "h ago</p>";
        html += "<p>Dernier caca Sully : " + String((millis() - sullyDernierCaca) / 3600000) +
                "h ago</p>";
        html += "<p>Dernier caca Krokmou : " + String((millis() - krokmouDernierCaca) / 3600000) +
                "h ago</p>";
        html += "<a href='/logs'>Logs</a> | <a href='/tare'>Tare</a> | <a href='/update'>OTA</a>";
        html += "</body></html>";
        server.send(200, "text/html", html);
    });

    server.on("/status", []() {
        String json = "{";
        json += "\"occupe\":" + String(occupe ? "true" : "false") + ",";
        json += "\"uptime\":" + String(millis() / 1000) + ",";
        json += "\"sully_dernier_pipi\":" + String(sullyDernierPipi) + ",";
        json += "\"krokmou_dernier_pipi\":" + String(krokmouDernierPipi);
        json += "}";
        server.send(200, "application/json", json);
    });

    server.on("/tare", []() {
        M5.dis.fillpix(LED_BLEU);
        scale.tare();
        delay(500);
        M5.dis.fillpix(LED_VERT);
        verifierConnexion();
        envoyerNotification("Système", "Tare distante faite!", 0, 0, 0, "");
        server.send(200, "text/plain", "Tare effectuée !");
    });

    server.on("/reset-nvs", []() {
        prefs.clear();
        sullyDernierPipi = 0;
        sullyDernierCaca = 0;
        krokmouDernierPipi = 0;
        krokmouDernierCaca = 0;
        addLog("NVS réinitialisé ✅");
        server.send(200, "text/plain", "NVS réinitialisé !");
    });

    server.on("/logs", []() {
        String html = "<html><head>";
        html += "<meta charset='UTF-8'>";
        html += "<meta http-equiv='refresh' content='10'>";
        html += "<style>body{font-family:monospace;background:#111;color:#0f0;padding:20px;}";
        html += "h2{color:#fff;}</style></head><body>";
        html += "<h2>🐱 Litière — Logs</h2>";
        html += logBuffer.length() > 0 ? logBuffer : "Aucun log pour le moment.";
        html += "</body></html>";
        server.send(200, "text/html", html);
    });

    server.on("/reset-wifi", []() {
        prefs.remove("wifi_ssid");
        prefs.remove("wifi_pass");
        server.send(200, "text/plain", "WiFi réinitialisé. Redémarrage...");
        delay(1000);
        ESP.restart();
    });

    // OTA
    ElegantOTA.begin(&server);
    ElegantOTA.setAutoReboot(true);
    ElegantOTA.onStart([]() {
        otaInProgress = true;
        esp_task_wdt_delete(NULL);
        addLog("OTA démarré...");
    });
    ElegantOTA.onEnd([](bool success) {
        otaInProgress = false;
        esp_task_wdt_add(NULL);
        addLog(success ? "OTA réussi ✅" : "OTA échoué ❌");
    });
    server.begin();
    addLog("Serveur démarré.");

    // Balance
    M5.dis.fillpix(LED_ROUGE);
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(CALIBRATION_SCALE);
    M5.dis.fillpix(LED_BLEU);
    delay(2000);
    scale.tare();

    M5.dis.fillpix(LED_VERT);
    addLog("Système prêt et calibré.");

    // Notification démarrage
    verifierConnexion();
    envoyerNotification("Système", "Litière connectée et prête !", 0, 0, 0, "");

    // Valider le firmware (rollback protection)
    esp_ota_mark_app_valid_cancel_rollback();
    addLog("Firmware marqué comme valide ✅");
}

// ---------------------------------------------------------------------------
// LOOP
// ---------------------------------------------------------------------------
void loop() {
    server.handleClient();
    ElegantOTA.loop();
    esp_task_wdt_reset();
    M5.update();

    float weight = scale.get_units(5) / 1000.0;
    static unsigned long dernierCheckSante = 0;

    // Vérification WiFi périodique
    if (millis() - dernierCheckWifi > WIFI_CHECK_INTERVAL && !otaInProgress) {
        dernierCheckWifi = millis();
        verifierConnexion();
    }

    // 1. Détection entrée
    if (weight > SEUIL_ENTREE_KG && !occupe && !exitPending) {
        detecterEntree();
    }

    // 2. Anti-blocage 10 min
    if (occupe && (millis() - tempsEntree > DUREE_SESSION_MAX_MS)) {
        addLog("⚠️ Session +10 min détectée. Reset forcé.");
        verifierConnexion();
        envoyerNotification("Système", "*Alerte Sécurité*", 0, poidsEntree, 0,
                            "Session de +10 min détectée. Reset automatique.");
        scale.tare();
        occupe = false;
        exitPending = false;
        M5.dis.fillpix(LED_VERT);
    }

    // 3. Détection sortie — phase 1
    if (weight < SEUIL_SORTIE_KG && occupe && !exitPending) {
        exitPending = true;
        exitDetectedAt = millis();
    }

    // 3. Détection sortie — phase 2
    if (exitPending && (millis() - exitDetectedAt >= EXIT_STABILISATION_MS)) {
        exitPending = false;
        occupe = false;
        traiterSortieChat();
    }

    // 4. Détection nettoyage
    if (weight < SEUIL_NETTOYAGE_KG && !occupe && !exitPending) {
        detecterNettoyage();
    }

    // 5. Tare manuelle
    if (M5.Btn.wasPressed()) {
        M5.dis.fillpix(LED_BLEU);
        scale.tare();
        delay(500);
        M5.dis.fillpix(LED_VERT);
        addLog(">>> TARE MANUELLE");
        verifierConnexion();
        envoyerNotification("Système", "Tare manuelle faite!", 0, 0, 0, "");
    }

    // 6. Alertes santé
    if (millis() - dernierCheckSante > SANTE_CHECK_INTERVAL) {
        dernierCheckSante = millis();
        verifierAlertesSante();
    }

    if (!otaInProgress) delay(200);
}

// ---------------------------------------------------------------------------
// FONCTIONS
// ---------------------------------------------------------------------------

void detecterEntree() {
    addLog("Mouvement détecté, vérification stabilité...");
    delay(1500);

    float readings[10];
    for (int i = 0; i < 10; i++) {
        readings[i] = scale.get_units(10) / 1000.0;
        delay(500);
    }

    // Log all readings for debug
    String debugReadings = "Lectures : ";
    for (int i = 0; i < 10; i++) {
        debugReadings += String(readings[i], 2);
        if (i < 9) debugReadings += " | ";
    }
    addLog(debugReadings);

    // Sort (bubble sort)
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (readings[j] < readings[i]) {
                float tmp = readings[i];
                readings[i] = readings[j];
                readings[j] = tmp;
            }
        }
    }

    float weightStable = (readings[4] + readings[5]) / 2.0;
    addLog("Min=" + String(readings[0], 2) + " Max=" + String(readings[9], 2) +
           " Median=" + String(weightStable, 2) + "kg");

    if (weightStable > SEUIL_ENTREE_KG) {
        occupe = true;
        exitPending = false;
        poidsEntree = weightStable;
        tempsEntree = millis();
        M5.dis.fillpix(LED_ROUGE);
        addLog("--- CHAT ENTRÉ : " + String(poidsEntree, 2) + "kg ---");
    }
}

void traiterSortieChat() {
    // Vérifier que le chat est vraiment parti
    float weightCheck = scale.get_units(5) / 1000.0;
    if (weightCheck > SEUIL_SORTIE_KG) {
        occupe = true;
        M5.dis.fillpix(LED_ROUGE);
        addLog("Fausse sortie détectée, chat toujours présent.");
        return;
    }

    // Attendre stabilisation de la litière
    esp_task_wdt_reset();
    delay(3000);
    esp_task_wdt_reset();

    float poidsFinalGrames = scale.get_units(30);
    unsigned long dureeSession = (millis() - tempsEntree) / 1000;

    String nomChat = identifierChat(poidsEntree);
    setCouleurChat(nomChat);

    String diagnostic = "";
    String alerte = "";
    calculerDiagnostic(nomChat, poidsFinalGrames, dureeSession, diagnostic, alerte);

    if (dureeSession > DUREE_ALERTE_S && alerte == "") {
        alerte = "*Attention :* Session extrêmement longue (+4 min).";
    }

    addLog("Chat : " + nomChat + " | " + diagnostic + " | " + String(poidsFinalGrames, 1) + "g | " +
           String(dureeSession) + "s");

    verifierConnexion();
    envoyerNotification(nomChat, diagnostic, poidsFinalGrames, poidsEntree, dureeSession, alerte);
    envoyerDonneesSheets(nomChat, diagnostic, poidsFinalGrames, poidsEntree, dureeSession, alerte);
    envoyerDonneesGCP(nomChat, diagnostic, poidsFinalGrames, poidsEntree, dureeSession, alerte,
                      deviceId);

    // Reset
    esp_task_wdt_reset();
    delay(500);
    scale.tare();
    poidsEntree = 0;
    tempsEntree = 0;
    M5.dis.fillpix(LED_VERT);
    addLog("Balance remise à zéro.");
}

void detecterNettoyage() {
    addLog("Poids négatif détecté. Attente stabilité...");

    // Check 3 times over 15 seconds
    int confirmations = 0;
    for (int i = 0; i < 3; i++) {
        esp_task_wdt_reset();
        delay(NETTOYAGE_ATTENTE_MS);
        float check = scale.get_units(15) / 1000.0;
        if (check < SEUIL_NETTOYAGE_KG) confirmations++;
        addLog("Confirmation " + String(i + 1) + "/3 : " + String(check, 2) + "kg");
    }

    if (confirmations == 3) {  // all 3 must be negative
        M5.dis.fillpix(LED_CYAN);
        addLog(">>> AUTO-TARE (Nettoyage confirmé 3/3)");
        scale.tare();
        verifierConnexion();
        envoyerNotification("Système", "Nettoyage détecté", 0, 0, 0,
                            "La litière a été remise à zéro automatiquement.");
        delay(1000);
        M5.dis.fillpix(LED_VERT);
    } else {
        addLog("Fausse alerte nettoyage (" + String(confirmations) + "/3)");
    }
}
