#include <stm32f031x6.h>
#include "display.h"
#include "serial.h"
#include "sound.h"
#include "musical_notes.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//STAGES
#define START_MENU 0
#define BOAT_STAGE 1
#define BUCKET_STAGE 2
#define GAME_OVER 3
#define ABILITY 4

//SPRITE MAXIMUMS
#define MAX_FISHES 3
#define MAX_OBSTACLES 2

//DIMENSIONS
#define BOATWIDTH 48
#define BOATHEIGHT 31

#define BUCKETWIDTH 16
#define BUCKETHEIGHT 16

#define FISHWIDTH 16
#define FISHHEIGHT 16

#define OBSTACLEWIDTH 8
#define OBSTACLEHEIGHT 8

#define BOARDWIDTH 128
#define BOARDHEIGHT 160

#define ROCKETWIDTH 8
#define ROCKETHEIGHT 8

#define EXPLOSIONWIDTH 32
#define EXPLOSIONHEIGHT 32

//SETUP DISPLAY AND PORTS FUNCTION SIGNATURES
void initClock(void);
void initSysTick(void);
void initSerial(void);
void initSound(void);
void SysTick_Handler(void);
void delay(volatile uint32_t dly);
void setupIO();
int isInside(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t px, uint16_t py);
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);

//BUTTON FUNCTION SIGNATURES
int leftPressed(void);
int rightPressed(void);
int upPressed(void);
int downPressed(void);
int abilityPressed(void); 

//GAME LOGIC FUNCTION SIGNATURES
int collision (uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, int, int);
void randomise_fish (uint16_t [], uint16_t [], int );
void reset (int *,int *,int *, int *, int *, int *, uint16_t*, uint16_t*, int*, uint16_t[],uint16_t[]);
void add_score (int*, int);
void switch_stage (int*,int*,int);

//GAME HUD AND SERIAL FUNCTION SIGNATURES
void show_score (int*);
void showLives(uint16_t, uint16_t, int);
void print_serial (int, int, int, int,int);
void ascii (void);
void intro_screen (void);
void end_screen (int);
void ability_ready (int);
void print_grade (int);

//SPRITE MOVEMENT FUNCTION SIGNATURES
void spawnFish(uint16_t*, uint16_t*, int, int, const uint16_t*, const uint16_t*, const uint16_t*, int*, int);
void spawnObstacle(uint16_t *, uint16_t *, int, int, const uint16_t *, int *);

//CHARACTER MOVEMENT FUNCTION SIGNATURES
void move_right (uint16_t*,int*,int,int,int,int*);
void move_left (uint16_t*,int*,int,int,int*);
void move_down (uint16_t*, int*,int,int);
void move_up (uint16_t*,int*,int);

//SOUND FUNCTION SIGNATURE
void playChime(uint32_t*, uint32_t*, int);

//FOR SYSTICK HANDLER
volatile uint32_t milliseconds;

