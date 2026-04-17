/*
 * Copyright 2001-2010 Georges Menie (www.menie.org)
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "xmodem.h"
#include "picoRTOS.h"

static const int crc16_lut[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

static uint16_t crc16(uint16_t crc, const uint8_t *buf, size_t n)
{
    while (n-- != 0) {
        int index = (int)((uint8_t)(crc >> 8) ^ *buf++);
        crc = (crc << 8) ^ crc16_lut[index];
    }

    return crc;
}

static uint8_t sum8(uint8_t sum, const uint8_t *buf, size_t n)
{
    while (n-- != 0)
        sum += *buf++;

    return sum;
}

/*
 * XMODEM Rx
 */

#define C_SOH   0x01
#define C_STX   0x02
#define C_EOT   0x04
#define C_ACK   0x06
#define C_NAK   0x15
#define C_CAN   0x18
#define C_CTRLZ 0x1A

int xmodem_init(struct xmodem *ctx, struct uart *uart, void *priv)
{
    ctx->uart = uart;
    ctx->priv = priv;
    ctx->state = XMODEM_STATE_TRYCHAR_C;
    ctx->check = XMODEM_CHECK_CRC16;
    ctx->exp_block_num = (uint8_t)1;
    ctx->retrans_count = 0;
    ctx->err = 0;

    return 0;
}

static int read_for_1s(struct xmodem *ctx, void *buf, size_t n)
{
    size_t recv = 0;
    char *buf8 = (char*)buf;
    picoRTOS_tick_t ref = picoRTOS_get_tick();

    while (recv < n) {

        int res;

        if (PICORTOS_DELAY_ELAPSED(ref, PICORTOS_DELAY_SEC(1)))
            return -ETIMEDOUT;

        if ((res = uart_read(ctx->uart, &buf8[recv], n - recv)) == -EAGAIN) {
            picoRTOS_postpone();
            continue;
        }

        if (res < 0)
            return res;

        recv += (size_t)res;
        ref = picoRTOS_get_tick();
    }

    return (int)recv;
}

static int write_for_1s(struct xmodem *ctx, const void *buf, size_t n)
{
    size_t sent = 0;
    const char *buf8 = (const char*)buf;
    picoRTOS_tick_t ref = picoRTOS_get_tick();

    while (sent < n) {

        int res;

        if (PICORTOS_DELAY_ELAPSED(ref, PICORTOS_DELAY_SEC(1)))
            return -ETIMEDOUT;

        if ((res = uart_write(ctx->uart, &buf8[sent], n - sent)) == -EAGAIN) {
            picoRTOS_postpone();
            continue;
        }

        sent += (size_t)res;
        ref = picoRTOS_get_tick();
    }

    return (int)sent;
}

static int run_rx_sof(struct xmodem *ctx)
{
    int res;
    char sof = (char)0;

    if ((res = read_for_1s(ctx, &sof, sizeof(sof))) < 0)
        return res;

    switch (sof) {
    case C_SOH:
        ctx->exp_block_len = (size_t)130;
        if (ctx->check == XMODEM_CHECK_CRC16) ctx->exp_block_len += 2;
        else ctx->exp_block_len += 1;
        ctx->state = XMODEM_STATE_FRAME;
        break;

    case C_STX:
        ctx->exp_block_len = sizeof(ctx->frame);
        ctx->state = XMODEM_STATE_FRAME;
        break;

    case C_EOT: ctx->state = XMODEM_STATE_EOT; break;
    case C_CAN: ctx->state = XMODEM_STATE_CANCEL; break;
    default: break; /* flush */
    }

    return 0;
}

static int run_rx_trychar(struct xmodem *ctx, char c)
{
    for (int n = 0; n < XMODEM_RETRY_COUNT; n++) {
        /* we're ready */
        (void)write_for_1s(ctx, &c, sizeof(c));
        if (run_rx_sof(ctx) != 0)
            continue;

        return 0;
    }

    switch (ctx->state) {
    case XMODEM_STATE_TRYCHAR_C:
        ctx->state = XMODEM_STATE_TRYCHAR_NAK;
        ctx->check = XMODEM_CHECK_SUM8;
        break;

    case XMODEM_STATE_TRYCHAR_NAK:
        ctx->state = XMODEM_STATE_ERROR;
        ctx->err = XMODEM_ERR_SYNC;
        break;

    default:
        picoRTOS_assert(false, return -EIO);
    }

    return -ETIMEDOUT;
}

