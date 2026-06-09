# Firmware řídící jednotky AF-23
Tento repozitář obsahuje firmware pro řídící jednotku formule AF-23.
Firmware je určen pro Raspberry Pi Pico W.

## Sestavení
1. Otevřete Visual Studio Code, s nainstalovaným rozšířením "PlatformIO"
2. Otevřete složku s projektem
3. Dole se nachází tlačítko "Switch PlatformIO Project Environment", klikněte na něj
4. Zvolte variantu Pico W
5. Sestavit lze tlačítkem "Build", případně nahrát pomocí "Upload"

Pokud tlačítko "Upload" nefunguje (Error 1), může to být špatným ovladačem
K nainstalování se používá program Zadig:
https://arduino-pico.readthedocs.io/en/latest/install.html (V sekci "Windows 7 Driver Notes")

Po připojení Pi Pico v BOOTSEL režimu, najděte v programu "RP2 Boot (Interface 1)" a nainstalujte ovladač WinUSB

## Vzdálený update
1. Zapnout formuli/řídící jednotku
2. Je třeba se připojit k síti AF_23_Update (heslo 12345678)
3. Nastavit ve Windows síť jako privátní
4. Zapsat si adresu výchozí brány (pravděpodobně 192.168.42.1)
5. Spustit tento příkaz v cmd (musí být nainstalován python a ve složce otevřené v cmd musí být espota skript)
python3 espota.py -i 192.168.42.1 -p 2241 -f firmware.bin
(parametry jsou popsány uvnitř skriptu)
6. Pokud skript nevypíše chybu, úspěšně se nahrál
7. Po nahrání by měla nahlas cvaknout a zhasnout jednotka motoru (vypne se kvůli restartu řídící jednotky), po několika vteřinách by se opět měla zapnout a rozsvít.

## Autoři
Autory programu jsou Patrik Švoma a Jakub Aldorf.

## Licence
Tento program je poskytován pod licencí [MIT](LICENSE).