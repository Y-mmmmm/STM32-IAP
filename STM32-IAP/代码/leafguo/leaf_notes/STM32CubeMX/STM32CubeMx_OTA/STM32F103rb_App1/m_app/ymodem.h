#ifndef __YMODEM_H
#define __YMODEM_H
#ifdef __cplusplus
extern "C" {
#endif



/*=====�û�����(�����Լ��ķ�����������)=====*/
#define BootLoader_Size 		0x5000U			///< BootLoader�Ĵ�С 20K
#define Application_Size		0xA000U			///< Ӧ�ó���Ĵ�С 40K

#define Application_1_Addr		0x08005000U		///< Ӧ�ó���1���׵�ַ
#define Application_2_Addr		0x0800F000U		///< Ӧ�ó���2���׵�ַ
/*==========================================*/



#define SOH		0x01
#define STX		0x02
#define ACK		0x06
#define NACK	0x15
#define EOT		0x04
#define CCC		0x43



/* �����Ĳ��� */
enum UPDATE_STATE
{
	TO_START = 0x01,
	TO_RECEIVE_DATA = 0x02,
	TO_RECEIVE_EOT1 = 0x03,
	TO_RECEIVE_EOT2 = 0x04,
	TO_RECEIVE_END = 0x05
};



void ymodem_fun(void);



#ifdef __cplusplus
}
#endif

#endif /* __YMODEM_H */
