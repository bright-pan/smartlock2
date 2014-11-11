/*********************************************************************
 * Filename:      gsm.c
 *
 *
 * Author:        Bright Pan <loststriker@gmail.com>
 * Created at:    2013-05-22 09:17:52
 *
 *
 * Change Log:
 *
 * Copyright (C) 2013 Yuettak Co.,Ltd
 ********************************************************************/

#include "gsm.h"
#include "local.h"

#define DEVICE_NAME_GSM_USART "uart3"
#define TEST_GSM_MODE_PUT_HEX
#define TEST_GSM_MODE_PUT_CHAR

rt_mq_t gsm_mq = RT_NULL;
rt_mutex_t mutex_gsm_mail_sequence;

TCP_DOMAIN_TYPEDEF tcp_domain;

const char *at_command_map[80];

void at_command_map_init(void)
{
	at_command_map[AT] = "AT\r";
	at_command_map[AT_CNMI] = "AT+CNMI=2,1\r";
	at_command_map[AT_CSCA] = "AT+CSCA?\r";
	at_command_map[AT_CMGF_0] = "AT+CMGF=0\r";
	at_command_map[AT_CMGF_1] = "AT+CMGF=1\r";
	at_command_map[AT_CMGD] = "AT+CMGD=50,4\r";
	at_command_map[AT_CPIN] = "AT+CPIN?\r";
	at_command_map[AT_CSQ] = "AT+CSQ\r";
	at_command_map[AT_CGREG] = "AT+CGREG?\r";
	at_command_map[AT_CGATT] = "AT+CGATT?\r";
	at_command_map[AT_CIPMODE] = "AT+CIPMODE=1\r";
	at_command_map[AT_CSTT] = "AT+CSTT\r";
	at_command_map[AT_CIICR] = "AT+CIICR\r";
	at_command_map[AT_CIFSR] = "AT+CIFSR\r";
	at_command_map[AT_CIPSHUT] = "AT+CIPSHUT\r";
	at_command_map[AT_CIPSTATUS] = "AT+CIPSTATUS\r";
	at_command_map[AT_CIPSTART] = "AT+CIPSTART=\"TCP\",\"%s\",%d\r";
	at_command_map[AT_CMGS] = "AT+CMGS=%d\x0D";
	at_command_map[AT_CMGS_SUFFIX] = "";
	at_command_map[ATO] = "ATO\r";
	at_command_map[PLUS3] = "+++";
	at_command_map[AT_CMMSINIT] = "AT+CMMSINIT\r";
	at_command_map[AT_CMMSTERM] = "AT+CMMSTERM\r";
	at_command_map[AT_CMMSCURL] = "AT+CMMSCURL=\"mmsc.monternet.com\"\r";
	at_command_map[AT_CMMSCID] = "AT+CMMSCID=1\r";
	at_command_map[AT_CMMSPROTO] = "AT+CMMSPROTO=\"10.0.0.172\",80\r";
	at_command_map[AT_CMMSSENDCFG] = "AT+CMMSSENDCFG=6,3,0,0,2,4\r";
	at_command_map[AT_SAPBR_CONTYPE] = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r";
	at_command_map[AT_SAPBR_APN_CMWAP] = "AT+SAPBR=3,1,\"APN\",\"CMWAP\"\r";
	at_command_map[AT_SAPBR_APN_CMNET] = "AT+SAPBR=3,1,\"APN\",\"CMNET\"\r";
	at_command_map[AT_SAPBR_OPEN] = "AT+SAPBR=1,1\r";
	at_command_map[AT_SAPBR_CLOSE] = "AT+SAPBR=0,1\r";
	at_command_map[AT_SAPBR_REQUEST] = "AT+SAPBR=2,1\r";
	at_command_map[AT_CMMSEDIT_OPEN] = "AT+CMMSEDIT=1\r";
	at_command_map[AT_CMMSEDIT_CLOSE] = "AT+CMMSEDIT=0\r";
	at_command_map[AT_CMMSDOWN_PIC] = "AT+CMMSDOWN=\"PIC\",%d,50000\r";
	at_command_map[AT_CMMSDOWN_TITLE] = "AT+CMMSDOWN=\"TITLE\",%d,50000\r";
	at_command_map[AT_CMMSDOWN_TEXT] = "AT+CMMSDOWN=\"TEXT\",%d,50000\r";
	at_command_map[AT_CMMSRECP] = "AT+CMMSRECP=\"%s\"\r";
	at_command_map[AT_CMMSSEND] = "AT+CMMSSEND\r";
	at_command_map[AT_CLCC] = "AT+CLCC\r";
	at_command_map[ATA] = "ATA\r";
	at_command_map[ATH5] = "ATH5\r";
	at_command_map[AT_RING] = "";
	at_command_map[AT_RECV_SMS] = "AT+CMGR=%d\r";
	at_command_map[ATI] = "ATI\r";
	at_command_map[AT_GSV] = "AT+GSV\r";
	at_command_map[AT_V] = "AT&V\r";
	at_command_map[AT_D1] = "AT&D1\r";
	at_command_map[AT_W] = "AT&W\r";
	at_command_map[AT_HTTPINIT] = "AT+HTTPINIT\r";
	at_command_map[AT_HTTPTERM] = "AT+HTTPTERM\r";
	at_command_map[AT_HTTPPARA_CID] = "AT+HTTPPARA=\"CID\",1\r";
	at_command_map[AT_HTTPPARA_URL] = "AT+HTTPPARA=\"URL\",\"%s\"\r";
	at_command_map[AT_HTTPACTION_POST] = "AT+HTTPACTION=1\r";
	at_command_map[AT_HTTPACTION_GET] = "AT+HTTPACTION=0\r";
	at_command_map[AT_HTTPACTION_HEAD] = "AT+HTTPACTION=2\r";
	at_command_map[AT_HTTPREAD] = "AT+HTTPREAD=%d,%d\r";
	at_command_map[AT_HTTPPARA_BREAK] = "AT+HTTPPARA=\"BREAK\",%d\r";
	at_command_map[AT_HTTPPARA_BREAKEND] = "AT+HTTPPARA=\"BREAKEND\",%d\r";
#ifndef USE_HANDS_FREE_CHANNEL
	at_command_map[AT_SIDET] = "AT+SIDET=0,0\r";
	at_command_map[AT_CMIC] = "AT+CMIC=0,8\r";
	at_command_map[AT_CLVL] = "AT+CLVL=11\r";
	at_command_map[AT_CHFA] = "AT+CHFA=0\r";
	at_command_map[AT_ECHO] = "AT+ECHO=0,7,5,1\r";
#else
	at_command_map[AT_SIDET] = "AT+SIDET=2,0\r";
	at_command_map[AT_CMIC] = "AT+CMIC=2,8\r";
	at_command_map[AT_CLVL] = "AT+CLVL=8\r";
	at_command_map[AT_CHFA] = "AT+CHFA=2\r";
	at_command_map[AT_ECHO] = "AT+ECHO=2,7,5,1\r";
#endif
	at_command_map[AT_CPAS] = "AT+CPAS\r";
	at_command_map[AT_IFC] = "AT+IFC=2,2\r";
	at_command_map[AT_IFC1] = "AT+IFC?\r";
}

