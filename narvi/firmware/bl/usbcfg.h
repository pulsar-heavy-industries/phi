/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _USBCFG_H_
#define _USBCFG_H_

#include "phi_lib/phi_usb_midi.h"

extern const USBConfig usbcfg;

//extern BulkUSBDriver BDU1;
//extern const BulkUSBConfig bulkusbcfg;



extern MIDIUSBDriver MDU1;
extern const MIDIUSBConfig midiusbcfg;

/*
 * Supported control requests from USB Audio Class
 */
#define UAC_REQ_SET_CUR             0x01
#define UAC_REQ_SET_MIN             0x02
#define UAC_REQ_SET_MAX             0x03
#define UAC_REQ_SET_RES             0x04
#define UAC_REQ_GET_CUR             0x81
#define UAC_REQ_GET_MIN             0x82
#define UAC_REQ_GET_MAX             0x83
#define UAC_REQ_GET_RES             0x84

#define UAC_FU_MUTE_CONTROL         0x01
#define UAC_FU_VOLUME_CONTROL       0x02

/*
 * Audio playback events
 */
#define AUDIO_EVENT                 EVENT_MASK(0)
#define AUDIO_EVENT_PLAYBACK        EVENT_MASK(1)
#define AUDIO_EVENT_MUTE            EVENT_MASK(2)
#define AUDIO_EVENT_VOLUME          EVENT_MASK(3)
#define AUDIO_EVENT_USB_STATE       EVENT_MASK(4)



/*
 * Audio parameters.
 */
#define AUDIO_SAMPLING_FREQUENCY    48000U
#define AUDIO_RESOLUTION            32U
#define AUDIO_SAMPLES_PER_FRAME     (AUDIO_SAMPLING_FREQUENCY / 1000)

#define DAC_AUDIO_CHANNELS              2U
#define DAC_AUDIO_PACKET_SIZE           (AUDIO_SAMPLES_PER_FRAME * DAC_AUDIO_CHANNELS \
                                     * AUDIO_RESOLUTION / 8)
/* Because of samplerate feedback host can send more samples per frame */
#define DAC_AUDIO_MAX_PACKET_SIZE       (DAC_AUDIO_PACKET_SIZE + 4)
#define DAC_AUDIO_BUFFER_SIZE           (AUDIO_SAMPLES_PER_FRAME * DAC_AUDIO_CHANNELS * 8)

#if DAC_AUDIO_BUFFER_SIZE != (DAC_AUDIO_PACKET_SIZE * 2)
#error wtf
#endif


#define USB_AUDIO_CHANNELS              4U
#define USB_AUDIO_PACKET_SIZE           (AUDIO_SAMPLES_PER_FRAME * USB_AUDIO_CHANNELS \
                                     * AUDIO_RESOLUTION / 8)
/* Because of samplerate feedback host can send more samples per frame */
#define USB_AUDIO_MAX_PACKET_SIZE       (USB_AUDIO_PACKET_SIZE + 16)
#define USB_AUDIO_BUFFER_SIZE           (AUDIO_SAMPLES_PER_FRAME * USB_AUDIO_CHANNELS * 8)

#if USB_AUDIO_BUFFER_SIZE != (USB_AUDIO_PACKET_SIZE * 2)
#error wtf
#endif





/*
 * USB Audio Class parameters
 */
#define AUDIO_PLAYBACK_ENDPOINT     0x01
#define AUDIO_FEEDBACK_ENDPOINT     0x02
#define MIDI_ENDPOINT               0x03
#define AUDIO_FEEDBACK_ENDPOINT     0x02
#define AUDIO_CONTROL_INTERFACE     0
#define AUDIO_STREAMING_INTERFACE   1
#define MIDI_STREAMING_INTERFACE    2

#define AUDIO_INPUT_UNIT_ID         1
#define AUDIO_FUNCTION_UNIT_ID      2
#define AUDIO_OUTPUT_UNIT_ID        3


typedef struct {

  /* Audio events source */
  event_source_t audio_events;

  /* Audio playback occurs */
  bool playback;
  /* Samplerate feedback valid */
  bool sof_feedback_valid;

  /* Buffer underflows/overflows */
  int buffer_errors;

  /* Channel mute states */
  bool mute[2];
  /* Channel volumes in 8.8 format (dB) */
  int16_t volume[2];
} audio_state_t;


#endif  /* _USBCFG_H_ */

/** @} */
