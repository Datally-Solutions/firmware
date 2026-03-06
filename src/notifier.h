#pragma once
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "config.h"
#include "logger.h"
#include "secrets.h"

// --- RECONNEXION WIFI ---
void verifierConnexion() {
    if (WiFi.status() == WL_CONNECTED) return;

    addLog("Connexion perdue. Tentative de reconnexion...");
    M5.dis.fillpix(LED_ROUGE_SOMBRE);

    WiFi.disconnect();
    WiFi.begin(ssid, password);

    int tentative = 0;
    while (WiFi.status() != WL_CONNECTED && tentative < 20) {
        delay(500);
        tentative++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        addLog("✅ Reconnecté !");
        M5.dis.fillpix(LED_VERT);
    }
}

// --- TELEGRAM ---
void envoyerNotification(String chat, String action, float poids, float poids_chat,
                         unsigned long duree, String alerte) {
    if (WiFi.status() != WL_CONNECTED) return;

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String message = "Rapport Litiere %0A";
    message += "Chat : " + chat + "%0A";
    message += "Poids " + String(poids_chat, 1) + "kg%0A";
    message += "Action : " + action + "%0A";
    if (poids > 0) message += "*Poids :* " + String(poids, 1) + "g %0A";
    if (duree > 0) message += "*Durée :* " + String(duree) + "s";
    if (alerte != "") message += "%0A%0A" + alerte;

    addLog("Envoi Telegram : " + message);
    message.replace(" ", "%20");

    String url = "https://api.telegram.org/bot" + String(botToken) +
                 "/sendMessage?chat_id=" + String(chatId) + "&text=" + message;

    if (http.begin(client, url)) {
        http.setTimeout(10000);
        int httpCode = http.GET();
        if (httpCode > 0)
            addLog("Telegram envoyé ! Code : " + String(httpCode));
        else
            addLog("Erreur Telegram : " + http.errorToString(httpCode));
        http.end();
    }
}

// --- GOOGLE SHEETS ---
void envoyerDonneesSheets(String chat, String action, float poids, float poids_chat,
                          unsigned long duree, String alerte) {
    if (WiFi.status() != WL_CONNECTED) return;

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String payload = "{";
    payload += "\"chat\":\"" + chat + "\",";
    payload += "\"action\":\"" + action + "\",";
    payload += "\"poids\":" + String(poids, 1) + ",";
    payload += "\"poids_chat\":" + String(poids_chat, 2) + ",";
    payload += "\"duree\":" + String(duree) + ",";
    payload += "\"alerte\":\"" + alerte + "\"";
    payload += "}";

    addLog("Payload Sheets: " + payload);

    if (http.begin(client, sheetsWebhookUrl)) {
        // http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS); disable for performance
        http.setTimeout(10000);
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(payload);
        if (httpCode == 200 || httpCode == 400 || httpCode == 302) {
            addLog("Sheets envoyé ! Code : " + String(httpCode));
        } else {
            addLog("Erreur Sheets : " + http.errorToString(httpCode));
        }
        http.end();
    }
}

// --- GOOGLE CLOUD RUN INGEST ---
void envoyerDonneesGCP(String chat, String action, float poids, float poids_chat,
                       unsigned long duree, String alerte) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String payload = "{";
    payload += "\"chat\":\"" + chat + "\",";
    payload += "\"action\":\"" + action + "\",";
    payload += "\"poids\":" + String(poids, 1) + ",";
    payload += "\"poids_chat\":" + String(poids_chat, 2) + ",";
    payload += "\"duree\":" + String(duree) + ",";
    payload += "\"alerte\":\"" + alerte + "\"";
    payload += "}";

    addLog("Payload GCP: " + payload);

    if (http.begin(client, ingestUrl)) {
        http.setTimeout(10000);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("X-Ingest-Token", ingestToken);
        int httpCode = http.POST(payload);
        if (httpCode > 0) {
            addLog("GCP envoyé ! Code : " + String(httpCode));
        } else {
            addLog("Erreur GCP : " + http.errorToString(httpCode));
        }
        http.end();
    }
}

// --- ALERTES SANTÉ ---
extern unsigned long sullyDernierPipi;
extern unsigned long sullyDernierCaca;
extern unsigned long krokmouDernierPipi;
extern unsigned long krokmouDernierCaca;

void verifierAlertesSante() {
    unsigned long maintenant = millis();

    if (sullyDernierPipi > 0 && (maintenant - sullyDernierPipi > ALERTE_PIPI_MS)) {
        envoyerNotification("Sully", "Alerte Santé", 0, 0, 0,
                            "⚠️ Sully n'a pas fait pipi depuis +24h !");
        addLog("⚠️ Sully n'a pas fait pipi depuis +24h !");
        sullyDernierPipi = maintenant;
        sauvegarderTimestamps();  // 👈
    }
    if (sullyDernierCaca > 0 && (maintenant - sullyDernierCaca > ALERTE_CACA_MS)) {
        envoyerNotification("Sully", "Alerte Santé", 0, 0, 0,
                            "⚠️ Sully n'a pas fait caca depuis +48h !");
        addLog("⚠️ Sully n'a pas fait caca depuis +48h !");
        sullyDernierCaca = maintenant;
        sauvegarderTimestamps();  // 👈
    }
    if (krokmouDernierPipi > 0 && (maintenant - krokmouDernierPipi > ALERTE_PIPI_MS)) {
        envoyerNotification("Krokmou", "Alerte Santé", 0, 0, 0,
                            "⚠️ Krokmou n'a pas fait pipi depuis +24h !");
        addLog("⚠️ Krokmou n'a pas fait pipi depuis +24h !");
        krokmouDernierPipi = maintenant;
        sauvegarderTimestamps();  // 👈
    }
    if (krokmouDernierCaca > 0 && (maintenant - krokmouDernierCaca > ALERTE_CACA_MS)) {
        envoyerNotification("Krokmou", "Alerte Santé", 0, 0, 0,
                            "⚠️ Krokmou n'a pas fait caca depuis +48h !");
        addLog("⚠️ Krokmou n'a pas fait caca depuis +48h !");
        krokmouDernierCaca = maintenant;
        sauvegarderTimestamps();  // 👈
    }
}

// Forward declare at top of notifier.h
extern Preferences prefs;

// In sauvegarderTimestamps() — save "how long ago" instead of raw millis
void sauvegarderTimestamps() {
    unsigned long now = millis();
    prefs.putULong("s_pipi", sullyDernierPipi > 0 ? now - sullyDernierPipi : 0);
    prefs.putULong("s_caca", sullyDernierCaca > 0 ? now - sullyDernierCaca : 0);
    prefs.putULong("k_pipi", krokmouDernierPipi > 0 ? now - krokmouDernierPipi : 0);
    prefs.putULong("k_caca", krokmouDernierCaca > 0 ? now - krokmouDernierCaca : 0);
    addLog("Timestamps sauvegardés en NVS ✅");
}
