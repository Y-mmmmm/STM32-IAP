/**
********************************************************
*
* @file      Protocol.c
* @author    Gizwtis
* @version   V2.3
* @date      2015-07-06
*
* @brief     机智云.只为智能硬件而生
*            Gizwits Smart Cloud  for Smart Products
*            链接|增值ֵ|开放|中立|安全|自有|自由|生态
*            www.gizwits.com
*
*********************************************************/

#include "gokit.h"

RingBuffer u_ring_buff;

UART_HandleTypeDef UART_HandleStruct;
Pro_M2W_ReturnInfoTypeDef Pro_M2W_ReturnInfoStruct;
Pro_CommonCmdTypeDef Pro_CommonCmdStruct;
Pro_Wait_AckTypeDef Wait_AckStruct;
Pro_HeadPartP0CmdTypeDef Pro_HeadPartP0Cmd;
Pro_W2D_WifiStatusTypeDef Pro_W2D_WifiStatusStruct;

Pro_Wait_AckTypeDef Wait_AckStruct;
uint8_t SN;

uint8_t g_DevStatus[MAX_P0_LEN];
uint8_t g_P0DataLen;
uint32_t SystemTimeCount;
uint32_t Last_ReportTime;
uint32_t Last_Report_10_Time;
uint8_t timeoutFlag = 0;//Read error package , timeoutFlag 1

/*环形缓冲区,全局变量*/
uint8_t packageFlag = 0;
uint16_t dataLen = 0;
uint16_t count = 0;
uint8_t tmp_buf[MAX_PACKAGE_LEN] = {0};
uint8_t lastValue = 0;
uint8_t curValue = 0;

static uint8_t GizWits_W2D_AckCmdHandle(void);
static void Pro_W2D_GetMcuInfo(void);
static void Pro_W2D_CommonCmdHandle(void);
static void Pro_W2D_WifiStatusHandle(void);
static void Pr0_W2D_RequestResetDeviceHandle(void);
static void Pro_W2D_ReadDevStatusHandle(void);
static void Pro_D2W_ReportDevStatusHandle(void);
static void Pro_UART_SendBuf(uint8_t *Buf, uint16_t PackLen, uint8_t Tag);
static uint8_t GizWits_W2D_AckCmdHandle(void);
static void Pro_W2D_ErrorCmdHandle(Error_PacketsTypeDef errorType);
static uint8_t Pro_GetFrame(void);
static uint8_t GizWits_D2W_Resend_AckCmdHandle(void);
extern uint8_t p0Flag ;

/**************BIG DATA TEST******************/
extern Pro_UpdataData_TypeDef						Pro_UpdataDataStruct;
/**************BIG DATA TEST******************/

/*重发机制*/
static uint8_t GizWits_D2W_Resend_AckCmdHandle(void)
{
    if(Wait_AckStruct.Flag == 1)
    {
        /*重发次数小于两次*/
        if(Wait_AckStruct.SendNum < Send_MaxNum)
        {
            if((SystemTimeCount - Wait_AckStruct.SendTime) > Send_MaxTime)//到达重发时间
            {
                Pro_UART_SendBuf((uint8_t *)&Wait_AckStruct.Cmd_Buff, Wait_AckStruct.ResendBufLen, 0);
#ifdef DEBUG
                printf("Send again\r\n");
#endif
				Wait_AckStruct.SendTime = SystemTimeCount;//赋值新的系统时间
                Wait_AckStruct.SendNum++;//重发次数加1
                return 2;
            }
        }
        /*重发两次,不再发送此报文*/
        else
        {
            memset(&Wait_AckStruct, 0, sizeof(Wait_AckStruct)); //Wait_AckStruct.Flag = 0;
			return 1;
        }
    }
    return 0;
}
/*判断ACK*/
static uint8_t GizWits_W2D_AckCmdHandle(void)
{
    Pro_HeadPartTypeDef * Wait_Ack_HeadPart = (Pro_HeadPartTypeDef *)Wait_AckStruct.Cmd_Buff;
    Pro_HeadPartTypeDef * Recv_HeadPart = (Pro_HeadPartTypeDef *)UART_HandleStruct.Message_Buf;

    if(Wait_AckStruct.Flag == 1)
    {
        /*判断收到的报文命令码和报文序号,是否是需要等到的ACK*/
        if((Wait_Ack_HeadPart->Cmd == (Recv_HeadPart->Cmd - 1)) && (Wait_Ack_HeadPart->SN == Recv_HeadPart->SN))
        {
            memset(&Wait_AckStruct, 0, sizeof(Wait_AckStruct)); //Wait_AckStruct.Flag = 0;
            return 0; //允许重新reprot
        }
    }
    return 1;
}