void gsm_put_char(const uint8_t *str, uint16_t length)
{
#ifdef TEST_GSM_MODE_PUT_CHAR
	uint16_t index = 0;

	while (index < length)
	{
		if (str[index] != '\r')
		{
			rt_kprintf("%c", str[index++]);
		}
		else
		{
			index++;
		}
	}
	if (length)
	{
		rt_kprintf("\n");
	}
#else
    return ;
#endif
}

void gsm_put_hex(const uint8_t *str, uint16_t length)
{
#ifdef TEST_GSM_MODE_PUT_HEX
	uint16_t index = 0;

	while (index < length)
	{
		rt_kprintf("%02X ", str[index++]);
	}
	if (length)
	{
		rt_kprintf("\n");
	}
#else
	return ;
#endif
}

/*
 * gsm power control, ENABLE/DISABLE
 */
void gsm_power(FunctionalState state)
{
	rt_int8_t dat = 0;
	rt_device_t device_gsm_power = device_enable(DEVICE_NAME_GSM_POWER);

	if (device_gsm_power == RT_NULL)
	{
		rt_kprintf("device %s is not exist!\n", DEVICE_NAME_GSM_POWER);
	}
	else
	{
		if(state == ENABLE)
		{
			dat = 1;
			rt_device_write(device_gsm_power, 0, &dat, 0);
		}
		else if(state == DISABLE)
		{
			dat = 0;
			rt_device_write(device_gsm_power, 0, &dat, 0);
		}
	}
}

GsmStatus gsm_setup(FunctionalState state)
{
	rt_int8_t dat = 0;
	rt_device_t device_gsm_status = device_enable(DEVICE_NAME_GSM_STATUS);

	if (device_gsm_status == RT_NULL)
	{
		rt_kprintf("\ndevice %s is not exist!\n", DEVICE_NAME_GSM_STATUS);
		if (state == ENABLE)
		{
			return GSM_SETUP_ENABLE_FAILURE;
		}
		else
		{
			return GSM_SETUP_DISABLE_FAILURE;
		}
	}
	else
	{
		if (state == ENABLE) // gsm setup
		{
			rt_device_read(device_gsm_status, 0, &dat, 0);
			if (dat == 0) // gsm status == 0
			{
				gsm_power(ENABLE);
				rt_thread_delay(150);
				gsm_power(DISABLE);
				rt_thread_delay(350);
				rt_device_read(device_gsm_status, 0, &dat, 0);
				if (dat != 0)
				{
					rt_kprintf("\nthe gsm device is setup!\n");
					return GSM_SETUP_ENABLE_SUCCESS;
				}
				else
				{
					rt_kprintf("\nthe gsm device can not setup! please try again.\n");
					return GSM_SETUP_ENABLE_FAILURE;
				}
			}
			else
			{
				// do nothing
				rt_kprintf("\nthe gsm device has been setup!\n");
				return GSM_SETUP_ENABLE_SUCCESS;
			}
		}
		else if(state == DISABLE) // gsm close
		{
			rt_device_read(device_gsm_status, 0, &dat, 0);
			if (dat != 0) // gsm status != 0
			{
				gsm_power(ENABLE);
				rt_thread_delay(200);
				gsm_power(DISABLE);
				rt_thread_delay(400);
				rt_device_read(device_gsm_status, 0, &dat, 0);
				if (dat == 0)
				{
					rt_kprintf("\nthe gsm device is close!\n");
					return GSM_SETUP_DISABLE_SUCCESS;
				}
				else
				{
					rt_kprintf("\nthe gsm device can not close! please try again.\n");
					return GSM_SETUP_DISABLE_FAILURE;
				}

			}
			else
			{
				// do nothing
				rt_kprintf("\nthe gsm device has been closed!\n");
				return GSM_SETUP_DISABLE_SUCCESS;
			}
		}
	}
	return GSM_SETUP_DISABLE_SUCCESS;
}

GsmStatus gsm_reset(void)
{
	if (gsm_setup(DISABLE) == GSM_SETUP_DISABLE_SUCCESS)
	{
		if (gsm_setup(ENABLE) == GSM_SETUP_ENABLE_SUCCESS)
		{
			return GSM_RESET_SUCCESS;
		}
		else
		{
			return GSM_RESET_FAILURE;
		}
	}
	else
	{
		return GSM_RESET_FAILURE;
	}
}
#ifdef GSM_AT_CMD_QUICK
volatile rt_uint16_t RecvFrameDelay = 50;

/* set_value = 0 : is read only RecvFrameDelay
 * set_value > 0 : set RecvFrameDelay value
 */
rt_uint16_t gsm_at_delay_value(rt_uint16_t set_value)
{
	if(RecvFrameDelay <= 130)
	{
		RecvFrameDelay = 130;
	}
	if(set_value <= 0)
	{
		return RecvFrameDelay;
	}
	else
	{
		RecvFrameDelay = set_value;
	}
	return RecvFrameDelay;
}
#endif
void gsm_recv_defult_time(void)
{
#ifdef GSM_AT_CMD_QUICK
	gsm_at_delay_value(150);
#endif
	return ;
}
uint16_t gsm_recv_frame(uint8_t *buf)
{
	uint8_t *buf_bk = buf;
	uint8_t temp = 0;
	uint16_t length = 0;
	uint16_t counts = 0;
	uint16_t result;
#ifdef	GSM_AT_CMD_QUICK
	uint16_t nonedat = 0;
#endif
	rt_device_t device_gsm_usart;

	device_gsm_usart = device_enable(DEVICE_NAME_GSM_USART);

rescan_frame:

	memset(buf, 0, length);
	buf_bk = buf;
	temp = 0;
	length = 0;
	counts = 0;

	while (counts < 512)
	{
		result = rt_device_read(device_gsm_usart, 0, &temp, 1);

		if (result == 1)
		{
			*buf_bk++ = temp;
			++length;
			if (strstr((char *)buf,"\r\n"))
			{
				if (length == 2)
				{
					goto rescan_frame;
				}
				else
				{
					break;
				}
			}
		}
#ifdef	GSM_AT_CMD_QUICK
		else
		{
			nonedat++;
			if(nonedat < gsm_at_delay_value(0))
			{
				rt_thread_delay(1);
			}
			//rt_kprintf("GSM AT CMD delay");
		}
#endif
		++counts;
	}
	/*
	  if (strstr((char *)buf,"RING"))
	  {
	  // has phone call
	  goto rescan_frame;
	  }
	  if (strstr((char *)buf,"+CMTI:"))
	  {
	  // has sms recv
	  goto rescan_frame;
	  }
	*/
	return length;
}




