# Experiment 009 — Simulated LED displays

A Proof of Principle for testing **native C display software** through a **Java 21 / JavaFX** desktop simulator, before using the same portable core on an ESP32.

Status: **active — design and LCD vertical slice**. No end-to-end simulator is implemented or qualified yet.

- [Cross-project experiment record](https://github.com/brainboxemb/brainboxemb.meta/blob/main/experiments/009-simulated-display/README.md)
- [Meta tracking issue #188](https://github.com/brainboxemb/brainboxemb.meta/issues/188)
- [Implementation issue #1](https://github.com/brainboxemb/exp.2026-009.simulated-display/issues/1)

## Reference displays

| Mode | Pixels | Output | Interpretation |
| --- | --- | --- | --- |
| Serial LED | 90 × 35 | Red monochrome LEDs | SparkFun Serial Graphic LCD-compatible bytes; 5×7 font in 6×8 character cells |
| HUB75 | 160 × 64 (five panels) | RGB LEDs | Synchronized display data with native C application interpreter |

The SparkFun command set is a protocol reference, not a claim that the simulated red LED hardware physically matches a SparkFun LCD. Exact supported commands and deviations must be verified.

## Architecture

```text
serial bytes / synchronized display data
                 |
        native C interpretation
                 |
       C drawing and font renderer
                 |
            framebuffer
                 |
        platform output port
           /             \
     host endpoint     ESP-IDF adapter
           |
     JavaFX simulator
```

JavaFX renders native pixels and supplies development controls; it must not reimplement serial command parsing or font rasterization. The portable C core should not depend on Java, JavaFX, libuv, ESP-IDF or Arduino. LVGL is an optional later HUB75 experiment, not a baseline requirement.

## Development baseline

- **IDE:** Apache NetBeans.
- **Java UI:** Java 21, JavaFX 21, Maven.
- **Native core and host:** C11, CMake and CTest.
- **Embedded follow-up:** ESP-IDF without Arduino; choose an appropriate HUB75 driver after qualification.
- **Transport:** a minimal framed host process connection; establish the actual wire format in the design before implementation.

See [the plan](docs/01-plan.md) and [architecture](docs/02-design.md). Build/Run instructions will be added with the first executable slice, not invented in advance.
