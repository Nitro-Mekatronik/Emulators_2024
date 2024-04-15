//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "LIN_Slave.h"

//--------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------
LIN_SLAVE_t LIN_SLAVE;
LIN_FRAME_t LIN_FRAME;

extern UART_HandleTypeDef huart3;

//--------------------------------------------------------------
// Internal Functions
//--------------------------------------------------------------

void p_LIN_activateTransceiver(void);
uint8_t p_LIN_makeChecksum(LIN_FRAME_t *frame);
void p_LIN_wait_us(uint32_t n);

//--------------------------------------------------------------
// Initialization of the LIN interface (in Slave Mode)
//--------------------------------------------------------------
void UB_LIN_Slave_Init(void)
{
  // Initialize all variables
  LIN_SLAVE.mode = WAIT_4_BREAK;
  LIN_SLAVE.data_ptr = 0;
  LIN_SLAVE.crc = 0;

  LIN_FRAME.frame_id = 0;
  LIN_FRAME.data_len = 0;
  LIN_FRAME.data[0] = 0;

  // Briefly wait until transceiver is ready
  p_LIN_wait_us(LIN_POWERON_DELAY);

  // Turn on LIN transceiver
  p_LIN_activateTransceiver();
}


//--------------------------------------------------------------
// Checks if an ID was received
// (this function must be polled)
//
// If "LIN_OK" the ID must be evaluated and
// immediately after that either, if necessary,
// "UB_LIN_SendData()" or "UB_LIN_ReceiveData()"
// must be called (otherwise the frame will be lost)
//
// Return value:
// LIN_OK = ID was received -> "frame.frame_id"
// LIN_NO_ID = no ID was received
//--------------------------------------------------------------
LIN_ERR_t UB_LIN_checkID(LIN_FRAME_t *frame)
{
  LIN_ERR_t ret_value = LIN_NO_ID;

  if (LIN_SLAVE.mode == ID_OK) {
    // ID-Field was received
    LIN_SLAVE.mode = WAIT_4_CMD;
    frame->frame_id = LIN_FRAME.frame_id;
    ret_value = LIN_OK;
  }
  else {
    // No ID was received
    ret_value = LIN_NO_ID;
  }

  return (ret_value);
}


//--------------------------------------------------------------
// Sends data via LIN interface
// (after receiving an ID)
// Frame:
// data_len = number of data to be sent
// data[] = data to be sent
//
// Return value:
// LIN_OK = Frame was sent
// LIN_NO_ID = no ID was received
// LIN_WRONG_LEN = wrong number of data
//--------------------------------------------------------------
LIN_ERR_t UB_LIN_SendData(LIN_FRAME_t *frame)
{
  uint8_t checksum, n;

  // Check the mode
  if (LIN_SLAVE.mode != WAIT_4_CMD) return (LIN_NO_ID);

  // Check the length
  if ((frame->data_len < 1) || (frame->data_len > LIN_MAX_DATA)) {
    LIN_SLAVE.mode = WAIT_4_BREAK;
    return (LIN_WRONG_LEN);
  }

  // Calculate checksum
  checksum = p_LIN_makeChecksum(frame);

  // Wait until the last byte has been sent
  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TXE) == RESET);

  // Small break
  p_LIN_wait_us(LIN_FRAME_RESPONSE_DELAY);

  // Send data
  LIN_SLAVE.mode = SEND_DATA;

  //------------------------
  // Data-Field [1...n]
  //------------------------
  for (n = 0; n < frame->data_len; n++) {
    huart3.Instance->DR = frame->data[n]; 
    // Wait for DataField to be sent
    while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET);

    // Small break
    p_LIN_wait_us(LIN_DATA_BYTE_DELAY);
  }

  //------------------------
  // CRC-Field
  //------------------------
  huart3.Instance->DR = checksum; // USART_SendData(&huart3, checksum);
  // Wait for CRCField to be sent
  while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TC) == RESET);

  // End of frame
  LIN_SLAVE.mode = WAIT_4_BREAK;

  return (LIN_OK);
}


//--------------------------------------------------------------
// Receives data via LIN interface
// (after receiving an ID)
// Frame:
// data_len = number of data to be received
// Return:
// data[] = data received (if LIN_OK)
//
// Return value:
// LIN_OK = Frame was received
// LIN_NO_ID = no ID was received
// LIN_WRONG_LEN = wrong number of data
// LIN_RX_EMPTY = no frame received
// LIN_WRONG_CRC = Wrong checksum
//--------------------------------------------------------------
LIN_ERR_t UB_LIN_ReceiveData(LIN_FRAME_t *frame)
{
  uint32_t rx_timeout;
  uint8_t checksum, n;

  // Check the mode
  if (LIN_SLAVE.mode != WAIT_4_CMD) return (LIN_NO_ID);

  // Check the length
  if ((frame->data_len < 1) || (frame->data_len > LIN_MAX_DATA)) {
    LIN_SLAVE.mode = WAIT_4_BREAK;
    return (LIN_WRONG_LEN);
  }

  // Prepare Slave
  LIN_SLAVE.mode = RECEIVE_DATA;
  LIN_SLAVE.data_ptr = 0;
  LIN_SLAVE.crc = 0;

  LIN_FRAME.data_len = frame->data_len;
  LIN_FRAME.data[0] = 0;

  // Wait until Frame is received
  // or Timeout
  rx_timeout = 0;
  n = 0;
  do {
    // Timeout counter
    rx_timeout++;
    HAL_Delay(1);
    if (rx_timeout > LIN_RX_TIMEOUT_CNT) {
      // Exit loop
      break;
    }
    // Reset timeout on data receipt
    if (LIN_SLAVE.data_ptr != n) {
      n = LIN_SLAVE.data_ptr;
      rx_timeout = 0;
    }
  } while (LIN_SLAVE.mode != WAIT_4_BREAK);

  // Check if frame was received
  if (LIN_SLAVE.mode != WAIT_4_BREAK) {
    // No Frame received
    LIN_SLAVE.mode = WAIT_4_BREAK;
    return (LIN_RX_EMPTY);
  }

  // Copy received data
  for (n = 0; n < frame->data_len; n++) {
    frame->data[n] = LIN_FRAME.data[n];
  }
  // Calculate checksum
  checksum = p_LIN_makeChecksum(frame);

  // Check if CRC is okay
  if (LIN_SLAVE.crc != checksum) {
    // Incorrect checksum
    return (LIN_WRONG_CRC);
  }

  // Data is okay
  return (LIN_OK);
}


