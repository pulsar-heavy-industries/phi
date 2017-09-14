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
#include <string.h>
#include "hal.h"
#include "usbcfg.h"
#include "phi_lib/phi_lib.h"

/* Virtual serial port over USB.*/
//SerialUSBDriver SDU1;
MIDIUSBDriver MDU1;
volatile int32_t dac_buffer[DAC_AUDIO_BUFFER_SIZE + DAC_AUDIO_MAX_PACKET_SIZE];
volatile int32_t dac_buffer2[DAC_AUDIO_BUFFER_SIZE + DAC_AUDIO_MAX_PACKET_SIZE];
/*
 * Endpoints to be used for USBD1.
 */
#define USBD1_DATA_REQUEST_EP           1
#define USBD1_DATA_AVAILABLE_EP         1
#define USBD2_DATA_REQUEST_EP           2
#define USBD2_DATA_AVAILABLE_EP         2


void audioObjectInit(audio_state_t *ap) {
  chEvtObjectInit(&ap->audio_events);

  ap->mute[0] = 0;
  ap->mute[1] = 0;
  ap->playback = false;
  ap->sof_feedback_valid = false;
  ap->buffer_errors = 0;
  ap->volume[0] = 0;
  ap->volume[1] = 0;

  memset(dac_buffer,0,sizeof(dac_buffer));
}


/*
 * USB Device Descriptor.
 */


static const uint8_t audio_device_descriptor_data[18] = {
  USB_DESC_DEVICE       (0x0110,        /* bcdUSB (1.1).                    */
                         0x00,          /* bDeviceClass (None).             */
                         0x00,          /* bDeviceSubClass.                 */
                         0x00,          /* bDeviceProtocol.                 */
                         0x40,          /* bMaxPacketSize.                  */
                         0x0483 + 20,        /* idVendor (ST).                   */
                         0x5343 + 20,        /* idProduct.                       */
                         0x0001,        /* bcdDevice.                       */
                         1,             /* iManufacturer.                   */
                         2,             /* iProduct.                        */
                         3,             /* iSerialNumber.                   */
                         1)             /* bNumConfigurations.              */
};

/*
 * Device Descriptor wrapper.
 */
static const USBDescriptor audio_device_descriptor = {
  sizeof audio_device_descriptor_data,
  audio_device_descriptor_data
};


