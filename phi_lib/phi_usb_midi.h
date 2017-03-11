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

/**
 * @file    midi_usb.h
 * @brief   MIDI over USB Driver macros and structures.
 *
 * @addtogroup MIDI_USB
 * @{
 */

#ifndef _PHI_LIB_USB_MIDI_H_
#define _PHI_LIB_USB_MIDI_H_


/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    MIDI_USB configuration options
 * @{
 */
/**
 * @brief   MIDI over USB buffers size.
 * @details Configuration parameter, the buffer size must be a multiple of
 *          the USB data endpoint maximum packet size.
 * @note    The default is 256 bytes for both the transmission and receive
 *          buffers.
 */
#if !defined(MIDI_USB_BUFFERS_SIZE) || defined(__DOXYGEN__)
#define MIDI_USB_BUFFERS_SIZE     64
#endif

/**
 * @brief   MIDI over USB number of buffers.
 * @note    The default is 2 buffers.
 */
#if !defined(MIDI_USB_BUFFERS_NUMBER) || defined(__DOXYGEN__)
#define MIDI_USB_BUFFERS_NUMBER   2
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief Driver state machine possible states.
 */
typedef enum {
  MDU_UNINIT = 0,                   /**< Not initialized.                   */
  MDU_STOP = 1,                     /**< Stopped.                           */
  MDU_READY = 2                     /**< Ready.                             */
} mdustate_t;

/**
 * @brief   Structure representing a midi over USB driver.
 */
typedef struct MIDIUSBDriver MIDIUSBDriver;

/**
 * @brief   MIDI over USB Driver configuration structure.
 * @details An instance of this structure must be passed to @p mduStart()
 *          in order to configure and start the driver operations.
 */
typedef struct {
  /**
   * @brief   USB driver to use.
   */
  USBDriver                 *usbp;
  /**
   * @brief   Bulk IN endpoint used for outgoing data transfer.
   */
  usbep_t                   bulk_in;
  /**
   * @brief   Bulk OUT endpoint used for incoming data transfer.
   */
  usbep_t                   bulk_out;
} MIDIUSBConfig;

/**
 * @brief   @p MIDIDriver specific data.
 */
#define _phi_usb_midi_driver_data                                             \
  _base_asynchronous_channel_data                                           \
  /* Driver state.*/                                                        \
  mdustate_t                state;                                          \
  /* Input buffers queue.*/                                                 \
  input_buffers_queue_t     ibqueue;                                        \
  /* Output queue.*/                                                        \
  output_buffers_queue_t    obqueue;                                        \
  /* Input buffer.*/                                                        \
  uint8_t                   ib[BQ_BUFFER_SIZE(MIDI_USB_BUFFERS_NUMBER,    \
                                              MIDI_USB_BUFFERS_SIZE)];    \
  /* Output buffer.*/                                                       \
  uint8_t                   ob[BQ_BUFFER_SIZE(MIDI_USB_BUFFERS_NUMBER,    \
                                              MIDI_USB_BUFFERS_SIZE)];    \
  /* End of the mandatory fields.*/                                         \
  /* Current configuration data.*/                                          \
  const MIDIUSBConfig     *config;

/**
 * @brief   @p MIDIUSBDriver specific methods.
 */
#define _phi_usb_midi_driver_methods                                          \
  _base_asynchronous_channel_methods

/**
 * @extends BaseAsynchronousChannelVMT
 *
 * @brief   @p MIDIDriver virtual methods table.
 */
struct MIDIUSBDriverVMT {
  _phi_usb_midi_driver_methods
};

/**
 * @extends BaseAsynchronousChannel
 *
 * @brief   Full duplex midi driver class.
 * @details This class extends @p BaseAsynchronousChannel by adding physical
 *          I/O queues.
 */
struct MIDIUSBDriver {
  /** @brief Virtual Methods Table.*/
  const struct MIDIUSBDriverVMT *vmt;
  _phi_usb_midi_driver_data
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void mduInit(void);
  void mduObjectInit(MIDIUSBDriver *mdup);
  void mduStart(MIDIUSBDriver *mdup, const MIDIUSBConfig *config);
  void mduStop(MIDIUSBDriver *mdup);
  void mduDisconnectI(MIDIUSBDriver *mdup);
  void mduConfigureHookI(MIDIUSBDriver *mdup);
  bool mduRequestsHook(USBDriver *usbp);
  void mduSOFHookI(MIDIUSBDriver *mdup);
  void mduDataTransmitted(USBDriver *usbp, usbep_t ep);
  void mduDataReceived(USBDriver *usbp, usbep_t ep);

  void phi_usb_midi_send1(MIDIUSBDriver *mdup, uint8_t port, uint8_t b0);
  void phi_usb_midi_send2(MIDIUSBDriver *mdup, uint8_t port, uint8_t b0, uint8_t b1);
  void phi_usb_midi_send3(MIDIUSBDriver *mdup, uint8_t port, uint8_t b0, uint8_t b1, uint8_t b2);
  void phi_usb_midi_send_sysex(MIDIUSBDriver *mdup, uint8_t port, const uint8_t * data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* _PHI_LIB_USB_MIDI_H_ */

/** @} */
