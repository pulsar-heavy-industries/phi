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
 * @file    bulk_usb.c
 * @brief   Bulk over USB Driver code.
 *
 * @addtogroup BULK_USB
 * @{
 */

#include "hal.h"
#include "phi_lib/phi_usb_bulk.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static bool bdu_start_receive(BulkUSBDriver *bdup) {
  uint8_t *buf;

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(bdup->config->usbp) != USB_ACTIVE) ||
      (bdup->state != BDU_READY)) {
    return true;
  }

  /* Checking if there is already a transaction ongoing on the endpoint.*/
  if (usbGetReceiveStatusI(bdup->config->usbp, bdup->config->bulk_in)) {
    return true;
  }

  /* Checking if there is a buffer ready for incoming data.*/
  buf = ibqGetEmptyBufferI(&bdup->ibqueue);
  if (buf == NULL) {
    return true;
  }

  /* Buffer found, starting a new transaction.*/
  usbStartReceiveI(bdup->config->usbp, bdup->config->bulk_out,
                   buf, BULK_USB_BUFFERS_SIZE);

  return false;
}

/*
 * Interface implementation.
 */

static size_t write(void *ip, const uint8_t *bp, size_t n) {
	size_t ret;

	if (usbGetDriverStateI(((BulkUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
		return 0;
	}

	  ret = obqWriteTimeout(&((BulkUSBDriver *)ip)->obqueue, bp,
	                         n, TIME_IMMEDIATE);
	  if (!ret) {
		  return 0;
	  }

	 // obqFlush(&((BulkUSBDriver *)ip)->obqueue);

	  return ret;
}

static size_t read(void *ip, uint8_t *bp, size_t n) {

  if (usbGetDriverStateI(((BulkUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  return ibqReadTimeout(&((BulkUSBDriver *)ip)->ibqueue, bp,
                        n, TIME_INFINITE);
}

static msg_t put(void *ip, uint8_t b) {
	msg_t ret;

  if (usbGetDriverStateI(((BulkUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  ret = obqPutTimeout(&((BulkUSBDriver *)ip)->obqueue, b, TIME_IMMEDIATE);
  if (ret == MSG_OK) {
	//  obqFlush(&((BulkUSBDriver *)ip)->obqueue);
  }
  return ret;
}

static msg_t get(void *ip) {

  if (usbGetDriverStateI(((BulkUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return ibqGetTimeout(&((BulkUSBDriver *)ip)->ibqueue, TIME_INFINITE);
}

static msg_t putt(void *ip, uint8_t b, systime_t timeout) {

  if (usbGetDriverStateI(((BulkUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return obqPutTimeout(&((BulkUSBDriver *)ip)->obqueue, b, timeout);
}

static msg_t gett(void *ip, systime_t timeout) {

  if (usbGetDriverStateI(((BulkUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return ibqGetTimeout(&((BulkUSBDriver *)ip)->ibqueue, timeout);
}

static size_t writet(void *ip, const uint8_t *bp, size_t n, systime_t timeout) {

  if (usbGetDriverStateI(((BulkUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  return obqWriteTimeout(&((BulkUSBDriver *)ip)->obqueue, bp, n, timeout);
}

static size_t readt(void *ip, uint8_t *bp, size_t n, systime_t timeout) {

  if (usbGetDriverStateI(((BulkUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  return ibqReadTimeout(&((BulkUSBDriver *)ip)->ibqueue, bp, n, timeout);
}

static const struct BulkUSBDriverVMT vmt = {
  write, read, put, get,
  putt, gett, writet, readt
};

/**
 * @brief   Notification of empty buffer released into the input buffers queue.
 *
 * @param[in] bqp       the buffers queue pointer.
 */
static void ibnotify(io_buffers_queue_t *bqp) {
  BulkUSBDriver *bdup = bqGetLinkX(bqp);
  (void) bdu_start_receive(bdup);
}

/**
 * @brief   Notification of filled buffer inserted into the output buffers queue.
 *
 * @param[in] bqp       the buffers queue pointer.
 */
static void obnotify(io_buffers_queue_t *bqp) {
  size_t n;
  BulkUSBDriver *bdup = bqGetLinkX(bqp);

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(bdup->config->usbp) != USB_ACTIVE) ||
      (bdup->state != BDU_READY)) {
    return;
  }

  /* Checking if there is already a transaction ongoing on the endpoint.*/
  if (!usbGetTransmitStatusI(bdup->config->usbp, bdup->config->bulk_in)) {
    /* Trying to get a full buffer.*/
    uint8_t *buf = obqGetFullBufferI(&bdup->obqueue, &n);
    if (buf != NULL) {
      /* Buffer found, starting a new transaction.*/
      usbStartTransmitI(bdup->config->usbp, bdup->config->bulk_in, buf, n);
    }
  }
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Bulk Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void bduInit(void) {
}

/**
 * @brief   Initializes a generic full duplex driver object.
 * @details The HW dependent part of the initialization has to be performed
 *          outside, usually in the hardware initialization code.
 *
 * @param[out] bdup     pointer to a @p BulkUSBDriver structure
 *
 * @init
 */
void bduObjectInit(BulkUSBDriver *bdup) {

  bdup->vmt = &vmt;
  osalEventObjectInit(&bdup->event);
  bdup->state = BDU_STOP;
  ibqObjectInit(&bdup->ibqueue, bdup->ib,
                BULK_USB_BUFFERS_SIZE, BULK_USB_BUFFERS_NUMBER,
                ibnotify, bdup);
  obqObjectInit(&bdup->obqueue, bdup->ob,
                BULK_USB_BUFFERS_SIZE, BULK_USB_BUFFERS_NUMBER,
                obnotify, bdup);
}

/**
 * @brief   Configures and starts the driver.
 *
 * @param[in] bdup      pointer to a @p BulkUSBDriver object
 * @param[in] config    the bulk over USB driver configuration
 *
 * @api
 */
void bduStart(BulkUSBDriver *bdup, const BulkUSBConfig *config) {
  USBDriver *usbp = config->usbp;

  osalDbgCheck(bdup != NULL);

  osalSysLock();
  osalDbgAssert((bdup->state == BDU_STOP) || (bdup->state == BDU_READY),
                "invalid state");
  usbp->in_params[config->bulk_in - 1U]   = bdup;
  usbp->out_params[config->bulk_out - 1U] = bdup;
  bdup->config = config;
  bdup->state = BDU_READY;
  osalSysUnlock();
}

/**
 * @brief   Stops the driver.
 * @details Any thread waiting on the driver's queues will be awakened with
 *          the message @p Q_RESET.
 *
 * @param[in] bdup      pointer to a @p BulkUSBDriver object
 *
 * @api
 */
void bduStop(BulkUSBDriver *bdup) {
  USBDriver *usbp = bdup->config->usbp;

  osalDbgCheck(bdup != NULL);

  osalSysLock();

  osalDbgAssert((bdup->state == BDU_STOP) || (bdup->state == BDU_READY),
                "invalid state");

  /* Driver in stopped state.*/
  usbp->in_params[bdup->config->bulk_in - 1U]   = NULL;
  usbp->out_params[bdup->config->bulk_out - 1U] = NULL;
  bdup->state = BDU_STOP;

  /* Enforces a disconnection.*/
  bduDisconnectI(bdup);
  osalOsRescheduleS();

  osalSysUnlock();
}

/**
 * @brief   USB device disconnection handler.
 * @note    If this function is not called from an ISR then an explicit call
 *          to @p osalOsRescheduleS() in necessary afterward.
 *
 * @param[in] bdup      pointer to a @p BulkUSBDriver object
 *
 * @iclass
 */
void bduDisconnectI(BulkUSBDriver *bdup) {

  /* Queues reset in order to signal the driver stop to the application.*/
  chnAddFlagsI(bdup, CHN_DISCONNECTED);
  ibqResetI(&bdup->ibqueue);
  obqResetI(&bdup->obqueue);
}

/**
 * @brief   USB device configured handler.
 *
 * @param[in] bdup      pointer to a @p BulkUSBDriver object
 *
 * @iclass
 */
void bduConfigureHookI(BulkUSBDriver *bdup) {

  ibqResetI(&bdup->ibqueue);
  obqResetI(&bdup->obqueue);
  chnAddFlagsI(bdup, CHN_CONNECTED);
  (void) bdu_start_receive(bdup);
}

/**
 * @brief   Default requests hook.
 * @details Applications wanting to use the Bulk over USB driver can use
 *          this function as requests hook in the USB configuration.
 *          The following requests are emulated:
 *          - CDC_GET_LINE_CODING.
 *          - CDC_SET_LINE_CODING.
 *          - CDC_SET_CONTROL_LINE_STATE.
 *          .
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @return              The hook status.
 * @retval true         Message handled internally.
 * @retval false        Message not handled.
 */
bool bduRequestsHook(USBDriver *usbp) {
	(void) usbp;
  return false;
}

/**
 * @brief   SOF handler.
 * @details The SOF interrupt is used for automatic flushing of incomplete
 *          buffers pending in the output queue.
 *
 * @param[in] bdup      pointer to a @p BulkUSBDriver object
 *
 * @iclass
 */
void bduSOFHookI(BulkUSBDriver *bdup) {

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(bdup->config->usbp) != USB_ACTIVE) ||
      (bdup->state != BDU_READY)) {
    return;
  }

  /* If there is already a transaction ongoing then another one cannot be
     started.*/
  if (usbGetTransmitStatusI(bdup->config->usbp, bdup->config->bulk_in)) {
    return;
  }

  /* Checking if there only a buffer partially filled, if so then it is
     enforced in the queue and transmitted.*/
  if (obqTryFlushI(&bdup->obqueue)) {
    size_t n;
    uint8_t *buf = obqGetFullBufferI(&bdup->obqueue, &n);

    osalDbgAssert(buf != NULL, "queue is empty");

    usbStartTransmitI(bdup->config->usbp, bdup->config->bulk_in, buf, n);
  }
}

/**
 * @brief   Default data transmitted callback.
 * @details The application must use this function as callback for the IN
 *          data endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        IN endpoint number
 */
void bduDataTransmitted(USBDriver *usbp, usbep_t ep) {
  uint8_t *buf;
  size_t n;
  BulkUSBDriver *bdup = usbp->in_params[ep - 1U];

  if (bdup == NULL) {
    return;
  }

  osalSysLockFromISR();

  /* Signaling that space is available in the output queue.*/
  chnAddFlagsI(bdup, CHN_OUTPUT_EMPTY);

  /* Freeing the buffer just transmitted, if it was not a zero size packet.*/
  if (usbp->epc[ep]->in_state->txsize > 0U) {
    obqReleaseEmptyBufferI(&bdup->obqueue);
  }

  /* Checking if there is a buffer ready for transmission.*/
  buf = obqGetFullBufferI(&bdup->obqueue, &n);

  if (buf != NULL) {
    /* The endpoint cannot be busy, we are in the context of the callback,
       so it is safe to transmit without a check.*/
    usbStartTransmitI(usbp, ep, buf, n);
  }
  else if ((usbp->epc[ep]->in_state->txsize > 0U) &&
           ((usbp->epc[ep]->in_state->txsize &
            ((size_t)usbp->epc[ep]->in_maxsize - 1U)) == 0U)) {
    /* Transmit zero sized packet in case the last one has maximum allowed
       size. Otherwise the recipient may expect more data coming soon and
       not return buffered data to app. See section 5.8.3 Bulk Transfer
       Packet Size Constraints of the USB Specification document.*/
    usbStartTransmitI(usbp, ep, usbp->setup, 0);

  }
  else {
    /* Nothing to transmit.*/
  }

  osalSysUnlockFromISR();
}

/**
 * @brief   Default data received callback.
 * @details The application must use this function as callback for the OUT
 *          data endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        OUT endpoint number
 */
void bduDataReceived(USBDriver *usbp, usbep_t ep) {
  BulkUSBDriver *bdup = usbp->out_params[ep - 1U];
  if (bdup == NULL) {
    return;
  }

  osalSysLockFromISR();

  /* Signaling that data is available in the input queue.*/
  chnAddFlagsI(bdup, CHN_INPUT_AVAILABLE);

  /* Posting the filled buffer in the queue.*/
  ibqPostFullBufferI(&bdup->ibqueue,
                     usbGetReceiveTransactionSizeX(bdup->config->usbp,
                                                   bdup->config->bulk_out));

  /* The endpoint cannot be busy, we are in the context of the callback,
     so a packet is in the buffer for sure. Trying to get a free buffer
     for the next transaction.*/
  bdu_start_receive(bdup);

  osalSysUnlockFromISR();
}

/**
 * @brief   Default data received callback.
 * @details The application must use this function as callback for the IN
 *          interrupt endpoint.
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @param[in] ep        endpoint number
 */
void bduInterruptTransmitted(USBDriver *usbp, usbep_t ep) {

  (void)usbp;
  (void)ep;
}

/** @} */
