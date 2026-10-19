# vesc-esp01-wifi-bridge

**The cheapest way we know of to talk to your VESC wirelessly.** A ~€1–2 ESP-01 (ESP8266) module turns your VESC's UART into a WiFi link, so VESC Tool on your phone or laptop can read live data and change settings with no USB cable and no €20 Bluetooth module.

- Live real-time data, trip stats and full configuration from the VESC Tool phone app.
- About €3 in total, including the regulator, capacitor and resistors.
- Lets you seal the ESC away from the weather. You don't need to reach its USB port.
- Tested on a Flipsky FSESC 6.6 (VESC 6) on an e-bike, with VESC Tool on Android over TCP. Solid connection while riding.

> **Do not upload VESC firmware over this link.** A WiFi dropout during a firmware update can brick your ESC. Use USB for firmware updates.

## How it works

The ESP-01 runs its own WiFi access point and a TCP server on port 65102, VESC Tool's default. Every byte from VESC Tool goes unchanged to the VESC's UART, and every byte back goes to VESC Tool. The VESC can't tell the difference from a cable.

## Parts

| Part | Notes |
|---|---|
| ESP-01 or ESP-01S module | ESP8266, 1 MB flash |
| AMS1117 3.3 V regulator module | Powers the ESP-01 from the VESC's 5 V |
| 220 µF electrolytic capacitor, 10 V or more | Optional. At the ESP-01's VCC/GND pins, for WiFi current peaks |
| 2 × 1 kΩ resistors | Optional. In series with TX and RX. They protect both sides if TX and RX get swapped |
| USB-serial adapter (3.3 V) | Only for flashing, e.g. FT232RL, CH340 or CP2102 |

The bridge works without the capacitor and the resistors; it was first tested on the bike with plain wires. They're cheap insurance: the resistors make a swapped TX/RX harmless, and the capacitor smooths the supply when the WiFi transmits.

## Wiring

![Wiring diagram](docs/wiring.svg)

| VESC COMM port | ESP-01 |
|---|---|
| 5V | AMS1117 input; the AMS1117 3.3 V output goes to ESP-01 **VCC** and **CH_PD** |
| GND | GND (also the AMS1117 GND) |
| TX | **RX**, through 1 kΩ |
| RX | **TX**, through 1 kΩ |

- **Power the ESP-01 from 5 V through the regulator, not from the VESC's 3.3 V pin.** The ESP-01 draws peaks of about 250–300 mA when transmitting. The VESC's 3.3 V rail also feeds its own processor and your throttle, and a dip there can cause throttle jitter or an ESC reset.
- **CH_PD (EN) must be at 3.3 V,** or the ESP-01 stays switched off. The ESP-01S has a pull-up on the board, but the original ESP-01 often doesn't.
- **On VESC 6-style COMM ports** (such as the FSESC 6.6: 5V, 3.3V, GND, ADC1, RX, TX, ADC2, ADC3), an ADC throttle can stay on the same port. Check your own ESC's pinout, since connectors differ between models.

### Finding the ESP-01 pins

The ESP-01 has no pin labels. The header is two rows of four:

```
row A:  GND    GPIO2   GPIO0   RX
row B:  TX     CH_PD   RST     VCC
```

GND and VCC are at diagonally opposite corners. To find GND with a multimeter, look at the small 8-pin flash chip: its pin 4 is GND and pin 8 is VCC (pin 1 is marked by a dot). The header pin that beeps to flash pin 4 is GND, and the rest follow from the layout above.

## VESC Tool settings

1. **App Settings > General > App to Use:** `UART`. If you also use an ADC throttle, choose `ADC and UART`, and the throttle keeps working.
2. **App Settings > UART > Baudrate:** `115200` (must match `VESC_BAUD`).
3. Write the app configuration.

## Build and flash

1. Install [PlatformIO](https://platformio.org/).
2. **Edit `platformio.ini`:** set `WIFI_SSID` and **your own `WIFI_PASS`** (8–63 characters). The build refuses to compile while the password is still `CHANGE_ME`. Anyone who knows the password can change your ESC's settings, including current limits, so use a strong one.
3. Wire the ESP-01 to a 3.3 V USB-serial adapter: adapter TX to ESP RX, adapter RX to ESP TX, GND shared, VCC and CH_PD to 3.3 V, **GPIO0 to GND**.
4. Power up the ESP-01 with GPIO0 on GND, then run:

```
pio run -t upload
```

5. Remove the GPIO0-to-GND link and power-cycle.

### Flashing tips

- **Don't power the ESP-01 from the adapter's 3.3 V pin.** It usually can't supply the current, and flashing fails or the adapter resets. Use a separate 3.3 V supply (a regulator, or a Li-ion cell through a silicon diode such as a 1N4007) with GND shared.
- **"No serial data received":** GPIO0 wasn't on GND when the ESP-01 powered up, CH_PD isn't high, or TX and RX aren't crossed.
- **macOS doesn't show a serial port for an FT232RL adapter:** some counterfeit FT232R chips report product ID `0x0000`, and macOS ignores them. Their product ID can be restored to `0x6001` with [pyftdi](https://eblot.github.io/pyftdi/).

## Connect

1. Join the WiFi network you set in `WIFI_SSID`. Your phone may warn about no internet access; stay connected.
2. In VESC Tool: **Connection > TCP**, address `192.168.4.1`, port `65102`.

## Configuration

All settings are build flags in `platformio.ini`:

| Flag | Default | Meaning |
|---|---|---|
| `WIFI_SSID` | `VESC-Bridge` | Network name |
| `WIFI_PASS` | `CHANGE_ME` (refused) | WPA2 password, 8–63 characters |
| `WIFI_TX_DBM` | `10.0` | Transmit power, 0–20.5 dBm. Lower means smaller current peaks; 10 dBm reaches a phone on the handlebar easily. |
| `VESC_BAUD` | `115200` | UART baud rate, must match VESC Tool |
| `TCP_PORT` | `65102` | VESC Tool's default TCP port |

## Related

- [Vesc_WIFI](https://github.com/NetworkDir/Vesc_WIFI): the same idea for the ESP32.

## Licence

MIT. See [LICENSE](LICENSE).
