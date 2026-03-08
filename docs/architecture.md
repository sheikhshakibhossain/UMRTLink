# UMRTLink Architecture and Design Rationale

## Design Goals

UMRTLink is engineered for command paths where latency, determinism, and CPU budget matter more than rich framing or dynamic payloads.

Primary goals:

- low latency from companion computer to motor controller
- deterministic decode on resource-constrained MCUs
- minimal RAM and code size footprint
- straightforward implementation in C and Python

## Why Binary Encoding

Binary packets are used instead of text formats (JSON/CSV) because:

- no string parsing overhead
- fixed offsets for fields
- lower bandwidth usage
- reduced heap/stack pressure
- predictable execution time

This directly benefits small MCUs (AVR, Cortex-M, ESP32) and high-rate control loops.

## Why Fixed Packet Length

A fixed 10-byte frame enables:

- constant-time parser logic
- simple state machine (wait start -> collect N bytes -> validate)
- no variable-length buffers
- robust re-synchronization after noise

The parser needs only one static buffer and an index counter.

## Frame Integrity Strategy

UMRTLink v1 uses:

- start byte (`0xAA`)
- end byte (`0x55`)
- XOR checksum over bytes 1..7

This combination gives strong practical protection for UART links with minimal compute cost.

## Microcontroller Performance Considerations

The C library is designed for embedded constraints:

- no dynamic allocation
- no recursion
- no floating-point operations
- byte-by-byte streaming API
- small, branch-light state machine

A typical integration pattern:

1. Read bytes from UART ISR buffer or polling loop
2. Feed each byte into parser
3. On valid packet, update motor outputs
4. If timeout occurs, force safe neutral/stop

## Safety and Control Integration

UMRTLink is a transport layer, not a full safety framework. Production systems should include:

- watchdog timeout for stale commands
- clamping/range checks before actuator output
- hardware emergency stop path
- sanity checks for command rate and values

## Extensibility Model

Message IDs reserve protocol space for telemetry, heartbeat, diagnostics, and emergency control while preserving the same framing contract.

Maintaining the envelope allows mixed firmware generations and simpler test tooling.
