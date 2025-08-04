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

# Test ESP32 validation
echo "🧪 Testing ESP32 validation..."
if python3 -m esphome config example-esp32.yaml > /dev/null 2>&1; then
    echo "✅ ESP32 configuration valid"
else
    echo "❌ ESP32 configuration invalid"
    python3 -m esphome config example-esp32.yaml
    exit 1
fi

# Compilation test ESP8266 (optional - takes time)
if [ "$1" = "--compile" ]; then
    echo "🔨 Testing ESP8266 compilation..."
    if python3 -m esphome compile example-esp8266.yaml > /dev/null 2>&1; then
        echo "✅ ESP8266 compilation successful"
    else
        echo "❌ ESP8266 compilation failed"
        exit 1
    fi

    echo "🔨 Testing ESP32 compilation..."
    if python3 -m esphome compile example-esp32.yaml > /dev/null 2>&1; then
        echo "✅ ESP32 compilation successful"
    else
        echo "❌ ESP32 compilation failed"
        exit 1
    fi
fi

echo "🎉 All tests passed successfully!"
echo ""
echo "📋 Summary:"
echo "   - ESP8266 Configuration: ✅ Valid"
echo "   - ESP32 Configuration: ✅ Valid"
if [ "$1" = "--compile" ]; then
    echo "   - ESP8266 Compilation: ✅ Successful"
    echo "   - ESP32 Compilation: ✅ Successful"
fi
echo ""
echo "🚀 HeishaMon module is ready for use!"