AT_RESPONSE_TYPEDEF
at_response_process(AT_COMMAND_INDEX_TYPEDEF index, uint8_t *buf, GSM_MAIL_CMD_DATA *cmd_data)
{
	uint8_t counts = 0;
	uint8_t no_response_counts = 0;
	uint8_t delay_counts = 0;
	uint16_t recv_counts = 0;
	AT_RESPONSE_TYPEDEF result = AT_RESPONSE_ERROR;
	uint8_t *process_buf = (uint8_t *)rt_malloc(512);
	int temp = 0;
	rt_device_t device_gsm_usart;
    
    RT_ASSERT(process_buf != RT_NULL);
	device_gsm_usart = device_enable(DEVICE_NAME_GSM_USART);
	while (counts < 10)
	{
		memset(process_buf, 0, 512);
		recv_counts = gsm_recv_frame(process_buf);
		gsm_put_char(process_buf, strlen((char *)process_buf));
		gsm_put_hex(process_buf, strlen((char *)process_buf));
		if (recv_counts)
		{
			switch (index)
			{
				case AT :
				case AT_D1 :
				case AT_W :
				case AT_CNMI :
				case AT_CMGF_0 :
				case AT_CMGF_1 :
				case AT_CMGD :
				case AT_CPIN :
				case AT_CIPMODE:
				case AT_CSTT :
				case AT_CIPSHUT :
				case AT_CMMSINIT :
				case AT_CMMSTERM :
				case AT_HTTPINIT :
				case AT_HTTPTERM :
				case AT_HTTPPARA_CID :
				case AT_CMMSCID :
				case AT_CMMSCURL :
				case AT_CMMSPROTO :
				case AT_CMMSSENDCFG :
				case AT_SAPBR_CONTYPE :
				case AT_SAPBR_APN_CMWAP :
				case AT_SAPBR_APN_CMNET :
				case AT_SAPBR_CLOSE :
				case AT_CMMSEDIT_OPEN :
				case AT_CMMSEDIT_CLOSE :
				case AT_SIDET :
				case AT_CMIC :
				case AT_CLVL :
				case AT_CHFA :
				case AT_ECHO :
				case ATA:
				case ATH5:
				case AT_IFC:
				case AT_IFC1:{

					if (strstr((char *)process_buf, at_command_map[index]))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, "OK"))
							{
								result = AT_RESPONSE_OK;
							}
						}
						goto complete;
					}
					break;
				};
				case AT_CMMSRECP : {

					if (strstr((char *)process_buf, (char *)buf))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, "OK"))
							{
								result = AT_RESPONSE_OK;
							}
						}
						goto complete;
					}
					break;
				};
				case AT_HTTPPARA_BREAK:
				case AT_HTTPPARA_BREAKEND:
				case AT_HTTPPARA_URL : {

					if (strstr((char *)process_buf, (char *)buf))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, "OK"))
							{
								result = AT_RESPONSE_OK;
							}
						}
						goto complete;
					}
					break;
				};
				case AT_CIICR : {

					if (strstr((char *)process_buf, at_command_map[index]))
					{
						delay_counts = 0;
						while (delay_counts++ < 10)
						{
							rt_thread_delay(200);
							memset(process_buf, 0, 512);
							recv_counts = gsm_recv_frame(process_buf);
							if (recv_counts)
							{
								gsm_put_char(process_buf, strlen((char *)process_buf));
								gsm_put_hex(process_buf, strlen((char *)process_buf));
								if (strstr((char *)process_buf, "OK"))
								{
									result = AT_RESPONSE_OK;
								}
								break;
							}
						}
						goto complete;
					}
					break;
				};
				case AT_SAPBR_OPEN : {

					if (strstr((char *)process_buf, at_command_map[index]))
					{
						delay_counts = 0;
						while (delay_counts++ < 10)
						{
							rt_thread_delay(200);
							memset(process_buf, 0, 512);
							recv_counts = gsm_recv_frame(process_buf);
							if (recv_counts)
							{
								gsm_put_char(process_buf, strlen((char *)process_buf));
								gsm_put_hex(process_buf, strlen((char *)process_buf));
								if (strstr((char *)process_buf, "OK"))
								{
									result = AT_RESPONSE_OK;
								}
								break;
							}
						}
						goto complete;
					}
					break;
				};
				case AT_CMMSSEND : {

					if (strstr((char *)process_buf, at_command_map[index]))
					{
						delay_counts = 0;
						while (delay_counts++ < 100)
						{
							rt_thread_delay(200);
							memset(process_buf, 0, 512);
							recv_counts = gsm_recv_frame(process_buf);
							if (recv_counts)
							{
								gsm_put_char(process_buf, strlen((char *)process_buf));
								gsm_put_hex(process_buf, strlen((char *)process_buf));
								if (strstr((char *)process_buf, "OK"))
								{
									result = AT_RESPONSE_OK;
								}
								break;
							}
						}
						goto complete;
					}
					break;
				};
				case AT_SAPBR_REQUEST : {

					if (strstr((char *)process_buf, at_command_map[index]))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (!strstr((char *)process_buf, "ERROR"))
							{
								memset(process_buf + 200, 0, 512-200);
								recv_counts = gsm_recv_frame(process_buf + 200);
								if (recv_counts)
								{
									gsm_put_char(process_buf+200, strlen((char *)process_buf+200));
									gsm_put_hex(process_buf+200, strlen((char *)process_buf+200));

									if (strstr((char *)process_buf + 200, "OK"))
									{
										//sscanf((char *)process_buf, "%*[^\"]\"+%[^\"]", smsc);
										result = AT_RESPONSE_OK;
									}
								}
							}
						}
						goto complete;
					}
					break;
				}

				case AT_CSCA : {

					if (strstr((char *)process_buf, at_command_map[index]))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (!strstr((char *)process_buf, "ERROR"))
							{
								memset(process_buf + 200, 0, 512-200);
								recv_counts = gsm_recv_frame(process_buf + 200);
								if (recv_counts)
								{
									gsm_put_char(process_buf+200, strlen((char *)process_buf+200));
									gsm_put_hex(process_buf+200, strlen((char *)process_buf+200));

									if (strstr((char *)process_buf + 200, "OK"))
									{
										memset(smsc, 0, sizeof(smsc));
										sscanf((char *)process_buf, "%*[^\"]\"+%[^\"]", smsc);
										result = AT_RESPONSE_OK;
										//send_ctx_mail(COMM_TYPE_GSM_SMSC, 0, 0, (uint8_t *)smsc, sizeof(smsc));
									}
								}
							}
						}
						goto complete;
					}
					break;
				}
				case AT_HTTPACTION_GET : {

					if (strstr((char *)process_buf, at_command_map[index]))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, "OK"))
							{
								delay_counts = 0;
								while (delay_counts++ < 100)
								{
									rt_thread_delay(200);
									memset(process_buf, 0, 512);
									recv_counts = gsm_recv_frame(process_buf);
									if (recv_counts)
									{
										gsm_put_char(process_buf, strlen((char *)process_buf));
										gsm_put_hex(process_buf, strlen((char *)process_buf));

										if (strstr((char *)process_buf, "+HTTPACTION:0"))
										{
											/* TODO:  aip bin size */
											sscanf((char *)process_buf, "+HTTPACTION:0,%d,%d", &temp , &temp);
											switch (temp)
											{
												case 200 : {
													result = AT_RESPONSE_OK;
													break;
												}
												case 206 : {
													result = AT_RESPONSE_PARTIAL_CONTENT;
													break;
												}
												case 400 : {
													result = AT_RESPONSE_BAD_REQUEST;
													break;
												}
												case 602 : {
													result = AT_RESPONSE_NO_MEMORY;
													break;
												}
												default : {
													break;
												}
											}
											break;
										}
										else
										{
											continue;
										}
									}
								}
							}
						}
						goto complete;
					}
					break;
				}
				case AT_HTTPREAD : {

					if (strstr((char *)process_buf, (char *)buf))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, "+HTTPREAD:"))
							{
								sscanf((char *)process_buf, "+HTTPREAD:%d", &temp);
								if (temp <= 512)
								{
#ifdef GSM_AT_CMD_QUICK
									rt_uint16_t exit = 0;
									rt_uint8_t  rev_cnt = 0;

									recv_counts = 0;
									while(1)
									{
										rev_cnt = rt_device_read(device_gsm_usart, 0,
																 (cmd_data->httpread.buf+recv_counts),
																 1);
										if(rev_cnt == 1)
										{
											recv_counts++;
											if(recv_counts == temp)
											{
												rt_kprintf("exit %d",exit);
												break;
											}
										}
										else
										{
											exit++;
											if(exit % 10 == 0)
											{
												rt_thread_delay(1);
											}
											if(exit > 5000)
											{
												rt_kprintf("recv HTTP data outtime\n");
												break;
											}
										}
									}
#else
									recv_counts = rt_device_read(device_gsm_usart, 0,
																 cmd_data->httpread.buf,
																 temp);
#endif

									if (recv_counts)
									{
										*(cmd_data->httpread.recv_counts) = recv_counts;
										memset(process_buf, 0, 512);
										recv_counts = gsm_recv_frame(process_buf);
										if (recv_counts)
										{

											if (strstr((char *)process_buf, "OK"))
											{
												gsm_put_char(process_buf, strlen((char *)process_buf));
												gsm_put_hex(process_buf, strlen((char *)process_buf));
												result = AT_RESPONSE_OK;
											}
										}
									}
								}
							}
						}
						goto complete;
					}
					break;
				}

				case AT_CIFSR : {

					if (strstr((char *)process_buf, at_command_map[index]))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							//sscanf((char *)process_buf, "%[^\"]", smsc);
							result = AT_RESPONSE_OK;
						}
						goto complete;
					}
					break;
				}
				case AT_CIPSTATUS : {

					if (strstr((char *)process_buf, at_command_map[index]))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, "OK"))
							{
								memset(process_buf, 0, 512);
								recv_counts = gsm_recv_frame(process_buf);
								if (recv_counts)
								{
									gsm_put_char(process_buf, strlen((char *)process_buf));
									gsm_put_hex(process_buf, strlen((char *)process_buf));
									if (strstr((char *)process_buf, "CONNECT OK"))
									{
										result = AT_RESPONSE_CONNECT_OK;
									}
									else if (strstr((char *)process_buf, "TCP CLOSED"))
									{
										result = AT_RESPONSE_TCP_CLOSED;
									}
								}
							}
						}
						goto complete;
					}

					break;
				};
				case AT_CIPSTART : {

					if (strstr((char *)process_buf, (char *)buf))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, "OK"))
							{
								delay_counts = 0;
								while (delay_counts++ < 10)
								{
									rt_thread_delay(200);
									memset(process_buf, 0, 512);
									recv_counts = gsm_recv_frame(process_buf);
									if (recv_counts)
									{
										gsm_put_char(process_buf, strlen((char *)process_buf));
										gsm_put_hex(process_buf, strlen((char *)process_buf));
										if (strstr((char *)process_buf, "CONNECT"))
										{
											result = AT_RESPONSE_CONNECT_OK;
										}
										break;
									}
								}
							}
						}
						goto complete;
					}

					break;
				}
				case AT_CMGS : {

					if (strstr((char *)process_buf, (char *)buf))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, ">"))
							{
								result = AT_RESPONSE_OK;
							}
						}
						goto complete;
					}

					break;
				}
				case AT_CMGS_SUFFIX : {

					if (strstr((char *)process_buf, (char *)buf))
					{
						memset(process_buf, 0, 512);
						//rt_kprintf("read AT+CMGS");
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, "+CMGS:"))
							{
								//rt_kprintf("read +CMGS:\n");
								memset(process_buf, 0, 512);
								recv_counts = gsm_recv_frame(process_buf);
								if (recv_counts)
								{
									//rt_kprintf("read ok\n");
									gsm_put_char(process_buf, strlen((char *)process_buf));
									gsm_put_hex(process_buf, strlen((char *)process_buf));
									if (strstr((char *)process_buf, "OK"))
									{
										result = AT_RESPONSE_OK;
									}
								}
							}
						}
						goto complete;
					}

					break;
				};
				case ATO : {

					if (strstr((char *)process_buf, at_command_map[index]))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, "NO CARRIER"))
							{
								result = AT_RESPONSE_NO_CARRIER;
							}
							else if (strstr((char *)process_buf, "CONNECT"))
							{
								result = AT_RESPONSE_OK;
							}
						}
						goto complete;
					}
					break;
				};
				case PLUS3 : {

					if (strstr((char *)process_buf, "OK"))
					{
						result = AT_RESPONSE_OK;
			goto complete;
					}
		    break;
				};
				case AT_CMMSDOWN_PIC :
				case AT_CMMSDOWN_TITLE :
				case AT_CMMSDOWN_TEXT : {

					if (strstr((char *)process_buf, (char *)buf))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (strstr((char *)process_buf, "CONNECT"))
							{
								result = AT_RESPONSE_CONNECT_OK;
							}
						}
						goto complete;
					}
					break;
				};

				case AT_CMMSDOWN_DATA : {

					if (strstr((char *)process_buf, "OK"))
					{
						result = AT_RESPONSE_OK;
						goto complete;
					}
					break;
				};
				case AT_RING : {

					if (strstr((char *)process_buf, "RING"))
					{
						result = AT_RESPONSE_OK;
						goto complete;
					}
					break;
				};
				case AT_CLCC : {
					if (strstr((char *)process_buf, at_command_map[index]))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (!strstr((char *)process_buf, "ERROR"))
							{
								memset(process_buf + 200, 0, 512-200);
								recv_counts = gsm_recv_frame(process_buf + 200);
								if (recv_counts)
								{
									gsm_put_char(process_buf+200, strlen((char *)process_buf+200));
									gsm_put_hex(process_buf+200, strlen((char *)process_buf+200));

									if (strstr((char *)process_buf + 200, "OK"))
									{
                                        union alarm_data data;
										memset(phone_call, 0, sizeof(phone_call));
										sscanf((char *)process_buf, "%*[^\"]\"%[^\"]", phone_call);
										result = AT_RESPONSE_OK;
										//send_ctx_mail(COMM_TYPE_GSM_PHONE_CALL, 0, 0, (uint8_t *)phone_call, sizeof(phone_call));
                                        rt_memcpy(&data.ring.phone_call, phone_call, sizeof(phone_call));
                                        send_local_mail(ALARM_TYPE_GSM_RING, 0, &data);
									}
								}
							}
						}
						goto complete;
					}
					break;
				};
				case AT_CPAS : {
					if (strstr((char *)process_buf, at_command_map[index]))
					{
						memset(process_buf, 0, 512);
						recv_counts = gsm_recv_frame(process_buf);
						if (recv_counts)
						{
							gsm_put_char(process_buf, strlen((char *)process_buf));
							gsm_put_hex(process_buf, strlen((char *)process_buf));
							if (!strstr((char *)process_buf, "ERROR"))
							{
								memset(process_buf + 200, 0, 512-200);
								recv_counts = gsm_recv_frame(process_buf + 200);
								if (recv_counts)
								{
									gsm_put_char(process_buf+200, strlen((char *)process_buf+200));
									gsm_put_hex(process_buf+200, strlen((char *)process_buf+200));

									if (strstr((char *)process_buf + 200, "OK"))
									{
										sscanf((char *)process_buf, "+CPAS: %d", &temp);
										switch (temp)
										{
											case 0 : {
												result = AT_RESPONSE_READY;
												break;
											}
											case 2 : {
												result = AT_RESPONSE_UNKNOW;
												break;
											}
											case 3 : {
												result = AT_RESPONSE_RING;
												break;
											}
											case 4 : {
												result = AT_RESPONSE_CALLING;
												break;
											}
											default : {
												break;
											}
										}
									}
								}
							}
						}
						goto complete;
					}
					break;
				};
				case AT_RECV_SMS://sms
					{
						if(rt_strstr((char *)process_buf,"AT+CMGR"))
						{
							/* sms information */
							memset(process_buf, 0, 512);
							recv_counts = gsm_recv_frame(process_buf);
							rt_memcpy(cmd_data->sms_rcv.info_buf,process_buf,recv_counts);

							/* sms content */
							memset(process_buf, 0, 512);
							recv_counts = gsm_recv_frame(process_buf);
							rt_memcpy(cmd_data->sms_rcv.text_buf,process_buf,recv_counts);

							/* sms read status */
							memset(process_buf, 0, 512);
							recv_counts = gsm_recv_frame(process_buf);

							if(rt_strstr((char *)process_buf, "OK"))
							{
								result = AT_RESPONSE_OK;
								goto complete;
							}
						}

						break;
					}
				default:{
					break;
				};
			}

		}
		else
		{
			// no result process
			no_response_counts++;
			if (no_response_counts >= 10)
			{
				result = AT_NO_RESPONSE;
			}
		}
		counts++;
		rt_thread_delay(20);
	}

