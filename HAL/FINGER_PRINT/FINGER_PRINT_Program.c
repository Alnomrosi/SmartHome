/*
 * FINGER_PRINT_Program.c
 *
 *  Created on: Mar 15, 2023
 *      Author: Dell
 */

#include"STD_TYPES.h"
#include"PORT/Port_int.h"
#include"USART_CONFIG.h"
#include"USART/USART_Init.h"
#include"GIE_init.h"
#include"FINGER_PRINT_Private.h"
#include"FINGER_PRINT_CONFIG.h"
#include"FINGER_PRINT_Types.h"
#include"FINGER_PRINT_Init.h"
#include"CLCD/CLCD_Init.h"
#include<util/delay.h>
void Display_data(void);
extern const Port_ConfigType PORT_Config_Arr[PORT_PINS_CONFIG_ARR_SIZE];
UART_CONFIG ARR_UART_CFG_3={Baud_16MHz_57600,Word_Size_8,Disabled,One_Stop_bit};
u8 Store[]={0xEF,0x01,0xFF,0xFF,0xFF,0xFF,0x01,0x00,0x06,0x06,0x01,0x00,0x01,0x00,0x0F};
static u8 Search_Buffer[16];
static u8 Buffer[30];
static u8 Local_counter=0;

static u8 Enrollement_flag=0;
static u8 Match_flag=0;
static u8 Gen_Img_Flag=0;
static u8 Gen_Img_Flag_Match=0;
static u8 Img_2_TZ_Flag=0;
static u8 Img_2_TZ_Flag_Match=0;
static u8 Reg_Model_Flag=0;
static u8 Store_Flag=0;
static u8 Search_Flag=0;
/*
u8 Enrollement_flag=0;
u8 Match_flag=0;
u8 Gen_Img_Flag=0;
u8 Gen_Img_Flag_Match=0;
u8 Img_2_TZ_Flag=0;
u8 Img_2_TZ_Flag_Match=0;
u8 Reg_Model_Flag=0;
u8 Store_Flag=0;
u8 Search_Flag=0;
*/
static u8 Empty_Flag=0;
static u8 Delete_Flag=0;
void Finger_Print_Init(void)
{
	Port_Init(PORT_Config_Arr);
	H_LCD_void_Init();
	UART_Init(&ARR_UART_CFG_3);
	UART_SetCallBack(Display_data);
	GIE_voidEnable();
}

