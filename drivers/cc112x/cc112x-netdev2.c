/**
 * @ingroup     drivers_cc112x
 * @{
 * @file
 * @brief       Implementation of netdev2 interface for cc112x
 *
 * @author      Kubaszek Mateusz <mathir.km.riot@gmail.com>
 * @}
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cc112x.h"
#include "cc112x-netdev2.h"
#include "cc112x-internal.h"
#include "cc112x-interface.h"

#include "net/eui64.h"

#include "periph/cpuid.h"
#include "periph/gpio.h"
#include "net/netdev2.h"
#include "net/gnrc/nettype.h"
#include "mutex.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"



static int _send(netdev2_t *dev, const struct iovec *vector, int count)
{
    netdev2_cc112x_t *netdev2_cc112x = (netdev2_cc112x_t*)dev;
    cc112x_pkt_t *cc112x_pkt = vector[0].iov_base;

    DEBUG("%s:%u - size[%u] dst[%u] src[%u] flags[%x]\n", __func__, __LINE__, cc112x_pkt->length+1, cc112x_pkt->address, cc112x_pkt->phy_src, cc112x_pkt->flags);

    return cc112x_send(&netdev2_cc112x->cc112x, cc112x_pkt);
}

static int _recv(netdev2_t *dev, char* buf, int len, void *info)
{
    DEBUG("%s:%u\n", __func__, __LINE__);

    cc112x_t *cc112x = &((netdev2_cc112x_t*)dev)->cc112x;

    cc112x_pkt_t *cc112x_pkt = &cc112x->rx_pkt_buf.packet;
    if(cc112x_pkt->length > len) {
        return -ENOSPC;
    }

    memcpy(buf, (void*)cc112x_pkt, cc112x_pkt->length);
    return cc112x_pkt->length;
}

static inline int _get_iid(netdev2_t *netdev, eui64_t *value, size_t max_len)
{
    cc112x_t *cc112x = &((netdev2_cc112x_t*) netdev)->cc112x;
    uint8_t *eui64 = (uint8_t*)value;

    if(max_len<sizeof(eui64_t)) {
        return -EOVERFLOW;
    }

    /* make address compatible to https://tools.ietf.org/html/rfc6282#section-3.2.2*/
    memset(eui64, 0, sizeof(eui64_t));
    eui64[3] = 0xff;
    eui64[4] = 0xfe;
    eui64[7] = cc112x->radio_address;

        return sizeof(eui64_t);

    return sizeof(eui64_t);
}

static int _get(netdev2_t *dev, netopt_t opt, void *value, size_t value_len)
{
    DEBUG("%s:%u\n", __func__, __LINE__);
    cc112x_t *cc112x = &((netdev2_cc112x_t*)dev)->cc112x;

    switch(opt) {
    case NETOPT_DEVICE_TYPE:
        assert(value_len == 2);
        *((uint16_t *)value) = NETDEV2_TYPE_CC110X;
        return 2;
    case NETOPT_PROTO:
        assert(value_len == sizeof(gnrc_nettype_t));
#ifdef MODULE_GNRC_SIXLOWPAN
        *((gnrc_nettype_t*)value) = GNRC_NETTYPE_SIXLOWPAN;
#else
        *((gnrc_nettype_t*)value) = GNRC_NETTYPE_UNDEF;
#endif
        return sizeof(gnrc_nettype_t);
    case NETOPT_CHANNEL:
        assert(value_len > 1);
        *((uint16_t *)value) = (uint16_t)cc112x->radio_channel;
        return 2;
    case NETOPT_ADDRESS:
        assert(value_len > 0);
        *((uint8_t *)value) = cc112x->radio_address;
        return 1;
    case NETOPT_MAX_PACKET_SIZE:
        assert(value_len > 0);
        *((uint8_t *)value) = CC112X_PACKET_LENGTH;
        return 1;
    case NETOPT_IPV6_IID:
        return _get_iid(dev, value, value_len);
    default:
        break;
    }

    return -ENOTSUP;
}

static int _set(netdev2_t *dev, netopt_t opt, void *value, size_t value_len)
{
    cc112x_t *cc112x = &((netdev2_cc112x_t*)dev)->cc112x;

    switch(opt) {
    case NETOPT_CHANNEL: {
        uint8_t *arg = (uint8_t*)value;
        uint8_t channel = arg[value_len - 1];
        if((channel < CC112X_MIN_CHANNR) || (channel > CC112X_MAX_CHANNR)) {
            return -EINVAL;
        }
        if(cc112x_set_channel(cc112x, channel) == -1) {
            return -EINVAL;
        }
        return 1;
    }
    case NETOPT_ADDRESS:
        if(value_len < 1) {
            return -EINVAL;
        }
        if(!cc112x_set_address(cc112x, *(uint8_t*)value)) {
            return -EINVAL;
        }
        return 1;
    default:
        return -ENOTSUP;
    }

    return 0;
}

static void _isr(netdev2_t *dev)
{
    dev->event_callback(dev, NETDEV2_EVENT_RX_COMPLETE, NULL);
}

static int _init(netdev2_t *dev)
{
    DEBUG("%s:%u\n", __func__, __LINE__);
    netdev2_cc112x_t *cc112x_netdev = ((netdev2_cc112x_t*)dev);
    cc112x_t *cc112x = &((netdev2_cc112x_t*)dev)->cc112x;

    gpio_init_int(cc112x->params.gpio2, GPIO_IN_PD, GPIO_BOTH, &cc112x_isr_handler, (void*)cc112x_netdev);

    gpio_irq_disable(cc112x->params.gpio2);

    /* Switch to RX mode */
    cc112x_rd_set_mode(cc112x, RADIO_MODE_ON);

    return 0;
}

const netdev2_driver_t netdev2_cc112x_driver = {
        .send = _send,
        .recv = _recv,
        .init = _init,
        .get = _get,
        .isr = _isr,
        .set = _set};

int netdev2_cc112x_setup(netdev2_cc112x_t *netdev2_cc112x,
        const cc112x_params_t *params)
{
    DEBUG("netdev2_cc112x_setup()\n");
    netdev2_cc112x->netdev.driver = &netdev2_cc112x_driver;

    return cc112x_setup(&netdev2_cc112x->cc112x, params);
}
