#ifndef XMODEM_H
#define XMODEM_H

#include "uart.h"

#include <stdint.h>
#include <stddef.h>

typedef enum {
    XMODEM_STATE_TRYCHAR_C,
    XMODEM_STATE_TRYCHAR_NAK,
    XMODEM_STATE_SOF,
    XMODEM_STATE_FRAME,
    XMODEM_STATE_EOT,
    XMODEM_STATE_CANCEL,
    XMODEM_STATE_ERROR,
    XMODEM_STATE_COUNT
} xmodem_state_t;

#define XMODEM_RETRY_COUNT   16
#define XMODEM_RETRANS_COUNT 25

struct xmodem_frame {
    uint8_t block_num;
    uint8_t iblock_num;
    uint8_t data[1026]; /* +2 for crc */
} __attribute__ ((packed));

#define XMODEM_ERR_NONE    0
#define XMODEM_ERR_SYNC    1
#define XMODEM_ERR_RETRANS 2
#define XMODEM_ERR_CANCEL  3

typedef enum {
    XMODEM_CHECK_SUM8,
    XMODEM_CHECK_CRC16,
    XMODEM_CHECK_COUNT
} xmodem_check_t;

struct xmodem {
    /*@temp@*/ struct uart *uart;
    /*@temp@*/ void *priv;
    /* internals */
    xmodem_state_t state;
    xmodem_check_t check;
    struct xmodem_frame frame;
    size_t exp_block_len;
    uint8_t exp_block_num;
    int retrans_count;
    /* error management */
    int err;
};

int xmodem_init(/*@out@*/ struct xmodem *ctx, struct uart *uart, void *priv);
int xmodem_run_rx(struct xmodem *ctx);

/*
 * Callbacks
 */
int xmodem_run_rx_callback(struct xmodem *xmodem, /*@observer@*/ const void *buf, size_t n);

#endif
