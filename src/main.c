#include <stm32f031x6.h>
#include "display.h"
#include "serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define START_MENU 0
#define BOAT_STAGE 1
#define BUCKET_STAGE 2
#define GAME_OVER 3
#define ABILITY 4

#define MAX_FISHES 3
#define MAX_OBSTACLES 2

#define BOATWIDTH 48
#define BOATHEIGHT 31

#define BUCKETWIDTH 16
#define BUCKETHEIGHT 16

#define FISHWIDTH 16
#define FISHHEIGHT 16

#define OBSTACLEWIDTH
#define OBSTACLEHEIGHT 

#define BOARDWIDTH 128
#define BOARDHEIGHT 160

void initClock(void);
void initSysTick(void);
void initSerial(void);
void SysTick_Handler(void);
void delay(volatile uint32_t dly);
void setupIO();
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py);
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);

void move_right (uint16_t*,int*,int,int,int,int*);
void move_left (uint16_t*,int*,int,int,int*);
void move_down (uint16_t*, int*,int,int);
void move_up (uint16_t*,int*,int);
int collision (uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, int, int);
void show_score (int*);

int rightPressed(void);
int leftPressed(void);
int upPressed(void);
int downPressed(void);

void moveSprite(uint16_t*, uint16_t*, int, int, const uint16_t*, char);
void spawnFish(uint16_t*, uint16_t*, int, int, const uint16_t*, int*);
void randomise_fish (uint16_t fishX[], uint16_t fishY[], int index);

// showLives and displayHUD are two versions of the same thing (display HUD more efficient)
void showLives(uint16_t, uint16_t, int);
void displayHUD(uint16_t, uint16_t, int);

void reset (int*,int*,int*,int*,int*,int*);
void print_serial (int games, int lives, int score, int fishcaught,int);

void move_rocket(uint16_t *x, uint16_t *y, int width, int height, const uint16_t *sprite, int *stage, int*score, uint16_t fishx[], uint16_t fishy[],int*,int,int,int);

volatile uint32_t milliseconds;

