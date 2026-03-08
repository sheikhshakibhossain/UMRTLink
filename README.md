# UMRTLink

UMRTLink (Universal Modular Rover Telemetry Link) is a minimal and extremely fast UART binary communication protocol for robotics command paths.

It is designed for low-latency control between:

- Raspberry Pi (Linux companion computer)
- Microcontroller (Arduino, ESP32, Teensy, STM32, and similar)

Primary use case: sending motor and steering commands with deterministic parsing and very low CPU overhead.

## Design Philosophy

- Fixed packet size for deterministic parsing
- Binary fields for low overhead and small bandwidth footprint
- Byte-by-byte state machine decoding on microcontrollers
- Lightweight integrity protection via XOR checksum
- Extensible message IDs for future protocol growth

## Protocol Summary

- Transport: UART
- Encoding: Binary
- Version: v1
- Packet length: 10 bytes (fixed)

### Packet Diagram

```text
Byte:  0    1    2    3    4    5    6    7    8    9
	+----+----+----+----+----+----+----+----+----+----+
	| AA | ID | L0 | L1 | R0 | R1 | S0 | S1 | CS | 55 |
	+----+----+----+----+----+----+----+----+----+----+
```

### Packet Structure

| Byte(s) | Field | Type | Notes |
|---|---|---|---|
| 0 | Start | `uint8` | Constant `0xAA` |
| 1 | Message ID | `uint8` | `0x01` motor command (implemented in v1) |
| 2-3 | Left PWM | `uint16 LE` | Typical range 1000..2000 |
| 4-5 | Right PWM | `uint16 LE` | Typical range 1000..2000 |
| 6-7 | Steering PWM | `uint16 LE` | Typical range 1000..2000 |
| 8 | Checksum | `uint8` | XOR of bytes 1..7 |
| 9 | End | `uint8` | Constant `0x55` |

PWM semantics:

- 1000 -> full reverse
- 1500 -> neutral
- 2000 -> full forward

## Message IDs

| ID | Meaning | v1 Status |
|---|---|---|
| `0x01` | Motor command | Implemented |
| `0x02` | Telemetry | Reserved |
| `0x03` | Heartbeat | Reserved |
| `0x04` | Emergency stop | Reserved |
| `0x05` | Diagnostics | Reserved |

## Checksum

Checksum is computed as:

```text
checksum = b1 ^ b2 ^ b3 ^ b4 ^ b5 ^ b6 ^ b7
```

Where `b1..b7` are packet bytes 1 through 7.

## Example Packet (Decoded)

Given bytes:

```text
AA 01 40 06 40 06 DC 05 XX 55
```

Decoding:

- `ID = 0x01` (motor command)
- Left PWM = `0x0640` = 1600
- Right PWM = `0x0640` = 1600
- Steering PWM = `0x05DC` = 1500

Checksum for this payload is `0xD8`, so the full packet is:

```text
AA 01 40 06 40 06 DC 05 D8 55
```

## Repository Layout

```text
.
├── docs
│   ├── architecture.md
│   └── protocol_spec.md
├── examples
│   └── motor_control_demo
├── microcontroller
│   ├── example_receiver.ino
│   ├── umrtlink.c
│   └── umrtlink.h
├── rpi
│   ├── example_sender.py
│   └── umrtlink.py
├── LICENSE
└── README.md
```

## Raspberry Pi Usage

1. Install dependency:

```bash
pip install pyserial
```

2. Run sender example:

```bash
cd rpi
python3 example_sender.py --port /dev/ttyUSB0 --baud 115200 --hz 50
```

`example_sender.py` uses `create_packet(left_pwm, right_pwm, steer_pwm)` and transmits packets periodically.

## Microcontroller Usage

Files:

- `microcontroller/umrtlink.h`
- `microcontroller/umrtlink.c`
- `microcontroller/example_receiver.ino`

The parser is a fixed-size byte-stream state machine:

1. Wait for start byte `0xAA`
2. Collect 10-byte packet
3. Verify end byte `0x55`
4. Verify checksum
5. Decode PWM fields

No dynamic memory is used.

## Safety Recommendations

For real robots, implement a motor failsafe timeout in firmware.

Recommended behavior:

- If no valid command packet is received for 100-200 ms, set outputs to neutral/stop.
- Clamp received PWM values to safe bounds before applying to actuators.
- Keep hardware emergency stop independent from software communication.

## Additional Documentation

- Protocol details: `docs/protocol_spec.md`
- Design rationale: `docs/architecture.md`

## License

MIT. See `LICENSE`.