/*******************************************************************************
* Function Name  : UART_SendBuf
* Description    : 向串口发送数据帧
* Input          : buf:数据起始地址； packLen:数据长度； tag=0,不等待ACK；tag=1,等待ACK；
* Output         : None
* Return         : None
* Attention		   : 若等待ACK，按照协议失败重发3次；数据区出现FF，在其后增加55
*******************************************************************************/

static void Pro_UART_SendBuf(uint8_t *Buf, uint16_t PackLen, uint8_t Tag)
{
    uint16_t i;

    for(i=0; i<PackLen; i++)
    {
        UART2_Send_DATA(Buf[i]);
        if(i >=2 && Buf[i] == 0xFF)
        {
            UART2_Send_DATA(0x55);
        }
    }
    //需要实现重发的报文
    if(Tag == 1)
    {
        Wait_AckStruct.SendTime = SystemTimeCount;
        Wait_AckStruct.SendNum = 0;
        Wait_AckStruct.Flag = 1;
        memcpy(Wait_AckStruct.Cmd_Buff, Buf, PackLen);
    }
}

/*******************************************************************************
* Function Name  : exchangeBytes
* Description    : 模拟的htons 或者 ntohs，如果系统支字节序更改可直接替换成系统函数
* Input          : value
* Output         : None
* Return         : 更改过字节序的short数值
* Attention		   : None
*******************************************************************************/
short	exchangeBytes(short	value)
{
    short			tmp_value;
    uint8_t		*index_1, *index_2;

    index_1 = (uint8_t *)&tmp_value;
    index_2 = (uint8_t *)&value;

    *index_1 = *(index_2+1);
    *(index_1+1) = *index_2;

    return tmp_value;
}

/*******************************************************************************
* Function Name  : CheckSum
* Description    : 校验和算法
* Input          : buf:数据起始地址； packLen:数据长度；
* Output         : None
* Return         : 校验码
* Attention		   : None
*******************************************************************************/
uint8_t CheckSum( uint8_t *buf, int packLen )
{
    int				i;
    uint8_t		sum;

    if(buf == NULL || packLen <= 0) return 0;
    sum = 0;
    for(i=2; i<packLen-1; i++)
        sum += buf[i];

    return sum;
}

void GizWits_init(uint8_t P0_Len)
{
    Pro_HeadPartP0CmdTypeDef *Pro_HeadPartP0Cmd = (Pro_HeadPartP0CmdTypeDef *)g_DevStatus;

    if(P0_Len > MAX_P0_LEN)
    {
        printf("Warning P0_Len out of range\r\n");
        while(1);
    }
    rb_new(&u_ring_buff);//New Ring buff

    memset((uint8_t *)&g_DevStatus, 0, MAX_P0_LEN);
    memset(&Pro_M2W_ReturnInfoStruct, 0, sizeof(Pro_M2W_ReturnInfoStruct));

    Pro_M2W_ReturnInfoStruct.Pro_HeadPart.Head[0] = 0xFF;
    Pro_M2W_ReturnInfoStruct.Pro_HeadPart.Head[1] = 0xFF;
    Pro_M2W_ReturnInfoStruct.Pro_HeadPart.Len = exchangeBytes(sizeof(Pro_M2W_ReturnInfoStruct) - 4);
    Pro_M2W_ReturnInfoStruct.Pro_HeadPart.Cmd = Pro_D2W__GetDeviceInfo_Ack_Cmd;
    memcpy(Pro_M2W_ReturnInfoStruct.Pro_ver, PRO_VER, strlen(PRO_VER));
    memcpy(Pro_M2W_ReturnInfoStruct.P0_ver, P0_VER, strlen(P0_VER));
    memcpy(Pro_M2W_ReturnInfoStruct.Hard_ver, HARD_VER, strlen(HARD_VER));
    memcpy(Pro_M2W_ReturnInfoStruct.Soft_ver, SOFT_VER, strlen(SOFT_VER));
    memcpy(Pro_M2W_ReturnInfoStruct.Product_Key, PRODUCT_KEY, strlen(PRODUCT_KEY));
    Pro_M2W_ReturnInfoStruct.Binable_Time = exchangeBytes(0);


    g_P0DataLen = P0_Len;
    Pro_HeadPartP0Cmd->Pro_HeadPart.Head[0] = 0xFF;
    Pro_HeadPartP0Cmd->Pro_HeadPart.Head[1] = 0xFF;
    Pro_HeadPartP0Cmd->Pro_HeadPart.Len = exchangeBytes(sizeof(Pro_HeadPartP0CmdTypeDef) - 4);
    Pro_HeadPartP0Cmd->Pro_HeadPart.Cmd = 0x0;
    Pro_HeadPartP0Cmd->Pro_HeadPart.SN = 0;
    Pro_HeadPartP0Cmd->Pro_HeadPart.Flags[0] = 0x0;
    Pro_HeadPartP0Cmd->Pro_HeadPart.Flags[1] = 0x0;
}
/*******************************************************************************
* Function Name  : Pro_GetFrame
* Description    :
* Input          : None
* Output         : New package
* Return         : 0 : Find new package ; 1 : Error
* Attention		 : Data adhering
*******************************************************************************/
static uint8_t Pro_GetFrame()
{
    if(rb_can_read(&u_ring_buff) >= 1)
    {
        if(packageFlag ==0)
        {
            rb_read(&u_ring_buff, &curValue, 1);
            if((lastValue == 0xFF)&&(curValue == 0xFF))
            {
                tmp_buf[0] = 0xFF;
                tmp_buf[1] = 0xFF;
                count = 2;
                return 1;
            }
            if((lastValue == 0xFF)&&(curValue == 0x55))
            {
                lastValue = curValue;
                return 1;
            }
            tmp_buf[count] = curValue;
            count ++ ;
            lastValue = curValue;

            if(count ==4)
            {
                dataLen = tmp_buf[2]*256+  tmp_buf[3];
            }
            if(count ==  (dataLen + 4))
            {
                memcpy(UART_HandleStruct.Message_Buf, tmp_buf, dataLen + 4);
                UART_HandleStruct.Message_Len = dataLen + 4;

#ifdef PROTOCOL_DEBUG
                printf("GAgentToMCU: ");
                for(uint8_t i=0; i<dataLen+4; i++)
                {
                    printf("%02X ", UART_HandleStruct.Message_Buf[i]);
                }
                printf("\r\n");
#endif
                memset(tmp_buf, 0, (dataLen + 4));
                packageFlag = 1;
                lastValue = curValue =0;
                return 0;
            }
        }
    }
	return 1;
}