void Finger_Print_Send_Command(u8 *Ptr_Package,u8 Package_Length )
{
	u8 counter=0;
	do
	{
		UART_SendByte(Ptr_Package[counter]);
		counter++;
		_delay_ms(500);
	}while(counter<Package_Length);
	_delay_ms(1000);
}
Error_States_FB Finger_Print_Start(u8 Process_type,u8 FB_ID)
{
	Error_States_FB Error_status;

	u8 Gen_Image_Counter=0;
	u8 IMG_2TZ_Counter=0;
	if(Process_type==FB_Enrollement)
	{
		H_LCD_void_ClearDisplay();
		H_LCD_void_sendString("Enroll New User");
		_delay_ms(3000);
		Store_Flag=0;
		Enrollement_flag=1;
		while(Enrollement_flag)
		{
			Gen_Img_Flag=1;


			while(Gen_Img_Flag)
			{
				H_LCD_void_ClearDisplay();
				H_LCD_void_sendString("Check Finger!");
				_delay_ms(2000);
				Finger_Print_Send_Command(GetImage,GEN_IMAGE_LENGTH);
				if(Buffer[Confirmation_Code]==Finger_Collection_Success)
				{
					if(Gen_Image_Counter==0)
					{
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Check 1 done");
						Gen_Image_Counter++;
						Flush_Buffer();
						Img_2_TZ_Flag=1;
						//_delay_ms(1000);
						Gen_Img_Flag=0;
						break;
					}
					else if(Gen_Image_Counter==1)
					{
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Check 2 done");
						Flush_Buffer();
						Img_2_TZ_Flag=1;
						//_delay_ms(1000);
						Gen_Img_Flag=0;
						Gen_Image_Counter=0;
						break;
					}
					/*
					Flush_Buffer();
					Img_2_TZ_Flag=1;
					//_delay_ms(1000);
					Gen_Img_Flag=0;
					break;
					*/
				}
				else if(Buffer[Confirmation_Code]==Error_When_Receiving_Package_Gen)
				{
					Flush_Buffer();
				}
				else if(Buffer[Confirmation_Code]==Can_not_detect_finger)
				{
					H_LCD_void_ClearDisplay();
					H_LCD_void_sendString("Check Finger again");
					Flush_Buffer();
				}
				else
				{
					Flush_Buffer();
				}
			}


			while(Img_2_TZ_Flag)
			{
				if(IMG_2TZ_Counter==0)
				{
					Finger_Print_Send_Command(Img2TZ_1,IMAGE_2TZ_1_LENGTH);
					if(Buffer[Confirmation_Code]==Generate_character_file_complete)
					{
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Buffer1 done");
						_delay_ms(1000);

						Flush_Buffer();
						Gen_Img_Flag=1;
						IMG_2TZ_Counter++;
						_delay_ms(500);
						Img_2_TZ_Flag=0;
						break;
					}
					else
					{
						Flush_Buffer();
						_delay_ms(250);
					}
			}
				else if(IMG_2TZ_Counter==1)
				{
					Finger_Print_Send_Command(Img2TZ_2,IMAGE_2TZ_2_LENGTH);
					if(Buffer[Confirmation_Code]==Generate_character_file_complete)
					{
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Buffer2 done");
						_delay_ms(1000);

						Flush_Buffer();
						Reg_Model_Flag=1;
						IMG_2TZ_Counter=0;
						_delay_ms(500);
						Img_2_TZ_Flag=0;
						break;
					}
					else
					{
						Flush_Buffer();
						//_delay_ms(1000);
					}
				}
		}


			while(Reg_Model_Flag)
			{
				Finger_Print_Send_Command(Reg_Model,REG_MODEL_LENGTH);
				if(Buffer[Confirmation_Code]==Operation_Sucess)
				{
					H_LCD_void_ClearDisplay();
					H_LCD_void_sendString("Register Model done");
					_delay_ms(1000);

					Flush_Buffer();
					Store_Flag=1;
					Reg_Model_Flag=0;
					break;
				}
				else
				{
					Flush_Buffer();
					//_delay_ms(1000);
				}
			}


			while(Store_Flag)
			{
				Check_Sum(FB_ID);
				//Store[PAGE_ID]=FB_ID;
				Finger_Print_Send_Command(Store,STORE_LENGTH);
				if(Buffer[Confirmation_Code]==Storage_Success)
				{
					H_LCD_void_ClearDisplay();
					H_LCD_void_sendString("Storing done");
					_delay_ms(1000);
					Flush_Buffer();
					Enrollement_flag=0;
					Store_Flag=0;
					break;
				}
				else
				{
					Flush_Buffer();
					//_delay_ms(1000);
				}
			}
		}

		Error_status=No_Error;
	}
	else if(Process_type==FB_Match)
	{
		Search_Flag=0;
		Match_flag=1;
		H_LCD_void_ClearDisplay();
		H_LCD_void_sendString("Check for match?");
		_delay_ms(3000);
		while(Match_flag)
		{
			Gen_Img_Flag_Match=1;

			while(Gen_Img_Flag_Match)
			{
				H_LCD_void_ClearDisplay();
				H_LCD_void_sendString("Check Finger!");
				_delay_ms(2000);
				Finger_Print_Send_Command(GetImage,GEN_IMAGE_LENGTH);
				if(Buffer[Confirmation_Code]==Finger_Collection_Success)
				{

					H_LCD_void_ClearDisplay();
					H_LCD_void_sendString("Check  done");

					Flush_Buffer();
					Img_2_TZ_Flag_Match=1;
					_delay_ms(1000);
					Gen_Img_Flag_Match=0;
					break;
				}
				else if(Buffer[Confirmation_Code]==Error_When_Receiving_Package_Gen)
				{
					Flush_Buffer();
				}
				else if(Buffer[Confirmation_Code]==Can_not_detect_finger)
				{
					H_LCD_void_ClearDisplay();
					H_LCD_void_sendString("Check Finger again");
					Flush_Buffer();
				}
				else
				{
					Flush_Buffer();
				}
			}


			while(Img_2_TZ_Flag_Match)
			{

				Finger_Print_Send_Command(Img2TZ_1,IMAGE_2TZ_1_LENGTH);
				if(Buffer[Confirmation_Code]==Generate_character_file_complete)
				{
					//H_LCD_void_ClearDisplay();
					//H_LCD_void_sendString("Check 1 done");
					Flush_Buffer();
					Search_Flag=1;
					H_LCD_void_ClearDisplay();
					H_LCD_void_sendString("Check done");
					H_LCD_void_gotoXY(2,0);
					H_LCD_void_sendString("Searching");
					_delay_ms(1000);
					Img_2_TZ_Flag_Match=0;
					break;
				}
				else
				{
					Flush_Buffer();
					_delay_ms(1000);
				}

			}
			while(Search_Flag)
			{
				Finger_Print_Send_Command(Search,SEARCH_LENGTH);
				if(Buffer[Confirmation_Code]==Found_matching_finger)
				{
					for(u8 i=0;i<16;i++)
					{
						Search_Buffer[i]=Buffer[i];
					}
					H_LCD_void_ClearDisplay();
					H_LCD_void_sendString("FingerPrint Found");
					_delay_ms(2000);
					H_LCD_void_ClearDisplay();

					H_LCD_void_sendIntNum(Search_Buffer[11]);
					_delay_ms(5000);
					if(Search_Buffer[11]==Nour_Template)
					{
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Nour in Da House ^_^");
						Flush_Buffer();
						_delay_ms(2000);
					}
					else if(Search_Buffer[11]==Zedan_Template)
					{
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Zedan in Da House ^_^");
						Flush_Buffer();
					}
					else if(Search_Buffer[11]==Abo_7med_Template)
					{
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Abo 7meed in Da House ^_^");
						Flush_Buffer();
					}
					else if(Search_Buffer[11]==Nomrosi_Template)
					{
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Nomrosi in Da House ^_^");
						Flush_Buffer();
					}
					else
					{
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("New User");
						Flush_Buffer();
					}
					/*
					switch(Buffer[PAGE_ID])
					{
					case Nour_Template:
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Nour in Da House ^_^");
						Flush_Buffer();
						_delay_ms(2000);
						break;
					case Zedan_Template:
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Zedan in Da House ^_^");
						Flush_Buffer();

						_delay_ms(2000);
						break;
					case Nomrosi_Template:
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Nomrosi in Da House ^_^");
						Flush_Buffer();

						_delay_ms(2000);
						break;
					case Abo_7med_Template:
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Abo 7med in Da House ^_^");
						Flush_Buffer();

						_delay_ms(2000);
						break;
					case Alaa_Template:
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("Alaa in Da House ^_^");
						Flush_Buffer();

						_delay_ms(2000);
						break;
					default:
						H_LCD_void_ClearDisplay();
						H_LCD_void_sendString("New User");
						Flush_Buffer();
						break;
					}
					*/

					//Flush_Buffer();
					_delay_ms(1000);
					Match_flag=0;
					Search_Flag=0;
					break;
				}
				else if(Buffer[Confirmation_Code]==No_matching_in_lib)
				{
					H_LCD_void_ClearDisplay();
					H_LCD_void_sendString("No matching Found");
					Flush_Buffer();
					_delay_ms(1000);
					Gen_Img_Flag_Match=1;
					Search_Flag=0;
					break;
				}
			}


		}
		Error_status=No_Error;
	}
	return Error_status;
}


