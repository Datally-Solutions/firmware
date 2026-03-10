// tests/test_notifier.cpp
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

typedef std::string String;

// Minimal float-to-string mock
String floatToStr(float f, int dec) {
    std::ostringstream ss;
    ss << std::fixed;
    ss.precision(dec);
    ss << f;
    return ss.str();
}

// Reproduce buildGCPPayload logic for testing
String buildGCPPayload(String deviceId, float entryWeightKg, float exitWeightDeltaG,
                       unsigned long durationSeconds) {
    String payload = "{";
    payload += "\"device_id\":\"" + deviceId + "\",";
    payload += "\"entry_weight_kg\":" + floatToStr(entryWeightKg, 3) + ",";
    payload += "\"exit_weight_delta_g\":" + floatToStr(exitWeightDeltaG, 1) + ",";
    payload += "\"duration_seconds\":" + std::to_string(durationSeconds);
    payload += "}";
    return payload;
}

void test_payload_structure() {
    String p = buildGCPPayload("30251CB7B3F8", 7.2, 148.3, 143);
    assert(p.front() == '{');
    assert(p.back() == '}');
    assert(p.find("\"device_id\":\"30251CB7B3F8\"") != std::string::npos);
    assert(p.find("\"entry_weight_kg\":7.200") != std::string::npos);
    assert(p.find("\"exit_weight_delta_g\":148.3") != std::string::npos);
    assert(p.find("\"duration_seconds\":143") != std::string::npos);
    std::cout << "✅ Payload structure OK\n";
}

void test_payload_no_trailing_comma() {
    String p = buildGCPPayload("30251CB7B3F8", 7.2, 148.3, 143);
    // Last field before } should not have a comma
    size_t closing = p.rfind('}');
    assert(p[closing - 1] != ',');
    std::cout << "✅ No trailing comma OK\n";
}

void test_payload_zero_delta() {
    // Simple visit — near zero delta
    String p = buildGCPPayload("30251CB7B3F8", 3.5, 5.0, 30);
    assert(p.find("\"exit_weight_delta_g\":5.0") != std::string::npos);
    assert(p.find("\"duration_seconds\":30") != std::string::npos);
    std::cout << "✅ Payload zero delta OK\n";
}

void test_payload_negative_delta() {
    // Scale noise — negative delta
    String p = buildGCPPayload("30251CB7B3F8", 3.5, -12.0, 15);
    assert(p.find("\"exit_weight_delta_g\":-12.0") != std::string::npos);
    std::cout << "✅ Payload negative delta OK\n";
}

void test_payload_long_session() {
    // Long session — 4+ minutes
    String p = buildGCPPayload("30251CB7B3F8", 7.5, 200.0, 300);
    assert(p.find("\"duration_seconds\":300") != std::string::npos);
    std::cout << "✅ Payload long session OK\n";
}

int main() {
    test_payload_structure();
    test_payload_no_trailing_comma();
    test_payload_zero_delta();
    test_payload_negative_delta();
    test_payload_long_session();
    std::cout << "\n✅ Tous les tests notifier passés !\n";
    return 0;
}