/*******************************************************************************
* Function Name  : MessageHandle
* Description    :
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/
u8 GizWits_MessageHandle(u8 * Message_Buf, u8 Length_buf)
{
    uint8_t ret = 0;
    Pro_HeadPartTypeDef *Recv_HeadPart = NULL;
    /*抓取一包*/
    Pro_GetFrame();
    memset(&Recv_HeadPart, 0, sizeof(Recv_HeadPart));
    /*重发机制*/
    GizWits_D2W_Resend_AckCmdHandle();
    if(packageFlag)
    {
        /*验证校验码*/
		if(CheckSum(UART_HandleStruct.Message_Buf, UART_HandleStruct.Message_Len) != UART_HandleStruct.Message_Buf[UART_HandleStruct.Message_Len - 1]) 
		{
			Pro_W2D_ErrorCmdHandle(Error_AckSum);
			packageFlag = 0;
			return 1;
		}
        /*判断ACK*/
		ret = GizWits_W2D_AckCmdHandle();
#ifdef DEBUG
       if(ret == 0)
       {
            printf("ACK: SUCCESS! ...\r\n");
       }
#endif
        Recv_HeadPart = (Pro_HeadPartTypeDef *)UART_HandleStruct.Message_Buf;
        switch (Recv_HeadPart->Cmd)
        {
            case Pro_W2D_UpdataReady_Ack_Cmd:
                break;
			case Pro_W2D_GetDeviceInfo_Cmd:
				Pro_W2D_GetMcuInfo();
				break;
			case Pro_W2D_P0_Cmd:
			{
				switch(UART_HandleStruct.Message_Buf[sizeof(Pro_HeadPartTypeDef)])
				{
					case P0_W2D_Control_Devce_Action:
					{
						Pro_W2D_CommonCmdHandle();
						memcpy(Message_Buf, UART_HandleStruct.Message_Buf+sizeof(Pro_HeadPartP0CmdTypeDef), Length_buf);
						p0Flag = 1;
						break;
					}
					case P0_W2D_ReadDevStatus_Action:
						Pro_W2D_ReadDevStatusHandle();
						break;
					default:
						break;
				}
			}
			break;
			case Pro_W2D_P0_Ack_Cmd:
				break;
			case Pro_W2D_Heartbeat_Cmd:
				Pro_W2D_CommonCmdHandle();
				break;
			case Pro_W2D_ControlWifi_Config_Ack_Cmd:
				break;
			case Pro_W2D_ResetWifi_Ack_Cmd:
				break;
			case Pro_W2D_ReportWifiStatus_Cmd:
				Pro_W2D_WifiStatusHandle();
				break;
			case Pro_W2D_ReportMCUReset_Cmd:
				Pr0_W2D_RequestResetDeviceHandle();
				break;
			case Pro_W2D_ErrorPackage_Cmd:
				//Pro_W2D_ErrorCmdHandle(Error_AckSum);//bug
				break;
            
            case Pro_W2D_Update_Cmd:
				Pro_W2D_UpdateCmdHandle();
				break;
			case Pro_W2D_Update_Data:
				Pro_W2D_UpdateDataHandle();
				break;
			case Pro_W2D_UpdateSuspend_Cmd:
				Pro_W2D_UpdateSuspendHandle();
				break;
            
			default:
				Pro_W2D_ErrorCmdHandle(Error_Cmd);
				break;
        }
        memset(&UART_HandleStruct.Message_Buf, 0, UART_HandleStruct.Message_Len);
        packageFlag = 0;
    }
    return 1;
}