complete:
	rt_free(process_buf);
	process_buf = RT_NULL;

	return result;
}

AT_RESPONSE_TYPEDEF
gsm_command(AT_COMMAND_INDEX_TYPEDEF index, uint16_t delay, GSM_MAIL_CMD_DATA *cmd_data)
{
	rt_device_t device_gsm_usart;
	AT_RESPONSE_TYPEDEF result = AT_RESPONSE_OK;
	uint8_t *process_buf = (uint8_t *)rt_malloc(512);
    RT_ASSERT(process_buf != RT_NULL);
	device_gsm_usart = device_enable(DEVICE_NAME_GSM_USART);

	switch (index)
	{
		case AT_HTTPPARA_BREAK:{

			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   cmd_data->httppara_break.start);

			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));

			gsm_put_char(process_buf, strlen((char *)process_buf));

			break;
		};
		case AT_HTTPPARA_BREAKEND:{

			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   cmd_data->httppara_breakend.end);

			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));

			gsm_put_char(process_buf, strlen((char *)process_buf));

			break;
		};
		case AT_HTTPPARA_URL:{

			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   cmd_data->httppara_url.buf);

			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));

			gsm_put_char(process_buf, strlen((char *)process_buf));

			break;
		};
		case AT_HTTPREAD:{

			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   cmd_data->httpread.start,
					   cmd_data->httpread.size_of_process);

			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));

			gsm_put_char(process_buf, strlen((char *)process_buf));

			break;
		};
		case AT_CIPSTART:{
			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   tcp_domain.domain,
					   tcp_domain.port);

			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));

			gsm_put_char(process_buf, strlen((char *)process_buf));
			break;
		};
		case AT_CMGS:{

			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   cmd_data->cmgs.cmgs_length);

			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));

			gsm_put_char(process_buf, strlen((char *)process_buf));

			break;
		};
		case AT_CMMSRECP:{

			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   cmd_data->cmmsrecp.buf);

			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));

			gsm_put_char(process_buf, strlen((char *)process_buf));

			break;
		};

		case AT_CMMSDOWN_PIC:{

			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   cmd_data->cmmsdown_pic.length);

			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));

			gsm_put_char(process_buf, strlen((char *)process_buf));

			break;
		};
		case AT_CMMSDOWN_TITLE:{

			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   cmd_data->cmmsdown_title.length);

			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));

			gsm_put_char(process_buf, strlen((char *)process_buf));

			break;
		};
		case AT_CMMSDOWN_TEXT:{

			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   cmd_data->cmmsdown_text.length);

			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));

			gsm_put_char(process_buf, strlen((char *)process_buf));

			break;
		};
		case AT_CMMSDOWN_DATA:{

			memset(process_buf, 0, 512);
			memcpy(process_buf, cmd_data->cmmsdown_data.buf, cmd_data->cmmsdown_data.length);
			rt_device_write(device_gsm_usart, 0,
							process_buf,
							cmd_data->cmmsdown_data.length);

			gsm_put_char(process_buf, cmd_data->cmmsdown_data.length);

			if(!cmd_data->cmmsdown_data.has_complete)
			{
				goto at_process_complete;
			}

			break;
		};

		case PLUS3 : {
			rt_thread_delay(150);
			rt_device_write(device_gsm_usart, 0,
							at_command_map[index],
							strlen(at_command_map[index]));

			gsm_put_char((uint8_t *)at_command_map[index], strlen(at_command_map[index]));
			break;
		};
		case AT_RING :{

			break;
		}
		case AT_RECV_SMS :{
			memset(process_buf, 0, 512);
			rt_sprintf((char *)process_buf,
					   at_command_map[index],
					   cmd_data->sms_rcv.pos);
			rt_device_write(device_gsm_usart, 0,
							process_buf,
							strlen((char *)process_buf));
			gsm_put_char(process_buf, strlen((char *)process_buf));
			break;
		};
		default: {

			rt_device_write(device_gsm_usart, 0,
							at_command_map[index],
							strlen(at_command_map[index]));

			gsm_put_char((uint8_t *)at_command_map[index], strlen(at_command_map[index]));
			break;
		}
	}
