/*
 * APP_prog.c
 *
 *  Created on: Mar 16, 2023
 *      Author: LENOVO
 */

#include <Avr/delay.h>
#include "STD_TYPES.h"
#include "APP_priv.h"
#include "APP_types.h"
#include "Port_cfg.h"
#include "Port_int.h"
#include "APP_init.h"
#include "CLCD_Init.h"
#include "KPD_Init.h"
#include "EEPROM_init.h"
#include "RTC_Init.h"
#include "Relay_Int.h"
#include "Buzzer_Int.h"
#include "TIMER0_int.h"
#include "GIE_init.h"

/***
 *	Global Variables
 */
const RTC_CONFIG_TIME TimeArr = {1,26,30} ;
extern const Port_ConfigType PORT_Config_Arr[PORT_PINS_CONFIG_ARR_SIZE];
const u8 Global_IDs[2] = {1,2};
u8 Global_PasswordCounter = 0;
u8 Global_Flag = 0 ;
void APP_INIT()
{

	/***
	 * 	Initialize section
	 ****/
	Port_Init(PORT_Config_Arr);
	H_LCD_void_Init();
	H_EEPROM_Void_Init();
	_delay_ms(200);
	H_RTC_Void_Init();
	_delay_ms(200);
	Finger_Print_Init();
	M_TIMER0_Void_Init(NORMAL_MODE, WG_NORMAL_PORT);
	M_TIMER0_setCallBack(OVRER_FLOW, Close_System);
	M_TIMER0_EnableInt(OVRER_FLOW);
}

/*
 *  Start of Algorithm
 */


void APP_Start()
{
 	u8 KeyPad_Value = 0 ;
	u8 User_Status = 0 ;
	u8 Temp_Holder = 0 ;
	/*
	 * Starting Display
	 */
	H_LCD_void_ClearDisplay();
	APP_WelcomeDisplay();
	H_LCD_void_gotoXY(2, 0);
	H_LCD_void_sendString("Chose Option:");
	H_LCD_void_gotoXY(3, 0);
	H_LCD_void_sendString("1-SignIn | 2-SingUp");
	H_LCD_void_gotoXY(4, 0);
	H_LCD_void_sendString("3-Rest All Pass");
	/*
	 * Chose option
	 */
	do{
		KeyPad_Value = H_KEYPAD_u8_getPressedKey();
	  }while(KeyPad_Value == NO_PRESSED_KEY);

	/* Check the options*/
		switch(KeyPad_Value)
		{
		case USER_SIGN_UP:	User_Status = 2 ;User_Authentication_SignUp(); break;
		case USER_SIGN_IN:	User_Status = User_Authentication_SignIn(); break;
		case USER_REST_ID:  User_Status = '3'; H_EEPROM_Void_WriteByte(EEPROM_USER_ID, 1); _delay_ms(200); break;
		default: User_Status = OUT_OF_RANGE;
		}

	/*
	 * in case of valid Login
	 */
		H_LCD_void_ClearDisplay();
	if (User_Status == VALID_LOGIN)
	{
		H_LCD_void_sendString("VALID_LOGIN");
		Successful_Authentication();
	}
	else if (User_Status == INVALID_LOGIN)
	{
		/* Loop to re-enter data */
		do
		{
		H_LCD_void_sendString("Invalid Login");
		H_LCD_void_gotoXY(2, 0);
		H_LCD_void_sendString("Re-enter ur data");
		_delay_ms(1000);
		H_LCD_void_ClearDisplay();
		Temp_Holder = Wrong_Authentication();
		if (Temp_Holder == Pass_Ok)
		{
			H_LCD_void_sendString("VALID_LOGIN");
			_delay_ms(2000);
			Successful_Authentication();
			break;
		}
		}while(Global_PasswordCounter != MAX_TRIALS);

		if(Global_PasswordCounter == MAX_TRIALS)
		{
			// System stop for 1 min
			if (Global_Flag == 0)
			{
			M_GIE_Void_Enable();
			H_LCD_void_sendString("Wait for 2 min");
			M_TIMER0_setDelayTimeMilliSec(10,NORMAL_MODE,CLK_1024);

			H_BUZZER_Void_TurnOn();

			do{


			}while(Global_Flag == 0);

			H_BUZZER_Void_TurnOff();

			H_LCD_void_ClearDisplay();
			M_GIE_Void_Disable();

			}
		}
		/*Clear Global Passowrd counter */
		Global_PasswordCounter = 0 ;
	}
	/* if user chose rest*/
	else if (User_Status == USER_REST_ID)
	{
		H_LCD_void_ClearDisplay();
		H_LCD_void_sendString("Rest Password");
		_delay_ms(1000);
	}

}