const uint16_t boat1[]=
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65296,65296,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23552,31744,23552,65296,23552,65296,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19201,12058,0,13576,0,5384,13576,23552,65296,23552,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11009,3866,0,0,0,13576,13576,13576,3866,65535,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7663,7663,7663,3866,0,0,0,0,0,3866,65535,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,7663,7663,60169,27393,0,0,51977,49151,63950,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,7663,7663,2817,51977,3866,0,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,7663,0,51977,0,49151,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49151,65535,32517,32517,32517,65535,65535,7663,0,51977,0,49151,49151,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,32517,57343,23556,65535,65535,24047,0,51977,0,49151,40959,57343,7663,47566,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,32517,15364,15364,65535,7663,63950,0,27393,0,0,49151,57343,49151,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,32517,65535,32517,65535,7172,65535,7663,0,0,2817,0,65535,49151,65535,7663,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49151,65535,57343,7172,32517,7172,65535,65535,7663,0,0,19201,0,49151,49151,49151,57343,7663,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63950,65535,65535,65535,65535,32517,65535,65535,65535,7663,63950,0,27393,0,65535,49151,49151,57343,49151,49151,55758,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24047,7663,7663,24047,7663,65535,65535,7663,65535,7663,63950,0,2817,0,49151,49151,49151,49151,49151,7663,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27393,35593,27393,11009,7663,15855,7663,15855,7663,7663,7663,7663,63950,0,27393,65535,65535,49151,49151,7663,7663,7663,7663,24047,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35593,0,0,0,0,0,0,0,0,0,0,0,0,11009,27393,3866,11009,11009,2817,2817,7663,7663,0,51977,7663,7663,24047,7663,7663,19201,11009,51977,60169,12058,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,27393,0,0,27393,11009,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2817,51977,51977,43785,14636,27393,11009,3866,11009,19201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15364,7172,32517,32517,32517,7172,7172,7172,32517,0,0,0,0,0,14636,0,0,14636,0,0,0,0,0,0,0,60169,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19482,19482,19482,4387,4387,4387,4387,4387,32517,7172,0,0,0,0,55909,55909,14636,39525,0,0,0,0,48430,0,0,14636,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27674,19482,3098,3866,3866,3866,20250,28442,4387,32517,7172,7172,0,0,64101,3866,55909,3866,0,0,0,48430,53644,0,0,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19482,7663,7663,3866,3866,15855,3866,7663,3866,4387,32517,7172,7172,39525,64101,55909,55909,0,0,48430,0,53644,0,0,27393,0,0,0,0,0,0,0,0,0,0,0,0,19482,19482,19482,19482,0,0,0,0,0,0,0,19482,24047,3866,15855,7663,3866,3866,3866,3866,4387,4387,4387,7172,7172,24325,32517,32517,13908,13908,7172,7172,53644,32517,32517,7172,7172,32517,32517,32517,7172,7172,7172,32517,32517,32517,7172,19482,19482,0,0,0,0,0,0,0,0,0,0,19482,3866,3866,3866,3866,12058,3866,3866,3866,3866,3866,3866,3866,4387,4387,4387,4387,4387,12579,12579,4387,4492,3866,3866,4387,4387,20771,4387,4387,4387,3866,4387,4387,4387,4387,20771,19482,0,0,0,0,0,0,0,0,0,0,0,19482,3866,3866,3866,3866,3866,3866,3866,20250,3866,3866,3866,3866,3866,3866,3866,3866,3866,3866,12058,3866,45452,3866,7172,3866,32517,3866,7172,3866,32517,3866,7172,3866,7172,19482,19482,0,0,0,0,0,0,0,0,0,0,0,0,19482,19482,19482,19482,19482,19482,19482,19482,19482,3866,3866,20250,3866,12058,20250,3866,12058,3866,3866,12058,3866,53644,3866,3866,3866,20250,20250,12058,12058,3866,3866,3866,3866,12058,19482,0,0,0,0,0,0,0,0,0,0,57343,65535,65535,65535,65535,19482,19482,19482,19482,19482,19482,3098,19482,19482,19482,19482,19482,19482,19482,19482,19482,19482,19482,19482,21669,19482,19482,19482,3098,19482,19482,19482,19482,27674,19482,19482,19482,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,57343,65535,65535,57343,65535,49151,65535,65535,65535,65535,65535,65535,65535,19482,19482,27674,65535,65535,19482,65535,37260,65535,49151,65535,19482,65535,65535,19482,19482,65535,27674,19482,65535,65535,65535,65535,10750,0,0,0,0,0,0,0,65535,65535,57343,65535,65535,65535,57343,49151,65535,10750,49151,65535,65535,10750,10750,10750,65535,65535,49151,49151,10750,65535,65535,50348,58540,58540,10750,65535,65535,65535,57343,65535,10750,10750,65535,65535,65535,65535,65535,10750,0,0,0,0,0,0,0,0,0,0,42469,58853,58853,58853,10750,58853,58853,58853,58853,42469,58853,58853,10750,58853,58853,58853,58853,10750,10750,10750,42156,58540,58540,58540,58853,58853,58853,58853,10750,2558,58853,42469,58853,58853,65535,58853,58853,0,0,0,0,0,0,0,0,0,0,0,0,0,50661,58853,58853,58853,58853,58853,58853,0,58853,58853,58853,58853,58853,58853,58853,58853,58853,58853,58853,10750,58853,58853,58853,58853,58853,50661,58853,58853,58853,0,0,0,10750,58853,0,0,0,0,0,0,0,
};
const uint16_t boat2[]= 
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65296,23552,65296,19201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27393,3866,0,5384,0,57104,23552,15360,23552,23552,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2817,3866,13576,7168,23552,13576,0,13576,3866,65535,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7663,7663,7663,3866,13576,13576,0,0,0,12058,65535,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,7663,7663,60169,19201,0,0,51977,65535,63950,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,7663,7663,11009,51977,3866,0,7663,47566,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,49151,65535,15855,0,51977,0,49151,24047,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49151,65535,32517,32517,32517,57343,65535,7663,0,51977,0,49151,49151,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,32517,57343,7172,65535,49151,7663,0,51977,0,40959,49151,57343,7663,47566,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,32517,7172,7172,65535,24047,63950,0,11009,0,0,49151,57343,49151,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57343,65535,16133,65535,32517,65535,7172,65535,7663,0,0,2817,0,49151,65535,49151,7663,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49151,65535,65535,7172,32517,7172,49151,65535,7663,0,0,19201,0,65535,49151,49151,57343,7663,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63950,65535,65535,65535,65535,32517,65535,49151,65535,7663,63950,0,27393,0,49151,49151,65535,57343,40959,49151,63950,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7663,7663,7663,7663,7663,65535,65535,15855,65535,15855,63950,0,2817,0,49151,49151,49151,49151,49151,7663,15855,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27393,35593,27393,11009,7663,24047,7663,7663,7663,7663,7663,32239,63950,0,11009,65535,65535,49151,49151,7663,7663,7663,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35593,0,0,0,0,0,0,0,0,0,0,0,0,11009,11009,3866,11009,11009,2817,2817,7663,7663,0,51977,7663,15855,24047,7663,15855,19201,11009,51977,43785,3866,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,27393,0,0,27393,27393,0,0,0,0,0,0,0,14636,0,0,14636,0,0,0,2817,51977,60169,43785,14636,27393,2817,3866,11009,19201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23556,15364,32517,16133,32517,7172,7172,7172,32517,0,0,0,0,0,55909,55909,31020,55909,0,0,0,0,48430,0,0,60169,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19482,27674,19482,4387,4387,4387,4387,4387,16133,7172,0,0,0,0,55909,3866,55909,12058,0,0,0,48430,53644,0,0,14636,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27674,19482,19482,20250,3866,3866,12058,3866,4387,32517,23556,7172,0,0,55909,55909,55909,55909,0,0,48430,0,21669,0,0,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19482,7663,63950,3866,3866,63950,3866,63950,3866,12579,32517,7172,23556,0,5716,13908,0,5716,13908,0,0,21669,0,0,27393,0,0,0,0,0,0,0,0,0,0,0,0,19482,19482,19482,19482,0,0,0,0,0,0,0,19482,7663,3866,63950,7663,3866,3866,3866,3866,4387,4387,4387,15364,7172,32517,32517,24325,24325,32517,7172,7172,37260,32517,32517,15364,7172,32517,32517,32517,7172,7172,7172,32517,32517,32517,7172,19482,19482,0,0,0,0,0,0,0,0,0,0,19482,3866,20250,20250,3866,12058,3866,3866,3866,20250,3866,3866,3866,4387,4387,4387,4387,20771,12579,4387,12579,21669,3866,12058,4387,4387,4387,4387,4387,4387,3866,4387,4387,4387,4387,4387,19482,19482,0,0,0,0,0,0,0,0,0,0,19482,3866,3866,3866,3866,3866,3866,3866,20250,3866,3866,12058,12058,3866,3866,3866,3866,3866,3866,3866,3866,37260,3866,7172,3866,32517,3866,7172,12058,32517,3866,7172,3866,7172,19482,19482,19482,0,0,0,0,0,0,0,0,65535,65535,65535,19482,19482,19482,19482,19482,19482,19482,3098,19482,3866,3866,3866,3866,3866,28442,3866,3866,3866,20250,12058,3866,53644,3866,3866,3866,3866,3866,3866,12058,12058,3866,12058,12058,3866,19482,0,0,0,0,0,0,0,0,57343,65535,65535,65535,65535,65535,65535,49151,57343,65535,65535,65535,65535,65535,65535,57343,49151,65535,65535,19482,19482,65535,65535,65535,19482,65535,21669,65535,49151,58853,65535,57343,65535,19482,19482,65535,27674,3098,49151,57343,65535,58853,58853,0,0,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,65535,65535,49151,65535,65535,65535,65535,58853,27134,65535,58853,65535,65535,65535,10750,34277,65535,50348,37260,58540,50661,58853,10750,65535,65535,65535,10750,42469,65535,65535,65535,65535,10750,34277,0,0,0,0,0,0,0,0,0,0,42469,58853,10750,10750,42469,10750,10750,10750,58853,2558,10750,27134,42469,58853,50661,42469,10750,10750,42469,42469,50661,58540,58540,58540,58540,34277,10750,10750,58853,58853,58853,27134,10750,58853,58853,2558,42469,0,0,0,0,0,0,0,0,0,0,0,0,0,42469,34277,58853,10750,10750,10750,42469,0,58853,58853,58853,42469,58853,42469,42469,58853,42469,58853,0,34277,58540,42469,42469,42469,58853,50661,10750,2558,58853,0,0,0,58853,42469,0,0,0,0,0,0,0,
};
const uint16_t bucket[]= 
{
	0,0,0,0,25880,25880,25880,25880,25880,25880,25880,25880,0,0,0,0,0,0,25880,25880,35113,35113,28202,28202,28202,28202,28986,28986,25880,25880,0,0,0,25880,35113,35113,35113,60556,60556,29117,29117,37318,37318,28986,28986,28986,25880,0,0,25880,35113,60556,60556,60556,60556,29117,29117,29117,37318,37318,37318,28986,25880,0,0,25880,25880,25880,60556,60556,29117,29117,29117,37318,37318,37318,25880,25880,25880,0,0,25880,18457,35113,25880,25880,25880,25880,25880,37318,25880,25880,28986,28986,25880,0,0,25880,18457,35113,35113,18457,28202,28202,18457,28202,28202,18457,28986,28986,25880,0,0,25880,52323,13212,35113,18457,28202,28202,18457,28202,28202,18457,13212,52323,25880,0,0,25880,18457,35113,13212,13212,52323,52323,52323,52323,13212,13212,28986,28986,25880,0,0,25880,18457,35113,35113,18457,28202,28202,18457,28202,28202,18457,28986,28986,25880,0,0,25880,18457,35113,35113,18457,28202,28202,18457,28202,28986,18457,28986,28986,25880,0,0,0,18457,35113,35113,18457,28202,28202,18457,28202,28986,18457,28986,28986,0,0,0,0,52323,52323,35113,18457,28202,28202,18457,28202,28986,18457,13212,13212,0,0,0,0,25880,35113,52323,52323,13212,13212,52323,52323,52323,13212,28986,25880,0,0,0,0,0,25880,35113,18457,28202,28202,18457,28202,28986,18457,25880,0,0,0,0,0,0,0,25880,25880,25880,25880,25880,25880,25880,25880,0,0,0,0,
};
const uint16_t fish[]=
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,59168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,59168,59168,59168,0,0,0,0,0,0,59168,59168,0,0,0,0,0,0,59168,59168,59168,0,0,0,0,0,0,59168,59168,0,0,0,59168,59168,60301,34404,60301,59168,59168,0,0,0,0,59168,59168,59168,0,60301,34404,60301,34404,60301,34404,34404,34404,60301,59168,0,0,0,59168,59168,59168,58443,34404,58443,34404,34404,34404,58443,60301,34404,34404,59168,0,59168,59168,59168,58443,34404,58443,34404,58443,34404,59168,34404,58443,0,34404,59168,0,59168,59168,0,0,59168,58443,58443,59168,59168,59168,58443,34404,34404,34404,59168,59168,0,0,0,0,0,0,59168,59168,59168,59168,59168,59168,59168,0,0,0,0,0,0,0,0,0,0,59168,59168,0,0,0,0,0,0,0,0,0,0,0,0,0,59168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