/*******************************************************************************
* Function Name  : Pro_GetMcuInfo
* Description    : WiFi模组请求设备信息
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/
static void Pro_W2D_GetMcuInfo(void)
{
    memcpy(&Pro_CommonCmdStruct, UART_HandleStruct.Message_Buf, sizeof(Pro_CommonCmdStruct));

    Pro_M2W_ReturnInfoStruct.Pro_HeadPart.SN = Pro_CommonCmdStruct.Pro_HeadPart.SN;
    Pro_M2W_ReturnInfoStruct.Sum = CheckSum((uint8_t *)&Pro_M2W_ReturnInfoStruct, sizeof(Pro_M2W_ReturnInfoStruct));
    Pro_UART_SendBuf((uint8_t *)&Pro_M2W_ReturnInfoStruct,sizeof(Pro_M2W_ReturnInfoStruct), 0);

#ifdef PROTOCOL_DEBUG
    printf("[%d] MCU   : ", SystemTimeCount);
    for(uint16_t i=0; i<sizeof(Pro_M2W_ReturnInfoStruct); i++)
    {
        printf("%2x ", *((uint8_t *)&Pro_M2W_ReturnInfoStruct + i));
    }
    printf("\r\n");
#endif
}

static void Pro_W2D_ReadDevStatusHandle(void)
{
    Pro_D2W_ReportDevStatusHandle();
}


/*******************************************************************************
* Function Name  : Pro_Pro_W2D_Heartbeat
* Description    : 
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/
static void Pro_W2D_CommonCmdHandle(void)
{
    memcpy(&Pro_CommonCmdStruct, UART_HandleStruct.Message_Buf, sizeof(Pro_CommonCmdStruct));
    Pro_CommonCmdStruct.Pro_HeadPart.Len = exchangeBytes(sizeof(Pro_CommonCmdTypeDef) - 4);
    Pro_CommonCmdStruct.Pro_HeadPart.Cmd = Pro_CommonCmdStruct.Pro_HeadPart.Cmd + 1;
    Pro_CommonCmdStruct.Sum = CheckSum((uint8_t *)&Pro_CommonCmdStruct, sizeof(Pro_CommonCmdStruct));
    Pro_UART_SendBuf((uint8_t *)&Pro_CommonCmdStruct, sizeof(Pro_CommonCmdStruct), 0);

#ifdef PROTOCOL_DEBUG
    printf("[%d] MCU   : ", SystemTimeCount);
    for(uint16_t i=0; i<sizeof(Pro_CommonCmdStruct); i++)
    {
        printf("%2x ", *((uint8_t *)&Pro_CommonCmdStruct + i));
    }
    printf("\r\n");
#endif

    memset(&Pro_CommonCmdStruct, 0, sizeof(Pro_CommonCmdStruct));
}

/*******************************************************************************
* Function Name  : Pro_W2D_WifiStatusHandle
* Description    : 将WiFi的状态保存到 Pro_W2D_WifiStatusStruct中。并回复ACK
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void (*callBackFunc)(uint16_t);
static void Pro_W2D_WifiStatusHandle(void)
{
    memcpy(&Pro_W2D_WifiStatusStruct, UART_HandleStruct.Message_Buf, sizeof(Pro_W2D_WifiStatusStruct));
    Pro_W2D_CommonCmdHandle();
    callBackFunc  = GizWits_WiFiStatueHandle;
    (*callBackFunc)(exchangeBytes(Pro_W2D_WifiStatusStruct.Wifi_Status));

}


/*******************************************************************************
* Function Name  : Pr0_W2D_RequestResetDeviceHandle
* Description    : WiFi模组请求复位设备MCU，MCU回复ACK，并执行设备复位
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/
void Pr0_W2D_RequestResetDeviceHandle(void)
{
    Pro_W2D_CommonCmdHandle();
	uint32_t timeDelay = SystemTimeCount;
	/*Wait 600ms*/
	while((SystemTimeCount - timeDelay) <= 600);
    printf("W2D_RequestResetDevice...\r\n");
    printf("MCU Restart...\r\n");

    /****************************MCU RESTART****************************/
    __set_FAULTMASK(1);
    NVIC_SystemReset();
    /******************************************************************************/
}


