#pragma once
#include <Arduino.h>
#include <M5Atom.h>

#include "config.h"

void sauvegarderTimestamps();

// --- SUIVI DERNIÈRE ACTIVITÉ ---
extern unsigned long sullyDernierPipi;
extern unsigned long sullyDernierCaca;
extern unsigned long krokmouDernierPipi;
extern unsigned long krokmouDernierCaca;

// --- IDENTIFICATION ---
String identifierChat(float poids) {
    if (poids > SEUIL_SULLY_MIN && poids < SEUIL_SULLY_MAX) return "Sully";
    if (poids > SEUIL_KROKMOU_MIN && poids < SEUIL_KROKMOU_MAX) return "Krokmou";
    return "Inconnu";
}

// --- COULEUR LED SELON CHAT ---
void setCouleurChat(String nomChat) {
    if (nomChat == "Sully")
        M5.dis.fillpix(LED_BLEU);
    else if (nomChat == "Krokmou")
        M5.dis.fillpix(LED_ORANGE);
    else
        M5.dis.fillpix(LED_VERT);
}

// --- DIAGNOSTIC ---
void calculerDiagnostic(String nomChat, float poids, unsigned long duree, String& diagnostic,
                        String& alerte) {
    if (nomChat == "Sully") {
        if (poids < SULLY_VISITE_MAX) {
            diagnostic = "Simple visite";
            if (duree > DUREE_GRATTAGE_S) alerte = "*Alerte :* Grattage long sans résultat.";
        } else if (poids < SULLY_PIPI_MAX) {
            diagnostic = duree > DUREE_VIGILANCE_S ? "Petit Pipi 🟡" : "Pipi 🟡";
            sullyDernierPipi = millis();
            sauvegarderTimestamps();
            if (duree > DUREE_VIGILANCE_S) alerte = "*Vigilance :* Long pour un petit résultat.";
        } else {
            if (duree > DUREE_GRATTAGE_S) {
                diagnostic = "Caca 🟤";
                sullyDernierCaca = millis();
            } else {
                diagnostic = "Gros Pipi 🟡";
                sullyDernierPipi = millis();
            }
            sauvegarderTimestamps();
        }
    } else if (nomChat == "Krokmou") {
        if (poids < KROKMOU_VISITE_MAX) {
            diagnostic = "Simple visite";
            if (duree > DUREE_GRATTAGE_S) alerte = "*Alerte :* Grattage long sans résultat.";
        } else if (poids < KROKMOU_PIPI_MAX) {
            diagnostic = duree > DUREE_VIGILANCE_S ? "Petit Pipi 🟡" : "Pipi 🟡";
            krokmouDernierPipi = millis();
            sauvegarderTimestamps();
            if (duree > DUREE_VIGILANCE_S) alerte = "*Vigilance :* Long pour un petit résultat.";
        } else {
            if (duree > DUREE_GRATTAGE_S) {
                diagnostic = "Caca 🟤";
                krokmouDernierCaca = millis();
            } else {
                diagnostic = "Gros Pipi 🟡";
                krokmouDernierPipi = millis();
            }
            sauvegarderTimestamps();
        }
    } else {
        if (poids < INCONNU_VISITE_MAX)
            diagnostic = "Simple visite";
        else if (poids < INCONNU_PIPI_MAX)
            diagnostic = "Pipi 🟡";
        else
            diagnostic = duree > DUREE_GRATTAGE_S ? "Caca 🟤" : "Gros Pipi 🟡";
    }
}
