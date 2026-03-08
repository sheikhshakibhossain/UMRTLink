#include <Arduino.h>
#include "umrtlink.h"

UMRTLinkParser g_parser;

static void applyMotorCommand(const UMRTLinkMotorCommand &cmd);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  umrtlink_parser_init(&g_parser);
  Serial.println("UMRTLink receiver ready");
}

void loop() {
  while (Serial.available() > 0) {
    uint8_t byte_in = (uint8_t)Serial.read();
    UMRTLinkDecodedPacket packet;

    UMRTLinkParseStatus status = umrtlink_parser_feed(&g_parser, byte_in, &packet);

    if (status == UMRTLINK_PARSE_OK) {
      if (packet.message_id == UMRTLINK_MSG_MOTOR_COMMAND) {
        applyMotorCommand(packet.motor_command);
      }
    } else if (status == UMRTLINK_PARSE_BAD_END) {
      Serial.println("UMRTLink warning: bad end byte");
    } else if (status == UMRTLINK_PARSE_BAD_CHECKSUM) {
      Serial.println("UMRTLink warning: checksum mismatch");
    }
  }
}

static void setLeftMotorPwm(uint16_t pwm) {
  // TODO: Replace with board-specific motor driver output.
  (void)pwm;
}

static void setRightMotorPwm(uint16_t pwm) {
  // TODO: Replace with board-specific motor driver output.
  (void)pwm;
}

static void setSteeringPwm(uint16_t pwm) {
  // TODO: Replace with board-specific steering servo output.
  (void)pwm;
}

static void applyMotorCommand(const UMRTLinkMotorCommand &cmd) {
  setLeftMotorPwm(cmd.left_pwm);
  setRightMotorPwm(cmd.right_pwm);
  setSteeringPwm(cmd.steer_pwm);

  Serial.print("RX Motor cmd | L=");
  Serial.print(cmd.left_pwm);
  Serial.print(" R=");
  Serial.print(cmd.right_pwm);
  Serial.print(" S=");
  Serial.println(cmd.steer_pwm);
}