const uint16_t bucketFish[] = 
{
	0,0,0,0,25880,25880,25880,25880,25880,25880,60301,59168,59168,0,0,0,0,0,25880,25880,35113,59168,59168,60301,59168,34404,60301,34404,34404,59168,0,0,0,25880,35113,35113,44006,44006,60301,60301,34404,58443,60301,0,34404,59168,25880,0,0,25880,35113,44006,44006,60301,60301,60301,34404,58443,58443,34404,34404,44006,25880,0,0,25880,25880,44006,44006,60301,34404,34404,34404,34404,58443,34404,44006,25880,25880,0,0,25880,18457,35113,44006,44006,44006,44006,44006,44006,44006,44006,44006,28986,25880,0,0,25880,18457,35113,35113,18457,44006,28202,18457,44006,44006,18457,28986,28986,25880,0,0,25880,52323,13212,35113,18457,28202,28202,18457,28202,44006,18457,13212,52323,25880,0,0,25880,18457,35113,13212,13212,44006,52323,52323,52323,44006,13212,28986,28986,25880,0,0,25880,18457,35113,35113,18457,44006,28202,18457,28202,28202,18457,28986,28986,25880,0,0,25880,18457,35113,35113,18457,44006,28202,18457,28202,28986,18457,28986,28986,25880,0,0,0,18457,35113,35113,18457,28202,28202,18457,28202,44006,18457,28986,28986,0,0,0,0,52323,52323,35113,18457,28202,28202,18457,28202,28986,18457,13212,13212,0,0,0,0,25880,35113,52323,52323,13212,13212,52323,52323,52323,13212,28986,25880,0,0,0,0,0,25880,35113,18457,28202,28202,18457,28202,28986,18457,25880,0,0,0,0,0,0,0,25880,25880,25880,25880,25880,25880,25880,25880,0,0,0,0,
};
// heart not obs
const uint16_t heart[]=
{
	0,0,0,0,0,0,0,0,0,65535,65535,0,0,65535,65535,0,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,0,65535,65535,65535,65535,65535,65535,0,0,0,65535,65535,65535,65535,0,0,0,0,0,65535,65535,0,0,0,
};

