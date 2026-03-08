#!/usr/bin/env python3
"""UMRTLink sender example for Raspberry Pi.

Sends motor commands periodically over UART using pyserial.
"""

from __future__ import annotations

import argparse
import math
import time

from umrtlink import PWM_NEUTRAL, UMRTLinkSerial


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="UMRTLink periodic motor command sender")
    parser.add_argument("--port", required=True, help="Serial device (e.g. /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=115200, help="UART baudrate")
    parser.add_argument("--hz", type=float, default=50.0, help="Send rate in Hz")
    parser.add_argument(
        "--amplitude",
        type=int,
        default=200,
        help="Oscillation amplitude around 1500 (typical 50-400)",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    period = 1.0 / args.hz

    link = UMRTLinkSerial(port=args.port, baudrate=args.baud)
    print(f"UMRTLink sender started on {args.port} @ {args.baud} baud")

    t0 = time.monotonic()
    try:
        while True:
            t = time.monotonic() - t0
            wave = int(args.amplitude * math.sin(2.0 * math.pi * 0.2 * t))

            left = PWM_NEUTRAL + wave
            right = PWM_NEUTRAL + wave
            steer = PWM_NEUTRAL

            link.send_motor_command(left, right, steer)
            print(f"TX L={left} R={right} S={steer}")

            time.sleep(period)
    except KeyboardInterrupt:
        print("Stopping sender")
    finally:
        link.close()


if __name__ == "__main__":
    main()
