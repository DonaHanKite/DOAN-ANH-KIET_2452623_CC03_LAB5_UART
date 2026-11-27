#include "uart_communication.h"
#include "command_parser.h"
#include <stdio.h>

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart2;

typedef enum {
    COMM_IDLE,
    COMM_SEND_ADC,
    COMM_WAIT_OK
} CommState;

static CommState comm_state = COMM_IDLE;
static uint32_t ADC_value = 0;
static uint32_t timer_start = 0;
static char str[30];

void uart_communiation_fsm(void) {
    switch (comm_state) {
        case COMM_IDLE:
            if (command_flag == 1) {
                command_flag = 0;
                if (command_data == CMD_RST) {
                    // Chi doc gia tri ADC moi khi bat dau phien !RST#
                    ADC_value = HAL_ADC_GetValue(&hadc1);
                    comm_state = COMM_SEND_ADC;
                }
            }
            break;

        case COMM_SEND_ADC: {
            int len = sprintf(str, "\r\n!ADC=%lu#\r\n", (unsigned long)ADC_value);
            HAL_UART_Transmit(&huart2, (uint8_t *)str, len, 1000);
            timer_start = HAL_GetTick();
            comm_state = COMM_WAIT_OK;
            break;
        }

        case COMM_WAIT_OK:
            if (command_flag == 1) {
                command_flag = 0;
                if (command_data == CMD_OK) {
                    comm_state = COMM_IDLE;
                }
            } else if (HAL_GetTick() - timer_start >= 3000) {
                // Qua 3 giay chua nhan !OK# -> gui lai gia tri ADC_value cu
                comm_state = COMM_SEND_ADC;
            }
            break;

        default:
            comm_state = COMM_IDLE;
            break;
    }
}
