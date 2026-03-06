#include <cassert>
#include <iostream>

#include "Arduino.h"  //

// --- MOCKS (must come first, before any src includes) ---
void addLog(String message) {}  // stub, does nothing
void sauvegarderTimestamps() {}

// --- SRC INCLUDES ---
#include "../src/config.h"
#include "../src/diagnostic.h"

// --- GLOBAL STATE (required by diagnostic.h) ---
unsigned long sullyDernierPipi = 0;
unsigned long sullyDernierCaca = 0;
unsigned long krokmouDernierPipi = 0;
unsigned long krokmouDernierCaca = 0;

// --- TESTS ---
void test_identifierChat() {
    assert(identifierChat(3.5) == "Sully");
    assert(identifierChat(7.5) == "Krokmou");
    assert(identifierChat(5.5) == "Inconnu");
    assert(identifierChat(1.0) == "Inconnu");
    assert(identifierChat(10.0) == "Inconnu");
    std::cout << "✅ identifierChat OK\n";
}

void test_diagnostic_sully_visite() {
    String d, a;
    calculerDiagnostic("Sully", 5.0, 30, d, a);
    assert(d == "Simple visite");
    assert(a == "");
    std::cout << "✅ Sully visite simple OK\n";
}

void test_diagnostic_sully_visite_longue() {
    String d, a;
    calculerDiagnostic("Sully", 5.0, 95, d, a);
    assert(d == "Simple visite");
    assert(a != "");
    std::cout << "✅ Sully visite longue OK\n";
}

void test_diagnostic_sully_pipi_normal() {
    String d, a;
    calculerDiagnostic("Sully", 20.0, 60, d, a);
    assert(d == "Pipi 🟡");
    assert(a == "");
    std::cout << "✅ Sully pipi normal OK\n";
}

void test_diagnostic_sully_petit_pipi() {
    String d, a;
    calculerDiagnostic("Sully", 20.0, 130, d, a);
    assert(d == "Petit Pipi 🟡");
    assert(a != "");
    std::cout << "✅ Sully petit pipi OK\n";
}

void test_diagnostic_sully_caca() {
    String d, a;
    calculerDiagnostic("Sully", 40.0, 100, d, a);
    assert(d == "Caca 🟤");
    std::cout << "✅ Sully caca OK\n";
}

void test_diagnostic_sully_gros_pipi() {
    String d, a;
    calculerDiagnostic("Sully", 40.0, 60, d, a);
    assert(d == "Gros Pipi 🟡");
    std::cout << "✅ Sully gros pipi OK\n";
}

void test_diagnostic_krokmou_visite() {
    String d, a;
    calculerDiagnostic("Krokmou", 10.0, 30, d, a);
    assert(d == "Simple visite");
    std::cout << "✅ Krokmou visite OK\n";
}

void test_diagnostic_krokmou_pipi() {
    String d, a;
    calculerDiagnostic("Krokmou", 50.0, 60, d, a);
    assert(d == "Pipi 🟡");
    std::cout << "✅ Krokmou pipi OK\n";
}

void test_diagnostic_krokmou_caca() {
    String d, a;
    calculerDiagnostic("Krokmou", 80.0, 100, d, a);
    assert(d == "Caca 🟤");
    std::cout << "✅ Krokmou caca OK\n";
}

void test_boundaries_sully() {
    String d, a;
    calculerDiagnostic("Sully", SULLY_VISITE_MAX, 60, d, a);
    assert(d == "Pipi 🟡");
    calculerDiagnostic("Sully", SULLY_PIPI_MAX, 60, d, a);
    assert(d == "Gros Pipi 🟡");
    std::cout << "✅ Sully boundaries OK\n";
}

void test_boundaries_krokmou() {
    String d, a;
    calculerDiagnostic("Krokmou", KROKMOU_VISITE_MAX, 60, d, a);
    assert(d == "Pipi 🟡");
    calculerDiagnostic("Krokmou", KROKMOU_PIPI_MAX, 60, d, a);
    assert(d == "Gros Pipi 🟡");
    std::cout << "✅ Krokmou boundaries OK\n";
}

void test_diagnostic_krokmou_gros_pipi_updates_pipi_timestamp() {
    sullyDernierPipi = 0;
    sullyDernierCaca = 0;
    krokmouDernierPipi = 0;
    krokmouDernierCaca = 0;
    String d, a;
    calculerDiagnostic("Krokmou", 80.0, 60, d, a);  // Gros Pipi
    assert(d == "Gros Pipi 🟡");
    assert(krokmouDernierPipi > 0);   // pipi updated ✅
    assert(krokmouDernierCaca == 0);  // caca NOT updated ✅
    std::cout << "✅ Krokmou Gros Pipi updates dernierPipi OK\n";
}

void test_diagnostic_sully_gros_pipi_updates_pipi_timestamp() {
    sullyDernierPipi = 0;
    sullyDernierCaca = 0;
    String d, a;
    calculerDiagnostic("Sully", 40.0, 60, d, a);  // Gros Pipi
    assert(d == "Gros Pipi 🟡");
    assert(sullyDernierPipi > 0);   // pipi updated ✅
    assert(sullyDernierCaca == 0);  // caca NOT updated ✅
    std::cout << "✅ Sully Gros Pipi updates dernierPipi OK\n";
}

int main() {
    test_identifierChat();
    test_diagnostic_sully_visite();
    test_diagnostic_sully_visite_longue();
    test_diagnostic_sully_pipi_normal();
    test_diagnostic_sully_petit_pipi();
    test_diagnostic_sully_caca();
    test_diagnostic_sully_gros_pipi();
    test_diagnostic_krokmou_visite();
    test_diagnostic_krokmou_pipi();
    test_diagnostic_krokmou_caca();
    test_boundaries_sully();
    test_boundaries_krokmou();
    std::cout << "\n✅ Tous les tests diagnostic passés !\n";
    return 0;
}
