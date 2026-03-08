# UMRTLink Protocol Specification (v1)

UMRTLink (Universal Modular Rover Telemetry Link) is a minimal binary UART protocol designed for deterministic parsing on microcontrollers and low-latency command transmission from Linux companion computers.

## 1. Protocol Overview

- Transport: UART
- Encoding: Binary
- Protocol version: v1
- Packet length: fixed, 10 bytes
- Primary message in v1: motor command (`0x01`)

## 2. Packet Layout

```text
Byte:  0    1    2    3    4    5    6    7    8    9
      +----+----+----+----+----+----+----+----+----+----+
      | AA | ID | L0 | L1 | R0 | R1 | S0 | S1 | CS | 55 |
      +----+----+----+----+----+----+----+----+----+----+
```

- `AA`: start byte (`0xAA`)
- `ID`: message ID
- `L0 L1`: left motor PWM (`uint16`, little-endian)
- `R0 R1`: right motor PWM (`uint16`, little-endian)
- `S0 S1`: steering PWM (`uint16`, little-endian)
- `CS`: checksum = XOR of bytes 1..7
- `55`: end byte (`0x55`)

## 3. Field Definitions

| Byte(s) | Name | Type | Description |
|---|---|---|---|
| 0 | Start | `uint8` | Constant `0xAA` |
| 1 | Message ID | `uint8` | Message type |
| 2-3 | Left PWM | `uint16 LE` | 1000..2000 typical RC range |
| 4-5 | Right PWM | `uint16 LE` | 1000..2000 typical RC range |
| 6-7 | Steering PWM | `uint16 LE` | 1000..2000 typical RC range |
| 8 | Checksum | `uint8` | XOR of bytes 1..7 |
| 9 | End | `uint8` | Constant `0x55` |

PWM semantic convention:

- `1000`: full reverse
- `1500`: neutral
- `2000`: full forward

## 4. Message IDs (Reserved)

| ID | Meaning | v1 Status |
|---|---|---|
| `0x01` | Motor command | Implemented |
| `0x02` | Telemetry | Reserved |
| `0x03` | Heartbeat | Reserved |
| `0x04` | Emergency stop | Reserved |
| `0x05` | Diagnostics | Reserved |

## 5. Checksum

Checksum uses bitwise XOR over bytes 1 through 7:

```text
checksum = b1 ^ b2 ^ b3 ^ b4 ^ b5 ^ b6 ^ b7
```

This is intentionally lightweight for microcontrollers and catches common single-byte corruption and many burst errors.

## 6. Example Packet

Example bytes:

```text
AA 01 40 06 40 06 DC 05 D8 55
```

Decode:

- `ID = 0x01` (motor command)
- Left PWM: `0x0640` = `1600`
- Right PWM: `0x0640` = `1600`
- Steering PWM: `0x05DC` = `1500`
- Checksum: `0xD8`

## 7. Timing Considerations

At 115200 baud, UART byte time is approximately 86.8 microseconds (10 bits with start/stop), so one 10-byte packet is approximately 0.868 ms on wire.

Examples:

- 50 Hz command stream -> 20 ms interval (ample margin)
- 100 Hz command stream -> 10 ms interval

To improve control safety:

- Use watchdog/failsafe timeout on MCU (e.g. stop motors if no valid packet for 100-200 ms)
- Reject invalid checksum or bad frame packets

## 8. Future Extensibility

UMRTLink keeps parsing deterministic by preserving fixed packet size. Future message types can reuse bytes 2..7 as message-specific fields while keeping framing/checksum unchanged.

Recommendations for v2+ evolution:

- Keep start/end/checksum positions stable
- Add command sequence counters for loss detection
- Add telemetry framing with same 10-byte envelope
- Add optional CRC variant for noisy links
