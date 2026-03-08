"""UMRTLink v1 packet utilities for Raspberry Pi.

This module provides packet creation and serial transmission helpers
for the UMRTLink UART binary protocol.
"""

from __future__ import annotations

from dataclasses import dataclass
import struct
from typing import Final

import serial  # type: ignore

START_BYTE: Final[int] = 0xAA
END_BYTE: Final[int] = 0x55
PACKET_LEN: Final[int] = 10

MSG_MOTOR_COMMAND: Final[int] = 0x01
MSG_TELEMETRY: Final[int] = 0x02
MSG_HEARTBEAT: Final[int] = 0x03
MSG_EMERGENCY_STOP: Final[int] = 0x04
MSG_DIAGNOSTICS: Final[int] = 0x05

PWM_MIN: Final[int] = 1000
PWM_NEUTRAL: Final[int] = 1500
PWM_MAX: Final[int] = 2000


def _validate_pwm(pwm: int, field_name: str) -> None:
    if not (PWM_MIN <= pwm <= PWM_MAX):
        raise ValueError(f"{field_name} must be in [{PWM_MIN}, {PWM_MAX}], got {pwm}")


def compute_checksum(message_id: int, left_pwm: int, right_pwm: int, steer_pwm: int) -> int:
    """Compute UMRTLink XOR checksum for bytes 1..7."""
    payload = struct.pack("<BHHH", message_id, left_pwm, right_pwm, steer_pwm)
    checksum = 0
    for b in payload:
        checksum ^= b
    return checksum


def create_packet(left_pwm: int, right_pwm: int, steer_pwm: int) -> bytes:
    """Create a 10-byte UMRTLink motor command packet (message id 0x01)."""
    _validate_pwm(left_pwm, "left_pwm")
    _validate_pwm(right_pwm, "right_pwm")
    _validate_pwm(steer_pwm, "steer_pwm")

    message_id = MSG_MOTOR_COMMAND
    checksum = compute_checksum(message_id, left_pwm, right_pwm, steer_pwm)

    return struct.pack(
        "<BBHHHBB",
        START_BYTE,
        message_id,
        left_pwm,
        right_pwm,
        steer_pwm,
        checksum,
        END_BYTE,
    )


@dataclass
class UMRTLinkSerial:
    """Thin pyserial wrapper for sending UMRTLink packets."""

    port: str
    baudrate: int = 115200
    timeout: float = 0.05

    def __post_init__(self) -> None:
        self._ser = serial.Serial(self.port, self.baudrate, timeout=self.timeout)

    def close(self) -> None:
        """Close serial port."""
        if self._ser.is_open:
            self._ser.close()

    def send_packet(self, packet: bytes) -> None:
        """Send raw UMRTLink packet bytes."""
        if len(packet) != PACKET_LEN:
            raise ValueError(f"packet must be {PACKET_LEN} bytes, got {len(packet)}")
        self._ser.write(packet)
        self._ser.flush()

    def send_motor_command(self, left_pwm: int, right_pwm: int, steer_pwm: int) -> None:
        """Create and send a motor command packet."""
        packet = create_packet(left_pwm, right_pwm, steer_pwm)
        self.send_packet(packet)
