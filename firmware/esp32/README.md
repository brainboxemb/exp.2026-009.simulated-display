# ESP32 firmware — Arduino + shared native renderer (Experiment 009)

Dit is de **embedded uitvoer** van het bestaande JavaFX/C-simulatieproject. De desktopapplicatie blijft JavaFX + CMake; **alleen de fysieke ESP32-firmware** draait nu op het Arduino-framework in PlatformIO.

## Wat wordt gedeeld?

De firmware importeert **dezelfde 12 C-bronbestanden uit `native/src`** als de desktop-CMake build. Er zijn geen aparte kopieën van:
- matrix-serial parser (`matrix_serial.c`) met pagina's, atomic `/UB`…`/UE`, kleurpalet, meldingen en sprites;
- 160×64 RGB565 framebuffer, fonts, race-/start-up-schermen;
- LVGL-schermrenderer (`matrix_race_screen.c`).

`scripts/build_shared_native.py` compileert de C-bestanden direct vanuit `native/src`. Het script leest de softwareversie uit de `pom.xml` van het hoofdproject. Wijzigingen aan de gedeelde software moeten altijd in `native/` plaatsvinden, zodat simulator en ESP32 gelijk blijven.

## PlatformIO

| Omgeving | Board |
| --- | --- |
| `esp32dev` (standaard) | Klassieke ESP32-WROOM |
| `esp32doit-devkit-v1` | DOIT ESP32 DevKit v1 |

De versie van de toolchain is gepind op `espressif32@8.0.0`, met **`framework = arduino`**. LVGL `v9.6.0` en MatrixPanel_I2S_DMA `3.0.10` zijn eveneens op Git-tags vastgezet.

Uitvoeren vanuit de **repositoryroot**:

```powershell
pio run -d firmware/esp32 -e esp32dev
pio run -d firmware/esp32 -e esp32dev -t upload
pio device monitor -b 115200
```

Of vanuit `firmware/esp32`:

```powershell
pio run -e esp32dev
pio run -e esp32dev -t upload
pio device monitor -b 115200
```

## Architectuur

```text
                        Eén gedeelde implementatie
                  native/ (C11 + LVGL 9.6 + RGB565)
                    /                         \
          desktop CMake                    PlatformIO
              |                              |
       matrix-display-host            Arduino setup()/loop()
              |                              |
       JavaFX-simulator              UART 115200 -> parser
                                             |
                                   160x64 RGB565 framebuffer
                                             |
                                   MatrixPanel_I2S_DMA::drawPixel(RGB565)
                                             |
                                  5 x HUB75 64x32 -> 160x64
```

PlatformIO compileert C- en C++-bronnen samen; de gedeelde C-library blijft **zuiver C11**. De hardware-adapter staat in `src/hub75_output.cpp` en gebruikt een kleine C-callable API (`include/hub75_output.h`). De ESP32 krijgt geen tweede protocolparser of eigen GUI-layout.

## HUB75 (bewezen op Experiment 012)

| Eigenschap | Waarde |
| --- | --- |
| Panelen | 5 × 64×32, in serie |
| Logische resolutie | **160×64** RGB565 |
| Elektrische DMA-afmetingen | 320×32 |
| Paneelmapping | `CHAIN_TOP_RIGHT_DOWN`, rotatie 90° |
| HUB75-clock | **10 MHz** |
| BCM-kleurdiepte | 6 bitplanes per kanaal |
| Helderheid bij opstarten | **24/255 (~9,4%)** |
| Displaybuffer | Eén fysieke DMA-buffer |

De GPIO-bedrading blijft gelijk aan de werkende hardwaretest:
`R1=18, G1=26, B1=27, R2=14, G2=12, B2=13, A=23, B=19, C=5, D=17, CLK=16, LAT=4, OE=15, E=-1`.

De logische mapping komt uit de in CI geteste `include/hub75_geometry.h` en stuurt **RGB565** rechtstreeks door naar `MatrixPanel_I2S_DMA::drawPixel(x, y, uint16_t)`. De driver maakt intern zelf de HUB75-bitplanes. Er is geen RGB888-framebuffer of Adafruit GFX nodig.

Let op: GPIO12 en GPIO15 zijn ESP32-strapping-pinnen. Gebruik de bestaande, bewezen bedrading en een voldoende krachtige afzonderlijke 5V-paneelvoeding met gemeenschappelijke GND.

## Serieel protocol

ESP32 UART0 werkt op **115200 baud**. De Arduino `loop()` leest bytes zonder blokkeren en geeft die door aan de identieke desktopparser. Het tick-interval voor sprites blijft **20 ms**.

Bij het opstarten verschijnt hetzelfde gedeelde opstartscherm als in de simulator, met softwareversie, display-ID en SSID-ID. Een transactioneel `/UB`…`/UE`-bericht tekent een nieuw frame atomair.

Het protocol `/XB00..99` stuurt de **OE-helderheid** van de HUB75-library aan; de RGB565-framebuffer blijft ongewijzigd. De hardware begint bewust op 24/255 totdat de eerste volledige seriële update wordt getoond.

Kopieer voor tests de inhoud van een bestand uit `examples/`, bijvoorbeeld `examples/race-screen.txt` of `examples/hello.txt`, naar de seriële monitor.

Volledige protocoldocumentatie: [docs/06-matrix-serial-protocol.md](../../docs/06-matrix-serial-protocol.md).

## Compacte LVGL-build

De bestaande `native/lv_conf.h` configureert onder andere RGB565, alleen labels, UNSCII 8 en 16 en schakelt onnodige thema's en widgets uit. De `scripts/lvgl_source_filter.py` voorkomt bovendien dat PlatformIO desondanks honderden ongebruikte LVGL-bestanden compileert. Dit is dezelfde aanpak als de succesvolle kwalificatie in Experiment 012, met de extra UNSCII-16-font die ons gedeelde C-renderpad nodig heeft.

De CI rapporteert:
- eerste build inclusief downloads;
- schone rebuild met toolchain geïnstalleerd;
- incrementele build;
- gecompileerde LVGL- en gedeelde C-bronbestanden;
- RAM/flash.

## Testgrens

CI bevestigt compilatie en geometrie; het bewijst niet het fysieke beeld, seriële timing of voldoende **vrij DMA-geheugen tijdens runtime**. De vijfpanelenopstelling en de eenvoudige LVGL-demo werkten al in Experiment 012, maar de volledige parser/renderer in Experiment 009 moet nog fysiek worden gecontroleerd.
