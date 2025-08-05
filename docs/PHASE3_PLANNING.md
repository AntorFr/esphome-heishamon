# Phase 3 Planning - Advanced Climate & Number Entities

## Objectif Phase 3
Implémenter les entités **Number** et les fonctionnalités **Climate avancées** pour atteindre ~80% de parité avec le module Home Assistant.

## Analyse Gap Restant

### Progression Actuelle
- **Phase 0** : ~11 entités (original)
- **Phase 1** : +19 entités = ~30 total (sensors avancés)
- **Phase 2** : +32 entités = **~62 total** (controls & selects)
- **Objectif HA** : ~130 entités

### Gap Restant : ~68 entités

## Phase 3 Scope - Number Entities

### Priorité 1 : Temperature Controls
```yaml
number:
  # Zone temperature controls
  - platform: heishamon
    type: "z1_heat_target_temp"
    name: "Zone 1 Heat Target Temperature"
    min_value: 15
    max_value: 30
    step: 0.5
    unit_of_measurement: "°C"
    
  - platform: heishamon
    type: "z2_heat_target_temp" 
    name: "Zone 2 Heat Target Temperature"
    
  - platform: heishamon
    type: "z1_cool_target_temp"
    name: "Zone 1 Cool Target Temperature"
    min_value: 15
    max_value: 30
    
  # DHW temperature control
  - platform: heishamon
    type: "dhw_target_temp"
    name: "DHW Target Temperature"
    min_value: 40
    max_value: 65
    
  # Offset controls
  - platform: heishamon
    type: "room_temp_offset_z1"
    name: "Room Temperature Offset Zone 1"
    min_value: -5
    max_value: 5
    step: 0.1
```

### Priorité 2 : Flow Controls
```yaml
number:
  # Flow rate controls
  - platform: heishamon
    type: "z1_water_flow"
    name: "Zone 1 Water Flow Rate"
    min_value: 5
    max_value: 25
    unit_of_measurement: "L/min"
    
  # Pump speed controls  
  - platform: heishamon
    type: "pump_speed"
    name: "Pump Speed"
    min_value: 30
    max_value: 100
    unit_of_measurement: "%"
```

### Priorité 3 : Timing Controls
```yaml
number:
  # Schedule timers
  - platform: heishamon
    type: "quiet_mode_start_time"
    name: "Quiet Mode Start Time"
    min_value: 0
    max_value: 23
    unit_of_measurement: "h"
    
  - platform: heishamon
    type: "dhw_boost_time"
    name: "DHW Boost Time"
    min_value: 10
    max_value: 120
    unit_of_measurement: "min"
```

## Phase 3 Scope - Climate Advanced

### Enhanced Climate Features
```yaml
climate:
  # Advanced multi-zone with curve control
  - platform: heishamon
    name: "Zone 1 Advanced"
    zone_id: 1
    supports_heat: true
    supports_cool: true
    # New Phase 3 features:
    heating_curve_support: true
    external_sensor_support: true
    schedule_support: true
    
  # DHW Climate control (if ESPHome supports)
  - platform: heishamon
    name: "Domestic Hot Water"
    type: "dhw"
    min_temp: 40
    max_temp: 65
```

### Climate Modes Extensions
- **Heating curves** : Automatic temperature compensation
- **Schedule integration** : Time-based temperature control  
- **External sensors** : Room temperature override
- **Multi-zone coordination** : Synchronized heating/cooling

## Implementation Plan

### Étape 1 : Number Component Creation
1. **number.py** : Configuration schema et validation
2. **number.h** : C++ class definitions
3. **number.cpp** : Implementation avec command mapping
4. **heishamon.cpp** : Extension pour number commands

### Étape 2 : Climate Enhancements  
1. **climate.py** : Extensions advanced features
2. **climate.cpp** : Nouvelles fonctionnalités
3. **Heating curves** : Algorithmes compensation
4. **Schedule support** : Intégration temporal

### Étape 3 : Protocol Extensions
1. **Command mapping** : Number value → HeishaMon commands
2. **Range validation** : Min/max par type de number
3. **Step control** : Precision des ajustements
4. **Unit handling** : Gestion unités de mesure

## Technical Architecture

### Number Entity Structure
```cpp
class HeishamonNumber : public number::Number, public Component {
  public:
    void set_number_type(const std::string &type);
    void set_min_value(float min);
    void set_max_value(float max);
    void set_step(float step);
    
  protected:
    void control(float value) override;
    std::string number_type_;
    float min_value_, max_value_, step_;
};
```

### Command Mapping Strategy
```cpp
// Temperature commands
case "z1_heat_target_temp": return format_temp_command(0x30, value);
case "dhw_target_temp": return format_temp_command(0x42, value);

// Flow commands  
case "pump_speed": return format_percent_command(0x50, value);

// Time commands
case "quiet_mode_start": return format_time_command(0x60, value);
```

## Expected Outcomes

### Nouvelles Entités Phase 3
- **Number entities** : ~15-20 contrôles précis
- **Climate enhanced** : 4-6 thermostats avancés
- **Total ajouté** : ~20-25 entités

### Progression Post-Phase 3
- **Entités totales** : ~82-87 (vs ~62 actuel)
- **Parité HA** : **~65-67%** (vs 47% actuel)
- **Gap restant** : ~43-48 entités pour Phase 4

## Configuration Examples

### ESP32 (Full Phase 3)
```yaml
# Temperature controls
number:
  - platform: heishamon
    type: "z1_heat_target_temp"
    name: "Zone 1 Heat Target"
    min_value: 15
    max_value: 30
    step: 0.5

# Advanced climate  
climate:
  - platform: heishamon
    name: "Zone 1 Advanced"
    zone_id: 1
    heating_curve_support: true
    schedule_support: true
```

### ESP8266 (Essential Phase 3)
```yaml
# Essential number controls only
number:
  - platform: heishamon
    type: "dhw_target_temp"
    name: "DHW Target Temperature"
    min_value: 40
    max_value: 65

# Basic enhanced climate
climate:
  - platform: heishamon
    name: "Zone 1 Enhanced"
    zone_id: 1
```

## Success Criteria

1. **Number entities** : 15+ contrôles opérationnels
2. **Climate advanced** : Heating curves fonctionnelles
3. **ESP32** : Configuration complète validée
4. **ESP8266** : Configuration essentielle validée
5. **HA Integration** : Interface number native
6. **Documentation** : Guides utilisateur complets

## Ready to Start? 🚀

Veux-tu commencer par :
1. **Number entities** (plus direct, impact immédiat)
2. **Climate advanced** (plus complexe, fonctionnalités sophistiquées)
3. **Les deux simultanément** (développement parallèle)

Quelle approche préfères-tu ? 🤔