//--------------------------------------------------------------
// Internal Function
// Switch LIN Transceiver (MAX13020)
// from "Sleep Mode" to "Normal Slope Mode"
//--------------------------------------------------------------
void p_LIN_activateTransceiver(void)
{
  // NSLP-Pin to Low level
  //  LIN_NSLP_PORT->BSRRH = LIN_NSLP_PIN; // in "Sleep Mode"
  // Briefly wait (min. 10us)
  p_LIN_wait_us(LIN_ACTIVATE_DELAY);
  // NSLP-Pin to High level
  //  LIN_NSLP_PORT->BSRRL = LIN_NSLP_PIN; // in "Normal Slope Mode"
  // Briefly wait (min. 10us)
  p_LIN_wait_us(LIN_ACTIVATE_DELAY);
}


//--------------------------------------------------------------
// Internal Function
// Calculate checksum over all data
// (Classic Mode = inverted Modulo 256 Sum)
//
// Return value = checksum
//--------------------------------------------------------------
uint8_t p_LIN_makeChecksum(LIN_FRAME_t *frame)
{
  uint8_t ret_value = 0, n;
  uint16_t dummy;

  // Calculate checksum
  dummy = 0;
  for (n = 0; n < frame->data_len; n++) {
    dummy += frame->data[n];
    if (dummy > 0xFF) {
      dummy -= 0xFF;
    }
  }
  ret_value = (uint8_t)(dummy);
  ret_value ^= 0xFF;

  return (ret_value);
}

//--------------------------------------------------------------
// Internal Function
// Pause on K-line (without Timer)
// 1   = approx. 1us
// 10  = approx. 10us
// 100 = approx. 100us
//--------------------------------------------------------------
void p_LIN_wait_us(uint32_t n)
{
  volatile uint32_t p, t;

  // K-line Pause
  for (p = 0; p < n; p++) {
    for (t = 0; t < 15; t++); // 1us
  }
}


//--------------------------------------------------------------
// Internal Function
// UART ISR
// Called when a LIN field or a break field
// was received
// (all slave TX fields are also received as RX)
//--------------------------------------------------------------
void LIN_UART_ISR(void) {
  uint16_t rxData;
  
  if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_LBD) == SET) { // __HAL_UART_GET_IT
    //---------------------------
    // BreakField
    //---------------------------
    // Clear flag
    __HAL_UART_CLEAR_FLAG(&huart3, UART_FLAG_LBD);

    LIN_SLAVE.mode = RECEIVE_SYNC;
  }

  if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE) == SET) {
    // Read data
    rxData = huart3.Instance->DR; // USART_ReceiveData(&huart3);

    // Check current mode
    if (LIN_SLAVE.mode == RECEIVE_SYNC) {
      //---------------------------
      // SyncField
      //---------------------------
      if (rxData == LIN_SYNC_DATA) {
        LIN_SLAVE.mode = RECEIVE_ID;
      }
      else {
        LIN_SLAVE.mode = WAIT_4_BREAK;
      }
    }
    else if (LIN_SLAVE.mode == RECEIVE_ID) {
      //---------------------------
           // IDField
      //---------------------------
      LIN_SLAVE.mode = ID_OK;
      LIN_FRAME.frame_id = (uint8_t)(rxData);
    }
    else if (LIN_SLAVE.mode == RECEIVE_DATA) {
      //---------------------------
      // DataField
      //---------------------------
      // Save data
      LIN_FRAME.data[LIN_SLAVE.data_ptr] = (uint8_t)(rxData);
      // Increase pointer
      LIN_SLAVE.data_ptr++;
      // Check if all data has been received
      if (LIN_SLAVE.data_ptr >= LIN_FRAME.data_len) {
        LIN_SLAVE.mode = RECEIVE_CRC;
      }
    }
    else if (LIN_SLAVE.mode == RECEIVE_CRC) {
      //---------------------------
      // CRCField
      //---------------------------
      LIN_SLAVE.crc = (uint8_t)(rxData);
      LIN_SLAVE.mode = WAIT_4_BREAK;
    }
  }
}
