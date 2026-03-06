# 🐱 Cat Connected Litter

A smart cat litter box monitoring system that tracks bathroom activity, identifies individual cats, and sends health alerts via Telegram and Google Sheets.

## What It Does

Cat Connected Litter transforms a standard litter box into an IoT-enabled health monitoring system. Using precision weight sensors and machine learning-inspired diagnostics, it:

- **Detects bathroom activity** in real-time as your cats use the litter box
- **Identifies which cat** is using the box (Sully or Krokmou) based on weight
- **Categorizes activities** as simple visits, urination, or defecation
- **Sends notifications** via Telegram for important events
- **Logs data** to Google Sheets for long-term health tracking
- **Alerts on health concerns** (missed bathroom visits, unusual patterns)
- **Provides web monitoring** with live status and activity logs
- **Supports firmware updates** over-the-air without disassembly

## Why This Project Is Useful

Unexpected changes in litter box behavior can signal serious health issues in cats:
- Urinary blockages (life-threatening, especially in males)
- Kidney disease
- Diabetes
- Gastrointestinal issues

This system provides **early warning** of health problems by:
- Establishing baseline patterns for each cat
- Triggering 24h/48h health alerts when patterns break
- Creating an accessible historical log for veterinary consultations
- Eliminating guesswork about "how long since they went"

## Getting Started

### Prerequisites

- **M5Stack Atom** (ESP32-based device)
- **HX711 Weight Sensor** with load cell
- **WiFi network** access
- **PlatformIO** installed (via VS Code or CLI)
- **Telegram Bot Token** (optional, for notifications)
- **Google Sheets API credentials** (optional, for data logging)

### Hardware Setup

1. **Connect the HX711 weight sensor** to the M5Stack Atom:
   - HX711 DT (data) pin → GPIO 32 (configurable in [src/config.h](src/config.h))
   - HX711 CLK (clock) pin → GPIO 26 (configurable in [src/config.h](src/config.h))
   - Power and GND as per HX711 documentation

2. **Mount the load cell** under or integrated with your litter box

3. **Place M5Stack Atom** nearby with USB power

### Software Installation

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/cat-connected-litter.git
   cd cat-connected-litter
   ```

2. **Configure credentials:**
   - Copy [src/secrets.h](src/secrets.h) and add your WiFi SSID/password
   - Add your Telegram bot token and chat ID (get these from [@BotFather](https://t.me/botfather))
   - Add your Google Sheets webhook URL (or leave blank to disable)

3. **Build and upload:**
   ```bash
   pio run -t upload -e m5stack-atom
   ```

4. **Monitor the serial output:**
   ```bash
   pio device monitor --baud 115200
   ```

### Configuration

Key parameters in [src/config.h](src/config.h):

| Parameter | Purpose | Default |
|-----------|---------|---------|
| `CALIBRATION_SCALE` | Weight sensor calibration constant | 24.79 |
| `SEUIL_SULLY_MIN/MAX` | Weight range to identify Sully | 2.5–5.5 kg |
| `SEUIL_KROKMOU_MIN/MAX` | Weight range to identify Krokmou | 5.5–9.0 kg |
| `SULLY_PIPI_MAX` | Max weight change for Sully urination | 35g |
| `KROKMOU_PIPI_MAX` | Max weight change for Krokmou urination | 70g |
| `ALERTE_PIPI_MS` | Health alert if no urination in 24h | 86,400,000 ms |
| `ALERTE_CACA_MS` | Health alert if no defecation in 48h | 172,800,000 ms |

### Web Interface

Once running, access the web dashboard at `http://<device-ip>/`:

- **`/`** — Status dashboard with last activity times
- **`/status`** — JSON API with current state
- **`/logs`** — Real-time activity log
- **`/tare`** — Recalibrate the scale
- **`/update`** — Over-the-air firmware update

### First-Time Calibration

1. Place the scale on a level surface with litter box empty
2. Access `http://<device-ip>/tare` to zero the scale
3. Weigh each cat individually to verify detection weight ranges
4. Update `SEUIL_*_MIN/MAX` in [src/config.h](src/config.h) if needed

## Testing

Run the diagnostic tests (mock-based, no hardware required):

```bash
pio test -e native
```

Tests verify:
- Cat identification by weight
- Activity type detection (visit vs. pipi vs. caca)
- Health alert thresholds

See [tests/test_diagnostic.cpp](tests/test_diagnostic.cpp) for test details.

## How to Get Help

- **Documentation:** Check [src/](src/) for inline code comments and module descriptions
- **Issues:** [GitHub Issues](https://github.com/yourusername/cat-connected-litter/issues)
- **Configuration Help:** Review [src/config.h](src/config.h) comments for tuning guidance

## Project Structure

```
src/
  main.cpp              — Core event loop and WiFi/web server
  config.h              — Tuning parameters (thresholds, timings, calibration)
  diagnostic.h          — Activity type classification logic
  notifier.h            — Telegram and Google Sheets integration
  logger.h              — System logging utilities
  secrets.h             — WiFi and API credentials (not in git)

tests/
  test_diagnostic.cpp   — Unit tests for diagnostic logic
  mocks/                — Mock implementations for testing

platformio.ini          — Build configuration
partitions_ota.csv      — ESP32 firmware partition table for OTA updates
cat-connected-litter-scale.ino  — Original monolithic sketch (for reference)
```

## Maintainers

This project is maintained by the cat health monitoring community. For questions or contributions, please open an issue or pull request.

## Contributing

Contributions welcome! Areas of interest:
- Improved cat identification algorithms
- Support for additional notification methods
- Better health alert thresholds based on data
- Web UI enhancements

## License

See [LICENSE](LICENSE.MD) for details.