static const uint8_t audio_configuration_descriptor_data[122 + 4 + (9+66) + 9+9+1+13] = {
  /* Configuration Descriptor. (UAC 4.2) */
  USB_DESC_CONFIGURATION(122 + 4 + (9+66) + 9+9+1+13,           /* wTotalLength.                    */
                         0x03,          /* bNumInterfaces.                  */
                         0x01,          /* bConfigurationValue.             */
                         0,             /* iConfiguration.                  */
                         0xC0,          /* bmAttributes (self powered).     */
                         50),           /* bMaxPower (100mA).               */
  /*  Standard Audio Control Interface Descriptor (UAC 4.3.1) */
  USB_DESC_INTERFACE    (AUDIO_CONTROL_INTERFACE, /* bInterfaceNumber.      */
                         0x00,          /* bAlternateSetting.               */
                         0x00,          /* bNumEndpoints.                   */
                         0x01,          /* bInterfaceClass (AUDIO).         */
                         0x01,          /* bInterfaceSubClass
                                           (AUDIO_CONTROL).                 */
                         0x00,          /* bInterfaceProtocol (none).       */
                         0),            /* iInterface.                      */
  /*  Class-specific AC Interface Descriptor (UAC 4.3.2) */
  USB_DESC_BYTE         (10),            /* bLength.                         */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x01),         /* bDescriptorSubtype (Header).     */
  USB_DESC_BCD          (0x0100),       /* bcdADC.                          */
  USB_DESC_WORD         (47),           /* wTotalLength.                    */
  USB_DESC_BYTE         (0x02),         /* bInCollection (1 streaming
                                           interfaces).                     */
  USB_DESC_BYTE         (AUDIO_STREAMING_INTERFACE), /* baInterfaceNr.      */
  USB_DESC_BYTE         (MIDI_STREAMING_INTERFACE), /* baInterfaceNr.      */
  /* Input Terminal Descriptor (UAC 4.3.2.1) */
  USB_DESC_BYTE         (12),           /* bLength.                         */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType.                 */
  USB_DESC_BYTE         (0x02),         /* bDescriptorSubtype (Input
                                           Terminal).                       */
  USB_DESC_BYTE         (AUDIO_INPUT_UNIT_ID), /* bTerminalID.              */
  USB_DESC_WORD         (0x0101),       /* wTerminalType (USB streaming).   */
  USB_DESC_BYTE         (0x00),         /* bAssocTerminal (none).           */
  USB_DESC_BYTE         (4),            /* bNrChannels (2).                 */
  USB_DESC_WORD         (0x0003),       /* wChannelConfig (left, right).    */
  USB_DESC_BYTE         (0x07),         /* iChannelNames (none).            */
  USB_DESC_BYTE         (0x00),         /* iTerminal (none).                */
  /* Feature Unit Descriptor (UAC 4.3.2.5) */
  USB_DESC_BYTE         (13 + 4),           /* bLength.                         */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType.                 */
  USB_DESC_BYTE         (0x06),         /* bDescriptorSubtype (Feature
                                           Unit).                           */
  USB_DESC_BYTE         (AUDIO_FUNCTION_UNIT_ID), /* bUnitID.               */
  USB_DESC_BYTE         (AUDIO_INPUT_UNIT_ID), /* bSourceID.                */
  USB_DESC_BYTE         (2),            /* bControlSize (2).                */
  USB_DESC_WORD         (0x0000),       /* Master controls.                 */
  USB_DESC_WORD         (0x0003),       /* Channel 0 controls               */
  USB_DESC_WORD         (0x0003),       /* Channel 1 controls               */
  USB_DESC_WORD         (0x0003),       /* Channel 0 controls               */
  USB_DESC_WORD         (0x0003),       /* Channel 1 controls               */
  USB_DESC_BYTE         (0x00),         /* iFeature (none)                  */
  /* Output Terminal Descriptor (UAC 4.3.2.2) */
  USB_DESC_BYTE         (9),            /* bLength.                         */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType.                 */
  USB_DESC_BYTE         (0x03),         /* bDescriptorSubtype (Output
                                            Terminal).                      */
  USB_DESC_BYTE         (AUDIO_OUTPUT_UNIT_ID), /* bTerminalID.             */
  USB_DESC_WORD         (0x0302),       /* wTerminalType (Headphones).      */
  USB_DESC_BYTE         (0x00),         /* bAssocTerminal (none).           */
  USB_DESC_BYTE         (AUDIO_FUNCTION_UNIT_ID), /* bSourceID.             */
  USB_DESC_BYTE         (0x00),         /* iTerminal (none).                */
  /* Standard AS Interface Descriptor (empty) (UAC 4.5.1) */
  USB_DESC_INTERFACE    (AUDIO_STREAMING_INTERFACE, /* bInterfaceNumber.    */
                         0x00,          /* bAlternateSetting.               */
                         0x00,          /* bNumEndpoints.                   */
                         0x01,          /* bInterfaceClass (AUDIO).         */
                         0x02,          /* bInterfaceSubClass
                                           (AUDIO_STREAMING).               */
                         0x00,          /* bInterfaceProtocol (none).       */
                         0),            /* iInterface.                      */
  /* Standard AS Interface Descriptor (functional) (UAC 4.5.1) */
  USB_DESC_INTERFACE    (AUDIO_STREAMING_INTERFACE, /* bInterfaceNumber.    */
                         0x01,          /* bAlternateSetting.               */
                         0x02,          /* bNumEndpoints.                   */
                         0x01,          /* bInterfaceClass (AUDIO).         */
                         0x02,          /* bInterfaceSubClass
                                           (AUDIO_STREAMING).               */
                         0x00,          /* bInterfaceProtocol (none).       */
                         0),            /* iInterface.                      */
  /* Class-specific AS Interface Descriptor (UAC 4.5.2) */
  USB_DESC_BYTE         (7),            /* bLength.                         */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x01),         /* bDescriptorSubtype (General).    */
  USB_DESC_BYTE         (AUDIO_INPUT_UNIT_ID), /* bTerminalLink.            */
  USB_DESC_BYTE         (0x00),         /* bDelay (none).                   */
  USB_DESC_WORD         (0x0001),       /* wFormatTag (PCM format).         */
  /* Class-Specific AS Format Type Descriptor (UAC 4.5.3) */
  USB_DESC_BYTE         (11),           /* bLength.                         */
  USB_DESC_BYTE         (0x24),         /* bDescriptorType (CS_INTERFACE).  */
  USB_DESC_BYTE         (0x02),         /* bDescriptorSubtype (Format).     */
  USB_DESC_BYTE         (0x01),         /* bFormatType (Type I).            */
  USB_DESC_BYTE         (0x04),         /* bNrChannels (2).                 */
  USB_DESC_BYTE         (0x04),         /* bSubFrameSize (2).               */
  USB_DESC_BYTE         (24),           /* bBitResolution.              */
  USB_DESC_BYTE         (0x01),         /* bSamFreqType (Type I).           */
  USB_DESC_BYTE         (AUDIO_SAMPLING_FREQUENCY & 0xFF),
  USB_DESC_BYTE         ((AUDIO_SAMPLING_FREQUENCY >> 8) & 0xFF),
  USB_DESC_BYTE         ((AUDIO_SAMPLING_FREQUENCY >> 16) & 0xFF),
  /* Standard AS Isochronous Audio Data Endpoint Descriptor (UAC 4.6.1.1) */
  USB_DESC_BYTE         (9),            /* bLength (9).                     */
  USB_DESC_BYTE         (0x05),         /* bDescriptorType (Endpoint).      */
  USB_DESC_BYTE         (AUDIO_PLAYBACK_ENDPOINT), /* bEndpointAddress.     */
  USB_DESC_BYTE         (0x05),         /* bmAttributes (asynchronous).     */
  USB_DESC_WORD         (USB_AUDIO_MAX_PACKET_SIZE), /* wMaxPacketSize          */
  USB_DESC_BYTE         (0x01),         /* bInterval (1 ms).                */
  USB_DESC_BYTE         (0x00),         /* bRefresh (0).                    */
  USB_DESC_BYTE         (AUDIO_FEEDBACK_ENDPOINT | 0x80), /* bSynchAddress. */
  /* C-S AS Isochronous Audio Data Endpoint Descriptor (UAC 4.6.1.2) */
  USB_DESC_BYTE         (7),            /* bLength.                         */
  USB_DESC_BYTE         (0x25),         /* bDescriptorType (CS_ENDPOINT).   */
  USB_DESC_BYTE         (0x01),         /* bDescriptorSubtype (General).    */
  USB_DESC_BYTE         (0x00),         /* bmAttributes (none).             */
  USB_DESC_BYTE         (0x02),         /* bLockDelayUnits (PCM Samples).   */
  USB_DESC_WORD         (0x0000),       /* bLockDelay (0).                  */
  /* Standard Isochronous Audio Feedback Endpoint Descriptor */
  USB_DESC_BYTE         (9),            /* bLength (9).                     */
  USB_DESC_BYTE         (0x05),         /* bDescriptorType (Endpoint).      */
  USB_DESC_BYTE         (AUDIO_FEEDBACK_ENDPOINT | 0x80), /* bEndpointAddress.*/
  USB_DESC_BYTE         (0x11),         /* bmAttributes (nosync, feedback). */
  USB_DESC_WORD         (4),            /* wMaxPacketSize                   */
  USB_DESC_BYTE         (0x01),         /* bInterval (1 ms).                */
  USB_DESC_BYTE         (0x05),         /* bRefresh (32 ms).                */
  USB_DESC_BYTE         (0x00),         /* bSynchAddress (none).            */

  USB_DESC_INTERFACE    (MIDI_STREAMING_INTERFACE, /* bInterfaceNumber.    */
                         0x00,          /* bAlternateSetting.               */
                         0x02,          /* bNumEndpoints.                   */
                         0x01,          /* bInterfaceClass (AUDIO).         */
                         0x03,          /* bInterfaceSubClass
                                           (MIDI_STREAMING).               */
                         0x00,          /* bInterfaceProtocol (none).       */
                         0),            /* iInterface.                      */

         0x07, 0x24, 0x01, 0x00, 0x01, 0x41 + 9+9+1+13, 0x00,             // CS Interface (midi)      CLASS SPECIFIC MS INTERFACE DESC
         0x06, 0x24, 0x02, 0x01, 0x01, 0x05,                   //   IN  Jack 1 (emb)       MIDI IN JACK DESC (bLength bDescType bDescSubType bJackType bJackID iJack)
         0x06, 0x24, 0x02, 0x02, 0x02, 0x05,                   //   IN  Jack 2 (ext)       MIDI IN JACK DESC (bLength bDescType bDescSubType bJackType bJackID iJack)
         0x06, 0x24, 0x02, 0x01, 0x03, 0x06,                   //   IN  Jack 3 (emb)       MIDI IN JACK DESC (bLength bDescType bDescSubType bJackType bJackID iJack)
         0x06, 0x24, 0x02, 0x02, 0x04, 0x06,                   //   IN  Jack 4 (ext)       MIDI IN JACK DESC (bLength bDescType bDescSubType bJackType bJackID iJack)
         0x09, 0x24, 0x03, 0x01, 0x05, 0x01, 0x02, 0x01, 0x05, //   OUT Jack 5 (emb)       MIDI OUT JACK DESC (bLength bDescType bDescSubType bJackType bJackID bNrInputPins baSourceID(1) baSourceID(1) iJack)
         0x09, 0x24, 0x03, 0x02, 0x06, 0x01, 0x01, 0x01, 0x05, //   OUT Jack 6 (ext)       MIDI OUT JACK DESC (bLength bDescType bDescSubType bJackType bJackID bNrInputPins baSourceID(1) baSourceID(1) iJack)
		 0x09, 0x24, 0x03, 0x01, 0x07, 0x01, 0x04, 0x01, 0x06, //   OUT Jack 7 (emb)       MIDI OUT JACK DESC (bLength bDescType bDescSubType bJackType bJackID bNrInputPins baSourceID(1) baSourceID(1) iJack)
		 0x09, 0x24, 0x03, 0x02, 0x08, 0x01, 0x03, 0x01, 0x06, //   OUT Jack 8 (ext)       MIDI OUT JACK DESC (bLength bDescType bDescSubType bJackType bJackID bNrInputPins baSourceID(1) baSourceID(1) iJack)
         0x09, 0x05, 0x03, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, // Endpoint OUT             ENDPOINT DESC  (bLength bDescType bEndpointAddr bmAttr wMaxPacketSize(2 bytes)  bInterval bRefresh bSyncAddress)
         0x06, 0x25, 0x01, 0x02, 0x01, 0x03,                         //   CS EP IN  Jack         CLASS SPECIFIC MS BULK DATA EP DESC
         0x09, 0x05, 0x83, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, // Endpoint IN              ENDPOINT DESC  (bLength bDescType bEndpointAddr bmAttr wMaxPacketSize(2 bytes)  bInterval bRefresh bSyncAddress)
         0x06, 0x25, 0x01, 0x02, 0x05, 0x07,                         //   CS EP OUT Jack          CLASS SPECIFIC MS BULK DATA EP DESC};
};

