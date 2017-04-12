#include "phi_lib/phi_lib.h"
#include "phi_lib/phi_usb_midi.h"

MIDIUSBDriver MDU1;


/*
 * Endpoints to be used for USBD1.
 */
#define USBD1_DATA_REQUEST_EP           1
#define USBD1_DATA_AVAILABLE_EP         1


/*
 * USB Device Descriptor.
 */
static const uint8_t vcom_device_descriptor_data[18] = {
  USB_DESC_DEVICE       (0x0200,        /* bcdUSB (2.0).                    */
						 0xEF,          /* bDeviceClass (Misc).             */
						 0x02,          /* bDeviceSubClass.                 */
						 0x01,          /* bDeviceProtocol. (IAD)           */
						 0x40,          /* bMaxPacketSize.                  */
						 0x16CA,        /* idVendor.                        */
						 0x4349,        /* idProduct.                       */
						 0x0200,        /* bcdDevice.                       */
						 1,             /* iManufacturer.                   */
						 4,             /* iProduct.                        */
						 3,             /* iSerialNumber.                   */
						 1)             /* bNumConfigurations.              */
};

/*
 * Device Descriptor wrapper.
 */
static const USBDescriptor vcom_device_descriptor = {
  sizeof vcom_device_descriptor_data,
  vcom_device_descriptor_data
};

/* Configuration Descriptor tree for a CDC.*/
static const uint8_t vcom_configuration_descriptor_data[] = {
  /* Configuration Descriptor.*/
 USB_DESC_CONFIGURATION(109,            /* wTotalLength.                    */
						0x02,          /* bNumInterfaces.                  */
						0x01,          /* bConfigurationValue.             */
						5,             /* iConfiguration.                  */
						0xC0,          /* bmAttributes (self powered).     */
						50),           /* bMaxPower (100mA).               */
 /* Interface Association Descriptor.*/
 USB_DESC_INTERFACE_ASSOCIATION(0x00, /* bFirstInterface.                  */
								0x02, /* bInterfaceCount.                  */
								0x01, /* bFunctionClass (Audio).           */
								0x03, /* bFunctionSubClass.                */
								0x00, /* bFunctionProcotol                 */
								0),   /* iInterface.                       */
  0x09, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, // Interface 0              INTERFACE DESC (bLength bDescType bInterfaceNumber bAltSetting bNumEndpoints bInterfaceClass bInterfaceSubClass bInterfaceProtocol iInterface)
  0x09, 0x24, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x01, // CS Interface (audio)     CLASS SPECIFIC AC INTERFACE DESC
  0x09, 0x04, 0x01, 0x00, 0x02, 0x01, 0x03, 0x00, 0x00, // Interface 1              INTERFACE DESC (bLength bDescType bInterfaceNumber bAltSetting bNumEndpoints bInterfaceClass bInterfaceSubClass bInterfaceProtocol iInterface)
  0x07, 0x24, 0x01, 0x00, 0x01, 0x41, 0x00,             // CS Interface (midi)      CLASS SPECIFIC MS INTERFACE DESC
  0x06, 0x24, 0x02, 0x01, 0x01, 0x05,                   //   IN  Jack 1 (emb)       MIDI IN JACK DESC (bLength bDescType bDescSubType bJackType bJackID iJack)
  0x06, 0x24, 0x02, 0x02, 0x02, 0x05,                   //   IN  Jack 2 (ext)       MIDI IN JACK DESC (bLength bDescType bDescSubType bJackType bJackID iJack)
  0x09, 0x24, 0x03, 0x01, 0x03, 0x01, 0x02, 0x01, 0x05, //   OUT Jack 3 (emb)       MIDI OUT JACK DESC (bLength bDescType bDescSubType bJackType bJackID bNrInputPins baSourceID(1) baSourceID(1) iJack)
  0x09, 0x24, 0x03, 0x02, 0x04, 0x01, 0x01, 0x01, 0x02, //   OUT Jack 4 (ext)       MIDI OUT JACK DESC (bLength bDescType bDescSubType bJackType bJackID bNrInputPins baSourceID(1) baSourceID(1) iJack)
  0x09, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, // Endpoint OUT             ENDPOINT DESC  (bLength bDescType bEndpointAddr bmAttr wMaxPacketSize(2 bytes)  bInterval bRefresh bSyncAddress)
  0x05, 0x25, 0x01, 0x01, 0x01,                         //   CS EP IN  Jack         CLASS SPECIFIC MS BULK DATA EP DESC
  0x09, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, // Endpoint IN              ENDPOINT DESC  (bLength bDescType bEndpointAddr bmAttr wMaxPacketSize(2 bytes)  bInterval bRefresh bSyncAddress)
  0x05, 0x25, 0x01, 0x01, 0x03,                         //   CS EP OUT Jack          CLASS SPECIFIC MS BULK DATA EP DESC

};

/*
 * Configuration Descriptor wrapper.
 */
static const USBDescriptor vcom_configuration_descriptor = {
  sizeof vcom_configuration_descriptor_data,
  vcom_configuration_descriptor_data
};

/*
 * U.S. English language identifier.
 */