void Close_System()
{
	Global_Flag = 1 ;
}

/*************
 * Functions
 **************/


void Successful_Authentication(void)
{
	H_LCD_void_gotoXY(1,0);
    H_LCD_void_sendString("Welcome to your House");
    H_LCD_void_gotoXY(2,0);
    H_LCD_void_sendString("Last Login was:");
    H_LCD_void_gotoXY(3,0);
    Display_Time_Processing();
    _delay_ms(3000);
    H_LCD_void_ClearDisplay();
    Show_Time_Processing();
    Save_Time_Processing();
    H_Relay_Void_WorkOn(Relay_1);
    _delay_ms(5000);
    H_Relay_Void_WorkOff(Relay_1);
    H_LCD_void_ClearDisplay();
}

void Show_Time_Processing(void)
{
	//Display Time
	H_LCD_void_gotoXY(2, 0);
	H_LCD_void_sendString("Today is: ");
	H_LCD_void_gotoXY(3, 0);
	H_LCD_void_sendString("Time is: ");
	H_LCD_void_gotoXY(2, 10);
	H_LCD_void_sendIntNum((u16)H_RTC_Void_GetDate().RTC_Day);
	H_LCD_void_sendData('/');
	_delay_ms(200);
	H_LCD_void_sendIntNum((u16)H_RTC_Void_GetDate().RTC_Month);
	H_LCD_void_sendData('/');
	_delay_ms(200);
	H_LCD_void_sendIntNum((u16)H_RTC_Void_GetDate().RTC_Year);

		H_LCD_void_gotoXY(3, 9);
		H_LCD_void_sendIntNum((u16)H_RTC_Void_GetTime().RTC_Hours);
		H_LCD_void_sendData(':');
		_delay_ms(200);
		H_LCD_void_sendIntNum((u16)H_RTC_Void_GetTime().RTC_Minutes);
		H_LCD_void_sendData(':');
		_delay_ms(200);
		H_LCD_void_sendIntNum((u16)H_RTC_Void_GetTime().RTC_Seconed);
		_delay_ms(200);
		if(H_RTC_Void_GetTime().RTC_DayTime == RTC_AM)
		{
			H_LCD_void_gotoXY(3, 16);
			H_LCD_void_sendString(" AM");
		}
		else if(H_RTC_Void_GetTime().RTC_DayTime == RTC_PM)
		{
			H_LCD_void_gotoXY(3, 16);
			H_LCD_void_sendString(" PM");
		}
		_delay_ms(200);
}

void Save_Time_Processing(void)
{
   H_LCD_void_gotoXY(2,0);
   H_EEPROM_Void_WriteByte(20,H_RTC_Void_GetTime().RTC_Hours);
   _delay_ms(200);
   H_EEPROM_Void_WriteByte(21,H_RTC_Void_GetTime().RTC_Minutes);
   _delay_ms(200);
   H_EEPROM_Void_WriteByte(22,H_RTC_Void_GetTime().RTC_Seconed);
   _delay_ms(200);
   H_LCD_void_gotoXY(1,0);
   H_EEPROM_Void_WriteByte(23,H_RTC_Void_GetTime().RTC_DayTime);
   _delay_ms(200);
   H_EEPROM_Void_WriteByte(24,H_RTC_Void_GetDate().RTC_Day);
   _delay_ms(200);
   H_EEPROM_Void_WriteByte(25,H_RTC_Void_GetDate().RTC_Month);
   _delay_ms(200);
   H_EEPROM_Void_WriteByte(26,H_RTC_Void_GetDate().RTC_Year);
   _delay_ms(200);
}

