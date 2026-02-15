# HeishaMon Water Heater Component

Le composant Water Heater fournit un contrôle complet de l'eau chaude sanitaire (DHW - Domestic Hot Water) de votre pompe à chaleur Panasonic via HeishaMon.

## Configuration

```yaml
water_heater:
  - platform: heishamon
    heishamon_id: heisha_main
    name: "Heat Pump Water Heater"
    id: my_water_heater
```

### Options de configuration

- **heishamon_id** (*Requis*, ID): L'ID du composant HeishaMon parent.
- **name** (*Optionnel*, string): Le nom du chauffe-eau. Par défaut: `"HeishaMon Water Heater"`
- **id** (*Optionnel*, ID): Définit un ID pour ce composant.
- Toutes les autres options du [composant Water Heater de base](https://esphome.io/components/water_heater/index.html).

## Fonctionnalités

### Modes de fonctionnement

Le chauffe-eau prend en charge les modes suivants :

- **OFF** : Désactive la production d'eau chaude sanitaire
- **HEAT_PUMP** : Mode pompe à chaleur standard (mode Normal Panasonic)
- **ECO** : Mode économique - réduit la consommation d'énergie
- **PERFORMANCE** : Mode performance haute température (mode Powerful Panasonic)

> **Note** : Ces modes correspondent aux modes DHW natifs de la pompe à chaleur Panasonic : Normal → HEAT_PUMP, Eco → ECO, Powerful → PERFORMANCE.

### Contrôle de température

- **Plage de température** : 40°C à 75°C
- **Pas de réglage** : 1°C
- **Température actuelle** : Affiche la température mesurée du ballon DHW
- **Température cible** : Permet de définir la température souhaitée

### États automatiques

Le composant met automatiquement à jour ses états en fonction des données reçues du HeishaMon :

- **Température actuelle** : Mise à jour automatique depuis le capteur DHW
- **Température cible** : Synchronisée avec les réglages de la pompe à chaleur
- **Mode de fonctionnement** : Reflète l'état du mode DHW de la pompe à chaleur

## Actions

### `water_heater.set_target_temperature`

Définit la température cible du chauffe-eau.

```yaml
# Dans une automation
then:
  - water_heater.set_target_temperature:
      id: my_water_heater
      temperature: 50
```

### `water_heater.set_operation_mode`

Change le mode de fonctionnement du chauffe-eau.

```yaml
# Dans une automation
then:
  - water_heater.set_operation_mode:
      id: my_water_heater
      operation_mode: ECO
```

## Automatisations

### Exemple : Programmation horaire

```yaml
# Automation pour mode éco la nuit
automation:
  - id: dhw_night_eco
    trigger:
      - platform: time
        at: "22:00:00"
    action:
      - water_heater.set_operation_mode:
          id: my_water_heater
          operation_mode: ECO
          
  - id: dhw_day_performance
    trigger:
      - platform: time
        at: "06:00:00"
    action:
      - water_heater.set_operation_mode:
          id: my_water_heater
          operation_mode: PERFORMANCE
```

### Exemple : Contrôle basé sur la consommation

```yaml
# Désactiver DHW si consommation trop élevée
automation:
  - id: dhw_power_control
    trigger:
      - platform: numeric_state
        entity_id: sensor.heat_power_consumption
        above: 3000  # Watts
    action:
      - water_heater.set_operation_mode:
          id: my_water_heater
          operation_mode: "OFF"
    condition:
      - water_heater.is_action:
          id: my_water_heater
          action: HEATING
```

## Capteurs associés

Pour une surveillance complète, vous pouvez également ajouter ces capteurs HeishaMon :

```yaml
sensor:
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "dhw_temp"
    name: "DHW Temperature"
    device_class: temperature
    unit_of_measurement: "°C"
    
  - platform: heishamon
    heishamon_id: heisha_main
    topic: "dhw_target_temp"
    name: "DHW Target Temperature"
    device_class: temperature
    unit_of_measurement: "°C"
```

## Intégration Home Assistant

Une fois configuré, le chauffe-eau apparaîtra automatiquement dans Home Assistant avec :

- Une carte de contrôle de température
- Sélection des modes de fonctionnement
- Affichage de l'état actuel (chauffage, inactif, etc.)
- Graphiques de température historiques

### Carte Lovelace exemple

```yaml
type: thermostat
entity: water_heater.heat_pump_water_heater
name: Eau Chaude Sanitaire
```

## Dépannage

### Le chauffe-eau n'apparaît pas

1. Vérifiez que le composant HeishaMon principal fonctionne
2. Vérifiez les logs ESPHome pour les erreurs
3. Assurez-vous que la communication UART est stable

### Les commandes ne fonctionnent pas

1. Vérifiez que `listen_only: false` dans la configuration HeishaMon
2. Vérifiez que la pompe à chaleur accepte les commandes externes
3. Consultez les logs pour les erreurs de communication

### Températures incorrectes

1. Vérifiez la calibration de la sonde DHW de la pompe à chaleur
2. Attendez quelques cycles de mise à jour (30s par défaut)
3. Comparez avec l'affichage de la pompe à chaleur

## Notes

- Le composant utilise les protocoles HeishaMon standard pour communiquer avec la pompe à chaleur
- Les changements de température peuvent prendre quelques minutes à prendre effet
- En mode ECO, la température est automatiquement réduite pour économiser l'énergie
- Le mode PERFORMANCE optimise la vitesse de chauffe au détriment de la consommation