#ifndef	GSM_AT_CMD_QUICK
	rt_thread_delay(delay);
#else
	gsm_at_delay_value(delay);
#endif
	result = at_response_process(index, process_buf, cmd_data);

	switch (index)
	{
		case AT_CMGS:{
			if (result == AT_RESPONSE_OK)
			{
				memset(process_buf, 0, 512);
				memcpy(process_buf, cmd_data->cmgs.buf, cmd_data->cmgs.length);
				rt_device_write(device_gsm_usart, 0,
								process_buf,
								cmd_data->cmgs.length);
				rt_device_write(device_gsm_usart, 0, "\x1A", 1);
				gsm_put_char(process_buf, cmd_data->cmgs.length);
				rt_thread_delay(650);
				result = at_response_process(AT_CMGS_SUFFIX, process_buf, cmd_data);
			}
			else
			{
				result = AT_RESPONSE_ERROR;
			}
			break;
		};
		default: {
			break;
		}
	}
at_process_complete:
	rt_free(process_buf);
	process_buf = RT_NULL;
	//RT_DEBUG_LOG(PRINTF_AT_CMD_SEND_RESULT,("\n\nresult = %d\n\n", result));
	return result;
}


static AT_RESPONSE_TYPEDEF
gsm_dialing(void)
{
	AT_RESPONSE_TYPEDEF result = AT_RESPONSE_ERROR;
	GSM_MAIL_CMD_DATA gsm_mail_cmd_data;
	if ((gsm_command(AT_CIPSHUT,50,&gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_CIPMODE,50,&gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_CSTT,50,&gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_CIICR,300,&gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_CIFSR,50,&gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_CIPSTART,300,&gsm_mail_cmd_data)== AT_RESPONSE_CONNECT_OK))
	{
		result = AT_RESPONSE_OK;
	}

	return result;
}

