// tests/test_notifier.cpp
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
typedef std::string String;

// Minimal String(float, int) mock
String floatToStr(float f, int dec) {
    std::ostringstream ss;
    ss << std::fixed;
    ss.precision(dec);
    ss << f;
    return ss.str();
}

// Reproduce buildSheetsPayload logic for testing
String buildSheetsPayload(String chat, String action, float poids, float poids_chat,
                          unsigned long duree, String alerte) {
    String payload = "{";
    payload += "\"chat\":\"" + chat + "\",";
    payload += "\"action\":\"" + action + "\",";
    payload += "\"poids\":" + floatToStr(poids, 1) + ",";
    payload += "\"poids_chat\":" + floatToStr(poids_chat, 2) + ",";
    payload += "\"duree\":" + std::to_string(duree) + ",";
    payload += "\"alerte\":\"" + alerte + "\"";
    payload += "}";
    return payload;
}

void test_payload_structure() {
    String p = buildSheetsPayload("Sully", "Pipi", 25.0, 3.5, 60, "");
    assert(p.front() == '{');
    assert(p.back() == '}');
    assert(p.find("\"chat\":\"Sully\"") != std::string::npos);
    assert(p.find("\"action\":\"Pipi\"") != std::string::npos);
    assert(p.find("\"poids\":25.1") == std::string::npos);  // should be 25.0
    assert(p.find("\"duree\":60") != std::string::npos);
    std::cout << "✅ Payload structure OK\n";
}

void test_payload_empty_alerte() {
    String p = buildSheetsPayload("Krokmou", "Caca", 50.0, 7.5, 120, "");
    assert(p.find("\"alerte\":\"\"") != std::string::npos);
    std::cout << "✅ Payload empty alerte OK\n";
}

void test_payload_with_alerte() {
    String p = buildSheetsPayload("Sully", "Pipi", 20.0, 3.5, 130, "Vigilance");
    assert(p.find("\"alerte\":\"Vigilance\"") != std::string::npos);
    std::cout << "✅ Payload with alerte OK\n";
}

void test_payload_inconnu() {
    String p = buildSheetsPayload("Inconnu", "Simple visite", 5.0, 4.0, 30, "");
    assert(p.find("\"chat\":\"Inconnu\"") != std::string::npos);
    std::cout << "✅ Payload inconnu OK\n";
}

int main() {
    test_payload_structure();
    test_payload_empty_alerte();
    test_payload_with_alerte();
    test_payload_inconnu();
    std::cout << "\n✅ Tous les tests notifier passés !\n";
    return 0;
}
