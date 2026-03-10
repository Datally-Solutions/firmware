#pragma once
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "config.h"
#include "logger.h"
#include "secrets.h"

extern String wifiSsid;
extern String wifiPassword;

// --- RECONNEXION WIFI ---
void verifierConnexion() {
    if (WiFi.status() == WL_CONNECTED) return;

    addLog("Connexion perdue. Tentative de reconnexion...");
    M5.dis.fillpix(LED_ROUGE_SOMBRE);

    WiFi.disconnect();
    WiFi.begin(wifiSsid, wifiPassword);

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
void envoyerNotification(String context, String message) {
    if (WiFi.status() != WL_CONNECTED) return;

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String text = context + "%0A" + message;
    text.replace(" ", "%20");

    String url = "https://api.telegram.org/bot" + String(botToken) +
                 "/sendMessage?chat_id=" + String(chatId) + "&text=" + text;

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
// void envoyerDonneesSheets(String chat, String action, float poids, float poids_chat,
//                           unsigned long duree, String alerte) {
//     if (WiFi.status() != WL_CONNECTED) return;

//     WiFiClientSecure client;
//     client.setInsecure();
//     HTTPClient http;

//     String payload = "{";
//     payload += "\"chat\":\"" + chat + "\",";
//     payload += "\"action\":\"" + action + "\",";
//     payload += "\"poids\":" + String(poids, 1) + ",";
//     payload += "\"poids_chat\":" + String(poids_chat, 2) + ",";
//     payload += "\"duree\":" + String(duree) + ",";
//     payload += "\"alerte\":\"" + alerte + "\"";
//     payload += "}";

//     addLog("Payload Sheets: " + payload);

//     if (http.begin(client, sheetsWebhookUrl)) {
//         // http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS); disable for performance
//         http.setTimeout(10000);
//         http.addHeader("Content-Type", "application/json");
//         int httpCode = http.POST(payload);
//         if (httpCode == 200 || httpCode == 400 || httpCode == 302) {
//             addLog("Sheets envoyé ! Code : " + String(httpCode));
//         } else {
//             addLog("Erreur Sheets : " + http.errorToString(httpCode));
//         }
//         http.end();
//     }
// }

// --- GOOGLE CLOUD RUN INGEST ---
void envoyerDonneesGCP(float entryWeightKg, float exitWeightDeltaG, unsigned long durationSeconds,
                       String deviceId) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    String payload = "{";
    payload += "\"device_id\":\"" + deviceId + "\",";
    payload += "\"entry_weight_kg\":" + String(entryWeightKg, 3) + ",";
    payload += "\"exit_weight_delta_g\":" + String(exitWeightDeltaG, 1) + ",";
    payload += "\"duration_seconds\":" + String(durationSeconds);
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
