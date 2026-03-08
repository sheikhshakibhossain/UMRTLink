#ifndef UMRTLINK_H
#define UMRTLINK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UMRTLINK_PROTOCOL_VERSION 1u

#define UMRTLINK_PACKET_LEN 10u
#define UMRTLINK_START_BYTE 0xAAu
#define UMRTLINK_END_BYTE 0x55u

#define UMRTLINK_PWM_MIN 1000u
#define UMRTLINK_PWM_NEUTRAL 1500u
#define UMRTLINK_PWM_MAX 2000u

/** Message IDs (v1). */
typedef enum {
    UMRTLINK_MSG_MOTOR_COMMAND = 0x01u,
    UMRTLINK_MSG_TELEMETRY = 0x02u,
    UMRTLINK_MSG_HEARTBEAT = 0x03u,
    UMRTLINK_MSG_EMERGENCY_STOP = 0x04u,
    UMRTLINK_MSG_DIAGNOSTICS = 0x05u,
} UMRTLinkMessageId;

/** Decoder return status for a single fed byte. */
typedef enum {
    UMRTLINK_PARSE_NONE = 0,
    UMRTLINK_PARSE_OK = 1,
    UMRTLINK_PARSE_BAD_END = -1,
    UMRTLINK_PARSE_BAD_CHECKSUM = -2,
} UMRTLinkParseStatus;

/** Decoded v1 motor command. */
typedef struct {
    uint16_t left_pwm;
    uint16_t right_pwm;
    uint16_t steer_pwm;
} UMRTLinkMotorCommand;

/** Generic decoded packet envelope. */
typedef struct {
    uint8_t message_id;
    UMRTLinkMotorCommand motor_command;
} UMRTLinkDecodedPacket;

typedef enum {
    UMRTLINK_STATE_WAIT_START = 0,
    UMRTLINK_STATE_COLLECT = 1,
} UMRTLinkParserState;

/** Streaming parser context (no dynamic allocation). */
typedef struct {
    UMRTLinkParserState state;
    uint8_t index;
    uint8_t buffer[UMRTLINK_PACKET_LEN];
} UMRTLinkParser;

/** Initialize parser state. */
void umrtlink_parser_init(UMRTLinkParser *parser);

/**
 * Feed one UART byte into the parser.
 * Returns UMRTLINK_PARSE_OK when a complete valid packet was decoded.
 */
UMRTLinkParseStatus umrtlink_parser_feed(
    UMRTLinkParser *parser,
    uint8_t byte,
    UMRTLinkDecodedPacket *out_packet);

/** XOR checksum for bytes 1..7 (inclusive) from a full packet buffer. */
uint8_t umrtlink_compute_checksum_from_packet(const uint8_t packet[UMRTLINK_PACKET_LEN]);

/** Validate packet framing and checksum. */
bool umrtlink_validate_packet(const uint8_t packet[UMRTLINK_PACKET_LEN]);

/** Decode bytes 2..7 as PWM fields. */
void umrtlink_decode_motor_command(
    const uint8_t packet[UMRTLINK_PACKET_LEN],
    UMRTLinkMotorCommand *out_command);

#ifdef __cplusplus
}
#endif

#endif /* UMRTLINK_H */