static const uint8_t vcom_string0[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};
/*
 * Vendor string.
 */
static const uint8_t vcom_string1[] = {
  USB_DESC_BYTE(2 + 46),                /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'P', 0, 'u', 0, 'l', 0, 's', 0, 'a', 0, 'r', 0, ' ', 0,
  'H', 0, 'e', 0, 'a', 0, 'v', 0, 'y', 0, ' ', 0,
  'I', 0, 'n', 0, 'd', 0, 'u', 0, 's', 0, 't', 0, 'r', 0, 'i', 0, 'e', 0, 's', 0,
};

/*
 * Device Description string.
 */
static const uint8_t vcom_string2[] = {
  USB_DESC_BYTE(12),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'C', 0, 'E', 0, 'N', 0, 'X', 0, '4', 0,
};

/*
 * Serial Number string.
 */
static const uint8_t vcom_string3[] = {
  USB_DESC_BYTE(8),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '0' + CH_KERNEL_MAJOR, 0,
  '0' + CH_KERNEL_MINOR, 0,
  '0' + CH_KERNEL_PATCH, 0
};

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


/*
 * Device Description string.
 */
static const uint8_t vcom_string4[] = {
  USB_DESC_BYTE(2 + 58),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'P', 0, 'u', 0, 'l', 0, 's', 0, 'a', 0, 'r', 0, ' ', 0,
  'H', 0, 'e', 0, 'a', 0, 'v', 0, 'y', 0, ' ', 0,
  'I', 0, 'n', 0, 'd', 0, 'u', 0, 's', 0, 't', 0, 'r', 0, 'i', 0, 'e', 0, 's', 0, ' ', 0,
  'C', 0, 'E', 0, 'N', 0, 'X', 0, '4', 0,
};



/*
 * Device Description string.
 */
static const uint8_t vcom_string5[] = {
  USB_DESC_BYTE(2 + 20),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'C', 0, 'E', 0, 'N', 0, 'X', 0, '4', 0, ' ', 0,
  'M', 0, 'I', 0, 'D', 0, 'I', 0,
};




/*
 * Strings wrappers array.
 */
static const USBDescriptor vcom_strings[] = {
  {sizeof vcom_string0, vcom_string0},
  {sizeof vcom_string1, vcom_string1},
  {sizeof vcom_string2, vcom_string2},
  {sizeof vcom_string3, vcom_string3},
  {sizeof vcom_string4, vcom_string4},
  {sizeof vcom_string5, vcom_string5},
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
    return &vcom_device_descriptor;
  case USB_DESCRIPTOR_CONFIGURATION:
    return &vcom_configuration_descriptor;
  case USB_DESCRIPTOR_STRING:
    if (dindex == 3) {
      inttohex(*((uint32_t*)STM32_REG_UNIQUE_ID),&descriptor_serial_string[2]);
      inttohex(*((uint32_t*)STM32_REG_UNIQUE_ID + 4),&descriptor_serial_string[2+16]);
      inttohex(*((uint32_t*)STM32_REG_UNIQUE_ID + 8),&descriptor_serial_string[2+32]);
      return &descriptor_serial;
    }
    if (dindex < 6)
      return &vcom_strings[dindex];

  }
  return NULL;
}

/**
 * @brief   IN EP1 state.
 */
static USBInEndpointState ep1instate;

/**
 * @brief   OUT EP1 state.
 */
static USBOutEndpointState ep1outstate;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep1config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  mduDataTransmitted,
  mduDataReceived,
  0x0040,
  0x0040,
  &ep1instate,
  &ep1outstate,
  2,
  NULL
};


/*
 * Handles the USB driver global events.
 */
static void usb_event(USBDriver *usbp, usbevent_t event) {
  switch (event) {
  case USB_EVENT_RESET:
    return;
  case USB_EVENT_ADDRESS:
    return;
  case USB_EVENT_CONFIGURED:
    chSysLockFromISR();

    /* Enables the endpoints specified into the configuration.
       Note, this callback is invoked from an ISR so I-Class functions
       must be used.*/
    usbInitEndpointI(usbp, USBD1_DATA_REQUEST_EP, &ep1config);

    /* Resetting the state of the CDC subsystem.*/
    mduConfigureHookI(&MDU1);

    chSysUnlockFromISR();
    return;
  case USB_EVENT_UNCONFIGURED:
    return;
  case USB_EVENT_SUSPEND:
    chSysLockFromISR();

    /* Disconnection event on suspend.*/
    // ??? mduDisconnectI(&SDU1);

    chSysUnlockFromISR();
    return;
  case USB_EVENT_WAKEUP:
    return;
  case USB_EVENT_STALLED:
    return;
  }
  return;
}

/*
 * Handles the USB driver global events.
 */

static bool specialRequestsHook(USBDriver *usbp) {
  return FALSE;
}

/*
 * USB driver configuration.
 */
const USBConfig usbcfg = {
  usb_event,
  get_descriptor,
  specialRequestsHook,
  NULL,
};


const MIDIUSBConfig midiusbcfg = {
  &USBD1,
  USBD1_DATA_REQUEST_EP,
  USBD1_DATA_AVAILABLE_EP,
};
