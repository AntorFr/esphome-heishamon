#!/bin/bash
# ESPHome HeishaMon compilation test script

set -e

echo "=== ESPHome HeishaMon Compilation Test ==="

# Check if Python3 is installed
if ! command -v python3 &> /dev/null; then
    echo "❌ Python3 is not installed"
    exit 1
fi

# Setup Python virtual environment if needed
if [ ! -d ".venv" ]; then
    echo "📦 Creating Python virtual environment..."
    python3 -m venv .venv
fi

source .venv/bin/activate

# Install/update ESPHome
echo "📦 Installing/updating ESPHome..."
pip install --upgrade esphome > /dev/null 2>&1

# Create test secrets if they don't exist
if [ ! -f "secrets.yaml" ]; then
    echo "📝 Creating test secrets.yaml..."
    cat > secrets.yaml << EOF
wifi_ssid: "test_network"
wifi_password: "test_password"
EOF
fi

# Check ESPHome version
echo "📋 ESPHome version: $(python3 -m esphome version)"

# Test ESP8266 validation
echo "🧪 Testing ESP8266 validation..."
if python3 -m esphome config example-esp8266.yaml > /dev/null 2>&1; then
    echo "✅ ESP8266 configuration valid"
else
    echo "❌ ESP8266 configuration invalid"
    python3 -m esphome config example-esp8266.yaml
    exit 1
fi

# Test ESP32 validation (Arduino)
echo "🧪 Testing ESP32 validation (Arduino)..."
if python3 -m esphome config example-esp32.yaml > /dev/null 2>&1; then
    echo "✅ ESP32 configuration valid (Arduino)"
else
    echo "❌ ESP32 configuration invalid (Arduino)"
    python3 -m esphome config example-esp32.yaml
    exit 1
fi

# Test ESP32 validation (ESP-IDF)
echo "🧪 Testing ESP32 validation (ESP-IDF)..."
cp example-esp32.yaml example-esp32-espidf.yaml
# Remplace framework: type: arduino par framework: type: esp-idf
sed -i.bak 's/type: arduino/type: esp-idf/' example-esp32-espidf.yaml
if python3 -m esphome config example-esp32-espidf.yaml > /dev/null 2>&1; then
    echo "✅ ESP32 configuration valid (ESP-IDF)"
else
    echo "❌ ESP32 configuration invalid (ESP-IDF)"
    python3 -m esphome config example-esp32-espidf.yaml
    rm -f example-esp32-espidf.yaml example-esp32-espidf.yaml.bak
    exit 1
fi
rm -f example-esp32-espidf.yaml example-esp32-espidf.yaml.bak

# Compilation test ESP8266/ESP32/ESP-IDF (optional - takes time)
if [ "$1" = "--compile" ]; then
    echo "🔨 Testing ESP8266 compilation..."
    if python3 -m esphome compile example-esp8266.yaml > /dev/null 2>&1; then
        echo "✅ ESP8266 compilation successful"
    else
        echo "❌ ESP8266 compilation failed"
        exit 1
    fi

    echo "🔨 Testing ESP32 compilation (Arduino)..."
    if python3 -m esphome compile example-esp32.yaml > /dev/null 2>&1; then
        echo "✅ ESP32 compilation successful (Arduino)"
    else
        echo "❌ ESP32 compilation failed (Arduino)"
        exit 1
    fi

    echo "🔨 Testing ESP32 compilation (ESP-IDF)..."
    cp example-esp32.yaml example-esp32-espidf.yaml
    sed -i.bak 's/type: arduino/type: esp-idf/' example-esp32-espidf.yaml
    if python3 -m esphome compile example-esp32-espidf.yaml > /dev/null 2>&1; then
        echo "✅ ESP32 compilation successful (ESP-IDF)"
    else
        echo "❌ ESP32 compilation failed (ESP-IDF)"
        rm -f example-esp32-espidf.yaml example-esp32-espidf.yaml.bak
        exit 1
    fi
    rm -f example-esp32-espidf.yaml example-esp32-espidf.yaml.bak
fi

echo "🎉 All tests passed successfully!"
echo ""
echo "📋 Summary:"
echo "   - ESP8266 Configuration: ✅ Valid"
echo "   - ESP32 Configuration: ✅ Valid (Arduino)"
echo "   - ESP32 Configuration: ✅ Valid (ESP-IDF)"
if [ "$1" = "--compile" ]; then
    echo "   - ESP8266 Compilation: ✅ Successful"
    echo "   - ESP32 Compilation: ✅ Successful (Arduino)"
    echo "   - ESP32 Compilation: ✅ Successful (ESP-IDF)"
fi
echo ""
echo "🚀 HeishaMon module is ready for use!"
