/*
 * FINGER_PRINT_Init.h
 *
 *  Created on: Mar 12, 2023
 *      Author: Dell
 */

#ifndef HAL_FINGER_PRINT_FINGER_PRINT_INIT_H_
#define HAL_FINGER_PRINT_FINGER_PRINT_INIT_H_

#include"STD_TYPES.h"
#include"FINGER_PRINT_Types.h"
void Finger_Print_Init(void);

void Finger_Print_Send_Command(u8 *Ptr_Package,u8 Package_Length );

Error_States_FB Finger_Print_Start(u8 Process_type,u8 FB_ID);
Error_States_FB Finger_Print_Enroll(u8 FB_ID);
u8 Finger_Print_Match(void);

void Finger_Print_Delete_Template(u8 FB_ID);
void Finger_Print_Empty_Templates(void);
void Flush_Buffer(void);

void Check_Sum(u8 FB_ID);
#endif /* HAL_FINGER_PRINT_FINGER_PRINT_INIT_H_ */