void Finger_Print_Delete_Template(u8 FB_ID)
{
	Delete_Flag=1;
	while(Delete_Flag)
	{
		Finger_Print_Send_Command(Delete_Char,DELETE_CHAR_LENGTH);
		if(Buffer[Confirmation_Code]==Delete_Success)
		{
			H_LCD_void_ClearDisplay();
			H_LCD_void_sendString("Template Deleted");
			Flush_Buffer();
			_delay_ms(1000);
			Delete_Flag=0;
		}
		else
		{
			Flush_Buffer();
			_delay_ms(500);
		}
	}
}
void Finger_Print_Empty_Templates(void)
{
	Empty_Flag=1;
	while(Empty_Flag)
	{
		Finger_Print_Send_Command(Empty_Library,EMPTY_LIBRARY_LENGTH);
		if(Buffer[Confirmation_Code]==Empty_success)
		{
			H_LCD_void_ClearDisplay();
			H_LCD_void_sendString("Template list empty");
			Flush_Buffer();
			_delay_ms(1000);
			Empty_Flag=0;
		}
		else
		{
			Flush_Buffer();
			_delay_ms(500);
		}
	}

}
void Flush_Buffer(void)
{
	Local_counter=0;
	for(u8 i=0;i<30;i++)
	{
		Buffer[i]=0xAA;
	}
}
void Check_Sum(u8 FB_ID)
{
	u8 Check_Sum_Value=0;
	u8 Counter=0;
	Store[PAGE_ID]=FB_ID;
	for(Counter=Package_Identifier;Counter<STORE_LENGTH-2;Counter++)
	{
		Check_Sum_Value+=Store[Counter];
	}
	Store[Check_Sum_byte]=Check_Sum_Value;
}

void Display_data(void)
{
	Buffer[Local_counter]=UART_RecieveByteASynchCallback();
	//H_LCD_void_sendIntNum(Buffer[Local_counter]);
	Local_counter++;
}
