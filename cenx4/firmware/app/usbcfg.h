#ifndef _USBCFG_H_
#define _USBCFG_H_

#include "phi_lib/phi_usb_bulk.h"
#include "phi_lib/phi_usb_midi.h"

extern const USBConfig usbcfg;

extern BulkUSBDriver BDU1;
extern const BulkUSBConfig bulkusbcfg;

extern MIDIUSBDriver MDU1;
extern const MIDIUSBConfig midiusbcfg;

#endif  /* _USBCFG_H_ */
