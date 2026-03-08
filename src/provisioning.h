#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#include "logger.h"

extern Preferences prefs;

bool chargerCredentialsWifi(String& ssid, String& password) {
    Preferences localPrefs;
    localPrefs.begin("litiere", true);  // read-only
    ssid     = localPrefs.getString("wifi_ssid", "");
    password = localPrefs.getString("wifi_pass", "");
    localPrefs.end();
    addLog("NVS load — ssid: " + ssid);
    return ssid.length() > 0;
}

void sauvegarderCredentialsWifi(const String& ssid, const String& password) {
    Preferences localPrefs;
    localPrefs.begin("litiere", false);  // read-write
    bool r1 = localPrefs.putString("wifi_ssid", ssid);
    bool r2 = localPrefs.putString("wifi_pass", password);
    localPrefs.end();
    addLog("NVS save — ssid:" + String(r1) + " pass:" + String(r2));
}

// ─── Connect with saved credentials ──────────────────────────────────────────
bool connecterWifi(const String& ssid, const String& password, int maxTentatives = 20) {
    addLog("Connexion Wi-Fi à : " + ssid);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid.c_str(), password.c_str());

    int tentative = 0;
    while (WiFi.status() != WL_CONNECTED && tentative < maxTentatives) {
        delay(500);
        tentative++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        addLog("✅ Wi-Fi connecté ! IP : " + WiFi.localIP().toString());
        return true;
    }

    addLog("❌ Échec connexion Wi-Fi après " + String(maxTentatives) + " tentatives.");
    return false;
}

// ─── AP Mode provisioning ─────────────────────────────────────────────────────
static bool wifiProvisioned = false;
static String pendingSsid;
static String pendingPassword;

void demarrerModeProvisionning(const String& deviceId) {
    String apName = "LitiereSetup-" + deviceId.substring(0, 6);
    addLog("Démarrage mode AP : " + apName);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName.c_str());
    addLog("AP démarré. IP : " + WiFi.softAPIP().toString());

    M5.dis.fillpix(0xFFA500);  // Orange — provisioning mode

    WebServer apServer(80);

    // ── GET /ping — app polls this to detect ESP32 hotspot connection ─────────
    apServer.on("/ping", HTTP_GET, [&]() {
        apServer.sendHeader("Access-Control-Allow-Origin", "*");
        apServer.send(200, "application/json", "{\"status\":\"ok\",\"device\":\"" + deviceId + "\"}");
    });

    // ── GET /networks — returns scanned WiFi networks as JSON ─────────────────
    apServer.on("/networks", HTTP_GET, [&]() {
        int n = WiFi.scanNetworks();
        StaticJsonDocument<2048> doc;
        JsonArray networks = doc.createNestedArray("networks");
        for (int i = 0; i < n && i < 20; i++) {
            JsonObject net = networks.createNestedObject();
            net["ssid"]    = WiFi.SSID(i);
            net["rssi"]    = WiFi.RSSI(i);
            net["secure"]  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        }
        String json;
        serializeJson(doc, json);
        apServer.sendHeader("Access-Control-Allow-Origin", "*");
        apServer.send(200, "application/json", json);
    });

    // ── POST /configure — app sends WiFi credentials ──────────────────────────
    apServer.on("/configure", HTTP_POST, [&]() {
        apServer.sendHeader("Access-Control-Allow-Origin", "*");

        if (!apServer.hasArg("plain")) {
            apServer.send(400, "application/json", "{\"error\":\"No body\"}");
            return;
        }

        StaticJsonDocument<256> doc;
        DeserializationError err = deserializeJson(doc, apServer.arg("plain"));
        if (err) {
            apServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        pendingSsid     = doc["ssid"].as<String>();
        pendingPassword = doc["password"].as<String>();

        if (pendingSsid.isEmpty()) {
            apServer.send(400, "application/json", "{\"error\":\"SSID required\"}");
            return;
        }

        apServer.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Rebooting...\"}");
        addLog("Credentials reçus pour : " + pendingSsid);
        wifiProvisioned = true;
    });

    // ── OPTIONS — CORS preflight for app HTTP calls ───────────────────────────
    apServer.on("/configure", HTTP_OPTIONS, [&]() {
        apServer.sendHeader("Access-Control-Allow-Origin", "*");
        apServer.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        apServer.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        apServer.send(204);
    });

    apServer.begin();
    addLog("Serveur AP démarré. En attente de configuration...");

    // Wait until app sends credentials
    while (!wifiProvisioned) {
        apServer.handleClient();
        delay(10);
    }

    // Save and reboot
    sauvegarderCredentialsWifi(pendingSsid, pendingPassword);
    addLog("Redémarrage pour connexion WiFi...");
    delay(500);
    ESP.restart();
}