/*
 * Configuration Descriptor wrapper.
 */
static const USBDescriptor audio_configuration_descriptor = {
  sizeof audio_configuration_descriptor_data,
  audio_configuration_descriptor_data
};

/*
 * U.S. English language identifier.
 */
static const uint8_t audio_string0[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t audio_string1[] = {
	USB_DESC_BYTE(2 + 46),                /* bLength.                         */
	USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
	'P', 0, 'u', 0, 'l', 0, 's', 0, 'a', 0, 'r', 0, ' ', 0,
	'H', 0, 'e', 0, 'a', 0, 'v', 0, 'y', 0, ' ', 0,
	'I', 0, 'n', 0, 'd', 0, 'u', 0, 's', 0, 't', 0, 'r', 0, 'i', 0, 'e', 0, 's', 0,
};

/*
 * Device Description string.
 */
static const uint8_t audio_string2[] = {
	USB_DESC_BYTE(2 + 58),                    /* bLength.                         */
	USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
	'P', 0, 'u', 0, 'l', 0, 's', 0, 'a', 0, 'r', 0, ' ', 0,
	'H', 0, 'e', 0, 'a', 0, 'v', 0, 'y', 0, ' ', 0,
	'I', 0, 'n', 0, 'd', 0, 'u', 0, 's', 0, 't', 0, 'r', 0, 'i', 0, 'e', 0, 's', 0, ' ', 0,
	'N', 0, 'a', 0, 'r', 0, 'v', 0, 'i', 0,
};

/*
 * Serial Number string.
 */
static const uint8_t audio_string3[] = {
  USB_DESC_BYTE(8),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '0' + CH_KERNEL_MAJOR, 0,
  '0' + CH_KERNEL_MINOR, 0,
  '0' + CH_KERNEL_PATCH, 0
};

/* Unused? */
static const uint8_t audio_string4[] = {
	  USB_DESC_BYTE(2 + 58),                    /* bLength.                         */
	  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
	  'P', 0, 'u', 0, 'l', 0, 's', 0, 'a', 0, 'r', 0, ' ', 0,
	  'H', 0, 'e', 0, 'a', 0, 'v', 0, 'y', 0, ' ', 0,
	  'I', 0, 'n', 0, 'd', 0, 'u', 0, 's', 0, 't', 0, 'r', 0, 'i', 0, 'e', 0, 's', 0, ' ', 0,
	  'N', 0, 'a', 0, 'r', 0, 'v', 0, 'i', 0,
};

static const uint8_t audio_string5[] = {
  USB_DESC_BYTE(20),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'N', 0,
  'a', 0,
  'r', 0,
  'v', 0,
  'i', 0,
  'C', 0,
  't', 0,
  'r', 0,
  'l', 0,
};

static const uint8_t audio_string6[] = {
	USB_DESC_BYTE(18),                     /* bLength.                         */
	USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
	'N', 0,
	'a', 0,
	'r', 0,
	'v', 0,
	'i', 0,
	'E', 0,
	'x', 0,
	't', 0,
};


static const uint8_t audio_string7[] = {
	USB_DESC_BYTE(34),                     /* bLength.                         */
	USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
	'H', 0,
	'e', 0,
	'a', 0,
	'd', 0,
	'p', 0,
	'h', 0,
	'o', 0,
	'n', 0,
	'e', 0,
	's', 0,
	' ', 0,
	'R', 0,
	'i', 0,
	'g', 0,
	'h', 0,
	't', 0,
};

static const uint8_t audio_string8[] = {
	USB_DESC_BYTE(32),                     /* bLength.                         */
	USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
	'H', 0,
	'e', 0,
	'a', 0,
	'd', 0,
	'p', 0,
	'h', 0,
	'o', 0,
	'n', 0,
	'e', 0,
	's', 0,
	' ', 0,
	'L', 0,
	'e', 0,
	'f', 0,
	't', 0,
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor audio_strings[] = {
  {sizeof audio_string0, audio_string0},
  {sizeof audio_string1, audio_string1},
  {sizeof audio_string2, audio_string2},
  {sizeof audio_string3, audio_string3},
  {sizeof audio_string4, audio_string4},
  {sizeof audio_string5, audio_string5},
  {sizeof audio_string6, audio_string6},
  {sizeof audio_string7, audio_string7},
  {sizeof audio_string8, audio_string8},
};

void inttohex(uint32_t v, unsigned char *p){
  int nibble;
  for (nibble = 0;nibble<8;nibble++){
    unsigned char c = (v>>(28-nibble*4))&0xF;
    if (c<10) c=c+'0';
    else c=c+'A'-10;
    *p = c;
    p += 2;
  }
}


static uint8_t descriptor_serial_string[] = {
  USB_DESC_BYTE(50),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0,
  '0', 0, '0', 0, '0', 0, '0', 0
};

static const USBDescriptor descriptor_serial = {
   sizeof descriptor_serial_string, descriptor_serial_string,
};



audio_state_t audio;
/* I2S buffer */
static uint32_t usb_buffer[USB_AUDIO_BUFFER_SIZE + USB_AUDIO_MAX_PACKET_SIZE];

/* I2S buffer write address */
static uint32_t dac_buffer_wr_addr = 0;
static uint32_t dac_buffer_wr_addr2 = 0;

/*
 * Handles the GET_DESCRIPTOR callback. All required descriptors must be
 * handled here.
 */
static const USBDescriptor *get_descriptor(USBDriver *usbp,
                                           uint8_t dtype,
                                           uint8_t dindex,
                                           uint16_t lang) {

  (void)usbp;
  (void)lang;
  switch (dtype) {
  case USB_DESCRIPTOR_DEVICE:
    return &audio_device_descriptor;
  case USB_DESCRIPTOR_CONFIGURATION:
    return &audio_configuration_descriptor;
  case USB_DESCRIPTOR_STRING:
	  if (dindex == 3) {
	        inttohex(*((uint32_t*)STM32_REG_UNIQUE_ID),&descriptor_serial_string[2]);
	        inttohex(*((uint32_t*)STM32_REG_UNIQUE_ID + 4),&descriptor_serial_string[2+16]);
	        inttohex(*((uint32_t*)STM32_REG_UNIQUE_ID + 8),&descriptor_serial_string[2+32]);
	        return &descriptor_serial;
	      }

    if (dindex < PHI_ARRLEN(audio_strings))
      return &audio_strings[dindex];
    else
    	chDbgCheck(FALSE);
  }
  return NULL;
}


static volatile uint32_t sof_last_counter = 0;
static volatile uint32_t sof_delta = 0;
static volatile bool sof_first = true;
static volatile int sof_delta_count = 0;
static uint8_t sof_feedback_data[3];


/*
 * TIM2 interrupt handler
 * TIM2 clocked by I2S master clock (PC7 (I2S_MCK) connected to PA5 (TIM2_ETR)).
 * TIM2 triggers on USB start of frame.
 */
OSAL_IRQ_HANDLER(STM32_TIM2_HANDLER) {
  OSAL_IRQ_PROLOGUE();

  uint32_t value = TIM2->CNT;
  uint32_t sr = TIM2->SR;
  TIM2->SR = 0;

  if (sr & TIM_SR_TIF) {
    chSysLockFromISR();
    if (!sof_first) {
      if (sof_last_counter < value)
        sof_delta += value - sof_last_counter;
      else
        sof_delta += UINT32_MAX - sof_last_counter + value;

      /* Feedback value calculated every 32 SOFs = 32 ms */
      if (sof_delta_count == 31) {
        /* 10.14 format F = 256fs (8 bit), 32 frames (5 bits) = 19.13 */
        uint32_t f1014 = (sof_delta << 1) & 0xFFFFFFUL;
        sof_feedback_data[0] = f1014 & 0xFF;
        sof_feedback_data[1] = (f1014 >> 8) & 0xFF;
        sof_feedback_data[2] = (f1014 >> 16) & 0xFF;
        sof_delta = 0;
        sof_delta_count = 0;
        audio.sof_feedback_valid = true;
      } else {
        sof_delta_count++;
      }
    }
    sof_first = false;
    sof_last_counter = value;
    chSysUnlockFromISR();
  }

  OSAL_IRQ_EPILOGUE();
}

/*
 * Start frame interval measure.
 */
void start_sof_capture(void) {
	  palSetPadMode(GPIOA, 5, PAL_MODE_ALTERNATE(1));

	  rccEnableTIM2(FALSE);

  /* Reset TIM2 */
  rccResetTIM2();
  nvicEnableVector(STM32_TIM2_NUMBER, STM32_GPT_TIM2_IRQ_PRIORITY);

  chSysLock();
  sof_last_counter = 0;
  sof_delta = 0;
  sof_first = true;
  sof_delta_count = 0;
  audio.sof_feedback_valid = false;

  /* Enable TIM2 counting */
  TIM2->CR1 = TIM_CR1_CEN;
  /* Timer clock = ETR pin, slave mode, trigger on ITR1 */
  TIM2->SMCR = TIM_SMCR_ECE | TIM_SMCR_TS_0 | TIM_SMCR_SMS_2 | TIM_SMCR_SMS_1;
  /* TIM2 enable interrupt */
  TIM2->DIER = TIM_DIER_TIE;
  /* Remap ITR1 to USB_FS SOF signal */
  TIM2->OR = TIM_OR_ITR1_RMP_1 | TIM_OR_ITR1_RMP_0;
  chSysUnlock();
}

/*
 * Stop frame interval measure.
 */
void stop_sof_capture(void) {
  chSysLock();
  nvicDisableVector(STM32_TIM2_NUMBER);
  TIM2->CR1 = 0;
  audio.sof_feedback_valid = false;
  chSysUnlock();
}

/*
 * Feedback transmitted (or dropped) in current frame.
 */
void audio_feedback(USBDriver *usbp, usbep_t ep) {
  if (audio.playback) {
    /* Transmit feedback data */
    chSysLockFromISR();
    if (audio.sof_feedback_valid)
      usbStartTransmitI(usbp, ep, sof_feedback_data, 3);
    else
      usbStartTransmitI(usbp, ep, NULL, 0);
    chSysUnlockFromISR();
  }
}


/*
 */
uint16_t usb_w;

#define M_PI  3.14159265358979323846f
#define FREQUENCY 440.0f // that extra ".0" is important
#define SAMPLES  48000
#define PERIOD (1.0f / FREQUENCY)
const float TIME_STEP = 1 / (float)SAMPLES;

volatile int total_samples_received = 0;
volatile int last = 0;
volatile int calls = 0;

uint32_t VU_Level_Left, VU_Level_Right;
uint32_t HAL_SAI_GetVULevels(void)
{
	uint32_t vuLevel;
 	vuLevel  = (uint32_t)((VU_Level_Left >> 16)& 0xFF);
	vuLevel <<= 16;
	vuLevel |= ((VU_Level_Right>>16) & 0xFF);
	VU_Level_Left = VU_Level_Right = 0;
	return vuLevel;
}


void usb_reset_audio_bufs(void)
{
	memset(dac_buffer, 0, sizeof(dac_buffer));
	memset(dac_buffer2, 0, sizeof(dac_buffer2));
	dac_buffer_wr_addr = 0;
}

void audio_received(USBDriver *usbp, usbep_t ep) {
  if (audio.playback) {
      int samples_received = usbGetReceiveTransactionSizeX(usbp, ep) / 8;
      uint16_t new_addr = dac_buffer_wr_addr + samples_received;

      last = usbGetReceiveTransactionSizeX(usbp, ep);
      total_samples_received += samples_received;
      ++calls;

      int i, j, i2;
      static float time = 0;


      for (i = 0, j = 0, i2 = 0; i < samples_received; )
      {
          int32_t s;

          s = usb_buffer[j++] >> 8;
          dac_buffer2[dac_buffer_wr_addr + i] = s;

          if (s & 0x800000)
		  {
        	  s |= 0xff << 24;
		  }
          s = abs(s);
          if (s > VU_Level_Left) {
        	  VU_Level_Left = s;
		  }


          s = usb_buffer[j++] >> 8;
          dac_buffer2[dac_buffer_wr_addr + i + 1] = s;
          if (s & 0x800000)
		  {
        	  s |= 0xff << 24;
		  }
          s = abs(s);
          if (s > VU_Level_Right) {
        	  VU_Level_Right = s;
		  }

          s = usb_buffer[j++];// << 8;
//          s = ((s & 0xFFFF) << 16) | (s >> 16);
          dac_buffer[dac_buffer_wr_addr + i] = s >> 8;


          s = usb_buffer[j++];
//          s = ((s & 0xFFFF) << 16) | (s >> 16);
          dac_buffer[dac_buffer_wr_addr + i + 1] = s >> 8;


          i += 2;

      }

    /* Handle buffer wrap */
    if (new_addr >= DAC_AUDIO_BUFFER_SIZE) {
      for (int i = DAC_AUDIO_BUFFER_SIZE; i < new_addr; i++) {
        dac_buffer[i - DAC_AUDIO_BUFFER_SIZE] = dac_buffer[i];
        dac_buffer2[i - DAC_AUDIO_BUFFER_SIZE] = dac_buffer2[i];
      }
      new_addr -= DAC_AUDIO_BUFFER_SIZE;
    }
    dac_buffer_wr_addr = new_addr;

    chSysLockFromISR();
    usbStartReceiveI(usbp, ep, (uint8_t *)&usb_buffer[0], USB_AUDIO_MAX_PACKET_SIZE);
    chSysUnlockFromISR();
  }
}



MIDIUSBDriver MDU1;
const MIDIUSBConfig midiusbcfg = {
  &USBD2,
  3,
  3,
};


/*
 * EP1 states.
 */
static USBOutEndpointState ep1outstate;

/*
 * EP1 initialization structure (IN & OUT).
 */
static const USBEndpointConfig ep1config = {
  USB_EP_MODE_TYPE_ISOC,
  NULL,
  NULL,
  audio_received,
  0x0000,
  USB_AUDIO_MAX_PACKET_SIZE,
  NULL,
  &ep1outstate,
  1,
  NULL
};

/*
 * EP2 states.
 */
static USBInEndpointState ep2instate;

/*
 * EP1 initialization structure (IN & OUT).
 */
static const USBEndpointConfig ep2config = {
  USB_EP_MODE_TYPE_ISOC,
  NULL,
  audio_feedback,
  NULL,
  0x0004,
  0x0000,
  &ep2instate,
  NULL,
  1,
  NULL
};



/**
 * @brief   IN EP3 state.
 */
static USBInEndpointState ep3instate;

/**
 * @brief   OUT EP3 state.
 */
static USBOutEndpointState ep3outstate;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep3config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  mduDataTransmitted,
  mduDataReceived,
  0x0040,
  0x0040,
  &ep3instate,
  &ep3outstate,
  2,
  NULL
};

/*
 * Temporary buffer for control data.
 */
static uint8_t control_data[8];
static uint8_t control_channel;

/*
 * Volume data received.
 */
static void notify_volume(USBDriver *usbp) {
  (void)usbp;

  if (control_channel == 0xff) {
    memcpy(audio.volume, control_data + sizeof(int16_t), 2 * sizeof(int16_t));
  } else {
    memcpy(&audio.volume[control_channel - 1], control_data, sizeof(int16_t));
  }
  chSysLockFromISR();
  chEvtBroadcastFlagsI(&audio.audio_events, AUDIO_EVENT_VOLUME);
  chSysUnlockFromISR();
}

/*
 * Mute data received.
 */
static void notify_mute(USBDriver *usbp) {
  (void)usbp;

  if (control_channel == 0xff) {
    audio.mute[0] = control_data[1];
    audio.mute[1] = control_data[2];
  } else {
    audio.mute[control_channel - 1] = control_data[0];
  }
  chSysLockFromISR();
  chEvtBroadcastFlagsI(&audio.audio_events, AUDIO_EVENT_MUTE);
  chSysUnlockFromISR();
}

/*
 * Handles requests for audio function unit (volume & mute).
 */
bool audio_volume_control(USBDriver *usbp, uint8_t req, uint8_t ctrl,
                          uint8_t channel, uint16_t length) {
  switch(req) {
  case UAC_REQ_SET_MAX:
  case UAC_REQ_SET_MIN:
  case UAC_REQ_SET_RES:
    if (ctrl == UAC_FU_VOLUME_CONTROL) {
      usbSetupTransfer(usbp, control_data, length, NULL);
      return true;
    }
  case UAC_REQ_GET_MAX:
    if (ctrl == UAC_FU_VOLUME_CONTROL) {
      for(int i = 0; i < length; i++)
        ((int16_t*)control_data)[i] = 0;
      usbSetupTransfer(usbp, control_data, length, NULL);
      return true;
    }
  case UAC_REQ_GET_MIN:
    if (ctrl == UAC_FU_VOLUME_CONTROL) {
      for(int i = 0; i < length; i++)
        ((int16_t*)control_data)[i] = -96 * 256;
      usbSetupTransfer(usbp, control_data, length, NULL);
      return true;
    }
  case UAC_REQ_GET_RES:
    if (ctrl == UAC_FU_VOLUME_CONTROL) {
      for(int i = 0; i < length; i++)
        ((int16_t*)control_data)[i] = 128;
      usbSetupTransfer(usbp, control_data, length, NULL);
      return true;
    }
  case UAC_REQ_GET_CUR:
    if (ctrl == UAC_FU_MUTE_CONTROL) {
      if (channel == 0xff) {
        uint8_t value[3] = {0, audio.mute[0], audio.mute[1]};
        memcpy(control_data, value, sizeof(value));
        usbSetupTransfer(usbp, control_data, length, NULL);
      } else {
        memcpy(control_data, &audio.mute[channel - 1], sizeof(uint8_t));
        usbSetupTransfer(usbp, control_data, length, NULL);
      }
      return true;
    } else if (ctrl == UAC_FU_VOLUME_CONTROL) {
      if (channel == 0xff) {
        int16_t value[3] = {0, audio.volume[0], audio.volume[1]};
        memcpy(control_data, value, sizeof(value));
        usbSetupTransfer(usbp, control_data, length, NULL);
      } else {
        memcpy(control_data, &audio.volume[channel - 1], sizeof(int16_t));
        usbSetupTransfer(usbp, control_data, length, NULL);
      }
      return true;
    }
  case UAC_REQ_SET_CUR:
    if (ctrl == UAC_FU_MUTE_CONTROL) {
      control_channel = channel;
      usbSetupTransfer(usbp, control_data, length, notify_mute);
      return true;
    } else if (ctrl == UAC_FU_VOLUME_CONTROL) {
      control_channel = channel;
      usbSetupTransfer(usbp, control_data, length, notify_volume);
      return true;
    }
  default:
    return false;
  }
}

/*
 * Handles UAC-specific controls.
 */
bool audio_control(USBDriver *usbp, uint8_t iface, uint8_t entity, uint8_t req,
                   uint16_t wValue, uint16_t length) {
  /* Only requests to audio control iface are supported */
  if (iface == AUDIO_CONTROL_INTERFACE) {
    /* Feature unit */
    if (entity == AUDIO_FUNCTION_UNIT_ID) {
      return audio_volume_control(usbp, req, (wValue >> 8) & 0xFF,
                                  wValue & 0xFF, length);
    }
  }
  return false;
}

/*
 * Starts USB transfers, and notifies control thread.
 */
void start_playback(USBDriver *usbp) {
  if (!audio.playback) {
    audio.playback = true;
    audio.buffer_errors = 0;
    dac_buffer_wr_addr = DAC_AUDIO_PACKET_SIZE / 2;
    chSysLockFromISR();
    chEvtBroadcastFlagsI(&audio.audio_events, AUDIO_EVENT_PLAYBACK);
    usbStartTransmitI(usbp, AUDIO_FEEDBACK_ENDPOINT, NULL, 0);
    usbStartReceiveI(usbp, AUDIO_PLAYBACK_ENDPOINT, (uint8_t *)&usb_buffer[0], USB_AUDIO_MAX_PACKET_SIZE);
    chSysUnlockFromISR();
  }
}

/*
 * Stops playback, and notifies control thread.
 */
void stop_playback(USBDriver *usbp) {
  (void)usbp;

  if (audio.playback) {
    audio.playback = false;
    chSysLockFromISR();
    chEvtBroadcastFlagsI(&audio.audio_events, AUDIO_EVENT_PLAYBACK);
    chSysUnlockFromISR();
  }
}

/*
 * Handles SETUP requests.
 */
bool audio_requests_hook(USBDriver *usbp) {
  if ((usbp->setup[0] & (USB_RTYPE_TYPE_MASK | USB_RTYPE_RECIPIENT_MASK)) ==
      (USB_RTYPE_TYPE_STD | USB_RTYPE_RECIPIENT_INTERFACE)) {
    if (usbp->setup[1] == USB_REQ_SET_INTERFACE) {
      /* Switch between empty interface and normal one. */
      if (((usbp->setup[5] << 8) | usbp->setup[4]) == AUDIO_STREAMING_INTERFACE) {
        if(((usbp->setup[3] << 8) | usbp->setup[2]) == 1) {
          start_playback(usbp);
        } else {
          stop_playback(usbp);
        }
        usbSetupTransfer(usbp, NULL, 0, NULL);
        return true;
      }
    }
    return false;
  }
  else if ((usbp->setup[0] & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_CLASS) {
    switch(usbp->setup[0] & USB_RTYPE_RECIPIENT_MASK) {
    case USB_RTYPE_RECIPIENT_INTERFACE:
      return audio_control(usbp, usbp->setup[4], usbp->setup[5], usbp->setup[1],
                           (usbp->setup[3] << 8) | (usbp->setup[2]),
                           (usbp->setup[7] << 8) | (usbp->setup[6]));
    case USB_RTYPE_RECIPIENT_ENDPOINT:
    default:
      return false;
    }
  }
  return false;
}

/*
 * Handles the USB driver global events.
 */
static void usb_event(USBDriver *usbp, usbevent_t event) {
  chSysLockFromISR();
  chEvtBroadcastFlagsI(&audio.audio_events, AUDIO_EVENT_USB_STATE);
  chSysUnlockFromISR();

  switch (event) {
  case USB_EVENT_RESET:
    stop_playback(usbp);
    return;
  case USB_EVENT_ADDRESS:
    return;
  case USB_EVENT_CONFIGURED:
    chSysLockFromISR();
    /* Enables the endpoints specified into the configuration.
       Note, this callback is invoked from an ISR so I-Class functions
       must be used.*/
    usbInitEndpointI(usbp, AUDIO_PLAYBACK_ENDPOINT, &ep1config);
    usbInitEndpointI(usbp, AUDIO_FEEDBACK_ENDPOINT, &ep2config);
    usbInitEndpointI(usbp, MIDI_ENDPOINT, &ep3config);

    mduConfigureHookI(&MDU1);

    chSysUnlockFromISR();
    return;
  case USB_EVENT_SUSPEND:
    return;
  case USB_EVENT_WAKEUP:
    return;
  case USB_EVENT_STALLED:
    return;
  }
  return;
}

/*
 * USB driver configuration.
 */
 const USBConfig usbcfg = {
  usb_event,
  get_descriptor,
  audio_requests_hook,
  NULL,
};