/*******************************************************************************
* Function Name  : Pro_W2D_ErrorCmdHandle
* Description    : WiFi发送收到非法信息通知，设备MCU回复ACK，并执行相应的动作
* Input          : None
* Output         : None
* Return         : None
* Attention		   : None
*******************************************************************************/
static void Pro_W2D_ErrorCmdHandle(Error_PacketsTypeDef errorType)
{
    uint8_t i;
    Pro_ErrorCmdTypeDef           	 Pro_ErrorCmdStruct;
    memcpy(&Pro_ErrorCmdStruct, UART_HandleStruct.Message_Buf, sizeof(Pro_ErrorCmdStruct));

    Pro_ErrorCmdStruct.Pro_HeadPart.Len = exchangeBytes(sizeof(Pro_ErrorCmdStruct) - 4);
    Pro_ErrorCmdStruct.Pro_HeadPart.Cmd = Pro_D2W_ErrorPackage_Ack_Cmd;
    Pro_ErrorCmdStruct.Error_Packets = errorType;
    Pro_ErrorCmdStruct.Sum = CheckSum((uint8_t *)&Pro_ErrorCmdStruct, sizeof(Pro_ErrorCmdStruct));
    Pro_UART_SendBuf((uint8_t *)&Pro_ErrorCmdStruct, sizeof(Pro_ErrorCmdStruct), 0);

#ifdef PROTOCOL_DEBUG
    printf(" MCU  : ");
    for(i = 0; i < sizeof(Pro_ErrorCmdStruct); i++)
    {
        printf("%2x ", *((uint8_t *)&Pro_ErrorCmdStruct + i));
    }
    printf("\r\n");
#endif

    /*************************Gagent to MCU ,Error*****************************/
    switch (Pro_ErrorCmdStruct.Error_Packets)
    {
		case Error_AckSum:
			printf("W2D Error Command ->Error_AckSum\r\n");
			break;
		case Error_Cmd:
			printf("W2D Error Command ->Error_Cmd\r\n");
			break;
		case Error_Other:
			printf("W2D Error Command ->Error_Other\r\n");
			break;
		default:
			break;
    }
    /******************************************************************************/
}

static void Pro_D2W_ReportDevStatusHandle(void)
{
    Pro_HeadPartP0CmdTypeDef *Pro_HeadPartP0Cmd = (Pro_HeadPartP0CmdTypeDef *)g_DevStatus;
    memcpy(Pro_HeadPartP0Cmd, UART_HandleStruct.Message_Buf, sizeof(Pro_HeadPartTypeDef));
    Pro_HeadPartP0Cmd->Pro_HeadPart.Cmd = Pro_D2W_P0_Ack_Cmd;
    Pro_HeadPartP0Cmd->Pro_HeadPart.Len = exchangeBytes((sizeof(Pro_HeadPartP0CmdTypeDef)+g_P0DataLen+1) - 4);
    Pro_HeadPartP0Cmd->Action = P0_D2W_ReadDevStatus_Action_ACK;
    g_DevStatus[sizeof(Pro_HeadPartP0CmdTypeDef)+g_P0DataLen] = CheckSum(g_DevStatus, sizeof(Pro_HeadPartP0CmdTypeDef)+g_P0DataLen+1);
    Pro_UART_SendBuf((uint8_t *)Pro_HeadPartP0Cmd, sizeof(Pro_HeadPartP0CmdTypeDef)+g_P0DataLen+1, 0);

#ifdef PROTOCOL_DEBUG
    printf("[%d] MCU   : ", SystemTimeCount);
    for(uint16_t i=0; i<(sizeof(Pro_HeadPartP0CmdTypeDef)+g_P0DataLen+1); i++)
    {
        printf("%2x ", *((uint8_t *)Pro_HeadPartP0Cmd + i));
    }
    printf("\r\n");
#endif

}