const uint16_t obstacle[]=
{
	0,0,65535,65535,65535,65535,0,0,0,65535,65535,65535,65535,65535,65535,0,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,0,65535,65535,65535,65535,65535,65535,0,0,0,65535,65535,65535,65535,0,0,
};

const uint16_t rocket[]=
{
	0,0,0,29332,61572,0,0,0,0,0,6912,6870,56286,6912,0,0,0,0,6870,7936,7936,56286,0,0,0,0,56286,56286,6870,6870,0,0,0,0,6912,6870,56286,6912,0,0,0,0,56286,7936,6912,56286,0,0,0,0,15620,65287,65303,15620,0,0,0,0,0,32516,15620,0,0,0,
};

int main()
{
	int stage = START_MENU;
	int score = 0;
    int lives = 3;
	int toggle = 0; // used for switching between animations
	int count = 0;
	int currentFish = -1;
	char restart;
	int games_played = 1;
	int fish_caught = 0;
	int abilities_used = 0;

    uint16_t fishX[3] = {82, 52, 22};
    uint16_t fishY[3] = {90, 110, 144};


	uint16_t obstacleX[] = {0,129};
	uint16_t obstacleY[] = {70,130}; 
	int direction[] = {0, 1, 1};
	int obsDir[] = {0, 1, 0};
	initClock();
	initSysTick();
	setupIO();
	initSerial();

 	int beginGame = 1;

	// MAY BE REPLACED BY JUST X AND Y

    uint16_t bucket_x = 40;
	uint16_t bucket_y = 40;
	uint16_t bucket_oldx = 0;
    uint16_t bucket_oldy = 0;
	int bucket_horizontal_moved = 0;
    int bucket_vertical_moved = 0;
	int bucket_invert = 0;

	int has_fish = 0;

	uint16_t boat_x = 64 -(BOATWIDTH/2);
	uint16_t boat_y = 10;
	uint16_t boat_oldx = boat_x;
	int boat_horizontal_moved = 0;
	int boat_invert = 0;

	uint16_t rocket_x = 0;
	uint16_t rocket_y = 0;
	int ability = 0;

	// Game Loop
	while (1)
	{
		// Start menu stage
		while (stage == START_MENU)
		{
			count++;
			// Display only once
			if (beginGame) {
				fillRectangle(0,0,128,160,0);
				fillRectangle(11,8,109,18,RGBToWord(255,255,255));
				printTextX2("CarpaDiem", 13, 10, RGBToWord(0,0,0), RGBToWord(255,255,255));
				printText("Help Pat the cat", 0, 35, RGBToWord(0,255,0), 0);
				printText("cath some food!", 0, 45, RGBToWord(0,255,0), 0);
				printText("RL = Move Boat", 0, 55, RGBToWord(255,255,255), 0);
				printText("D = Release bucket", 0, 65, RGBToWord(255,255,255), 0);
				printText("RLUD = Move bucket", 0, 75, RGBToWord(255,255,255), 0);
				printText("U = Rocket", 0, 85, RGBToWord(255,255,255), 0);
				printText("U(again) = Explode", 0, 95, RGBToWord(255,255,255), 0);
				printText("Get Fish and go", 0, 105, RGBToWord(255,255,0), 0);
				printText("back to boat, try", 0, 115, RGBToWord(255,255,0), 0);
				printText("not to get hit!", 0, 125, RGBToWord(255,255,0), 0);
				beginGame = 0;
			}
			
			// Blinking effect for "Press any button"
			if (count > 40 && count <= 80) 
			{
				printText("Press any button", 10, 140, 255, 0);
				printText("to start", 37, 150, 255, 0);
			}
			else if (count > 80) 
			{
				count = 0;
				fillRectangle(0,140,128,20,0);
			}

			if (rightPressed() || leftPressed() || upPressed() || downPressed()) {
				eputs("\n");
				eputs("_________                           ________  .__                \n");
				eputs("\\_   ___ \\_____ __________________  \\______ \\ |__| ____   _____  \n");
				eputs("/    \\  \\/\\__  \\\\_  __ \\____ \\__  \\  |    |  \\|  |/ __ \\ /     \\ \n");
				eputs("\\     \\____/ __ \\|  | \\/  |_> > __ \\_|    `   \\  \\  ___/|  Y Y  \\\n");
				eputs(" \\______  (____  /__|  |   __(____  /_______  /__|\\___  >__|_|  /\n");
				eputs("        \\/     \\/      |__|       \\/        \\/        \\/      \\/ \n");
				eputs("\n");
				print_serial(games_played,lives,score,fish_caught,abilities_used);
				delay(100);
				stage = BOAT_STAGE;
			}
			count++;
			delay(16); // is a delay at the end beneficial?
		}
		// Clear the screen before next stage
		fillRectangle(0, 0, 128, 160, 0);

		// Boat stage
		while (stage == BOAT_STAGE)
		{
			
			if (ability < 3)
			{
				fillRectangle(80,0,8,8,RGBToWord(255,0,0));
			}
			else if (ability >= 3)
			{
				fillRectangle(80,0,8,8,RGBToWord(0,255,0));
			}

			showLives(120, 0, lives);
			show_score(&score);
			putImage(boat_x, boat_y, BOATWIDTH, BOATHEIGHT, boat1, boat_invert, 0);

			// Spawn fishes
			for (int i = 0; i < MAX_FISHES; i++)
			{
				spawnFish(&fishX[i], &fishY[i], 16, 16, fish, &direction[i]);
			}
			for (int i = 0; i < MAX_OBSTACLES; i++)
			{
				spawnFish(&obstacleX[i], &obstacleY[i], 8, 8, obstacle, &obsDir[i]);
			}
			// Control boat left and right
			// Down to release bucket
			// Up for ability

			// Can possibly #define the parameters for these functions?

			boat_horizontal_moved = 0;
			move_right(&boat_x, &boat_horizontal_moved, BOARDWIDTH, BOATWIDTH,1,&boat_invert);
			move_left(&boat_x, &boat_horizontal_moved, 0, 1, &boat_invert);
			// Up pressed
			if (upPressed() == 1 && ability >= 3) 
			{
				ability = 0;
				abilities_used ++;
				rocket_x = boat_x + (BOATWIDTH/2) - (BUCKETWIDTH/2);
				rocket_y = 40;

				fillRectangle(0, 0, 128, 160, 0);
				showLives(120, 0, lives);
				show_score(&score);
				putImage(boat_x, boat_y, BOATWIDTH, BOATHEIGHT, boat1, boat_invert, 0);
				fillRectangle(80,0,8,8,RGBToWord(0,255,0));

				stage = ABILITY;
			}
			// Down pressed
			if (downPressed() == 1) {
				bucket_x = boat_x + (BOATWIDTH/2) - (BUCKETWIDTH/2);
				bucket_y = 40;
				stage = BUCKET_STAGE;
			}

			if (boat_horizontal_moved == 1)
            {
                // only redraw if there has been some movement (reduces flicker)
                fillRectangle(boat_oldx, boat_y, BOATWIDTH, BOATHEIGHT, 0);
                boat_oldx = boat_x;
				if (toggle == 0)
				{
					putImage(boat_x, boat_y, BOATWIDTH, BOATHEIGHT, boat1, boat_invert, 0);
				}
				else
				{
					putImage(boat_x, boat_y, BOATWIDTH, BOATHEIGHT, boat2, boat_invert, 0);
				}
				toggle = toggle ^ 1;
            }
			delay(16);
		}
		// Bucket stage
		while (stage == BUCKET_STAGE)
        {
			showLives(120, 0, lives);
			// Spawn fishes
			for (int i = 0; i < MAX_FISHES; i++)
			{
				if (i != currentFish) { // Don't show fish currently in bucket
					spawnFish(&fishX[i], &fishY[i], 16, 16, fish, &direction[i]);
				}
			}
			for (int i = 0; i < MAX_OBSTACLES; i++)
			{
				spawnFish(&obstacleX[i], &obstacleY[i], 8, 8, obstacle, &obsDir[i]);
			}

            // MOVEMENT SYSTEM START
            bucket_horizontal_moved = 0;
			bucket_vertical_moved = 0;
			move_right(&bucket_x, &bucket_horizontal_moved, BOARDWIDTH, BUCKETWIDTH,0,&bucket_invert);
			move_left(&bucket_x, &bucket_horizontal_moved, 0,0,&bucket_invert); 
			move_down(&bucket_y, &bucket_vertical_moved, BOARDHEIGHT, BUCKETHEIGHT);  
			move_up(&bucket_y, &bucket_vertical_moved, boat_y+BOATHEIGHT);     
            // MOVEMENT SYSTEM END
            
            // DRAW IMAGE START
            if (bucket_vertical_moved == 1 || bucket_horizontal_moved == 1)
            {
                // only redraw if there has been some movement (reduces flicker)
                fillRectangle(bucket_oldx, bucket_oldy, BUCKETWIDTH, BUCKETHEIGHT, 0);
                bucket_oldx = bucket_x;
                bucket_oldy = bucket_y;
				putImage(bucket_x, bucket_y, BUCKETWIDTH, BUCKETHEIGHT, bucket, 0, 0);
            }
            // DRAW IMAGE END
            
            // COLLISION DETECTION START
			for (int i = 0; i < 3; i++)
			{
				if ((collision(bucket_x,bucket_y,16,16,fishX[i],fishY[i],16,16) || collision(fishX[i],fishY[i],16,16,bucket_x,bucket_y,16,16)) && (has_fish == 0))
				{
					has_fish = 1;
					currentFish = i;
					fillRectangle(fishX[i], fishY[i], 16, 16, 0); //draw over fish
					putImage(bucket_x, bucket_y, BUCKETWIDTH, BUCKETHEIGHT, bucket, 0, 0);//draw bucket again
					break;
				}
			}

			for (int i = 0; i < MAX_OBSTACLES; i++)
			{
				if (collision(bucket_x,bucket_y,16,16,obstacleX[i],obstacleY[i],8,8))
				{	
					currentFish = -1;
					has_fish = 0;
                	fillRectangle(bucket_oldx, bucket_oldy, BUCKETWIDTH, BUCKETHEIGHT, 0);
					if(lives == 1)
					{
						lives --;
						print_serial(games_played,lives,score,fish_caught,abilities_used);
						fillRectangle(0, 0, 128, 160, 0);
						fillRectangle(8,58,110,18,RGBToWord(255,255,255));
						printText("Pat the cat's",0,0,RGBToWord(255,255,255),0);
						printText("bucket broke so he",0,10,RGBToWord(255,255,255),0);
						printText("called it a day",0,20,RGBToWord(255,255,255),0);
						printText("and went home",0,30,RGBToWord(255,255,255),0);
						printTextX2("Trip Over", 10, 60, RGBToWord(0,0,0), RGBToWord(255,255,255));
						printText("Score:",10,80,RGBToWord(255,255,0),0);
						printNumber(score,50,80,RGBToWord(255,255,0),0);
						printText("Grade: ",73,100,RGBToWord(255,255,255),0);
						if (score >= 0 && score < 501) {
							printTextX2("F",85,110,RGBToWord(255,0,0),0);
						} else if (score >= 501 && score < 2000) {
							printTextX2("D",85,110,RGBToWord(255,128,0),0);
						} else if (score >= 2000 && score < 4000) {
							printTextX2("C",85,110,RGBToWord(255,255,0),0);
						} else if (score >= 4000 && score < 6000) {
							printTextX2("B",85,110,RGBToWord(128,255,0),0);
						} else if (score >= 6000 && score < 8000) {
							printTextX2("A",85,110,RGBToWord(0,255,255),0);
						} else if (score >= 8000) {
							printTextX2("S",85,110,RGBToWord(127,0,255),0);
						}
						lives = 3;
						score = 0;
						beginGame = 1;
						stage = GAME_OVER;
					}
					else
					{
						lives = lives - 1;
						print_serial(games_played,lives,score,fish_caught,abilities_used);
						stage = BOAT_STAGE;
					}
				}
			}

			if (collision(boat_x, boat_y+10, BOATHEIGHT, BOATWIDTH, bucket_x, bucket_y, BUCKETHEIGHT, BUCKETWIDTH) && (has_fish == 1))
			{
				fish_caught++;
				stage = BOAT_STAGE;
				has_fish = 0;
				if (currentFish == 0)
				{
					score += 250;
				}
				else if(currentFish == 1)
				{
					score += 500;
				}
				else if (currentFish == 2)
				{
					score += 1000;
				}
				print_serial(games_played,lives,score,fish_caught,abilities_used);

				randomise_fish(fishX,fishY,currentFish);

				currentFish = -1; // -1, no fish
				fillRectangle(bucket_oldx, bucket_oldy, BUCKETHEIGHT, BUCKETWIDTH, 0);
				ability++;
			}
            // COLLISION DETECTION END
            
            delay(16);
        }
		// Game over stage
		while (stage == GAME_OVER)
		{
			putImage(16, 100, BOATWIDTH, BOATHEIGHT, boat1, 0, 0);
			printText("Press keyboard (r)", 2, 140, RGBToWord(255, 255, 255), 0);
			printText("to restart", 33, 150, RGBToWord(255, 255, 255), 0);
			restart = egetchar();
			if (restart == 'r') 
			{
				eputs("\nNew Game Started!");
				games_played ++;
				reset(&score, &lives, &beginGame, &stage,&fish_caught,&abilities_used);
				boat_x = 64 -(BOATWIDTH/2);
				boat_y = 10;
				boat_invert = 0;
				delay(100);
			}
		}

		while (stage == ABILITY)
		{
			for (int i = 0; i < MAX_FISHES; i++)
			{
				spawnFish(&fishX[i], &fishY[i], 16, 16, fish, &direction[i]);
			}

			move_rocket(&rocket_x,&rocket_y,8,8,rocket,&stage,&score,fishX,fishY,&lives,games_played,fish_caught,abilities_used);
		}
	}
	return 0;
}


