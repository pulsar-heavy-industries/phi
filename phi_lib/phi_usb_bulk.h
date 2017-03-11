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
 * @file    bulk_usb.h
 * @brief   Bulk over USB Driver macros and structures.
 *
 * @addtogroup BULK_USB
 * @{
 */

#ifndef _PHI_LIB_USB_BULK_H_
#define _PHI_LIB_USB_BULK_H_


/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    BULK_USB configuration options
 * @{
 */
/**
 * @brief   Bulk over USB buffers size.
 * @details Configuration parameter, the buffer size must be a multiple of
 *          the USB data endpoint maximum packet size.
 * @note    The default is 256 bytes for both the transmission and receive
 *          buffers.
 */
#if !defined(BULK_USB_BUFFERS_SIZE) || defined(__DOXYGEN__)
#define BULK_USB_BUFFERS_SIZE     256
#endif

/**
 * @brief   Bulk over USB number of buffers.
 * @note    The default is 2 buffers.
 */
#if !defined(BULK_USB_BUFFERS_NUMBER) || defined(__DOXYGEN__)
#define BULK_USB_BUFFERS_NUMBER   2
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if HAL_USE_USB == FALSE
#error "Bulk over USB Driver requires HAL_USE_USB"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief Driver state machine possible states.
 */
typedef enum {
  BDU_UNINIT = 0,                   /**< Not initialized.                   */
  BDU_STOP = 1,                     /**< Stopped.                           */
  BDU_READY = 2                     /**< Ready.                             */
} bdustate_t;

/**
 * @brief   Structure representing a bulk over USB driver.
 */
typedef struct BulkUSBDriver BulkUSBDriver;

/**
 * @brief   Bulk over USB Driver configuration structure.
 * @details An instance of this structure must be passed to @p bduStart()
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
} BulkUSBConfig;

/**
 * @brief   @p BulkDriver specific data.
 */
#define _bulk_usb_driver_data                                             \
  _base_asynchronous_channel_data                                           \
  /* Driver state.*/                                                        \
  bdustate_t                state;                                          \
  /* Input buffers queue.*/                                                 \
  input_buffers_queue_t     ibqueue;                                        \
  /* Output queue.*/                                                        \
  output_buffers_queue_t    obqueue;                                        \
  /* Input buffer.*/                                                        \
  uint8_t                   ib[BQ_BUFFER_SIZE(BULK_USB_BUFFERS_NUMBER,    \
                                              BULK_USB_BUFFERS_SIZE)];    \
  /* Output buffer.*/                                                       \
  uint8_t                   ob[BQ_BUFFER_SIZE(BULK_USB_BUFFERS_NUMBER,    \
                                              BULK_USB_BUFFERS_SIZE)];    \
  /* End of the mandatory fields.*/                                         \
  /* Current configuration data.*/                                          \
  const BulkUSBConfig     *config;

/**
 * @brief   @p BulkUSBDriver specific methods.
 */
#define _bulk_usb_driver_methods                                          \
  _base_asynchronous_channel_methods

/**
 * @extends BaseAsynchronousChannelVMT
 *
 * @brief   @p BulkDriver virtual methods table.
 */
struct BulkUSBDriverVMT {
  _bulk_usb_driver_methods
};

/**
 * @extends BaseAsynchronousChannel
 *
 * @brief   Full duplex bulk driver class.
 * @details This class extends @p BaseAsynchronousChannel by adding physical
 *          I/O queues.
 */
struct BulkUSBDriver {
  /** @brief Virtual Methods Table.*/
  const struct BulkUSBDriverVMT *vmt;
  _bulk_usb_driver_data
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
  void bduInit(void);
  void bduObjectInit(BulkUSBDriver *bdup);
  void bduStart(BulkUSBDriver *bdup, const BulkUSBConfig *config);
  void bduStop(BulkUSBDriver *bdup);
  void bduDisconnectI(BulkUSBDriver *bdup);
  void bduConfigureHookI(BulkUSBDriver *bdup);
  bool bduRequestsHook(USBDriver *usbp);
  void bduSOFHookI(BulkUSBDriver *bdup);
  void bduDataTransmitted(USBDriver *usbp, usbep_t ep);
  void bduDataReceived(USBDriver *usbp, usbep_t ep);
  void bduInterruptTransmitted(USBDriver *usbp, usbep_t ep);
#ifdef __cplusplus
}
#endif

#endif /* _PHI_LIB_USB_BULK_H_ */

/** @} */