void GizWits_D2WResetCmd(void)
{
    Pro_CommonCmdTypeDef Pro_D2WReset;

    Pro_D2WReset.Pro_HeadPart.Head[0] = 0xFF;
    Pro_D2WReset.Pro_HeadPart.Head[1] = 0xFF;
    Pro_D2WReset.Pro_HeadPart.Len = exchangeBytes(sizeof(Pro_CommonCmdTypeDef) - 4);
    Pro_D2WReset.Pro_HeadPart.Cmd = Pro_D2W_ResetWifi_Cmd;
    Pro_D2WReset.Pro_HeadPart.SN = SN++;
    Pro_D2WReset.Pro_HeadPart.Flags[0] = 0x00;
    Pro_D2WReset.Pro_HeadPart.Flags[1] = 0x00;
    Pro_D2WReset.Sum = CheckSum((uint8_t *)&Pro_D2WReset, sizeof(Pro_CommonCmdTypeDef));
    Pro_UART_SendBuf((uint8_t *)&Pro_D2WReset, sizeof(Pro_CommonCmdTypeDef), 1);

#ifdef PROTOCOL_DEBUG
    printf("[%d] MCU   : ", SystemTimeCount);
    for(uint16_t i=0; i<sizeof(Pro_CommonCmdTypeDef); i++)
    {
        printf("%2x ", *((uint8_t *)&Pro_D2WReset + i));
    }
    printf("\r\n");
#endif
}

void GizWits_D2WConfigCmd(uint8_t WiFi_Mode)
{
    Pro_D2W_ConfigWifiTypeDef Pro_D2WConfigWiFiMode;

    Pro_D2WConfigWiFiMode.Pro_HeadPart.Head[0] = 0xFF;
    Pro_D2WConfigWiFiMode.Pro_HeadPart.Head[1] = 0xFF;
    Pro_D2WConfigWiFiMode.Pro_HeadPart.Len = exchangeBytes(sizeof(Pro_D2W_ConfigWifiTypeDef) - 4);
    Pro_D2WConfigWiFiMode.Pro_HeadPart.Cmd = Pro_D2W_ControlWifi_Config_Cmd;
    Pro_D2WConfigWiFiMode.Pro_HeadPart.SN = SN++;
    Pro_D2WConfigWiFiMode.Pro_HeadPart.Flags[0] = 0x00;
    Pro_D2WConfigWiFiMode.Pro_HeadPart.Flags[1] = 0x00;
    Pro_D2WConfigWiFiMode.Config_Method = WiFi_Mode;
    Pro_D2WConfigWiFiMode.Sum = CheckSum((uint8_t *)&Pro_D2WConfigWiFiMode, sizeof(Pro_D2W_ConfigWifiTypeDef));
    Pro_UART_SendBuf((uint8_t *)&Pro_D2WConfigWiFiMode, sizeof(Pro_D2W_ConfigWifiTypeDef), 1);

#ifdef PROTOCOL_DEBUG
    printf("[%d] MCU   : ", SystemTimeCount);
    for(uint16_t i=0; i<sizeof(Pro_D2W_ConfigWifiTypeDef); i++)
    {
        printf("%2x ", *((uint8_t *)&Pro_D2WConfigWiFiMode + i));
    }
    printf("\r\n");
#endif
}

void GizWits_DevStatusUpgrade(uint8_t *P0_Buff, uint32_t Time, uint8_t flag)
{
    uint8_t Report_Flag = 0;
    Pro_HeadPartP0CmdTypeDef *Pro_HeadPartP0Cmd = (Pro_HeadPartP0CmdTypeDef *)g_DevStatus;
    if(Wait_AckStruct.Flag == 1)
    {
        return;
    }
    if(flag == 1)
    {
        Report_Flag = 1;
        goto Report;
    }
    if((2 * 1000) < (SystemTimeCount - Last_ReportTime))
    {
        if(memcmp(g_DevStatus + sizeof(Pro_HeadPartP0CmdTypeDef), P0_Buff, g_P0DataLen) != 0)
        {
            Report_Flag = 1;

        }
    }
    if(Time < (SystemTimeCount-Last_Report_10_Time))
    {
        Report_Flag = 1;
        Last_Report_10_Time = SystemTimeCount;
    }

Report:
    if(Report_Flag == 1)
    {
        memcpy(g_DevStatus + sizeof(Pro_HeadPartP0CmdTypeDef), P0_Buff, g_P0DataLen);

        Pro_HeadPartP0Cmd->Pro_HeadPart.Len = exchangeBytes(sizeof(Pro_HeadPartP0CmdTypeDef) + 1 + g_P0DataLen - 4);
        Pro_HeadPartP0Cmd->Pro_HeadPart.Cmd = Pro_D2W_P0_Cmd;
        Pro_HeadPartP0Cmd->Pro_HeadPart.SN = SN++;
        Pro_HeadPartP0Cmd->Action = P0_D2W_ReportDevStatus_Action;
        g_DevStatus[sizeof(Pro_HeadPartP0CmdTypeDef) + g_P0DataLen] = CheckSum(g_DevStatus, sizeof(Pro_HeadPartP0CmdTypeDef) + g_P0DataLen + 1);

        Pro_UART_SendBuf((uint8_t *)Pro_HeadPartP0Cmd, sizeof(Pro_HeadPartP0CmdTypeDef) + g_P0DataLen + 1, 1);//×¢£º×îºóÒ»ÏîÎªÖ÷¶¯ÉÏ±¨Ê±ÖÃÎª1 ÊÊÓÃÓÚÐ­Òé4.3¡¢4.4¡¢4.9

        Last_ReportTime = SystemTimeCount;
        
#ifdef DEBUG
        printf("Report Sensor Data \r\n");
#endif        

#ifdef PROTOCOL_DEBUG
        printf("[%d] MCU   : ", SystemTimeCount);
        for(uint16_t i = 0; i < (sizeof(Pro_HeadPartP0CmdTypeDef) + g_P0DataLen + 1); i++)
        {
            printf("%2x ", g_DevStatus[i]);
        }
        printf("\r\n");
#endif
    }
    return;
}