void Display_Time_Processing(void)
{
		u8 datat;
		H_LCD_void_gotoXY(3,0);
		_delay_ms(200);
		   H_EEPROM_Void_ReadByte(24,&datat);
			H_LCD_void_sendIntNum((u16)datat);
			H_LCD_void_sendData('/');
		_delay_ms(200);
		   H_EEPROM_Void_ReadByte(25,&datat);
			H_LCD_void_sendIntNum((u16)datat);
			H_LCD_void_sendData('/');
		_delay_ms(200);
		   H_EEPROM_Void_ReadByte(26,&datat);
			H_LCD_void_sendIntNum((u16)datat);
		_delay_ms(500);
		H_LCD_void_gotoXY(4,0);
		_delay_ms(200);
		   H_EEPROM_Void_ReadByte(20,&datat);
			H_LCD_void_sendIntNum((u16)datat);
			H_LCD_void_sendData(':');
		_delay_ms(200);
		   H_EEPROM_Void_ReadByte(21,&datat);
			H_LCD_void_sendIntNum((u16)datat);
			H_LCD_void_sendData(':');
		_delay_ms(200);
		   H_EEPROM_Void_ReadByte(22,&datat);
			H_LCD_void_sendIntNum((u16)datat);
		_delay_ms(500);
}

void SYSTEM_OFF(void)
{
	H_BUZZER_Void_TurnOn();
	H_LCD_void_sendString("SYSTEM Closed");
	_delay_ms(1000);
	H_BUZZER_Void_TurnOff();
	H_LCD_void_ClearDisplay();

}

Pass_State Wrong_Authentication(void)
{
	Login_State Pass = Pass_Ok;
	u8 Local_Holder=0;

	/*Increment global counter */
	Global_PasswordCounter ++ ;
	H_LCD_void_gotoXY(2,0);
    H_LCD_void_sendString("Wrong Password");
    _delay_ms(1000);
    Local_Holder = User_Authentication_SignIn();

    H_LCD_void_ClearDisplay();
	if(Local_Holder== Login_Ok)
	{
	    Pass = Login_Ok;
	}
	if(Local_Holder== Login_Nok)
	{
		 Pass = Login_Nok;
	}
	return Pass;
}

void APP_WelcomeDisplay()
{
	H_LCD_void_gotoXY(1, 0);
	H_LCD_void_sendString("#Smart Home System#");
}

LOGIN_STATE User_Authentication_SignIn()
{
	/*
	 * Local Variables
	 */
	u8 Finger_Print_check=0;
	u8 checkTemp = 0 ;
	u8 Local_PassWordArr[4] = {0};
	u8 Local_ID = 0 ;
	/*
	 * Scan the finger print
	 * each finger print will have ID
	 */
	do
	{
		Local_ID=Finger_Print_Match();
	}while(Local_ID==0);
	if(Local_ID==1)
	{
		H_LCD_void_ClearDisplay();
		H_LCD_void_sendString("Welcome Nour");
	}
	else if(Local_ID==2)
	{
		H_LCD_void_ClearDisplay();
		H_LCD_void_sendString("Welcome Zedan");
	}
	else if(Local_ID==100)
	{
		H_LCD_void_ClearDisplay();
		H_LCD_void_sendString("No matching Found");
		return INVALID_LOGIN;

	}
	/*
	H_LCD_void_ClearDisplay();
	H_LCD_void_sendString("Enter ID:");

	do{
		Local_ID = H_KEYPAD_u8_getPressedKey();
	  }while(Local_ID == NO_PRESSED_KEY);
*/

	/* Enter Password */
	H_LCD_void_ClearDisplay();
	H_LCD_void_sendString("Enter Password:");
	H_LCD_void_gotoXY(2, 0);
	KeyPad_EnterPassWord(Local_PassWordArr);

	/*Call check password */
	H_LCD_void_ClearDisplay();
	checkTemp = Check_Password(Local_PassWordArr,Local_ID);

	/*if return call check password is correct then return valid else return IN VALID */
	if(checkTemp == CORRECT_PASS)
	{
		return VALID_LOGIN;
	}
	else
	{
		return INVALID_LOGIN;
	}
}

