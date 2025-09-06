# Fix pour Heat Pump State Binary Sensor

## Problème
Le capteur binaire "Heat Pump State" affichait "inconue" (unknown) au lieu de "On/Off" alors que dans HeishaMon original il montrait correctement 1 = On.

## Cause du problème
1. **Fonction de décodage incorrecte** : La fonction `get_bit_7_and_8()` faisait `(input & 0b11) - 1`, ce qui convertissait la valeur brute 1 (On) en 0, et 0 était interprété comme "false" par le capteur binaire.

2. **Logique de callback manquante** : Les capteurs binaires s'enregistraient avec `binary_sensor_callbacks_` mais la logique de décodage n'utilisait que `sensor_callbacks_` pour tous les topics.

## Solution implementée

### 1. Correction de la fonction de décodage
```cpp
float HeishamonComponent::get_bit_7_and_8(uint8_t input) {
  // For heat pump state: bits 7&8 (0-1) represent the state
  // Original HeishaMon: 1 = On, 0 = Off
  // Return raw bits value without subtracting 1
  return (input & 0b11);
}
```

### 2. Ajout de la logique pour capteurs binaires
```cpp
// Call callback if registered - check if it's a binary sensor first
auto binary_it = this->binary_sensor_callbacks_.find(topic.name);
if (binary_it != this->binary_sensor_callbacks_.end()) {
  // This is a binary sensor - convert float value to boolean
  bool bool_value = (value > 0.5f);  // Convert float to bool (> 0.5 = true)
  ESP_LOGD(TAG, "Calling binary sensor callback for %s with value %s (from %.1f)", 
           topic.name.c_str(), bool_value ? "true" : "false", value);
  binary_it->second(bool_value);
} else {
  // Check for regular sensor callback
  auto it = this->sensor_callbacks_.find(topic.name);
  if (it != this->sensor_callbacks_.end()) {
    ESP_LOGD(TAG, "Calling sensor callback for %s with value %.1f", topic.name.c_str(), value);
    it->second(value);
  }
}
```

### 3. Ajout de logs de debug
```cpp
} else if (topic.name == "heatpump_state") {
  ESP_LOGD(TAG, "Heat Pump State: byte_index=%d, raw_byte=0x%02X, decoded_value=%.1f", 
           topic.byte_index, data[topic.byte_index], value);
}
```

## Résultat attendu
- Le capteur binaire "Heat Pump State" devrait maintenant afficher "On" quand la pompe à chaleur fonctionne et "Off" quand elle est arrêtée
- Les logs de debug devraient montrer la valeur brute et décodée pour le débogage

## Test
Pour tester, déployez la configuration mise à jour et vérifiez :
1. Que le capteur binaire "Heat Pump State" affiche "On" ou "Off" au lieu de "inconue"
2. Les logs de debug pour voir les valeurs brutes et décodées
