# Modes de fonctionnement HeishaMon ESPHome

## Mode Passif (Listen Only) - CN-NMODE

**Configuration:** `listen_only: true`

**Utilisation:** 
- Connexion au port CN-NMODE de la pompe à chaleur
- Coexistence avec le module CZ-TAW1 original de Panasonic
- Surveillance passive des communications existantes

**Fonctionnalités:**
- ✅ Lecture de tous les capteurs (température, état, etc.)
- ✅ Monitoring passif des communications
- ❌ Pas de contrôle possible (climate, number, switch)
- ❌ Pas d'envoi de commandes
- ❌ Pas de requêtes actives

**Fichier de configuration:** `example-esp32s3-debug.yaml`

## Mode Actif - CN-CNT

**Configuration:** `listen_only: false`

**Utilisation:**
- Connexion au port CN-CNT de la pompe à chaleur  
- Remplacement complet du module CZ-TAW1 (incompatible avec CZ-TAW1)
- Communication bidirectionnelle complète

**Fonctionnalités:**
- ✅ Lecture de tous les capteurs
- ✅ Contrôle complet via climate component
- ✅ Réglage des températures via number components
- ✅ Commandes marche/arrêt via switch components
- ✅ Envoi de requêtes d'initialisation et périodiques
- ✅ Support des commandes optionnelles PCB

**Fichier de configuration:** `example-esp32s3-active.yaml`

## Migration Listen Only → Actif

Pour passer du mode passif au mode actif :

1. **Déconnecter physiquement :** 
   - Débrancher le HeishaMon du port CN-NMODE
   - Reconnecter au port CN-CNT

2. **Modifier la configuration :**
   ```yaml
   heishamon:
     listen_only: false  # Passer en mode actif
   ```

3. **Ajouter les composants de contrôle :**
   - Climate component pour le contrôle global
   - Number components pour les réglages de température
   - Switch components pour les commandes marche/arrêt

4. **Retirer le module CZ-TAW1 :**
   - Le mode actif est incompatible avec CZ-TAW1
   - HeishaMon remplace complètement les fonctionnalités CZ-TAW1

## Pins utilisées (HeishaMon v5)

**UART Pompe à chaleur :**
- TX: GPIO17 (vers pompe à chaleur)
- RX: GPIO18 (depuis pompe à chaleur)

**Ethernet W5500 :**
- CLK: GPIO12
- MOSI: GPIO11  
- MISO: GPIO13
- CS: GPIO10
- IRQ: GPIO15
- RST: GPIO14

**Relais :**
- Relais 1: GPIO21
- Relais 2: GPIO47

**LED Status :**
- WS2812: GPIO42

## Protocole de communication

**Paramètres UART :** 9600 baud, EVEN parity, 1 stop bit

**Mode Passif :**
- Écoute les communications existantes entre CZ-TAW1 et pompe à chaleur
- Aucune requête envoyée

**Mode Actif :**
- Requête d'initialisation : `0x31 0x05 0x10 0x01 0x00 0x00 0x00`
- Requête périodique : `0x71 0x6c 0x01 0x10 + données`
- Requête PCB optionnelle : `0xF1 0x11 0x01 0x50 + données`