PASS_STATE Check_Password(u8 *Copy_Password, u8 Copy_ID)
{
	u8 Local_Counter = 0;
	u8 AddressCounter = 0 ;
	u8 Temp_Holder = 0;

	switch(Copy_ID)
	{
	case '1':
		AddressCounter = 0 ;
		H_LCD_void_sendString("ID_1");
		break;
	case '2':
		AddressCounter = 4 ;
		H_LCD_void_sendString("ID_2");
		break;
	}

	for(Local_Counter = 0; Local_Counter < 4; Local_Counter++)
	{
		_delay_ms(200);
		H_EEPROM_Void_ReadByte(AddressCounter, &Temp_Holder);

		/*If there is incorrect number  */
		if(Temp_Holder != Copy_Password[Local_Counter])
		{
			return INCORRECT_PASS ;
		}
		AddressCounter ++ ;
	}
	AddressCounter = 0 ;
	return CORRECT_PASS ;
}

void User_Authentication_SignUp()
{
	/*
	 * Local Variables
	 */
	u8 Local_PassWordArr[4] = {0};
	u8 User_ID = 0;
	u8 Local_Counter =0;
	H_LCD_void_ClearDisplay();
	H_LCD_void_gotoXY(1, 0);
	H_LCD_void_sendString("Adding New User: ");

	/* Take the next ID*/
	_delay_ms(1000);
	H_EEPROM_Void_ReadByte(EEPROM_USER_ID, &User_ID);
	_delay_ms(200);

	/*Scan finger print  */
	Finger_Print_Enroll(User_ID);

	H_LCD_void_ClearDisplay();
	H_LCD_void_sendString("Enter Personal Pass: ");

	/*Enter Password */
	H_LCD_void_gotoXY(2, 0);
	KeyPad_EnterPassWord(Local_PassWordArr);
	H_LCD_void_ClearDisplay();

	/*Save to EEPROM*/
	Save_NewPassword(Local_PassWordArr,User_ID);

	/*Saving data animation*/
	H_LCD_void_sendString("Saving data");
	while(Local_Counter != 4)
	{
		H_LCD_void_sendString(".");
		_delay_ms(500);
		Local_Counter ++ ;
	}
	H_LCD_void_ClearDisplay();
}

void KeyPad_EnterPassWord(u8 *Copy_PasswordArr)
{
	u8 Local_Counter = 0 ;
	u8 KPD_Value = 0;

	/*
	 * Enter Password
	 */
	do{
		KPD_Value = H_KEYPAD_u8_getPressedKey() ;
		if( KPD_Value != '\0')
		{
			Copy_PasswordArr[Local_Counter] = KPD_Value;
			H_LCD_void_sendData(Copy_PasswordArr[Local_Counter]);
			Local_Counter ++ ;
		}

	}while(Local_Counter != 4);
}

void Save_NewPassword(u8 *Copy_NewPassword, u8 Copy_ID)
{
	u8 AddressCounter = 0 ;
	u8 LocalCounter = 0;

		/*Check ID number */
	switch(Copy_ID)
	{
		case 1:
			AddressCounter = 0 ;
			H_LCD_void_sendString("ID_1");
			break ;
		case 2:
			AddressCounter = 4 ;
			H_LCD_void_sendString("ID_2");
		break ;
	}

	if(Copy_ID <= 2)
		{
		/*Enter the array to the eeprom */
		for(LocalCounter = 0 ; LocalCounter < 4 ; LocalCounter ++)
		{
		_delay_ms(100);
		H_EEPROM_Void_WriteByte(AddressCounter, Copy_NewPassword[LocalCounter]);
		AddressCounter ++ ;
		}

		/*Increment the User ID */
		Copy_ID += 1 ;
		_delay_ms(200);
		H_EEPROM_Void_WriteByte(EEPROM_USER_ID,Copy_ID );
		}
	else
		{
		H_LCD_void_sendString("Exceeded Max Number");
		_delay_ms(2000);
		H_LCD_void_ClearDisplay();
		}

}

