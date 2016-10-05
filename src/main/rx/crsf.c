/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#ifdef SERIAL_RX

#include "build/version.h"

#if (FC_VERSION_MAJOR == 3) // not a very good way of finding out if this is betaflight or Cleanflight
#define BETAFLIGHT
#else
#define CLEANFLIGHT
#endif

#ifdef CLEANFLIGHT
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "fc/fc_debug.h"
#endif

#include "build/debug.h"

#include "common/utils.h"

#include "drivers/system.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"

#include "io/serial.h"

#include "rx/rx.h"
#include "rx/crsf.h"

#include "telemetry/telemetry.h"

#define CRSF_TIME_NEEDED_PER_FRAME 3000

#define CRSF_MAX_CHANNEL        16

#define CRSF_FRAME_BEGIN_BYTE   0x0F

#define CRSF_BAUDRATE           400000
#define CRSF_PORT_OPTIONS       (SERIAL_INVERTED | SERIAL_STOPBITS_1 | SERIAL_PARITY_NO)

#define CRSF_DIGITAL_CHANNEL_MIN 173
#define CRSF_DIGITAL_CHANNEL_MAX 1812

static bool crsfFrameDone = false;

static uint32_t crsfChannelData[CRSF_MAX_CHANNEL];

#define CRSF_FLAG_SIGNAL_LOSS       (1 << 2)
#define CRSF_FLAG_FAILSAFE_ACTIVE   (1 << 3)

/*
Structure
400kbaud
Inverted None
8 Bit
1 Stop bit None
Big endian

Every frame has the structure:
<Device address> <Frame length> < Type> <Payload> < CRC>

Device address: (uint8_t)
Frame length:   length in  bytes including Type (uint8_t)
Type:           (uint8_t)
CRC:            (uint8_t)
*/
struct crsfPayloadRcChannelsPacked_s {
    // 176 bits of data (11 bits per channel * 16 channels) = 22 bytes.
    unsigned int chan0 : 11;
    unsigned int chan1 : 11;
    unsigned int chan2 : 11;
    unsigned int chan3 : 11;
    unsigned int chan4 : 11;
    unsigned int chan5 : 11;
    unsigned int chan6 : 11;
    unsigned int chan7 : 11;
    unsigned int chan8 : 11;
    unsigned int chan9 : 11;
    unsigned int chan10 : 11;
    unsigned int chan11 : 11;
    unsigned int chan12 : 11;
    unsigned int chan13 : 11;
    unsigned int chan14 : 11;
    unsigned int chan15 : 11;
} __attribute__ ((__packed__));

typedef struct crsfPayloadRcChannelsPacked_s crsfPayloadRcChannelsPacked_t;

typedef union crsfFrameDef_s {
    uint8_t deviceAddress;
    uint8_t frameLength;
    uint8_t type;
    uint8_t payload[CRSF_PAYLOAD_SIZE_MAX + 1]; // +1 for CRC at end of payload
} crsfFrameDef_t;

typedef union crsfFrame_u {
    uint8_t bytes[CRSF_FRAME_SIZE_MAX];
    crsfFrameDef_t frame;
} crsfFrame_t;

static crsfFrame_t crsfFrame;

// Receive ISR callback, called back from serial port
static void crsfDataReceive(uint16_t c)
{
    static uint8_t crsfFramePosition = 0;
    static uint32_t crsfFrameStartAt = 0;
    const uint32_t now = micros();

    const int32_t crsfFrameTime = now - crsfFrameStartAt;
    DEBUG_SET(DEBUG_CRSF, 2, crsfFrameTime);

    if (crsfFrameTime > (long)(CRSF_TIME_NEEDED_PER_FRAME + 500)) {
        crsfFramePosition = 0;
    }

    if (crsfFramePosition == 0) {
        crsfFrameStartAt = now;
    }
    // assume frame is 5 bytes long until we have received the frame length
    const int frameLength = crsfFramePosition < 3 ? 5 : crsfFrame.frame.frameLength;

    if (crsfFramePosition < frameLength) {
        crsfFrame.bytes[crsfFramePosition++] = (uint8_t)c;
        crsfFrameDone = crsfFramePosition < frameLength ? false : true;
    }
}

uint8_t crsfFrameStatus(void)
{
    if (crsfFrameDone) {
        crsfFrameDone = false;
        if (crsfFrame.frame.type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED) {
            // unpack the RC channels
            const crsfPayloadRcChannelsPacked_t* rcChannels = (crsfPayloadRcChannelsPacked_t*)&crsfFrame.frame.payload;
            crsfChannelData[0] = rcChannels->chan0;
            crsfChannelData[1] = rcChannels->chan1;
            crsfChannelData[2] = rcChannels->chan2;
            crsfChannelData[3] = rcChannels->chan3;
            crsfChannelData[4] = rcChannels->chan4;
            crsfChannelData[5] = rcChannels->chan5;
            crsfChannelData[6] = rcChannels->chan6;
            crsfChannelData[7] = rcChannels->chan7;
            crsfChannelData[8] = rcChannels->chan8;
            crsfChannelData[9] = rcChannels->chan9;
            crsfChannelData[10] = rcChannels->chan10;
            crsfChannelData[11] = rcChannels->chan11;
            crsfChannelData[12] = rcChannels->chan12;
            crsfChannelData[13] = rcChannels->chan13;
            crsfChannelData[14] = rcChannels->chan14;
            crsfChannelData[15] = rcChannels->chan15;
            return RX_FRAME_COMPLETE;
        }
    }
    return RX_FRAME_PENDING;
}

static uint16_t crsfReadRawRC(const rxRuntimeConfig_t *rxRuntimeConfig, uint8_t chan)
{
    UNUSED(rxRuntimeConfig);
    /* conversion from RC value to PWM
     *       RC     PWM
     * min  172 ->  988us
     * mid  992 -> 1500us
     * max 1811 -> 2012us
     * scale factor = (2012-988) / (1811-172) = 0.62477120195241
     * offset = 988 - 172 * 0.62477120195241 = 880.53935326418548
     */
    return (0.62477120195241f * crsfChannelData[chan]) + 881;
}

bool crsfInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig)
{
    for (int ii = 0; ii < CRSF_MAX_CHANNEL; ++ii) {
        crsfChannelData[ii] = (16 * rxConfig->midrc) / 10 - 1408;
    }

    rxRuntimeConfig->channelCount = CRSF_MAX_CHANNEL;
    rxRuntimeConfig->rxRefreshRate = 11000; //!!TODO this needs checking

    rxRuntimeConfig->rcReadRawFunc = crsfReadRawRC;
    rxRuntimeConfig->rcFrameStatusFunc = crsfFrameStatus;

    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_RX_SERIAL);
    if (!portConfig) {
        return false;
    }

#if defined(TELEMETRY) && !defined(CLEANFLIGHT)
    const bool portShared = telemetryCheckRxPortShared(portConfig);
#else
    const bool portShared = false;
#endif

    const portOptions_t options = (rxConfig->sbus_inversion) ? (CRSF_PORT_OPTIONS | SERIAL_INVERTED) : CRSF_PORT_OPTIONS;
    serialPort_t *serialPort = openSerialPort(portConfig->identifier, FUNCTION_RX_SERIAL, crsfDataReceive, CRSF_BAUDRATE, portShared ? MODE_RXTX : MODE_RX, options);

#if defined(TELEMETRY) && !defined(CLEANFLIGHT)
    if (portShared) {
        telemetrySharedPort = serialPort;
    }
#endif

    return serialPort != NULL;
}
#endif