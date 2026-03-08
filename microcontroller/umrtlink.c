#include "umrtlink.h"

static uint16_t umrtlink_read_u16_le(const uint8_t *ptr) {
    return (uint16_t)ptr[0] | ((uint16_t)ptr[1] << 8u);
}

void umrtlink_parser_init(UMRTLinkParser *parser) {
    if (parser == NULL) {
        return;
    }

    parser->state = UMRTLINK_STATE_WAIT_START;
    parser->index = 0u;
}

uint8_t umrtlink_compute_checksum_from_packet(const uint8_t packet[UMRTLINK_PACKET_LEN]) {
    uint8_t checksum = 0u;
    uint8_t i;

    for (i = 1u; i <= 7u; ++i) {
        checksum ^= packet[i];
    }

    return checksum;
}

bool umrtlink_validate_packet(const uint8_t packet[UMRTLINK_PACKET_LEN]) {
    if (packet[0] != UMRTLINK_START_BYTE) {
        return false;
    }

    if (packet[9] != UMRTLINK_END_BYTE) {
        return false;
    }

    return packet[8] == umrtlink_compute_checksum_from_packet(packet);
}

void umrtlink_decode_motor_command(
    const uint8_t packet[UMRTLINK_PACKET_LEN],
    UMRTLinkMotorCommand *out_command) {
    if (out_command == NULL) {
        return;
    }

    out_command->left_pwm = umrtlink_read_u16_le(&packet[2]);
    out_command->right_pwm = umrtlink_read_u16_le(&packet[4]);
    out_command->steer_pwm = umrtlink_read_u16_le(&packet[6]);
}

UMRTLinkParseStatus umrtlink_parser_feed(
    UMRTLinkParser *parser,
    uint8_t byte,
    UMRTLinkDecodedPacket *out_packet) {
    if (parser == NULL) {
        return UMRTLINK_PARSE_NONE;
    }

    switch (parser->state) {
        case UMRTLINK_STATE_WAIT_START:
            if (byte == UMRTLINK_START_BYTE) {
                parser->buffer[0] = byte;
                parser->index = 1u;
                parser->state = UMRTLINK_STATE_COLLECT;
            }
            return UMRTLINK_PARSE_NONE;

        case UMRTLINK_STATE_COLLECT:
            parser->buffer[parser->index++] = byte;

            if (parser->index < UMRTLINK_PACKET_LEN) {
                return UMRTLINK_PARSE_NONE;
            }

            /* We now have a full fixed-size frame. Reset parser first. */
            parser->state = UMRTLINK_STATE_WAIT_START;
            parser->index = 0u;

            if (parser->buffer[9] != UMRTLINK_END_BYTE) {
                return UMRTLINK_PARSE_BAD_END;
            }

            if (parser->buffer[8] != umrtlink_compute_checksum_from_packet(parser->buffer)) {
                return UMRTLINK_PARSE_BAD_CHECKSUM;
            }

            if (out_packet != NULL) {
                out_packet->message_id = parser->buffer[1];
                umrtlink_decode_motor_command(parser->buffer, &out_packet->motor_command);
            }
            return UMRTLINK_PARSE_OK;

        default:
            parser->state = UMRTLINK_STATE_WAIT_START;
            parser->index = 0u;
            return UMRTLINK_PARSE_NONE;
    }
}
