# 🚰 ESPHome HeishaMon v0.2.0-beta2 - Water Heater Edition

## 🎊 Major Release: Water Heater Production Ready!

### ✅ What's New in Beta 2

#### 🚰 Water Heater Climate Entity
- **Fully operational** Water Heater as native climate entity
- **Modes**: OFF (arrêt), HEAT (manuel), AUTO (programmé)  
- **Temperature control**: 40-65°C avec validation automatique
- **Real-time feedback**: État chauffe, températures, mode actuel

#### 🎛️ Enhanced Climate Platform
- Extended climate schema supporting `type: water_heater`
- Conditional routing between zone thermostats and water heater
- Seamless integration with existing climate entities

#### 📊 Total Entities: ~80 (61% Home Assistant Parity)
- **Phase 1**: Advanced sensors (+19 entities)
- **Phase 2**: Controls & binary sensors (+32 entities)  
- **Phase 3**: Number entities (+16 entities)
- **Phase 4**: Water Heater climate (+1 unified entity)

### 🔧 Technical Improvements

#### Water Heater Infrastructure
- `water_heater.py`: Climate-based configuration schema
- `water_heater.h/cpp`: Complete C++ implementation
- Extended heishamon core with DHW state tracking
- Native ESPHome climate integration

#### Schema Modernization  
- Fixed deprecated `climate.CLIMATE_SCHEMA` warnings
- Migration to `climate.climate_schema()` for ESPHome 2025.7+ compatibility
- Enhanced validation and error handling

### 📱 Platform Support

#### ESP32 (Full Configuration)
```yaml
climate:
  # Zone thermostats (4 entities)
  - platform: heishamon
    name: "Zone 1 Heating" 
    zone_id: 1
    supports_heat: true
    
  # Water Heater (NEW!)
  - platform: heishamon
    type: water_heater
    name: "DHW Water Heater"
```

#### ESP8266 (Memory-Optimized)
```yaml
climate:
  # Essential thermostats (2 entities)
  - platform: heishamon
    name: "Zone 1 Heating"
    zone_id: 1
    
  # Water Heater (Essential)  
  - platform: heishamon
    type: water_heater
    name: "DHW Water Heater"
```

### 🏠 Home Assistant Integration

#### Native Climate Entity
- **Entity ID**: `climate.dhw_water_heater`
- **Interface**: Standard HA thermostat card
- **Controls**: Mode selector + temperature slider (40-65°C)
- **Status**: Current temp, target temp, heating action

#### Unified DHW Control
- **Before**: Separate Number entity (dhw_target_temp) + Switch entities
- **After**: Single climate entity with integrated mode + temperature control
- **Benefits**: Intuitive interface, automatic validation, real-time feedback

### ✅ Production Ready

#### Validation Complete
- ✅ ESP32 configuration validated
- ✅ ESP8266 configuration validated  
- ✅ Both platforms compile successfully
- ✅ All test cases pass

#### Quality Assurance
```bash
./test-compilation.sh
# ✅ ESP8266 configuration valid
# ✅ ESP32 configuration valid
# 🎉 All tests passed successfully!
```

### 🚀 Ready for Real-World Testing

#### What You Can Do Now
1. **Flash ESP32/ESP8266** with example configurations
2. **Test Water Heater** climate entity in Home Assistant
3. **Control DHW** via native thermostat interface
4. **Monitor real-time** temperature and heating status

#### Configuration Files
- `example-esp32.yaml`: Full feature demonstration
- `example-esp8266.yaml`: Memory-optimized essential features
- Both include activated Water Heater for immediate testing

### 📈 Roadmap to 1.0

#### Next: Phase 5 - Advanced Features
- Climate advanced: Heating curves, schedules
- Energy management: Detailed consumption monitoring  
- Smart features: Automation, tariff integration
- Diagnostic tools: Maintenance monitoring

#### Target: 90%+ Home Assistant Parity
- Current: 61% (~80 entities)
- Goal: ~120+ entities matching HA module functionality
- Focus: Advanced controls, diagnostics, automation

### 🙏 Credits & Thanks

Based on the original [HeishaMon](https://github.com/Egyras/HeishaMon) project by Egyras.

### 📥 Installation

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/AntorFr/esphome-heishamon
      ref: v0.2.0-beta2
    components: [heishamon]
```

**🎊 Ready to revolutionize your heat pump control with unified Water Heater management! 🎊**