void initSysTick(void)
{
	SysTick->LOAD = 48000;
	SysTick->CTRL = 7;
	SysTick->VAL = 10;
	__asm(" cpsie i "); // enable interrupts
}
void SysTick_Handler(void)
{
	milliseconds++;
}
void initClock(void)
{
	// This is potentially a dangerous function as it could
	// result in a system with an invalid clock signal - result: a stuck system
	// Set the PLL up
	// First ensure PLL is disabled
	RCC->CR &= ~(1u << 24);
	while ((RCC->CR & (1 << 25))); // wait for PLL ready to be cleared

	// Warning here: if system clock is greater than 24MHz then wait-state(s) need to be
	// inserted into Flash memory interface

	FLASH->ACR |= (1 << 0);
	FLASH->ACR &= ~((1u << 2) | (1u << 1));
	// Turn on FLASH prefetch buffer
	FLASH->ACR |= (1 << 4);
	// set PLL multiplier to 12 (yielding 48MHz)
	RCC->CFGR &= ~((1u << 21) | (1u << 20) | (1u << 19) | (1u << 18));
	RCC->CFGR |= ((1 << 21) | (1 << 19));

	// Need to limit ADC clock to below 14MHz so will change ADC prescaler to 4
	RCC->CFGR |= (1 << 14);

	// and turn the PLL back on again
	RCC->CR |= (1 << 24);
	// set PLL as system clock source
	RCC->CFGR |= (1 << 1);
}
void delay(volatile uint32_t dly)
{
	uint32_t end_time = dly + milliseconds;
	while (milliseconds != end_time)
		__asm(" wfi "); // sleep
}