/*******************************************************************************
* Function Name  : Pro_W2D_UpdateCmdHandle
* Description    : ʽܶļ®Ԧm
* Input          : buf:˽ߝǰʼַ֘û packLen:˽ߝӤ׈û
* Output         : None
* Return         : Уҩë
* Attention		   : None
*******************************************************************************/
uint32_t updateFileSize = 0;	//ʽܶτݾճС
#define FILEMD5MAXLEN  32			//خճMD5˽ߝӤ׈
uint8_t fileMD5value[FILEMD5MAXLEN];
uint16_t fileMD5len = 0;			//ʵ݊MD5˽ߝӤ׈

uint8_t  offset = 0;					//ƫӆ
#define UARTTXMAXLEN 256			//Ԯࠚע̍ۺԥȸخճӤ׈
uint8_t uartTxBuf[UARTTXMAXLEN]={0};//Ԯࠚע̍ۺԥȸ

/*******************************************************************************
* Function Name  : Pro_W2D_UpdateCmdHandle
* Description    : ֪ͨʽܶļ®Ԧm
* Input          : ׵ܘACK,Ɛ׏ʽܶτݾճС,MCU׼Ѹߍѷע̍Readyхۅ
* Output         : None
* Return         : Уҩë
* Attention		   : None
*******************************************************************************/
void Pro_W2D_UpdateCmdHandle()
{	
	uint8_t i;
	uint16_t uartSendLen = 0;
	offset = 8;
	Pro_W2D_CommonCmdHandle();//ack	
	
	updateFileSize = ((uint32_t)(UART_HandleStruct.Message_Buf[offset]<<24))|((uint32_t)(UART_HandleStruct.Message_Buf[offset+1]<<16))|((uint32_t)(UART_HandleStruct.Message_Buf[offset+2]<<8))|((uint32_t)(UART_HandleStruct.Message_Buf[offset+3]));
	//judge flash size
	printf("updateFileSize = %d\r\n" , updateFileSize);
    if(updateFileSize > BIG_DATA_SAVE_MAX_SIZE)
    {
        /*超过允许最大的FLASH空间*/
        printf("UpdateFileSize Error \r\n");
        /* 取消升级 */
        printf("Update Failed \r\n");
        Pro_W2D_UpdateSuspendHandle();
        return ;
    }
    else
    {
        /* 擦除APP区域 , 此处应该加错误处理,升级过程出现错误，App能返回出厂代码 */
        EraseFlash(BIG_DATA_SAVE_MAX_SIZE);
    }
	fileMD5len = UART_HandleStruct.Message_Buf[offset+4]*256 + UART_HandleStruct.Message_Buf[offset+5];
	for (i=0;i<fileMD5len;i++)
	{
		fileMD5value[i] = UART_HandleStruct.Message_Buf[offset+6+i];
	}
	memset(uartTxBuf, 0, UARTTXMAXLEN);
	uartSendLen = Pro_Make_Frame(uartTxBuf);
	Pro_UART_SendBuf(uartTxBuf,uartSendLen,0);//ready
	memset(uartTxBuf, 0, UARTTXMAXLEN);
}
/*******************************************************************************
* Function Name  : Pro_W2D_UpdateDataHandle
* Description    : ʽܶ˽ߝԦm
* Input          : 
* Output         : None
* Return         : 
* Attention		   : Ɛ׏ؖѼѲۅ,ɧڻˇخ۳һѼ,дɫFlash۳ע̍ԉ٦хۅ
									ݫѨτޢ϶Զ˽ߝ,дFlash,Ȼ۳ע̍ack
*******************************************************************************/
void Pro_W2D_UpdateDataHandle()
{
	uint16_t piecenum = 0;
	uint16_t piececount = 0;
	uint16_t dataLen =0;
	uint32_t tempWFlashAddr = 0;
		
	
	dataLen = UART_HandleStruct.Message_Buf[2]*256 + UART_HandleStruct.Message_Buf[3];
	memcpy(&Pro_UpdataDataStruct, UART_HandleStruct.Message_Buf, dataLen+3);
	
	piecenum = Pro_UpdataDataStruct.piecenum = exchangeBytes(Pro_UpdataDataStruct.piecenum);///ؖѼѲۅ
	piececount = Pro_UpdataDataStruct.piececount = exchangeBytes(Pro_UpdataDataStruct.piececount);//ؖѼ؜˽
	printf("******piecenum = %d , piececount = %d********\r\n",piecenum,piececount);
	//Save updates to memory space
	
	/*将图像分片数据写入Flash*/
   tempWFlashAddr = SYS_APP_SAVE_ADDR_BASE + (piecenum-1) * PIECEMAXLEN;
   wFlashData((uint8_t *)&Pro_UpdataDataStruct.piececontent , dataLen - 9, tempWFlashAddr);
		/*updata package data ,ack*/
	Pro_W2D_CommonCmdHandle();//ack
	if(piecenum == piececount)
	{
		LED_ON(LED4);
		printf("Update Success \r\n");
		printf("System Restart... \r\n");
		/****************************MCU RESTART****************************/
    __set_FAULTMASK(1);
    NVIC_SystemReset();
    /******************************************************************************/
		//last package , updata ok
		//MD5 checkout :Failed clear updata,Success , write flash ,begin updata
	}
}

