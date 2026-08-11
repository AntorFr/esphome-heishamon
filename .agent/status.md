# Status — esphome-heishamon

> MàJ : 2026-08-10

**État :** Composant compatible protocole HeishaMon v4.1.6. **Mode actif EN SERVICE,
commandes SET validées sur la PAC réelle** (2026-08-11) : SetQuietMode ✅,
SetForceDHW ✅ (TOP2 confirmé), SetDHWTemp ✅ (60→61→60 confirmé par TOP9).
PAC : WH-ADC1216H6E5 (All-In-One T-CAP 12 kW, TOP92 `E2 CF 0B 75 09 12 D0 0C 05 11`).
SetZ1HeatRequestTemperature ignoré en mode « DHW only » estival (zones off) — attendu,
à revalider en saison de chauffe. Fixes de la campagne : troncature buffer commandes
(20 → 110 o), cadence optional PCB, switches pilotables, `tx_enable_pin` (GPIO5),
**mapping des numbers** (7 noms de commandes inventés → vrais noms de la table ; 9
contrôles sans équivalent protocole supprimés — ils échouaient en silence).

**☠️ Gotcha bus (résolu 2026-08-11) :** le port CZ-TAW1 de la carte HeishaMon est
**couplé électriquement au bus CN-CNT**. L'ancien HeishaMon resté branché dessus a
d'abord parasité le bus en maître (polls 5 s, collisions → nos queries timeout), puis
l'a **bloqué net** une fois passé en listen-only (driver désactivé pendu sur la ligne
→ plus aucune réponse de la PAC, même à nous). Fix : le débrancher physiquement.
Documenté dans le README (section Hardware Wiring).

**Limitation connue :** le mode DHW Eco/Powerful n'est **pas pilotable** via CN-CNT
(aucune commande upstream) — le water heater log un warning « Unknown command:
SetDHWMode » si on change le mode ; seuls OFF (SetForceDHW=0) et la consigne
(SetDHWTemp) agissent.

**Gotcha carte HeishaMon ESP32 (résolu 2026-08-10) :** premier passage en actif =
silence total de la PAC (queries émises, zéro octet reçu). Cause : le driver TX vers
le CN-CNT est **gaté par GPIO5** (`ENABLEPIN` dans le firmware upstream — HIGH = actif,
LOW = listen-only) et notre firmware ne le pilotait pas. Fix : option `tx_enable_pin`
du composant (pilotée selon `listen_only`), `tx_enable_pin: GPIO5` dans le YAML device.

**Prochaines étapes :**
- [ ] Revalider SetZ1HeatRequestTemperature (et le climate Z1) en saison de chauffe
- [ ] Implémenter le forwarding proxy vers le port CZ-TAW1 (uart2 GPIO8/9, comme l'upstream) si l'ancien HeishaMon doit resservir de moniteur
- [ ] Compléter la table de commandes depuis l'upstream (SetSmartGridMode, SetZ*RoomTemp/WaterTemp…) pour ré-offrir proprement les contrôles retirés de number.py
- [ ] Implémenter le décodage des trames « extra » (0x21) et optional PCB (0xF1) — TODO dans `on_protocol_data_received`
- [ ] Taguer une release vX.Y.Z pour épingler les devices (actuellement `ref: dev`)