void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber)
{
	Port->PUPDR = Port->PUPDR & ~(3u << BitNumber * 2); // clear pull-up resistor bits
	Port->PUPDR = Port->PUPDR | (1u << BitNumber * 2);	// set pull-up bit
}
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode)
{
	/*
	*/
	uint32_t mode_value = Port->MODER;
	Mode = Mode << (2 * BitNumber);
	mode_value = mode_value & ~(3u << (BitNumber * 2));
	mode_value = mode_value | Mode;
	Port->MODER = mode_value;
}
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py)
{
	// checks to see if point px,py is within the rectangle defined by x,y,w,h
	uint16_t x2, y2;
	x2 = x1 + w;
	y2 = y1 + h;
	int rvalue = 0;
	if ((px >= x1) && (px <= x2))
	{
		// ok, x constraint met
		if ((py >= y1) && (py <= y2))
			rvalue = 1;
	}
	return rvalue;
}

void setupIO()
{
	RCC->AHBENR |= (1 << 18) + (1 << 17); // enable Ports A and B
	display_begin();
	pinMode(GPIOB,4,0);
	pinMode(GPIOB,5,0);
	pinMode(GPIOA,8,0);
	pinMode(GPIOA,11,0);
	enablePullUp(GPIOB,4);
	enablePullUp(GPIOB,5);
	enablePullUp(GPIOA,11);
	enablePullUp(GPIOA,8);
}