//SPRITE MAPS
const uint16_t boat1[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65296,65296,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23552,31744,23552,65296,23552,65296,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19201,12058,0,13576,0,5384,13576,23552,65296,23552,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11009,3866,0,0,0,13576,13576,13576,3866,65535,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7663,7663,7663,3866,0,0,0,0,0,3866,65535,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,7663,7663,60169,27393,0,0,51977,49151,63950,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,7663,7663,2817,51977,3866,0,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,7663,0,51977,0,49151,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49151,65535,32517,32517,32517,65535,65535,7663,0,51977,0,49151,49151,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,32517,57343,23556,65535,65535,24047,0,51977,0,49151,40959,57343,7663,47566,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,32517,15364,15364,65535,7663,63950,0,27393,0,0,49151,57343,49151,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,32517,65535,32517,65535,7172,65535,7663,0,0,2817,0,65535,49151,65535,7663,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49151,65535,57343,7172,32517,7172,65535,65535,7663,0,0,19201,0,49151,49151,49151,57343,7663,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63950,65535,65535,65535,65535,32517,65535,65535,65535,7663,63950,0,27393,0,65535,49151,49151,57343,49151,49151,55758,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24047,7663,7663,24047,7663,65535,65535,7663,65535,7663,63950,0,2817,0,49151,49151,49151,49151,49151,7663,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27393,35593,27393,11009,7663,15855,7663,15855,7663,7663,7663,7663,63950,0,27393,65535,65535,49151,49151,7663,7663,7663,7663,24047,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35593,0,0,0,0,0,0,0,0,0,0,0,0,11009,27393,3866,11009,11009,2817,2817,7663,7663,0,51977,7663,7663,24047,7663,7663,19201,11009,51977,60169,12058,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,27393,0,0,27393,11009,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2817,51977,51977,43785,14636,27393,11009,3866,11009,19201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15364,7172,32517,32517,32517,7172,7172,7172,32517,0,0,0,0,0,14636,0,0,14636,0,0,0,0,0,0,0,60169,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19482,19482,19482,4387,4387,4387,4387,4387,32517,7172,0,0,0,0,55909,55909,14636,39525,0,0,0,0,48430,0,0,14636,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27674,19482,3098,3866,3866,3866,20250,28442,4387,32517,7172,7172,0,0,64101,3866,55909,3866,0,0,0,48430,53644,0,0,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19482,7663,7663,3866,3866,15855,3866,7663,3866,4387,32517,7172,7172,39525,64101,55909,55909,0,0,48430,0,53644,0,0,27393,0,0,0,0,0,0,0,0,0,0,0,0,19482,19482,19482,19482,0,0,0,0,0,0,0,19482,24047,3866,15855,7663,3866,3866,3866,3866,4387,4387,4387,7172,7172,24325,32517,32517,13908,13908,7172,7172,53644,32517,32517,7172,7172,32517,32517,32517,7172,7172,7172,32517,32517,32517,7172,19482,19482,0,0,0,0,0,0,0,0,0,0,19482,3866,3866,3866,3866,12058,3866,3866,3866,3866,3866,3866,3866,4387,4387,4387,4387,4387,12579,12579,4387,4492,3866,3866,4387,4387,20771,4387,4387,4387,3866,4387,4387,4387,4387,20771,19482,0,0,0,0,0,0,0,0,0,0,0,19482,3866,3866,3866,3866,3866,3866,3866,20250,3866,3866,3866,3866,3866,3866,3866,3866,3866,3866,12058,3866,45452,3866,7172,3866,32517,3866,7172,3866,32517,3866,7172,3866,7172,19482,19482,0,0,0,0,0,0,0,0,0,0,0,0,19482,19482,19482,19482,19482,19482,19482,19482,19482,3866,3866,20250,3866,12058,20250,3866,12058,3866,3866,12058,3866,53644,3866,3866,3866,20250,20250,12058,12058,3866,3866,3866,3866,12058,19482,0,0,0,0,0,0,0,0,0,0,57343,65535,65535,65535,65535,19482,19482,19482,19482,19482,19482,3098,19482,19482,19482,19482,19482,19482,19482,19482,19482,19482,19482,19482,21669,19482,19482,19482,3098,19482,19482,19482,19482,27674,19482,19482,19482,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,57343,65535,65535,57343,65535,49151,65535,65535,65535,65535,65535,65535,65535,19482,19482,27674,65535,65535,19482,65535,37260,65535,49151,65535,19482,65535,65535,19482,19482,65535,27674,19482,65535,65535,65535,65535,10750,0,0,0,0,0,0,0,65535,65535,57343,65535,65535,65535,57343,49151,65535,10750,49151,65535,65535,10750,10750,10750,65535,65535,49151,49151,10750,65535,65535,50348,58540,58540,10750,65535,65535,65535,57343,65535,10750,10750,65535,65535,65535,65535,65535,10750,0,0,0,0,0,0,0,0,0,0,42469,58853,58853,58853,10750,58853,58853,58853,58853,42469,58853,58853,10750,58853,58853,58853,58853,10750,10750,10750,42156,58540,58540,58540,58853,58853,58853,58853,10750,2558,58853,42469,58853,58853,65535,58853,58853,0,0,0,0,0,0,0,0,0,0,0,0,0,50661,58853,58853,58853,58853,58853,58853,0,58853,58853,58853,58853,58853,58853,58853,58853,58853,58853,58853,10750,58853,58853,58853,58853,58853,50661,58853,58853,58853,0,0,0,10750,58853,0,0,0,0,0,0,0,};
const uint16_t boat2[]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65296,23552,65296,19201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27393,3866,0,5384,0,57104,23552,15360,23552,23552,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2817,3866,13576,7168,23552,13576,0,13576,3866,65535,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7663,7663,7663,3866,13576,13576,0,0,0,12058,65535,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,7663,7663,60169,19201,0,0,51977,65535,63950,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,7663,7663,11009,51977,3866,0,7663,47566,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,65535,65535,65535,49151,65535,15855,0,51977,0,49151,24047,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49151,65535,32517,32517,32517,57343,65535,7663,0,51977,0,49151,49151,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,32517,57343,7172,65535,49151,7663,0,51977,0,40959,49151,57343,7663,47566,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65535,65535,32517,7172,7172,65535,24047,63950,0,11009,0,0,49151,57343,49151,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,57343,65535,16133,65535,32517,65535,7172,65535,7663,0,0,2817,0,49151,65535,49151,7663,7663,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,49151,65535,65535,7172,32517,7172,49151,65535,7663,0,0,19201,0,65535,49151,49151,57343,7663,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,63950,65535,65535,65535,65535,32517,65535,49151,65535,7663,63950,0,27393,0,49151,49151,65535,57343,40959,49151,63950,63950,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7663,7663,7663,7663,7663,65535,65535,15855,65535,15855,63950,0,2817,0,49151,49151,49151,49151,49151,7663,15855,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27393,35593,27393,11009,7663,24047,7663,7663,7663,7663,7663,32239,63950,0,11009,65535,65535,49151,49151,7663,7663,7663,7663,7663,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35593,0,0,0,0,0,0,0,0,0,0,0,0,11009,11009,3866,11009,11009,2817,2817,7663,7663,0,51977,7663,15855,24047,7663,15855,19201,11009,51977,43785,3866,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,27393,0,0,27393,27393,0,0,0,0,0,0,0,14636,0,0,14636,0,0,0,2817,51977,60169,43785,14636,27393,2817,3866,11009,19201,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,23556,15364,32517,16133,32517,7172,7172,7172,32517,0,0,0,0,0,55909,55909,31020,55909,0,0,0,0,48430,0,0,60169,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19482,27674,19482,4387,4387,4387,4387,4387,16133,7172,0,0,0,0,55909,3866,55909,12058,0,0,0,48430,53644,0,0,14636,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,27674,19482,19482,20250,3866,3866,12058,3866,4387,32517,23556,7172,0,0,55909,55909,55909,55909,0,0,48430,0,21669,0,0,27393,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,19482,7663,63950,3866,3866,63950,3866,63950,3866,12579,32517,7172,23556,0,5716,13908,0,5716,13908,0,0,21669,0,0,27393,0,0,0,0,0,0,0,0,0,0,0,0,19482,19482,19482,19482,0,0,0,0,0,0,0,19482,7663,3866,63950,7663,3866,3866,3866,3866,4387,4387,4387,15364,7172,32517,32517,24325,24325,32517,7172,7172,37260,32517,32517,15364,7172,32517,32517,32517,7172,7172,7172,32517,32517,32517,7172,19482,19482,0,0,0,0,0,0,0,0,0,0,19482,3866,20250,20250,3866,12058,3866,3866,3866,20250,3866,3866,3866,4387,4387,4387,4387,20771,12579,4387,12579,21669,3866,12058,4387,4387,4387,4387,4387,4387,3866,4387,4387,4387,4387,4387,19482,19482,0,0,0,0,0,0,0,0,0,0,19482,3866,3866,3866,3866,3866,3866,3866,20250,3866,3866,12058,12058,3866,3866,3866,3866,3866,3866,3866,3866,37260,3866,7172,3866,32517,3866,7172,12058,32517,3866,7172,3866,7172,19482,19482,19482,0,0,0,0,0,0,0,0,65535,65535,65535,19482,19482,19482,19482,19482,19482,19482,3098,19482,3866,3866,3866,3866,3866,28442,3866,3866,3866,20250,12058,3866,53644,3866,3866,3866,3866,3866,3866,12058,12058,3866,12058,12058,3866,19482,0,0,0,0,0,0,0,0,57343,65535,65535,65535,65535,65535,65535,49151,57343,65535,65535,65535,65535,65535,65535,57343,49151,65535,65535,19482,19482,65535,65535,65535,19482,65535,21669,65535,49151,58853,65535,57343,65535,19482,19482,65535,27674,3098,49151,57343,65535,58853,58853,0,0,0,0,0,0,65535,65535,65535,65535,65535,65535,65535,65535,65535,49151,65535,65535,65535,65535,58853,27134,65535,58853,65535,65535,65535,10750,34277,65535,50348,37260,58540,50661,58853,10750,65535,65535,65535,10750,42469,65535,65535,65535,65535,10750,34277,0,0,0,0,0,0,0,0,0,0,42469,58853,10750,10750,42469,10750,10750,10750,58853,2558,10750,27134,42469,58853,50661,42469,10750,10750,42469,42469,50661,58540,58540,58540,58540,34277,10750,10750,58853,58853,58853,27134,10750,58853,58853,2558,42469,0,0,0,0,0,0,0,0,0,0,0,0,0,42469,34277,58853,10750,10750,10750,42469,0,58853,58853,58853,42469,58853,42469,42469,58853,42469,58853,0,34277,58540,42469,42469,42469,58853,50661,10750,2558,58853,0,0,0,58853,42469,0,0,0,0,0,0,0,};
const uint16_t bucket[] = {0,0,0,0,25880,25880,25880,25880,25880,25880,25880,25880,0,0,0,0,0,0,25880,25880,35113,35113,28202,28202,28202,28202,28986,28986,25880,25880,0,0,0,25880,35113,35113,35113,60556,60556,29117,29117,37318,37318,28986,28986,28986,25880,0,0,25880,35113,60556,60556,60556,60556,29117,29117,29117,37318,37318,37318,28986,25880,0,0,25880,25880,25880,60556,60556,29117,29117,29117,37318,37318,37318,25880,25880,25880,0,0,25880,18457,35113,25880,25880,25880,25880,25880,37318,25880,25880,28986,28986,25880,0,0,25880,18457,35113,35113,18457,28202,28202,18457,28202,28202,18457,28986,28986,25880,0,0,25880,52323,13212,35113,18457,28202,28202,18457,28202,28202,18457,13212,52323,25880,0,0,25880,18457,35113,13212,13212,52323,52323,52323,52323,13212,13212,28986,28986,25880,0,0,25880,18457,35113,35113,18457,28202,28202,18457,28202,28202,18457,28986,28986,25880,0,0,25880,18457,35113,35113,18457,28202,28202,18457,28202,28986,18457,28986,28986,25880,0,0,0,18457,35113,35113,18457,28202,28202,18457,28202,28986,18457,28986,28986,0,0,0,0,52323,52323,35113,18457,28202,28202,18457,28202,28986,18457,13212,13212,0,0,0,0,25880,35113,52323,52323,13212,13212,52323,52323,52323,13212,28986,25880,0,0,0,0,0,25880,35113,18457,28202,28202,18457,28202,28986,18457,25880,0,0,0,0,0,0,0,25880,25880,25880,25880,25880,25880,25880,25880,0,0,0,0,};
const uint16_t fish [] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,59168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,59168,59168,59168,0,0,0,0,0,0,59168,59168,0,0,0,0,0,0,59168,59168,59168,0,0,0,0,0,0,59168,59168,0,0,0,59168,59168,60301,34404,60301,59168,59168,0,0,0,0,59168,59168,59168,0,60301,34404,60301,34404,60301,34404,34404,34404,60301,59168,0,0,0,59168,59168,59168,58443,34404,58443,34404,34404,34404,58443,60301,34404,34404,59168,0,59168,59168,59168,58443,34404,58443,34404,58443,34404,59168,34404,58443,0,34404,59168,0,59168,59168,0,0,59168,58443,58443,59168,59168,59168,58443,34404,34404,34404,59168,59168,0,0,0,0,0,0,59168,59168,59168,59168,59168,59168,59168,0,0,0,0,0,0,0,0,0,0,59168,59168,0,0,0,0,0,0,0,0,0,0,0,0,0,59168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
const uint16_t fish2[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33377,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,33377,33377,33377,0,0,0,0,0,0,33377,33377,0,0,0,0,0,0,33377,33377,33377,0,0,0,0,0,0,33377,33377,0,0,0,33377,33377,63485,28900,63485,33377,33377,0,0,0,0,33377,33377,33377,0,63485,28900,63485,28900,63485,28900,28900,28900,63485,33377,0,0,0,33377,33377,33377,43964,28900,43964,28900,28900,28900,43964,63485,28900,28900,33377,0,33377,33377,33377,43964,28900,33377,28900,43964,28900,33377,28900,43964,0,28900,33377,0,33377,33377,0,0,33377,33377,33377,33377,33377,33377,43964,28900,28900,28900,33377,33377,0,0,0,0,0,0,33377,33377,33377,33377,33377,33377,33377,0,0,0,0,0,0,0,0,0,0,33377,33377,0,0,0,0,0,0,0,0,0,0,0,0,0,33377,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
const uint16_t fish3[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1073,0,0,0,0,0,0,0,0,0,1073,0,0,0,0,0,0,1073,1073,1073,0,0,0,0,0,0,0,1073,1073,0,0,0,0,0,1073,1073,1073,0,0,0,0,0,0,0,1073,1073,0,0,0,1073,39968,64016,39968,1073,1073,0,0,0,0,0,1073,1073,1073,0,39968,64016,64016,39968,64016,64016,64016,39968,1073,0,0,0,0,1073,1073,1073,64016,22024,64016,22024,64016,22024,39968,22024,64016,1073,0,0,1073,1073,1073,22024,22024,64016,22024,64016,1073,64016,22024,0,22024,1073,0,0,1073,1073,0,0,1073,22024,1073,1073,1073,22024,22024,22024,22024,1073,1073,1073,0,0,0,0,0,1073,1073,1073,1073,1073,1073,1073,0,0,0,0,0,0,0,0,0,0,1073,1073,0,0,0,0,0,0,0,0,0,0,0,0,0,1073,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
const uint16_t bucketFish[] = {0,0,0,0,25880,25880,25880,25880,25880,25880,60301,59168,59168,0,0,0,0,0,25880,25880,35113,59168,59168,60301,59168,34404,60301,34404,34404,59168,0,0,0,25880,35113,35113,44006,44006,60301,60301,34404,58443,60301,0,34404,59168,25880,0,0,25880,35113,44006,44006,60301,60301,60301,34404,58443,58443,34404,34404,44006,25880,0,0,25880,25880,44006,44006,60301,34404,34404,34404,34404,58443,34404,44006,25880,25880,0,0,25880,18457,35113,44006,44006,44006,44006,44006,44006,44006,44006,44006,28986,25880,0,0,25880,18457,35113,35113,18457,44006,28202,18457,44006,44006,18457,28986,28986,25880,0,0,25880,52323,13212,35113,18457,28202,28202,18457,28202,44006,18457,13212,52323,25880,0,0,25880,18457,35113,13212,13212,44006,52323,52323,52323,44006,13212,28986,28986,25880,0,0,25880,18457,35113,35113,18457,44006,28202,18457,28202,28202,18457,28986,28986,25880,0,0,25880,18457,35113,35113,18457,44006,28202,18457,28202,28986,18457,28986,28986,25880,0,0,0,18457,35113,35113,18457,28202,28202,18457,28202,44006,18457,28986,28986,0,0,0,0,52323,52323,35113,18457,28202,28202,18457,28202,28986,18457,13212,13212,0,0,0,0,25880,35113,52323,52323,13212,13212,52323,52323,52323,13212,28986,25880,0,0,0,0,0,25880,35113,18457,28202,28202,18457,28202,28986,18457,25880,0,0,0,0,0,0,0,25880,25880,25880,25880,25880,25880,25880,25880,0,0,0,0,};
const uint16_t heart[]={0,0,0,0,0,0,0,0,0,65535,65535,0,0,65535,65535,0,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,65535,0,65535,65535,65535,65535,65535,65535,0,0,0,65535,65535,65535,65535,0,0,0,0,0,65535,65535,0,0,0,};
const uint16_t mine_bomb[]={0,0,4368,4368,4368,4368,0,0,0,4368,4368,4368,4368,4368,4368,0,4368,4368,4368,40224,40224,4368,4368,4368,4368,4368,40224,40224,40224,40224,4368,4368,4368,4368,40224,40224,40224,40224,4368,4368,4368,4368,4368,40224,40224,4368,4368,4368,0,4368,4368,4368,4368,4368,4368,0,0,0,4368,4368,4368,4368,0,0,};
const uint16_t rocket[] = {0,0,0,29332,61572,0,0,0,0,0,6912,6870,56286,6912,0,0,0,0,6870,7936,7936,56286,0,0,0,0,56286,56286,6870,6870,0,0,0,0,6912,6870,56286,6912,0,0,0,0,56286,7936,6912,56286,0,0,0,0,15620,65287,65303,15620,0,0,0,0,0,32516,15620,0,0,0,};
const uint16_t explosion [] = {0,0,0,0,0,0,0,65281,65281,65281,65281,65281,65281,65281,65281,65281,65281,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65281,65281,24323,24323,24323,24323,24323,24323,24323,24323,65281,65281,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65281,65281,24323,24323,24323,24323,24323,40709,40709,40709,24323,24323,24323,65281,65281,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65281,24323,24323,40709,40709,40709,40709,40709,40709,24323,24323,24323,24323,65281,65281,65281,65281,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65281,24323,40709,40709,40709,40709,40709,40709,40709,24323,24323,24323,24323,24323,24323,65281,0,0,0,0,0,0,0,0,0,0,0,65281,65281,0,0,0,65281,24323,40709,40709,40709,40709,40709,40709,40709,40709,24323,24323,24323,24323,65281,0,0,0,0,0,0,0,0,0,0,0,0,65281,24323,65281,0,0,65281,24323,40709,40709,40709,32519,32519,40709,40709,40709,40709,40709,40709,24323,65281,0,0,0,65281,0,0,0,0,0,0,0,65281,65281,24323,24323,65281,0,0,65281,24323,40709,32519,32519,32519,32519,32519,40709,40709,40709,24323,65281,0,0,0,65281,65281,65281,0,0,0,0,0,0,65281,24323,24323,24323,24323,65281,0,65281,24323,40709,32519,32519,65535,65535,32519,32519,40709,40709,24323,65281,0,0,65281,24323,24323,65281,65281,0,0,0,0,0,65281,24323,24323,40709,40709,24323,65281,24323,40709,32519,65535,65535,65535,65535,32519,32519,40709,24323,65281,0,0,0,65281,24323,24323,24323,65281,65281,0,0,0,0,65281,24323,24323,40709,40709,40709,24323,40709,32519,65535,65535,61572,61572,65535,65535,32519,40709,24323,65281,0,0,65281,24323,40709,24323,24323,24323,65281,65281,0,0,0,65281,24323,24323,40709,40709,40709,40709,32519,65535,65535,6912,56286,56286,6912,65535,32519,32519,40709,24323,65281,0,65281,24323,40709,24323,24323,24323,24323,65281,65281,0,0,65281,24323,24323,40709,40709,40709,32519,32519,65535,65535,56286,6912,6912,56286,65535,65535,65535,32519,40709,24323,65281,24323,40709,40709,40709,24323,24323,24323,24323,65281,0,0,65281,24323,24323,24323,24323,40709,32519,32519,32519,65535,65535,65535,65535,65535,65535,65535,65535,65535,32519,40709,24323,40709,40709,40709,40709,40709,24323,24323,24323,65281,0,0,65281,24323,24323,24323,24323,40709,40709,32519,32519,32519,65535,65535,65535,65535,6912,56286,56286,6912,65535,32519,40709,32519,32519,40709,40709,40709,24323,24323,24323,65281,0,0,65281,65281,24323,24323,24323,40709,40709,32519,32519,32519,65535,65535,65535,65535,56286,6912,6912,56286,65535,65535,32519,32519,32519,40709,40709,40709,24323,24323,24323,65281,0,0,0,65281,24323,24323,24323,24323,40709,32519,32519,65535,65535,61572,61572,65535,65535,65535,65535,65535,65535,65535,32519,32519,32519,32519,40709,40709,24323,24323,24323,65281,0,0,0,65281,24323,24323,65281,65281,24323,40709,32519,65535,61572,61572,65535,65535,65535,65535,65535,61572,65535,65535,32519,32519,32519,32519,40709,40709,24323,24323,24323,65281,0,0,0,65281,24323,65281,0,0,65281,24323,40709,32519,65535,65535,65535,32519,32519,32519,65535,61572,61572,65535,65535,32519,40709,32519,40709,40709,24323,24323,24323,65281,0,0,0,65281,65281,0,0,65281,24323,40709,32519,65535,65535,65535,32519,32519,40709,32519,65535,65535,61572,65535,32519,40709,24323,40709,40709,40709,24323,24323,65281,65281,0,0,0,65281,65281,0,0,65281,24323,40709,32519,32519,32519,32519,32519,40709,24323,40709,32519,65535,65535,32519,40709,24323,65281,24323,24323,24323,24323,24323,65281,0,0,0,0,65281,0,0,0,65281,24323,40709,32519,32519,32519,32519,40709,24323,65281,24323,40709,32519,32519,32519,40709,24323,65281,65281,65281,24323,24323,24323,65281,0,0,0,0,0,0,0,65281,24323,40709,40709,40709,40709,32519,40709,24323,65281,0,65281,24323,40709,32519,32519,40709,24323,65281,0,0,65281,24323,24323,65281,0,0,0,0,0,0,65281,24323,24323,24323,24323,40709,40709,40709,40709,24323,65281,0,65281,24323,40709,32519,40709,40709,40709,24323,65281,0,0,65281,24323,65281,0,0,0,0,0,65281,65281,24323,24323,24323,24323,24323,40709,40709,24323,65281,0,0,65281,24323,40709,40709,40709,40709,40709,40709,24323,65281,0,0,65281,65281,0,0,0,0,0,0,65281,65281,24323,24323,24323,24323,24323,40709,24323,65281,0,0,65281,24323,40709,40709,40709,40709,40709,40709,24323,65281,0,0,0,0,0,0,0,0,0,0,0,65281,65281,65281,24323,24323,24323,24323,24323,65281,0,0,65281,24323,24323,24323,24323,24323,24323,24323,24323,65281,0,0,0,0,0,0,0,0,0,0,0,0,0,65281,65281,24323,24323,24323,65281,0,0,0,0,65281,24323,24323,24323,24323,24323,24323,65281,65281,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65281,65281,65281,24323,65281,0,0,0,0,65281,24323,24323,24323,24323,24323,65281,65281,65281,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,65281,65281,65281,0,0,0,0,65281,65281,65281,65281,65281,65281,65281,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

int main()
{
	//SEED RANDOM NUMBERS USING TIME
	srand(time(NULL));

	//GAME LOGIC VARIABLES
	int stage = START_MENU;
	int score = 0;
    int lives = 3;
	int toggle = 0;
	int count = 0;
	int currentFish = -1;
	int games_played = 1;
	int fish_caught = 0;
	int abilities_used = 0;
	int new_stage = 1;
	char restart;

	//FISH 
    uint16_t fishX[MAX_FISHES] = {0, 0, 0};
    uint16_t fishY[MAX_FISHES] = {0, 0, 0};
	int direction[] = {0, 1, 1};

	//OBSTACLES
	uint16_t obstacleX[] = {0,129};
	uint16_t obstacleY[] = {70,130}; 
	int obsDir[] = {0, 1, 0};

	//BUCKET
	uint16_t bucket_x = 40;
	uint16_t bucket_y = 40;
	uint16_t bucket_oldx = 0;
	uint16_t bucket_oldy = 0;
	int bucket_horizontal_moved = 0;
	int bucket_vertical_moved = 0;
	int bucket_invert = 0;

	//BOAT
	uint16_t boat_x = 64 -(BOATWIDTH/2);
	uint16_t boat_y = 10;
	uint16_t boat_oldx = boat_x;
	int boat_horizontal_moved = 0;
	int boat_invert = 0;

	//ROCKET
	uint16_t rocket_x = 0;
	uint16_t rocket_y = 0;
	uint16_t rocket_oldx = 0;
	uint16_t rocket_oldy = 0;
	int ability = 0;

	//EXPLOSION(ROCKET)
	uint16_t explosion_center_x;
	uint16_t explosion_center_y;
	uint16_t explosion_x;
	uint16_t explosion_y;

	// SOUND/MUSIC
	uint32_t soundtrackNotes[] = {};
	uint32_t soundtrackDurs[] = {};
	uint32_t notesCatch1[] = {C5, E5, G5};
	uint32_t dursCatch1[] = {200, 200, 200};
	uint32_t notesCatch[] = {E6, G6, E7};
	uint32_t dursCatch[] = {150, 150, 150};
	uint32_t notesDamage[] = {E6, DS6_Eb6, D6};
	uint32_t dursDamage[] = {100, 100, 100};
	uint32_t notesOver[] = {E6, C6, D6, G5}; 
	uint32_t dursOver[] = {500, 250, 500, 750}; 
	int noteCount = 3;

	initClock();
	initSysTick();
	setupIO();
	initSerial();
	initSound();

	// GAME LOOP
	while (1)
	{

//START MENU START
		while (stage == START_MENU)
		{
			//DISPLAY BEGINNING TEXT
			if (new_stage) 
			{
				//initialise or reset scores and fish positions
				reset(&score, &lives, &new_stage, &stage,&fish_caught,&abilities_used, &boat_x, &boat_y, &boat_invert,fishX,fishY);

				//show title in serial
				ascii();

				//show intro on display
				intro();

				new_stage = 0;
			}

			//FLASHING TEXT
			count++;
			if (count > 40 && count <= 80) 
			{
				//flash text
				printText("Press any button", 10, 140, 255, 0);
				printText("to start", 37, 150, 255, 0);
			} 
			else if (count > 80) 
			{
				//cover text and loop back to 0
				fillRectangle(0,140,128,20,0);
				count = 0;
			}

			//START GAME ON BUTTON PRESS
			if (rightPressed() || leftPressed() || upPressed() || downPressed() || abilityPressed)
			{
				print_serial(games_played,lives,score,fish_caught,abilities_used);
				fillRectangle(0, 0, BOARDWIDTH, BOARDHEIGHT, 0);
				switch_stage(&new_stage,&stage,BOAT_STAGE);
			}

			delay(16);
		}
//START MENU END

//BOAT STAGE START
		while (stage == BOAT_STAGE)
		{
			//SHOW HUD
			if (new_stage)
			{
				showLives(120, 0, lives);
				show_score(&score);
				putImage(boat_x, boat_y, BOATWIDTH, BOATHEIGHT, boat1, boat_invert, 0);
				ability_ready(ability);
				new_stage = 0;
			}

			//SPAWN ALL FISH
			for (int i = 0; i < MAX_FISHES; i++)
			{
				spawnFish(&fishX[i], &fishY[i], 16, 16, fish,fish2,fish3, &direction[i],i);
			}

			//SPAWN ALL OBSTACLES
			for (int i = 0; i < MAX_OBSTACLES; i++)
			{
				spawnObstacle(&obstacleX[i], &obstacleY[i], 8, 8, mine_bomb, &obsDir[i]);
			}

			//ABILITY (ONLY IF READY)
			if (abilityButton() == 1 && ability >= 3) 
			{
				switch_stage(&new_stage,&stage,ABILITY);
			}

			//RELEASE BUCKET
			if (downPressed() == 1) {
				//set bucket to come from boat
				bucket_x = boat_x + (BOATWIDTH/2) - (BUCKETWIDTH/2);
				bucket_y = 40;

				//prevent unwanted fill rectangle
				bucket_oldx = bucket_x;
                bucket_oldy = bucket_y;

				switch_stage(&new_stage,&stage,BUCKET_STAGE);
			}

			
			//BOAT MOVEMENT
			boat_horizontal_moved = 0;
			if (rightPressed() == 1)
			{
				move_right(&boat_x, &boat_horizontal_moved, BOARDWIDTH, BOATWIDTH,1,&boat_invert);
			}
			if (leftPressed() == 1) {
				move_left(&boat_x, &boat_horizontal_moved, 0, 1, &boat_invert);
			}

			//Redraw every time so boat always bobs up and down
			fillRectangle(boat_oldx, boat_y, BOATWIDTH, BOATHEIGHT, 0);
			boat_oldx = boat_x;
			if (count > 40 && count <= 80)
			{
				putImage(boat_x, boat_y, BOATWIDTH, BOATHEIGHT, boat1, boat_invert, 0);
			}
			else if (count > 80)
			{
				putImage(boat_x, boat_y, BOATWIDTH, BOATHEIGHT, boat2, boat_invert, 0);
			}

			delay(16);
		}
//BOAT STAGE END

//BUCKET STAGE START
		while (stage == BUCKET_STAGE)
        {
			//SPAWN ALL FISH NOT IN BUCKET
			for (int i = 0; i < MAX_FISHES; i++)
			{
				if (i != currentFish) { // Don't show fish currently in bucket
					spawnFish(&fishX[i], &fishY[i], FISHWIDTH, FISHHEIGHT, fish,fish2,fish3, &direction[i], i);
				}
			}

			//SPAWN ALL OBSTACLES
			for (int i = 0; i < MAX_OBSTACLES; i++)
			{
				spawnObstacle(&obstacleX[i], &obstacleY[i], OBSTACLEWIDTH, OBSTACLEHEIGHT, mine_bomb,&obsDir[i]);
			}

            //BUCKET MOVEMENT
            bucket_horizontal_moved = 0;
			bucket_vertical_moved = 0;
			if (rightPressed() == 1)
			{
				move_right(&bucket_x, &bucket_horizontal_moved, BOARDWIDTH, BUCKETWIDTH,0,&bucket_invert);
			}
			if (leftPressed() == 1)
			{
				move_left(&bucket_x, &bucket_horizontal_moved, 0,0,&bucket_invert); 
			}
			if (upPressed() == 1)
			{
				move_up(&bucket_y, &bucket_vertical_moved, boat_y+BOATHEIGHT);   
			}
			if (downPressed() == 1)
			{
				move_down(&bucket_y, &bucket_vertical_moved, BOARDHEIGHT, BUCKETHEIGHT);  
			}
            
            //REDRAW IMAGE IF MOVED
            if (bucket_vertical_moved == 1 || bucket_horizontal_moved == 1)
            {
                // only redraw if there has been some movement
                fillRectangle(bucket_oldx, bucket_oldy, BUCKETWIDTH, BUCKETHEIGHT, 0);
                bucket_oldx = bucket_x;
                bucket_oldy = bucket_y;

				//DRAW BUCKET EMPTY/FULL
				if (currentFish != 1) {
					putImage(bucket_x, bucket_y, BUCKETWIDTH, BUCKETHEIGHT, bucketFish, 0, 0);
				}
				else {
					putImage(bucket_x, bucket_y, BUCKETWIDTH, BUCKETHEIGHT, bucket, 0, 0);
				}
            }
            
            //FISH COLLISION
			for (int i = 0; i < 3; i++)
			{
				if (collision(bucket_x,bucket_y,BUCKETWIDTH,BUCKETHEIGHT,fishX[i]-2,fishY[i]-2,FISHWIDTH+4,FISHHEIGHT+4) && (currentFish != 0))
				{
					currentFish = i;
					fillRectangle(fishX[i], fishY[i], FISHWIDTH, FISHHEIGHT, 0); //draw over fish
					putImage(bucket_x, bucket_y, BUCKETWIDTH, BUCKETHEIGHT, bucket, 0, 0);//draw bucket again
					playChime(notesCatch, dursCatch, noteCount);
				}
			}

			//OBSTACLE COLLISION
			for (int i = 0; i < MAX_OBSTACLES; i++)
			{
				if (collision(bucket_x,bucket_y,BUCKETWIDTH,BUCKETHEIGHT,obstacleX[i],obstacleY[i],OBSTACLEWIDTH,OBSTACLEHEIGHT))
				{	
					lives--;
					currentFish = -1;

					//delete bucket and draw explosion
					fillRectangle(bucket_oldx, bucket_oldy, BUCKETWIDTH,BUCKETHEIGHT, 0);
					putImage(obstacleX[i]-12,obstacleY[i]-12,EXPLOSIONWIDTH,EXPLOSIONHEIGHT,explosion,0,0);
					print_serial(games_played,lives,score,fish_caught,abilities_used);

					//damage sound
					playChime(notesDamage, dursDamage, 3);
					delay(500);

					//Switch stage
					if(lives == 0)
					{
						switch_stage(&new_stage,&stage,GAME_OVER);
					}
					else
					{
						switch_stage(&new_stage,&stage,BOAT_STAGE);
					}
				}
			}

			//RETURN TO BOAT COLLISION
			if (collision(boat_x, boat_y+10, BOATHEIGHT, BOATWIDTH, bucket_x, bucket_y, BUCKETHEIGHT, BUCKETWIDTH) && (currentFish != 1))
			{
				fish_caught++;
				ability++;

				fillRectangle(bucket_oldx, bucket_oldy, BUCKETHEIGHT, BUCKETWIDTH, 0);
				add_score(&score,currentFish);
				print_serial(games_played,lives,score,fish_caught,abilities_used);
				randomise_fish(fishX,fishY,currentFish);

				currentFish = -1;
				switch_stage(&new_stage,&stage,BOAT_STAGE);
			}
            
            delay(16);
        }
//BUCKET STAGE END

//ABILITY START
		while (stage == ABILITY)
		{
			if (new_stage)
			{
				ability = 0;
				//Clear screen & redraw all except obstacles
				fillRectangle(0, 0, BOATWIDTH, BOARDHEIGHT, 0);
				showLives(120, 0, lives);
				show_score(&score);
				putImage(boat_x, boat_y, BOATWIDTH, BOATHEIGHT, boat1, boat_invert, 0);
				ability_ready(ability);
				putImage(fishX[0],fishY[0],FISHWIDTH,FISHHEIGHT,fish,0,direction[0]);
				putImage(fishX[1],fishY[1],FISHWIDTH,FISHHEIGHT,fish,0,direction[1]);
				putImage(fishX[2],fishY[2],FISHWIDTH,FISHHEIGHT,fish,0,direction[2]);
				abilities_used ++;

				//set rocket to come from boat
				rocket_x = boat_x + (BOATWIDTH/2) - (BUCKETWIDTH/2);
				rocket_y = 40;
			}

			count++;
			if (count == 30000)
			{
				rocket_y += 1;
				count = 0;

				//REDRAW ROCKET
				fillRectangle(rocket_oldx, rocket_oldy, ROCKETWIDTH, ROCKETHEIGHT,0);
				rocket_oldx = rocket_x;
				rocket_oldy = rocket_y;
				putImage(rocket_x,rocket_y,ROCKETWIDTH,ROCKETHEIGHT,rocket,0,0);


				if(abilityButton() && rocket_y > 50)
				{
					//CALCULATE EXPLOSION COORDINATES
					explosion_center_x = rocket_x + (ROCKETWIDTH/2);
					explosion_center_y = rocket_y + (ROCKETHEIGHT/2);
					explosion_x = explosion_center_x - (EXPLOSIONWIDTH/2);
					explosion_y = explosion_center_y - (EXPLOSIONHEIGHT/2);

					//DRAW EXPLOSION
					putImage(explosion_x,explosion_y,EXPLOSIONWIDTH,EXPLOSIONHEIGHT,explosion,0,0);

					for (int i = 0; i<MAX_FISHES; i++)
					{
						if(collision(explosion_x,explosion_y,EXPLOSIONWIDTH,EXPLOSIONHEIGHT,fishX[i],fishY[i],FISHWIDTH,FISHHEIGHT))
						{
							//Add score of fish, remove fish, redraw explosion, randomise fish, print updated vars to serial
							add_score(score,i);
							print_serial(games_played,lives,score,fish_caught,abilities_used);
							fillRectangle(fishX[i],fishY[i],FISHWIDTH,FISHHEIGHT,0);
							putImage(explosion_x,explosion_y,32,32,explosion,0,0);
							randomise_fish(fishX,fishY,i);
						}
					}
					new_stage = 1;
					stage = BOAT_STAGE;
					delay(250);
				}
				else if(rocket_y >= 160)
				{
					new_stage = 1;
					stage = BOAT_STAGE;
				}
			}
		}
//ABILITY END

//GAME OVER START
		while (stage == GAME_OVER)
		{
			if (new_stage)
			{
				end_screen(score);
				print_grade(score);
				playChime(notesOver, dursOver, 3);
				new_stage = 0;
			}

			//GET PLAYER TO PRESS r TO RESTART
			restart = egetchar();
			if (restart == 'r') 
			{
				eputs("\nNew Game Started!");
				games_played ++;
				delay(100);

				new_stage = 1;
				stage = new_stage;
			}
		}
//GAME OVER END

	return 0;
	}
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
	pinMode(GPIOA,12,0);
	enablePullUp(GPIOB,4);
	enablePullUp(GPIOB,5);
	enablePullUp(GPIOA,11);
	enablePullUp(GPIOA,8);
	enablePullUp(GPIOA,12);
}

void move_right (uint16_t *x, int *horizontal_moved, int boundary, int object_width, int flip, int *invert)
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

void move_left (uint16_t *x, int *horizontal_moved, int boundary, int flip, int *invert)
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

void move_down (uint16_t *y, int *vertical_moved, int boundary, int object_height)
{
	if (*y + object_height < boundary)
	{
		*y = *y + 1;
		*vertical_moved = 1;
	}
}

void move_up (uint16_t *y, int *vertical_moved, int boundary)
{
	if (*y > boundary)
	{
		*y = *y - 1;
		*vertical_moved = 1;
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
	if ((GPIOB->IDR & (1 << 4)) == 0)
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

int abilityButton() {
	if ((GPIOA->IDR & (1 << 12)) == 0)
	{
		return 1;
	}
	else return 0;	
}

void spawnObstacle(uint16_t *x, uint16_t *y, int width, int height, const uint16_t *sprite, int *direction) {
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

void spawnFish(uint16_t *x, uint16_t *y, int width, int height, const uint16_t *sprite, const uint16_t *sprite2, const uint16_t *sprite3, int *direction, int index) {
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
	if (index == 0)
	{
		putImage(*x, *y, width, height, sprite, *direction, 0); 	
	} else if (index == 1)
	{
		putImage(*x, *y, width, height, sprite2, *direction, 0); 	
	} else if (index == 2)
	{
		putImage(*x, *y, width, height, sprite3, *direction, 0); 	
	}
}

void reset (int *score,int *lives,int *gamebegin, int *stage, int *fishcaught, int *abilities_used, uint16_t*boat_x, uint16_t*boat_y, int *boat_invert, uint16_t fishX[], uint16_t fishY[])
{
	*score = 0;
	*lives = 3;
	*gamebegin = 1;
	*stage = START_MENU;
	*fishcaught = 0;
	*abilities_used = 0;
	*boat_x = 64 -(BOATWIDTH/2);
	*boat_y = 10;
	*boat_invert = 0;

	//RANDOMISE ALL 3 FISH
	randomise_fish(fishX,fishY,0);//light blue fish
	randomise_fish(fishX,fishY,1);//blue fish
	randomise_fish(fishX,fishY,2);//red fish
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

void playChime(uint32_t *notes, uint32_t *durations, int count) 
{
	for (int i = 0; i < count; i++) {
		playNote(notes[i]);
		delay(durations[i]);
		playNote(0);
		delay(100); // may need to adjust
	}
}

void ascii (void)
{
	eputs("\n");
	eputs("_________                           ________  .__                \n");
	eputs("\\_   ___ \\_____ __________________  \\______ \\ |__| ____   _____  \n");
	eputs("/    \\  \\/\\__  \\\\_  __ \\____ \\__  \\  |    |  \\|  |/ __ \\ /     \\ \n");
	eputs("\\     \\____/ __ \\|  | \\/  |_> > __ \\_|    `   \\  \\  ___/|  Y Y  \\\n");
	eputs(" \\______  (____  /__|  |   __(____  /_______  /__|\\___  >__|_|  /\n");
	eputs("        \\/     \\/      |__|       \\/        \\/        \\/      \\/ \n");
	eputs("\n");
}

void intro_screen (void)
{
	//clear board
	fillRectangle(0,0,128,160,0);

	//print title
	fillRectangle(11,8,109,18,RGBToWord(255,255,255));
	printTextX2("CarpaDiem", 13, 10, RGBToWord(0,0,0), RGBToWord(255,255,255));

	//print description
	printText("Help Pat the cat", 0, 35, RGBToWord(0,255,0), 0);
	printText("cath some food!", 0, 45, RGBToWord(0,255,0), 0);
	printText("LR = Move Boat", 0, 55, RGBToWord(255,255,255), 0);
	printText("D = Release bucket", 0, 65, RGBToWord(255,255,255), 0);
	printText("LRUD = Move bucket", 0, 75, RGBToWord(255,255,255), 0);
	printText("U = Rocket", 0, 85, RGBToWord(255,255,255), 0);
	printText("U(again) = Explode", 0, 95, RGBToWord(255,255,255), 0);
	printText("Get Fish and go", 0, 105, RGBToWord(255,255,0), 0);
	printText("back to boat, try", 0, 115, RGBToWord(255,255,0), 0);
	printText("not to get hit!", 0, 125, RGBToWord(255,255,0), 0);
}

void ability_ready (int ability)
{
	//if not put red square in hud, if ready put green square in hud
	if (ability < 3)
	{
		fillRectangle(80,0,8,8,RGBToWord(255,0,0));
	}
	else if (ability >= 3)
	{
		fillRectangle(80,0,8,8,RGBToWord(0,255,0));
	}
}

void add_score (int* score, int fish_index)
{
	if (fish_index == 0)
	{
		*score += 250;
	}
	else if(fish_index == 1)
	{
		*score += 500;
	}
	else if (fish_index == 2)
	{
		*score += 1000;
	}
}

void print_grade (int score)
{
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
}

void end_screen (int score)
{
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
	printText("Press keyboard (r)", 2, 140, RGBToWord(255, 255, 255), 0);
	printText("to restart", 33, 150, RGBToWord(255, 255, 255), 0);
	putImage(16, 100, BOATWIDTH, BOATHEIGHT, boat1, 0, 0);
}

void switch_stage (int*new_stage,int*current_stage,int dest_stage)
{
	*new_stage = 1;
	*current_stage =  dest_stage;
}