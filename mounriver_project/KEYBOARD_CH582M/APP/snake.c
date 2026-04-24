/********************************** (C) COPYRIGHT *******************************
 * File Name          : snake.c
 * Author             : ChnMasterOG
 * Version            : V1.0
 * Date               : 2022/1/27
 * Description        : 应用层 - 贪吃蛇游戏源文件
 * Copyright (c) 2023 ChnMasterOG
 * SPDX-License-Identifier: GPL-3.0
 *******************************************************************************/

#include "HAL.h"
#include <stdlib.h>
#include "snake.h"

uint8_t BodyDir[MAX_SNAKE_LENGTH];              // 身体朝向
static SnakePos BodyPos[MAX_SNAKE_LENGTH];      // 身体位置(LED编号)
static SnakePos FoodPos;                        // 食物位置(LED编号)
static uint8_t SnakeLength;                     // 蛇长

static void ProduceFood( void );
static BOOL ComparePos(SnakePos pos1, SnakePos pos2);
static SnakePos PosDirToPos(SnakePos target, uint8_t target_dir);
static BOOL CheckOverlap( SnakePos target );
static void ShowSnake( void );

const uint8_t SnakeMatrix[COL_SIZE][ROW_SIZE] = {
    // COL0~COL12，每个数组包含5个LED编号（对应ROW0~ROW4）
    // 格式: { ROW0, ROW1, ROW2, ROW3, ROW4 }
    
    /* COL0 */  { 0,   25,  26,  51,  52 },
    /* COL1 */  { 1,   24,  27,  50,  53 },
    /* COL2 */  { 2,   23,  28,  49,  54 },
    /* COL3 */  { 3,   22,  29,  48,  55 },
    /* COL4 */  { 4,   21,  30,  47,  56 },
    /* COL5 */  { 5,   20,  31,  46,  57 },
    /* COL6 */  { 6,   19,  32,  45,  58 },
    /* COL7 */  { 7,   18,  33,  44,  59 },
    /* COL8 */  { 8,   17,  34,  43,  60 },
    /* COL9 */  { 9,   16,  35,  42,  61 },
    /* COL10 */ { 10,  15,  36,  41,  62 },
    /* COL11 */ { 11,  14,  37,  40,  63 },
    /* COL12 */ { 12,  13,  38,  39,  64 },
};

/*******************************************************************************
 * Function Name  : Snake_Init
 * Description    : 初始化贪吃蛇
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void Snake_Init( void )
{
	uint16_t i;
  srand(BAT_adcVal);  // 用电池电量作为随机种子
	for ( i = 0; i < MAX_SNAKE_LENGTH; i++ ) {
	  BodyPos[i].PosX = 0;
	  BodyPos[i].PosY = 0;
		BodyDir[i] = DirUp;
	}
	SnakeLength = 2;
	BodyPos[0].PosX = 3; BodyPos[0].PosY = 1; BodyDir[0] = DirRight; // 初始蛇头位置/朝向
	BodyPos[1].PosX = 2; BodyPos[1].PosY = 1; BodyDir[1] = DirRight; // 初始蛇身体位置/朝向
	ProduceFood();  // 放置食物
}

/*******************************************************************************
 * Function Name  : MoveSnake
 * Description    : 蛇移动下一步
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void MoveSnake( void )
{
  uint16_t i;
  SnakePos tail;
  tail.PosX = BodyPos[SnakeLength-1].PosX;
  tail.PosY = BodyPos[SnakeLength-1].PosY;
  // 蛇身跟随前一个状态
  for (i = SnakeLength-1; i > 0; i--) {
    BodyPos[i].PosX = BodyPos[i-1].PosX;
    BodyPos[i].PosY = BodyPos[i-1].PosY;
  }
  // 后移动蛇头
  BodyPos[0] = PosDirToPos(BodyPos[0], BodyDir[0]);
  if ( ComparePos( BodyPos[0], FoodPos ) ) { // 蛇吃食物
    if ( SnakeLength < MAX_SNAKE_LENGTH ) { // 蛇不超过最大长度
      ++SnakeLength;
      BodyPos[SnakeLength-1].PosX = tail.PosX;
      BodyPos[SnakeLength-1].PosY = tail.PosY;
      ProduceFood();
    }
  } else {
    // 如果蛇头撞身体则重新开始
    if ( CheckOverlap(BodyPos[0]) ) {
      Snake_Init( );
    }
  }
  ShowSnake( );
}

/*******************************************************************************
 * Function Name  : CheckOverlap
 * Description    : 检查目标是否和蛇身重叠
 * Input          : 待检测的坐标
 * Output         : None
 * Return         : 1=TRUE or 0=FALSE
 *******************************************************************************/