void move_right (uint16_t *x, int *horizontal_moved, int boundary, int object_width, int flip, int *invert)
{
	if (rightPressed()) // right pressed
	{
		if (*x + object_width < boundary)
		{
			*x = *x + 1;
			*horizontal_moved = 1;
			if (flip == 1)
			{
				*invert = 0;
			}
		}
	}
}

void move_left (uint16_t *x, int *horizontal_moved, int boundary, int flip, int *invert)
{
	if (leftPressed()) // left pressed
	{
		if (*x > boundary)
		{
			*x = *x - 1;
			*horizontal_moved = 1;
			if (flip == 1)
				{
					*invert = 1;
				}
		}
	}
}

void move_down (uint16_t *y, int *vertical_moved, int boundary, int object_height)
{
	if (downPressed()) // down pressed
	{
		if (*y + object_height < boundary)
		{
			*y = *y + 1;
			*vertical_moved = 1;
		}
	}
}

void move_up (uint16_t *y, int *vertical_moved, int boundary)
{
	if (upPressed()) // up pressed
	{
		if (*y > boundary)
		{
			*y = *y - 1;
			*vertical_moved = 1;
		}
	}
}

int collision (uint16_t hitbox_x, uint16_t hitbox_y, uint16_t hitbox_heigth, uint16_t hitbox_width, uint16_t object_x, uint16_t object_y, int object_height, int object_width)
{
	uint16_t hitbox_heigth_better = hitbox_width * 0.75;
	uint16_t hitbox_width_better = hitbox_heigth * 0.75;
	uint16_t hitbox_x_better = hitbox_x + (hitbox_width - hitbox_width_better) / 2;
	uint16_t hitbox_y_better = hitbox_y + (hitbox_heigth - hitbox_heigth_better) / 2;
	if (isInside(hitbox_x_better, hitbox_y_better, hitbox_heigth_better, hitbox_width_better, object_x, object_y) ||
	isInside(hitbox_x_better, hitbox_y_better, hitbox_heigth_better, hitbox_width_better, object_x + object_width, object_y) ||
	isInside(hitbox_x_better, hitbox_y_better, hitbox_heigth_better, hitbox_width_better, object_x, object_y+object_height)||
	isInside(hitbox_x_better, hitbox_y_better, hitbox_heigth_better, hitbox_width_better, object_x+object_width, object_y+object_height))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void show_score (int *score)
{
	printText("Score:",0,0,RGBToWord(255,255,255),0);
	printNumber(*score,40,0,RGBToWord(255,255,255),0);
}

void showLives(uint16_t x, uint16_t y, int lives) {
	while (lives--) {
		putImage(x, y, 8, 8, heart, 0, 0); // change to heart sprite
		x = x - 10; // spacing the health indicators
	}
}

void displayHUD(uint16_t x, uint16_t y, int lives) {
	while (lives--) {
		fillRectangle(x, y, 8, 4, 255);
		x = x - 8;
	}
}

int rightPressed() {
	if ((GPIOB->IDR & (1 << 4)) == 0 || getchar() == 'd')
	{
		return 1;
	}
	else return 0;	
}

int leftPressed() {
	if ((GPIOB->IDR & (1 << 5)) == 0)
	{
		return 1;
	}
	else return 0;	
}

int upPressed() {
	if ((GPIOA->IDR & (1 << 8)) == 0)
	{
		return 1;
	}
	else return 0;	
}

int downPressed() {
	if ((GPIOA->IDR & (1 << 11)) == 0)
	{
		return 1;
	}
	else return 0;	
}

void moveSprite(uint16_t *x, uint16_t *y, int width, int height, const uint16_t *sprite, char direction) {
	uint16_t prevX = *x; 
	uint16_t prevY = *y; 

	// Increase x or y depending on direction
	switch (direction)
	{
	case 'R': // right
		(*x)++;
		break;
	case 'L': // left 
		(*x)--;
		break;
	case 'U': // up
		(*y)--;
		break;
	case 'D': // down
		(*y)++;
		break;

	default:
		break;
	}
	// Replace previous image
	fillRectangle(prevX, prevY, width, height, 0);
	prevX = *x;
	prevY = *y;
	// Place image in new location
	putImage(*x, *y, width, height, sprite, direction, 0); // direction is passing char to int NOT WORKING FIX THIS		
}

void spawnFish(uint16_t *x, uint16_t *y, int width, int height, const uint16_t *sprite, int *direction) {
	uint16_t prevX = *x; 
	uint16_t prevY = *y; 

	// Keeps fish in bounds of screen
	if ((*x) <= 0) {
		*direction = 0;
	}
	else if ((*x) >= 112) { // 128px - width of sprite
		*direction = 1;
	}
	
	// Right and left movement
	if ((*direction) == 0) {
		(*x)++;
	}
	else if ((*direction) == 1) {
		(*x)--;
	}
	// Handles sprite image display
	fillRectangle(prevX, prevY, width, height, 0);
	prevX = *x;
	prevY = *y;
	putImage(*x, *y, width, height, sprite, *direction, 0); 
}

void reset (int *score,int *lives,int *gamebegin, int *stage, int *fishcaught, int *abilities_used)
{
	*score = 0;
	*lives = 3;
	*gamebegin = 1;
	*stage = START_MENU;
	*fishcaught = 0;
	*abilities_used = 0;
}

void move_rocket(uint16_t *x, uint16_t *y, int width, int height, const uint16_t *sprite, int *stage, int*score, uint16_t fishx[], uint16_t fishy[], int*lives, int games_played, int fish_caught,int abilities_used)
{
	delay(100);
	int count = 0;
	uint16_t prevX = *x; 
    uint16_t prevY = *y; 
	uint16_t center_x = 0;
    uint16_t center_y = 0;
	uint16_t explosion_x;
    uint16_t explosion_y;

	while(1)
	{
		count++;
		if (count == 30000)
		{
			*y += 1;
			count = 0;
			center_x = *x + 4;
			center_y = *y + 4;
			explosion_x = center_x - 24;
			explosion_y = center_y - 24;


			fillRectangle(prevX, prevY, width, height, 0);
			prevX = *x;
			prevY = *y;
			putImage(*x, *y, 8,8,sprite,0,0); 


			if(upPressed() && *y > 50)
			{
				fillRectangle(explosion_x, explosion_y, 48,48,RGBToWord(255,255,255));

				for (int i = 0; i<3; i++)
				{
					if(collision(explosion_x,explosion_y,48,48,fishx[i],fishy[i],16,16))
					{
						if (i == 0)
						{
							*score += 250;
						}
						else if(i == 1)
						{
							*score += 500;
						}
						else if (i == 2)
						{
							*score += 1000;
						}
						print_serial(games_played,*lives,*score,fish_caught,abilities_used);
						fillRectangle(fishx[i],fishy[i],16,16,0);
						fillRectangle(explosion_x, explosion_y, 48,48,RGBToWord(255,255,255));
						randomise_fish(fishx,fishy,i);
					}
				}
				*y = 160;
				delay(250);
				*stage = BOAT_STAGE;
			}
		}
		else if(*y >= 160)
		{
			break;
		}
	}
	*stage = BOAT_STAGE;   
}

void print_serial (int games, int lives, int score, int fishcaught, int abilities_used)
{
	eputs("\rGames: ");
	printDecimal(games);
	eputs(" Lives: ");
	printDecimal(lives);
	eputs(" Score: ");
	printDecimal(score);
	eputs(" Fish Caught: ");
	printDecimal(fishcaught);
	eputs(" Abilities used: ");
	printDecimal(abilities_used);
}

void randomise_fish (uint16_t fishX[], uint16_t fishY[], int index)
{
	fishX[index] = rand() % (BOARDWIDTH+1);
	do
	{
		fishY[index] = rand() % (BOARDHEIGHT+1);
	} while (fishY[index] < 50 || fishY[index] > 144 || (fishY[index] > 54 && fishY[index] < 70) || (fishY[index] > 114 && fishY[index] < 130));
	
}