static int check(struct xmodem *ctx)
{
    if (ctx->check == XMODEM_CHECK_SUM8) {
        if (sum8(0, ctx->frame.data, ctx->exp_block_len - 1) ==
            ctx->frame.data[ctx->exp_block_len - 1])
            return 0;
    }else if (crc16(0, ctx->frame.data, ctx->exp_block_len) == 0)
        return 0;

    return -EFAULT;
}

static int run_rx_frame(struct xmodem *ctx)
{
    int res;
    const char nak = (char)C_NAK;

    if ((res = read_for_1s(ctx, &ctx->frame, ctx->exp_block_len)) < 0) {
        /* reject: NAK */
        (void)write_for_1s(ctx, &nak, sizeof(nak));
        ctx->state = XMODEM_STATE_SOF;
        return res;
    }

    /* checks */
    if ((ctx->frame.block_num ^ ctx->frame.iblock_num) == (uint8_t)0xffu &&
        ctx->frame.block_num == ctx->exp_block_num &&
        check(ctx) == 0) {
        /* ACK */
        const char ack = (char)C_ACK;
        /* internal state machine */
        ctx->exp_block_num++;
        ctx->retrans_count = 0;
        ctx->state = XMODEM_STATE_SOF;
        /* callback */
        if (ctx->check == XMODEM_CHECK_CRC16) ctx->exp_block_len -= 4;
        else ctx->exp_block_len -= 3;
        res = xmodem_run_rx_callback(ctx, ctx->frame.data, ctx->exp_block_len);
        (void)write_for_1s(ctx, &ack, sizeof(ack));
        return res;
    }

    if (ctx->retrans_count++ >= XMODEM_RETRANS_COUNT) {
        /* CANCEL */
        const char can = (char)C_CAN;
        (void)write_for_1s(ctx, &can, sizeof(can));
        (void)write_for_1s(ctx, &can, sizeof(can));
        (void)write_for_1s(ctx, &can, sizeof(can));
        /* error */
        ctx->state = XMODEM_STATE_ERROR;
        ctx->err = XMODEM_ERR_RETRANS;
        return -EFAULT;
    }

    (void)write_for_1s(ctx, &nak, sizeof(nak));
    ctx->state = XMODEM_STATE_SOF;
    return 0;
}

static int run_rx_eot(struct xmodem *ctx)
{
    const char ack = (char)C_ACK;

    (void)write_for_1s(ctx, &ack, sizeof(ack));
    return 0;
}

static int run_rx_cancel(struct xmodem *ctx)
{
    int res;
    char can = (char)0;

    /* back to SOF by default */
    ctx->state = XMODEM_STATE_SOF;

    /* wait for a 2nd cancel */
    if ((res = read_for_1s(ctx, &can, sizeof(can))) < 0)
        return res;

    if (can == (char)C_CAN) {
        const char ack = (char)C_ACK;
        (void)write_for_1s(ctx, &ack, sizeof(ack));
        /* error */
        ctx->err = XMODEM_ERR_CANCEL;
        ctx->state = XMODEM_STATE_ERROR;
    }

    return 0;
}

int xmodem_run_rx(struct xmodem *ctx)
{
    for (;;) {
        switch (ctx->state) {
        case XMODEM_STATE_TRYCHAR_C: (void)run_rx_trychar(ctx, 'C'); break;
        case XMODEM_STATE_TRYCHAR_NAK: (void)run_rx_trychar(ctx, (char)C_NAK); break;
        case XMODEM_STATE_SOF: (void)run_rx_sof(ctx); break;
        case XMODEM_STATE_FRAME: (void)run_rx_frame(ctx); break;
        case XMODEM_STATE_EOT: return run_rx_eot(ctx);  /* the end */
        case XMODEM_STATE_CANCEL: (void)run_rx_cancel(ctx); break;
        case XMODEM_STATE_ERROR: return -EFAULT;        /* failure */
        default: picoRTOS_assert(false, return -EIO);
        }
    }

    /*@notreached@*/ return 0;
}