/*******************************************************************************
* Function Name  : Pro_W2D_UpdateSuspendHandle
* Description    : ʽܶאֹļ®,׵ܘACK
* Input          : 
* Output         : None
* Return         : 
* Attention		   : ޓ˕ʽܶאֹļ®,ȥԽӑޭӁдքFlashࠕݤ
*******************************************************************************/
void Pro_W2D_UpdateSuspendHandle()
{
	LED_ON(LED1);
	memcpy(&Pro_CommonCmdStruct, UART_HandleStruct.Message_Buf, sizeof(Pro_CommonCmdStruct));
	Pro_CommonCmdStruct.Pro_HeadPart.Len = 0x0500;	//20150609
	Pro_CommonCmdStruct.Pro_HeadPart.Cmd = Pro_CommonCmdStruct.Pro_HeadPart.Cmd + 1;
	Pro_CommonCmdStruct.Sum = CheckSum((uint8_t *)&Pro_CommonCmdStruct, sizeof(Pro_CommonCmdStruct));	
	Pro_UART_SendBuf((uint8_t *)&Pro_CommonCmdStruct,sizeof(Pro_CommonCmdStruct), 0);		
	memset(&Pro_CommonCmdStruct, 0, sizeof(Pro_CommonCmdStruct));	
	//ޓ˕אֹхۅ۳քԦm
}


/*******************************************************************************
* Function Name  : Pro_Make_Frame
* Description    : ةѼ
* Input          : 
* Output         : None
* Return         : 
* Attention		   : Ready хϢѨτة֡
*******************************************************************************/

uint16_t Pro_Make_Frame(uint8_t* tx_buf)
{
    uint8_t *pTxBuf = tx_buf;
    uint8_t i;
    uint16_t data_len=sizeof(Pro_CommonCmdStruct) + 2 + fileMD5len + 2 -4 ;
    memcpy(&Pro_CommonCmdStruct, UART_HandleStruct.Message_Buf, sizeof(Pro_CommonCmdStruct));
    *pTxBuf ++= 0xFF;
    *pTxBuf ++= 0xFF;
    *pTxBuf ++= (uint8_t)(data_len>>8);//len
    *pTxBuf ++= (uint8_t)(data_len);
    *pTxBuf ++= Pro_D2W_UpdataReady_Cmd;//0x1b cmd
    *pTxBuf ++= Pro_CommonCmdStruct.Pro_HeadPart.SN;//sn
    *pTxBuf ++= 0x00;//flag
    *pTxBuf ++= 0x00;//flag
    *pTxBuf ++= (uint8_t)(fileMD5len>>8);//len
    *pTxBuf ++= (uint8_t)(fileMD5len);
    for (i=0; i<fileMD5len; i++)
    {
        *pTxBuf ++= fileMD5value[i];
    }
    *pTxBuf ++= (uint8_t)(PIECEMAXLEN>>8);//len
    *pTxBuf ++= (uint8_t)(PIECEMAXLEN);
    *pTxBuf ++= CheckSum( tx_buf , (data_len+4));
    printf("MCU Ready \r\n");
    return (data_len+4);
}
