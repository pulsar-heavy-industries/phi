#include "hal.h"
#include "phi_lib/phi_usb_midi.h"

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

static bool mdu_start_receive(MIDIUSBDriver *mdup) {
  uint8_t *buf;

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(mdup->config->usbp) != USB_ACTIVE) ||
      (mdup->state != MDU_READY)) {
    return true;
  }

  /* Checking if there is already a transaction ongoing on the endpoint.*/
  if (usbGetReceiveStatusI(mdup->config->usbp, mdup->config->bulk_in)) {
    return true;
  }

  /* Checking if there is a buffer ready for incoming data.*/
  buf = ibqGetEmptyBufferI(&mdup->ibqueue);
  if (buf == NULL) {
    return true;
  }

  /* Buffer found, starting a new transaction.*/
  usbStartReceiveI(mdup->config->usbp, mdup->config->bulk_out,
                   buf, MIDI_USB_BUFFERS_SIZE);

  return false;
}

/*
 * Interface implementation.
 */

static size_t write(void *ip, const uint8_t *bp, size_t n) {
  size_t ret;

  if (usbGetDriverStateI(((MIDIUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  ret = obqWriteTimeout(&((MIDIUSBDriver *)ip)->obqueue, bp,
                         n, TIME_INFINITE);
  if (!ret) {
	  return 0;
  }

  obqFlush(&((MIDIUSBDriver *)ip)->obqueue);

  return ret;
}

static size_t read(void *ip, uint8_t *bp, size_t n) {

  if (usbGetDriverStateI(((MIDIUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  return ibqReadTimeout(&((MIDIUSBDriver *)ip)->ibqueue, bp,
                        n, TIME_INFINITE);
}

static msg_t put(void *ip, uint8_t b) {

  if (usbGetDriverStateI(((MIDIUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return obqPutTimeout(&((MIDIUSBDriver *)ip)->obqueue, b, TIME_INFINITE);
}

static msg_t get(void *ip) {

  if (usbGetDriverStateI(((MIDIUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return ibqGetTimeout(&((MIDIUSBDriver *)ip)->ibqueue, TIME_INFINITE);
}

static msg_t putt(void *ip, uint8_t b, systime_t timeout) {

  if (usbGetDriverStateI(((MIDIUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return obqPutTimeout(&((MIDIUSBDriver *)ip)->obqueue, b, timeout);
}

static msg_t gett(void *ip, systime_t timeout) {

  if (usbGetDriverStateI(((MIDIUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return MSG_RESET;
  }

  return ibqGetTimeout(&((MIDIUSBDriver *)ip)->ibqueue, timeout);
}

static size_t writet(void *ip, const uint8_t *bp, size_t n, systime_t timeout) {

  if (usbGetDriverStateI(((MIDIUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  return obqWriteTimeout(&((MIDIUSBDriver *)ip)->obqueue, bp, n, timeout);
}

static size_t readt(void *ip, uint8_t *bp, size_t n, systime_t timeout) {

  if (usbGetDriverStateI(((MIDIUSBDriver *)ip)->config->usbp) != USB_ACTIVE) {
    return 0;
  }

  return ibqReadTimeout(&((MIDIUSBDriver *)ip)->ibqueue, bp, n, timeout);
}

static const struct MIDIUSBDriverVMT vmt = {
  write, read, put, get,
  putt, gett, writet, readt
};

/**
 * @brief   Notification of empty buffer released into the input buffers queue.
 *
 * @param[in] bqp       the buffers queue pointer.
 */
static void ibnotify(io_buffers_queue_t *bqp) {
  MIDIUSBDriver *mdup = bqGetLinkX(bqp);
  (void) mdu_start_receive(mdup);
}

/**
 * @brief   Notification of filled buffer inserted into the output buffers queue.
 *
 * @param[in] bqp       the buffers queue pointer.
 */
static void obnotify(io_buffers_queue_t *bqp) {
  size_t n;
  MIDIUSBDriver *mdup = bqGetLinkX(bqp);

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(mdup->config->usbp) != USB_ACTIVE) ||
      (mdup->state != MDU_READY)) {
    return;
  }

  /* Checking if there is already a transaction ongoing on the endpoint.*/
  if (!usbGetTransmitStatusI(mdup->config->usbp, mdup->config->bulk_in)) {
    /* Trying to get a full buffer.*/
    uint8_t *buf = obqGetFullBufferI(&mdup->obqueue, &n);
    if (buf != NULL) {
      /* Buffer found, starting a new transaction.*/
      usbStartTransmitI(mdup->config->usbp, mdup->config->bulk_in, buf, n);
    }
  }
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   MIDI Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void mduInit(void) {
}

/**
 * @brief   Initializes a generic full duplex driver object.
 * @details The HW dependent part of the initialization has to be performed
 *          outside, usually in the hardware initialization code.
 *
 * @param[out] mdup     pointer to a @p MIDIUSBDriver structure
 *
 * @init
 */
void mduObjectInit(MIDIUSBDriver *mdup) {

  mdup->vmt = &vmt;
  osalEventObjectInit(&mdup->event);
  mdup->state = MDU_STOP;
  ibqObjectInit(&mdup->ibqueue, mdup->ib,
                MIDI_USB_BUFFERS_SIZE, MIDI_USB_BUFFERS_NUMBER,
                ibnotify, mdup);
  obqObjectInit(&mdup->obqueue, mdup->ob,
                MIDI_USB_BUFFERS_SIZE, MIDI_USB_BUFFERS_NUMBER,
                obnotify, mdup);
}

/**
 * @brief   Configures and starts the driver.
 *
 * @param[in] mdup      pointer to a @p MIDIUSBDriver object
 * @param[in] config    the midi over USB driver configuration
 *
 * @api
 */
void mduStart(MIDIUSBDriver *mdup, const MIDIUSBConfig *config) {
  USBDriver *usbp = config->usbp;

  osalDbgCheck(mdup != NULL);

  osalSysLock();
  osalDbgAssert((mdup->state == MDU_STOP) || (mdup->state == MDU_READY),
                "invalid state");
  usbp->in_params[config->bulk_in - 1U]   = mdup;
  usbp->out_params[config->bulk_out - 1U] = mdup;
  mdup->config = config;
  mdup->state = MDU_READY;
  osalSysUnlock();
}

/**
 * @brief   Stops the driver.
 * @details Any thread waiting on the driver's queues will be awakened with
 *          the message @p Q_RESET.
 *
 * @param[in] mdup      pointer to a @p MIDIUSBDriver object
 *
 * @api
 */
void mduStop(MIDIUSBDriver *mdup) {
  USBDriver *usbp = mdup->config->usbp;

  osalDbgCheck(mdup != NULL);

  osalSysLock();

  osalDbgAssert((mdup->state == MDU_STOP) || (mdup->state == MDU_READY),
                "invalid state");

  /* Driver in stopped state.*/
  usbp->in_params[mdup->config->bulk_in - 1U]   = NULL;
  usbp->out_params[mdup->config->bulk_out - 1U] = NULL;
  mdup->state = MDU_STOP;

  /* Enforces a disconnection.*/
  mduDisconnectI(mdup);
  osalOsRescheduleS();

  osalSysUnlock();
}

/**
 * @brief   USB device disconnection handler.
 * @note    If this function is not called from an ISR then an explicit call
 *          to @p osalOsRescheduleS() in necessary afterward.
 *
 * @param[in] mdup      pointer to a @p MIDIUSBDriver object
 *
 * @iclass
 */
void mduDisconnectI(MIDIUSBDriver *mdup) {

  /* Queues reset in order to signal the driver stop to the application.*/
  chnAddFlagsI(mdup, CHN_DISCONNECTED);
  ibqResetI(&mdup->ibqueue);
  obqResetI(&mdup->obqueue);
}

/**
 * @brief   USB device configured handler.
 *
 * @param[in] mdup      pointer to a @p MIDIUSBDriver object
 *
 * @iclass
 */
void mduConfigureHookI(MIDIUSBDriver *mdup) {

  ibqResetI(&mdup->ibqueue);
  obqResetI(&mdup->obqueue);
  chnAddFlagsI(mdup, CHN_CONNECTED);
  (void) mdu_start_receive(mdup);
}

/**
 * @brief   Default requests hook.
 * @details Applications wanting to use the MIDI over USB driver can use
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
bool mduRequestsHook(USBDriver *usbp) {
	(void)usbp;
	return FALSE;
}

/**
 * @brief   SOF handler.
 * @details The SOF interrupt is used for automatic flushing of incomplete
 *          buffers pending in the output queue.
 *
 * @param[in] mdup      pointer to a @p MIDIUSBDriver object
 *
 * @iclass
 */
void mduSOFHookI(MIDIUSBDriver *mdup) {

  /* If the USB driver is not in the appropriate state then transactions
     must not be started.*/
  if ((usbGetDriverStateI(mdup->config->usbp) != USB_ACTIVE) ||
      (mdup->state != MDU_READY)) {
    return;
  }

  /* If there is already a transaction ongoing then another one cannot be
     started.*/
  if (usbGetTransmitStatusI(mdup->config->usbp, mdup->config->bulk_in)) {
    return;
  }

  /* Checking if there only a buffer partially filled, if so then it is
     enforced in the queue and transmitted.*/
  if (obqTryFlushI(&mdup->obqueue)) {
    size_t n;
    uint8_t *buf = obqGetFullBufferI(&mdup->obqueue, &n);

    osalDbgAssert(buf != NULL, "queue is empty");

    usbStartTransmitI(mdup->config->usbp, mdup->config->bulk_in, buf, n);
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
void mduDataTransmitted(USBDriver *usbp, usbep_t ep) {
  uint8_t *buf;
  size_t n;
  MIDIUSBDriver *mdup = usbp->in_params[ep - 1U];

  if (mdup == NULL) {
    return;
  }

  osalSysLockFromISR();

  /* Signaling that space is available in the output queue.*/
  chnAddFlagsI(mdup, CHN_OUTPUT_EMPTY);

  /* Freeing the buffer just transmitted, if it was not a zero size packet.*/
  if (usbp->epc[ep]->in_state->txsize > 0U) {
    obqReleaseEmptyBufferI(&mdup->obqueue);
  }

  /* Checking if there is a buffer ready for transmission.*/
  buf = obqGetFullBufferI(&mdup->obqueue, &n);

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
void mduDataReceived(USBDriver *usbp, usbep_t ep) {
  MIDIUSBDriver *mdup = usbp->out_params[ep - 1U];
  if (mdup == NULL) {
    return;
  }

  osalSysLockFromISR();

  /* Signaling that data is available in the input queue.*/
  chnAddFlagsI(mdup, CHN_INPUT_AVAILABLE);

  /* Posting the filled buffer in the queue.*/
  ibqPostFullBufferI(&mdup->ibqueue,
                     usbGetReceiveTransactionSizeX(mdup->config->usbp,
                                                   mdup->config->bulk_out));

  /* The endpoint cannot be busy, we are in the context of the callback,
     so a packet is in the buffer for sure. Trying to get a free buffer
     for the next transaction.*/
  mdu_start_receive(mdup);

  osalSysUnlockFromISR();
}

// the Send etc, work for everything except Sysex
uint8_t calcDS1(uint8_t b0) {
// this works for everything bar SysEx,
// for sysex you need to use 0x4-0x7 to pack messages
  return (b0 & 0xF0) >> 4;

}

uint8_t calcCIN1(uint8_t port, uint8_t b0) {
  uint8_t ds = calcDS1(b0);
  uint8_t cin = (((port - 1) & 0x0F) << 4) | ds;
  return cin;
}

void phi_usb_midi_send1(MIDIUSBDriver *mdup, uint8_t port, uint8_t b0) {
  uint8_t tx[4];
  tx[0] = calcCIN1(port, b0);
  tx[1] = b0;
  tx[2] = 0;
  tx[3] = 0;
  write(mdup, &tx[0], 4);
}

void phi_usb_midi_send2(MIDIUSBDriver *mdup, uint8_t port, uint8_t b0, uint8_t b1) {
  uint8_t tx[4];
  tx[0] = calcCIN1(port, b0);
  tx[1] = b0;
  tx[2] = b1;
  tx[3] = 0;
  write(&mdup, &tx[0], 4);
}

void phi_usb_midi_send3(MIDIUSBDriver *mdup, uint8_t port, uint8_t b0, uint8_t b1, uint8_t b2) {
  uint8_t tx[4];
  tx[0] = calcCIN1(port, b0);
  tx[1] = b0;
  tx[2] = b1;
  tx[3] = b2;
  write(mdup, &tx[0], 4);
}

/*
void phi_usb_midi_send4(MIDIUSBDriver *mdup, uint8_t port, uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2) {
  uint8_t tx[4];
  tx[0] =  (((port - 1) & 0x0F) << 4) | cin;
  tx[1] = b0;
  tx[2] = b1;
  tx[3] = b2;
  write(mdup, &tx[0], 4);
}
*/

void phi_usb_midi_send_sysex(MIDIUSBDriver *mdup, uint8_t port, const uint8_t * data, size_t len)
{
    uint8_t tx[4];
    port = (((port - 1) & 0x0F) << 4);

    // SysEx Start
    tx[0] = port | 0x04;
    tx[1] = 0xf0;
    tx[2] = 0x41;
    tx[3] = 0x12;
    write(mdup, &tx[0], 4);

    while (len > 2)
    {
        tx[0] = port | 0x04;
        tx[1] = data[0];
        tx[2] = data[1];
        tx[3] = data[2];
        write(mdup, &tx[0], 4);

        data += 3;
        len -= 3;
    }

    // left with 2 bytes or less
    switch (len)
    {
    case 0:
        tx[0] = port | 0x05;
        tx[1] = 0xf7;
        write(mdup, &tx[0], 2);
        break;

    case 1:
        tx[0] = port | 0x06;
        tx[1] = data[0];
        tx[2] = 0xf7;
        write(mdup, &tx[0], 3);
        break;

    case 2:
        tx[0] = port | 0x07;
        tx[1] = data[0];
        tx[2] = data[1];
        tx[3] = 0xf7;
        write(mdup, &tx[0], 4);
        break;

    default:
        chDbgCheck(FALSE);
        break;
    }
}