static BOOL CheckOverlap( SnakePos target )
{
	uint8_t i;
	for ( i = 1; i < SnakeLength; i++ ) {
	  if ( ComparePos( target, BodyPos[i] ) ) {
	    return 1;
	  }
	}
	return 0;
}

/*******************************************************************************
 * Function Name  : PosDirToPos
 * Description    : 将目标位置和对应方向的下一步位置返回
 * Input          : 待检测的坐标和待检测目标的朝向
 * Output         : None
 * Return         : 下一步位置
 *******************************************************************************/
static SnakePos PosDirToPos(SnakePos target, uint8_t target_dir)
{
  switch (target_dir) {
    case DirUp:
      if ( target.PosY == 0 ) target.PosY = ROW_SIZE-1;  // 返回最后一行
      else --target.PosY;
      break;
    case DirDown:
      if ( target.PosY == ROW_SIZE-1 ) target.PosY = 0;  // 返回第一行
      else ++target.PosY;
      break;
    case DirLeft:
      if ( target.PosX == 0 ) target.PosX = COL_SIZE-1;  // 返回最后一列
      else {
        while ( SnakeMatrix[target.PosX-1][target.PosY] == SnakeMatrix[target.PosX][target.PosY] ) --target.PosX;
        --target.PosX;  // 第一列和第二列的LED一定不一样 故此设计
      }
      break;
    case DirRight:
      if ( target.PosX == COL_SIZE-1 ) target.PosX = 0;  // 返回第一列
      else {
        while ( SnakeMatrix[target.PosX+1][target.PosY] == SnakeMatrix[target.PosX][target.PosY] ) ++target.PosX;
        ++target.PosX;  // 最后一列和倒数第二列的LED一定不一样 故此设计
      }
      break;
  }
  return target;
}

/*******************************************************************************
 * Function Name  : ProduceFood
 * Description    : 产生一个新的食物
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
static void ProduceFood( void )
{
	do {
	  FoodPos.PosX = (int)(rand() % COL_SIZE);
	  FoodPos.PosY = (int)(rand() % ROW_SIZE);
	}	while( CheckOverlap( FoodPos ) || ComparePos(FoodPos, BodyPos[0]) );
}

/*******************************************************************************
 * Function Name  : ComparePos
 * Description    : 比较两个位置是否相同
 * Input          : 两个位置信息
 * Output         : None
 * Return         : 1-TRUE or 0-FALSE
 *******************************************************************************/
static BOOL ComparePos(SnakePos pos1, SnakePos pos2)
{
  if ( SnakeMatrix[pos1.PosX][pos1.PosY] == SnakeMatrix[pos2.PosX][pos2.PosY] ) return 1;
  else return 0;
}

/*******************************************************************************
 * Function Name  : ShowSnake
 * Description    : 在背光上显示蛇图案
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
static void ShowSnake( void )
{
  uint8_t i;
  tmos_memset(LED_BYTE_Buffer, 0, LED_NUMBER*3);
  // 食物颜色 - 红
  LED_BYTE_Buffer[SnakeMatrix[FoodPos.PosX][FoodPos.PosY]][RED_INDEX] = 0x7F;
  // 蛇头颜色 - 蓝
  LED_BYTE_Buffer[SnakeMatrix[BodyPos[0].PosX][BodyPos[0].PosY]][BLUE_INDEX] = 0x7F;
  // 蛇身颜色 - 绿
  for ( i = 1; i < SnakeLength; i++ ) {
    LED_BYTE_Buffer[SnakeMatrix[BodyPos[i].PosX][BodyPos[i].PosY]][GREEN_INDEX] = 0x7F;
  }
}

/*********************************************END OF FILE**********************/
