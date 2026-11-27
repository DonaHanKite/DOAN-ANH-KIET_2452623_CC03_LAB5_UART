#include "command_parser.h"
#include <string.h>

#define CMD_MAX_LEN 10

typedef enum {
    PARSER_WAIT_START,
    PARSER_READ_CMD
} ParserState;

uint8_t temp = 0;
uint8_t buffer[MAX_BUFFER_SIZE];
uint8_t index_buffer = 0;
uint8_t buffer_flag = 0;

uint8_t command_flag = 0;
uint8_t command_data = CMD_NONE;

static ParserState parser_state = PARSER_WAIT_START;
static char cmd_buf[CMD_MAX_LEN];
static uint8_t cmd_index = 0;

void command_parser_fsm(void) {
    // Lay ky tu moi nhat vua duoc ghi vao buffer trong ngat UART
    uint8_t current_char = (index_buffer == 0)
                           ? buffer[MAX_BUFFER_SIZE - 1]
                           : buffer[index_buffer - 1];

    switch (parser_state) {
        case PARSER_WAIT_START:
            if (current_char == '!') {
                cmd_index = 0;
                parser_state = PARSER_READ_CMD;
            }
            break;

        case PARSER_READ_CMD:
            if (current_char == '!') {
                // Gap lai '!' thi reset de doc lenh moi
                cmd_index = 0;
            } else if (current_char == '#') {
                cmd_buf[cmd_index] = '\0';
                if (strcmp(cmd_buf, "RST") == 0) {
                    command_data = CMD_RST;
                    command_flag = 1;
                } else if (strcmp(cmd_buf, "OK") == 0) {
                    command_data = CMD_OK;
                    command_flag = 1;
                }
                parser_state = PARSER_WAIT_START;
            } else {
                if (cmd_index < CMD_MAX_LEN - 1) {
                    cmd_buf[cmd_index++] = (char)current_char;
                } else {
                    // Chuoi rac qua dai -> huy va cho lenh moi
                    parser_state = PARSER_WAIT_START;
                }
            }
            break;

        default:
            parser_state = PARSER_WAIT_START;
            break;
    }
}
