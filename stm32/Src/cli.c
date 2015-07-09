#include "cli.h"
#include "usbd_cdc_if.h"
#include "errors.h"

#define RX_BUFF_SZ 1024
#define TX_BUFF_SZ 1024

extern USBD_HandleTypeDef *hUsbDevice_0;

uint8_t  rx_buff[RX_BUFF_SZ];
unsigned rx_sz;
int      rx_cmd_sz;
err_t    rx_err;

uint8_t  tx_buff[TX_BUFF_SZ];
unsigned tx_sz;

static inline int cli_tx_busy(void)
{
	if (hUsbDevice_0) {
		USBD_CDC_HandleTypeDef* hcdc = (USBD_CDC_HandleTypeDef*)hUsbDevice_0->pClassData;
		return !hcdc || hcdc->TxState != 0;
	} else
		return 1;
}

static err_t cli_reply(void)
{
	if (USBD_OK == CDC_Transmit_FS(tx_buff, tx_sz)) {
		return err_ok;
	} else {
		tx_sz = 0;
		return err_internal;
	}
}

static err_t cli_handle_input(unsigned sz)
{
	memcpy(tx_buff, rx_buff, tx_sz = sz);
	return cli_reply();
}

static void cli_respond_err(err_t res)
{
	tx_sz = snprintf((char*)tx_buff, TX_BUFF_SZ, "e%d\r", res);
	cli_reply();
}

int8_t cli_receive(uint8_t* Buf, uint32_t *Len)
{
	unsigned len = *Len;
	if (!rx_err) {
		if (rx_cmd_sz || rx_sz + len > RX_BUFF_SZ) {
			rx_err = err_proto;
		} else {
			memcpy(rx_buff + rx_sz, Buf, len);
			rx_sz += len;
		}
	}
	if (Buf[len-1] == '\r') {
		rx_cmd_sz = !rx_err ? rx_sz : -1;
		rx_sz = 0;
	}
	return USBD_OK;
}

void cli_run(void)
{
	err_t err;
	unsigned sz;
	if (cli_tx_busy()) {
		return;
	}
	if (tx_sz)
	{
		if (!(tx_sz % USB_FS_MAX_PACKET_SIZE)) {
			CDC_Transmit_FS(tx_buff, 0); // ZLP
		}
		tx_sz = 0;
		return;
	}
	if ((sz = rx_cmd_sz)) 
	{
		rx_cmd_sz = 0;
		if (!rx_err) {
			err = cli_handle_input(sz);
		} else {
			err = rx_err;
			rx_err = err_ok;
		}
		if (err) {
			cli_respond_err(err);
		}
	}
}

