# Status — esphome-heishamon

> MàJ : 2026-08-10

**État :** Composant compatible protocole HeishaMon v4.1.6. Passage en **mode actif**
(2026-08-10) : l'ESP est câblé seul sur le CN-CNT et interroge la PAC directement
(le HeishaMon d'origine est retiré du bus). Repasse pré-bascule faite : fix de la
troncature du buffer de commandes (20 → 110 octets), fix de la cadence du keep-alive
optional PCB, switches désormais pilotables (SetForceDHW, SetQuietMode…). Encodages
vérifiés contre l'upstream Egyras/HeishaMon.

**Limitation connue :** le mode DHW Eco/Powerful n'est **pas pilotable** via CN-CNT
(aucune commande upstream) — le water heater log un warning « Unknown command:
SetDHWMode » si on change le mode ; seuls OFF (SetForceDHW=0) et la consigne
(SetDHWTemp) agissent.

**⚠️ Bloquant matériel (2026-08-10 soir) :** firmware actif flashé et fonctionnel
(queries émises toutes les 30 s), mais la PAC ne répond **pas** — zéro octet reçu,
timeout à chaque cycle. Côté soft tout est vérifié (paquets identiques à l'upstream,
checksum, UART 9600 8E1 GPIO17/18). À vérifier côté câblage : TX/RX croisés ?
interface 5 V (le level-shifter du HeishaMon a disparu du bus avec lui) ? GND commun ?

**Prochaines étapes :**
- [ ] Débloquer la liaison CN-CNT (câblage/interface 5 V) — la PAC ne répond pas aux queries
- [ ] Valider en réel les commandes SET (Force DHW, setpoints) une fois la liaison rétablie
- [ ] Implémenter le décodage des trames « extra » (0x21) et optional PCB (0xF1) — TODO dans `on_protocol_data_received`
- [ ] Taguer une release vX.Y.Z pour épingler les devices (actuellement `ref: dev`)
