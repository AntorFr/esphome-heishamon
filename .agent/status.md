# Status — esphome-heishamon

> MàJ : 2026-08-10

**État :** Composant compatible protocole HeishaMon v4.1.6. **Mode actif EN SERVICE
et vérifié** (2026-08-10 soir) : l'ESP (carte HeishaMon ESP32) est seul maître sur le
CN-CNT, la PAC répond (paquets 203 o valides, 0 erreur CRC) et les entités publient
vers HA. Repasse pré-bascule : fix troncature du buffer de commandes (20 → 110 octets),
fix cadence keep-alive optional PCB, switches pilotables (SetForceDHW, SetQuietMode…),
option `tx_enable_pin` (GPIO5). Encodages vérifiés contre l'upstream Egyras/HeishaMon.
Observation : la PAC émet un paquet toutes les ~5 s sans sollicitation (bus activé —
possiblement un équipement sur le port proxy de la carte) ; nos queries 30 s peuvent
entrer en collision bénigne (timeout isolé, le flux continue).

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
- [ ] Valider en réel les commandes SET (Force DHW, setpoints) maintenant que le mode actif est en service
- [ ] Implémenter le décodage des trames « extra » (0x21) et optional PCB (0xF1) — TODO dans `on_protocol_data_received`
- [ ] Taguer une release vX.Y.Z pour épingler les devices (actuellement `ref: dev`)