GSM_ERROR_TYPEDEF
gsm_init_process(void)
{
	GSM_ERROR_TYPEDEF result = GSM_EERROR;
	uint8_t *process_buf = (uint8_t *)rt_malloc(512);
	GSM_MAIL_CMD_DATA gsm_mail_cmd_data;
    
    RT_ASSERT(process_buf != RT_NULL);
	memset(process_buf,0,512);
	gsm_recv_frame(process_buf);
	gsm_put_char(process_buf, strlen((char *)process_buf));
	gsm_put_hex(process_buf, strlen((char *)process_buf));
	if ((gsm_command(AT, 200, &gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_D1, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_SIDET, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_CMIC, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_CLVL, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_CHFA, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK) &&
		(gsm_command(AT_ECHO, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK)
		 //&& (gsm_command(AT_IFC, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK)
		 && (gsm_command(AT_W, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK)
		 && (gsm_command(AT_CNMI, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK)
		 && (gsm_command(AT_CSCA, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK)
		 && (gsm_command(AT_CMGF_0, 50, &gsm_mail_cmd_data) == AT_RESPONSE_OK)
    )
	{
		result = GSM_EOK;
	}

	rt_free(process_buf);
	process_buf = RT_NULL;

	return result;
}

AT_RESPONSE_TYPEDEF
gsm_mode_switch(uint8_t flag)
{
	AT_RESPONSE_TYPEDEF result = AT_RESPONSE_ERROR;
	GSM_MAIL_CMD_DATA gsm_mail_cmd_data;

	if (flag)
	{
		result = gsm_command(ATO, 100, &gsm_mail_cmd_data);
	}
	else
	{
		result = gsm_command(PLUS3, 100, &gsm_mail_cmd_data);
	}
	return result;
}

AT_RESPONSE_TYPEDEF
gsm_ring_process(uint8_t flag)
{
	GSM_MAIL_CMD_DATA cmd_data;
	AT_RESPONSE_TYPEDEF result = AT_RESPONSE_ERROR;

	if (gpio_pin_input(DEVICE_NAME_GSM_RING, 1) == GSM_RING_STATUS)
	{
		if(send_cmd_mail(AT_CLCC, 100, &cmd_data, flag) == AT_RESPONSE_OK)
		{
			result = AT_RESPONSE_OK;
		}
	}
	return result;
}
AT_RESPONSE_TYPEDEF
gsm_phone_call_process(int type, uint8_t flag)
{
	GSM_MAIL_CMD_DATA cmd_data;
	AT_RESPONSE_TYPEDEF result = AT_RESPONSE_ERROR;

	switch (type)
	{
		case GSM_CTRL_PHONE_CALL_ANSWER:
			{
				if(send_cmd_mail(ATA, 100, &cmd_data, flag) == AT_RESPONSE_OK)
				{
					result = AT_RESPONSE_OK;
				}
				break;
			}
		case GSM_CTRL_PHONE_CALL_HANG_UP:
			{
				if(send_cmd_mail(ATH5, 100, &cmd_data, flag) == AT_RESPONSE_OK)
				{
					result = AT_RESPONSE_OK;
				}
				break;
			}
	}
	return result;
}

void
gsm_thread_entry(void *parameters)
{
	rt_device_t device_gsm_status;
	rt_device_t device_gsm_usart;
//	static GSM_STATUS gsm_status = GSM_STATUS_CLOSE;
	GSM_MAIL_TYPEDEF gsm_mail_buf;
	GSM_MODE_TYPEDEF send_mode;
	rt_err_t result;
	AT_RESPONSE_TYPEDEF at_result = AT_RESPONSE_ERROR;
//    rt_size_t recv_cnts = 0;
//	int8_t send_counts = 0;
//	uint8_t process_buf[512];
	uint8_t data;

	device_gsm_usart = device_enable(DEVICE_NAME_GSM_USART);
	device_gsm_status = device_enable(DEVICE_NAME_GSM_STATUS);
	device_enable(DEVICE_NAME_GSM_RING);

	at_command_map_init();

	while (1) {

		result = rt_mq_recv(gsm_mq, &gsm_mail_buf, sizeof(gsm_mail_buf), 2*60*100);

		if (result == RT_EOK)
		{
			rt_device_read(device_gsm_status, 0, &data, 1);
			send_mode = gsm_mail_buf.send_mode;
			if (send_mode == GSM_MODE_CONTROL)
			{
				switch (gsm_mail_buf.mail_data.control.cmd)
				{
					case GSM_CTRL_CLOSE:
						{
                            if (gpio_pin_input(DEVICE_NAME_GSM_STATUS, 0)) {
                                gpio_pin_output(DEVICE_NAME_POWER_GSM, 0,1);
                                if (gsm_setup(DISABLE) == GSM_SETUP_DISABLE_SUCCESS)
                                    at_result = AT_RESPONSE_OK;
                            }
                            break;
						}
					case GSM_CTRL_OPEN:
						{
                            if (!gpio_pin_input(DEVICE_NAME_GSM_STATUS, 0)) {
                                gpio_pin_output(DEVICE_NAME_POWER_GSM, 1,1);
                                if (gsm_setup(ENABLE) == GSM_SETUP_ENABLE_SUCCESS && gsm_init_process() == GSM_EOK)
                                    at_result = AT_RESPONSE_OK;
                                                            
                            }
                            break;
						}
					case GSM_CTRL_RESET:
						{
							if (gsm_reset() == GSM_RESET_SUCCESS && gsm_init_process() == GSM_EOK)
								at_result = AT_RESPONSE_OK;
                            break;
						}
					case GSM_CTRL_DIALING:
						{
							if (data)
								at_result = gsm_dialing();
							else
								at_result = AT_NO_RESPONSE;
							break;
						}
					case GSM_CTRL_SWITCH_TO_CMD:
						{
							if (data)
								at_result = gsm_mode_switch(0);
							else
								at_result = AT_NO_RESPONSE;
							break;
						}
					case GSM_CTRL_SWITCH_TO_GPRS:
						{
							if (data)
								at_result = gsm_mode_switch(1);
							else
								at_result = AT_NO_RESPONSE;
							break;
						}
					case GSM_CTRL_PHONE_CALL_ANSWER:
					case GSM_CTRL_PHONE_CALL_HANG_UP:
						{
							if (data)
								at_result = gsm_phone_call_process(gsm_mail_buf.mail_data.control.cmd, 0);
							else
								at_result = AT_NO_RESPONSE;
							break;
						}
					default :
						{
							break;
						}
				}
                
			}
			if (send_mode == GSM_MODE_CMD)
			{
				at_result = gsm_command(gsm_mail_buf.mail_data.cmd.index,
										gsm_mail_buf.mail_data.cmd.delay,
										&(gsm_mail_buf.mail_data.cmd.cmd_data));
                
//                if (gsm_mail_buf.mail_data.cmd.index == AT_CMGS)
//                    rt_free(gsm_mail_buf.mail_data.cmd.cmd_data.cmgs.buf - 2);

			}
			if (send_mode == GSM_MODE_GPRS)
			{
				rt_device_write(device_gsm_usart, 0, gsm_mail_buf.mail_data.gprs.request, gsm_mail_buf.mail_data.gprs.request_length);
//                rt_free(gsm_mail_buf.mail_data.gprs.request);
				at_result = AT_RESPONSE_OK;
			}

            if (gsm_mail_buf.result != RT_NULL)
                *gsm_mail_buf.result = at_result;
            if(gsm_mail_buf.flag && gsm_mail_buf.result_sem != RT_NULL)
                rt_sem_release(gsm_mail_buf.result_sem);
		}
		else// time out
		{
            gpio_pin_output(DEVICE_NAME_POWER_GSM, 0,1);
            /*
            process_buf[0] = 0;// asyn comm
            recv_cnts = rt_device_read(device_gsm_usart, 0, process_buf + 1, sizeof(process_buf) - 1);
            //if (recv_cnts > 0)
                //send_ctx_mail(COMM_TYPE_GPRS, 0, 50, process_buf, recv_cnts+1);
            */
        }

	}
}

AT_RESPONSE_TYPEDEF
send_cmd_mail(AT_COMMAND_INDEX_TYPEDEF command_index, uint16_t delay, GSM_MAIL_CMD_DATA *cmd_data, u8 flag)
{
	GSM_MAIL_TYPEDEF gsm_mail_buf;
	AT_RESPONSE_TYPEDEF send_result = AT_RESPONSE_ERROR;
	rt_err_t result;

    rt_memset(&gsm_mail_buf, 0, sizeof(gsm_mail_buf));
	gsm_mail_buf.send_mode = GSM_MODE_CMD;
	gsm_mail_buf.result = &send_result;
    if (flag)
        gsm_mail_buf.result_sem = rt_sem_create("s_cmd", 0, RT_IPC_FLAG_FIFO);
    else
        gsm_mail_buf.result_sem = RT_NULL;
	gsm_mail_buf.flag = 1;
	gsm_mail_buf.mail_data.cmd.index = command_index;
	gsm_mail_buf.mail_data.cmd.delay = delay;
	gsm_mail_buf.mail_data.cmd.cmd_data = *cmd_data;

	if (gsm_mq != RT_NULL)
	{
		result = rt_mq_send(gsm_mq, &gsm_mail_buf, sizeof(GSM_MAIL_TYPEDEF));
		if (result == -RT_EFULL)
		{
			rt_kprintf("sms_mq is full!!!\n");
		}
		else
		{
            if (gsm_mail_buf.result_sem != RT_NULL)
                rt_sem_take(gsm_mail_buf.result_sem, RT_WAITING_FOREVER);
		}
        if (gsm_mail_buf.result_sem != RT_NULL)
            rt_sem_delete(gsm_mail_buf.result_sem);
	}
	else
	{
		rt_kprintf("sms_mq is RT_NULL!!!\n");
	}

	return send_result;
}

GSM_ERROR_TYPEDEF
send_gsm_sms_mail(uint8_t *buf, uint16_t length, uint8_t flag)
{
	GSM_MAIL_TYPEDEF gsm_mail_buf;
    GSM_ERROR_TYPEDEF error = GSM_EERROR;
	rt_err_t result;
    AT_RESPONSE_TYPEDEF *send_result = RT_NULL;
    uint8_t *buf_bk = RT_NULL;

    rt_memset(&gsm_mail_buf, 0, sizeof(gsm_mail_buf));

	if (flag) {
        
        if (length) {

            buf_bk = rt_malloc(length);
            if (buf_bk == RT_NULL)
                goto __free_process;
            rt_memcpy(buf_bk, buf, length);
        }
        else
            goto __free_process;
        
        send_result = rt_malloc(sizeof(*send_result));
        if (send_result == RT_NULL)
            goto __free_process;
        *send_result = AT_RESPONSE_ERROR;

		gsm_mail_buf.result_sem = rt_sem_create("s_sms", 0, RT_IPC_FLAG_FIFO);
		if (gsm_mail_buf.result_sem == RT_NULL) {
            goto __free_process;
        }
	}
	else
		gsm_mail_buf.result_sem = RT_NULL;

	gsm_mail_buf.send_mode = GSM_MODE_CMD;
	gsm_mail_buf.result = send_result;
	gsm_mail_buf.flag = flag;
	gsm_mail_buf.mail_data.cmd.index = AT_CMGS;
	gsm_mail_buf.mail_data.cmd.delay = 50;
	gsm_mail_buf.mail_data.cmd.cmd_data.cmgs.cmgs_length = *(uint16_t *)buf_bk;
	gsm_mail_buf.mail_data.cmd.cmd_data.cmgs.length = length - 2;
	gsm_mail_buf.mail_data.cmd.cmd_data.cmgs.buf = buf_bk + 2;

	if (gsm_mq != NULL)
	{
		result = rt_mq_send(gsm_mq, &gsm_mail_buf, sizeof(gsm_mail_buf));
		if (result == -RT_EFULL)
		{
			rt_kprintf("mq_gsm is full!!!\n");
		}		
        else
		{
            if (gsm_mail_buf.result_sem != RT_NULL)
                rt_sem_take(gsm_mail_buf.result_sem, RT_WAITING_FOREVER);
            error = GSM_EOK;
		}
	}
	else
	{
		rt_kprintf("mq_gsm is RT_NULL!!!\n");
	}
__free_process:
    if (buf_bk != RT_NULL)
        rt_free(buf_bk);
	if (send_result != RT_NULL)
        rt_free(send_result);
    if (gsm_mail_buf.result_sem != RT_NULL)
        rt_sem_delete(gsm_mail_buf.result_sem);
    return error;
}

GSM_ERROR_TYPEDEF
send_gsm_ctrl_mail(u8 ctrl_cmd, uint8_t *buf, uint16_t length, uint8_t flag)
{
	GSM_MAIL_TYPEDEF gsm_mail_buf;
    GSM_ERROR_TYPEDEF error = GSM_EERROR;
	rt_err_t result;
    AT_RESPONSE_TYPEDEF *send_result = RT_NULL;
    uint8_t *buf_bk = RT_NULL;
    rt_memset(&gsm_mail_buf, 0, sizeof(gsm_mail_buf));

	if (flag) {
        send_result = rt_malloc(sizeof(*send_result));
        if (send_result == RT_NULL)
            goto __free_process;
        *send_result = AT_RESPONSE_ERROR;
		gsm_mail_buf.result_sem = rt_sem_create("s_ctrl", 0, RT_IPC_FLAG_FIFO);
		if (gsm_mail_buf.result_sem == RT_NULL) {
            goto __free_process;
        }
	}
	else
		gsm_mail_buf.result_sem = RT_NULL;

	gsm_mail_buf.send_mode = GSM_MODE_CONTROL;
	gsm_mail_buf.result = send_result;
	gsm_mail_buf.flag = flag;
	gsm_mail_buf.mail_data.control.cmd = ctrl_cmd;

	if (gsm_mq != NULL)
	{
		result = rt_mq_send(gsm_mq, &gsm_mail_buf, sizeof(gsm_mail_buf));
		if (result == -RT_EFULL)
		{
			rt_kprintf("mq_gsm is full!!!\n");
            goto __free_process;
		}        
        else
		{
            if (gsm_mail_buf.result_sem != RT_NULL)
                rt_sem_take(gsm_mail_buf.result_sem, RT_WAITING_FOREVER);
            error = GSM_EOK;
		}
	}
	else
	{
		rt_kprintf("mq_gsm is RT_NULL!!!\n");
        goto __free_process;
	}

__free_process:
    if (buf_bk != RT_NULL)
        rt_free(buf_bk);
	if (send_result != RT_NULL)
        rt_free(send_result);
    if (gsm_mail_buf.result_sem != RT_NULL)
        rt_sem_delete(gsm_mail_buf.result_sem);
    return error;
}

GSM_ERROR_TYPEDEF
send_gsm_gprs_mail(uint8_t *buf, uint16_t length, uint8_t flag)
{
	GSM_MAIL_TYPEDEF gsm_mail_buf;
    GSM_ERROR_TYPEDEF error = GSM_EERROR;
	rt_err_t result;
    uint8_t *buf_bk = RT_NULL;
    AT_RESPONSE_TYPEDEF *send_result = RT_NULL;
    
    rt_memset(&gsm_mail_buf, 0, sizeof(gsm_mail_buf));

	if (flag) {
        if (length) {

            buf_bk = rt_malloc(length);
            if (buf_bk == RT_NULL)
                goto __free_process;
            rt_memcpy(buf_bk, buf, length);
        }
        else
            goto __free_process;
        
        send_result = rt_malloc(sizeof(*send_result));
        if (send_result == RT_NULL)
            goto __free_process;
        *send_result = AT_RESPONSE_ERROR;
        
        gsm_mail_buf.result_sem = rt_sem_create("s_gprs", 0, RT_IPC_FLAG_FIFO);
		if (gsm_mail_buf.result_sem == RT_NULL) {
            goto __free_process;
        }
	}
	else
		gsm_mail_buf.result_sem = RT_NULL;

	gsm_mail_buf.send_mode = GSM_MODE_GPRS;
	gsm_mail_buf.result = send_result;
	gsm_mail_buf.flag = flag;

	gsm_mail_buf.mail_data.gprs.request = buf_bk;
	gsm_mail_buf.mail_data.gprs.request_length = length;

	if (gsm_mq != NULL)
	{
		result = rt_mq_send(gsm_mq, &gsm_mail_buf, sizeof(gsm_mail_buf));
		if (result == -RT_EFULL)
		{
			rt_kprintf("mq_gsm is full!!!\n");
            goto __free_process;
		}        
        else
		{
            if (gsm_mail_buf.result_sem != RT_NULL)
                rt_sem_take(gsm_mail_buf.result_sem, RT_WAITING_FOREVER);
            error = GSM_EOK;
		}
	}
	else
	{
		rt_kprintf("mq_gsm is RT_NULL!!!\n");
        goto __free_process;
	}

__free_process:
    if (buf_bk != RT_NULL)
        rt_free(buf_bk);
	if (send_result != RT_NULL)
        rt_free(send_result);
    if (gsm_mail_buf.result_sem != RT_NULL)
        rt_sem_delete(gsm_mail_buf.result_sem);
    return error;
}

void gsm_muntex_control(rt_uint8_t cmd,char *username)
{
	if(cmd == RT_TRUE)
	{
		rt_mutex_take(mutex_gsm_mail_sequence,RT_WAITING_FOREVER);
		//RT_DEBUG_LOG(PRINTF_MUNTEX_USE_INFO,("%s take GSM %X\n",username,mutex_gsm_mail_sequence));
	}
	else
	{
		rt_mutex_release(mutex_gsm_mail_sequence);
		//RT_DEBUG_LOG(PRINTF_MUNTEX_USE_INFO,("%s release GSM %X\n",username,mutex_gsm_mail_sequence));
	}
}

int
rt_gsm_init(void)
{
	rt_thread_t gsm_thread;

	// initial gsm msg queue
	gsm_mq = rt_mq_create("m_gsm", sizeof(GSM_MAIL_TYPEDEF),
						  GSM_MAIL_MAX_MSGS, RT_IPC_FLAG_FIFO);
    if (gsm_mq == RT_NULL)
        return -1;
	// initial gsm thread
	gsm_thread = rt_thread_create("gsm",
								  gsm_thread_entry, RT_NULL,
								  2048, 102, 5);
	if (gsm_thread == RT_NULL)
        return -1;

    rt_thread_startup(gsm_thread);

    return 0;

}
INIT_APP_EXPORT(rt_gsm_init);

#ifdef RT_USING_FINSH
#include <finsh.h>
void gsm_ip_start(char *ip, int port)
{
	char *at_temp;
	rt_device_t device_gsm_usart = device_enable(DEVICE_NAME_GSM_USART);

	if (device_gsm_usart != RT_NULL)
	{
		at_temp = (char *)rt_malloc(512);
        RT_ASSERT(at_temp != RT_NULL);
		memset(at_temp, '\0', 512);
		rt_sprintf(at_temp,"AT+CIPSTART=\"TCP\",\"%s\",%d\r", ip, port);
		gsm_put_char((uint8_t *)at_temp, strlen(at_temp));
		rt_device_write(device_gsm_usart, 0, at_temp, strlen(at_temp));
		rt_free(at_temp);
		at_temp = RT_NULL;
	}
	else
	{
		rt_kprintf("gsm usart device is not exist !\n");
	}
}
FINSH_FUNCTION_EXPORT(gsm_ip_start, gsm_ip_start[cmd parameters])
FINSH_FUNCTION_EXPORT(gsm_command, gsm_command[at_index delay *buf])
FINSH_FUNCTION_EXPORT(send_cmd_mail, gsm_command[at_index delay *buf])
FINSH_FUNCTION_EXPORT_ALIAS(send_gsm_ctrl_mail, send_gc_mail, send_gsm_ctrl_mail[gsm_ctrl buf length flag]);
#endif
