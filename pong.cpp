#define BLACK  false
#define WHITE  true

#define FIX_FPS false
#define PAD_SPEED 4

#define KY_W 25
#define KY_S 39
#define KY_UP 111
#define KY_DOWN 116
#define SPACE 65

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctime>
#include <chrono> // ugh needs to g++ bcs of this

Display *display;
int screen;
Window window;

using namespace std;

int random(int n){ return abs((1103515245 * n + 12345) % 1048576); }

float lerp(const float start,const float end,const float pct){ return (start + (end - start) * pct);}

struct V2 {
	float x,y;
};

struct Player {
	double height; 
	short pad_length = 200;
	double next_height; // used to smoothly interoplate
};

struct Ball {
	V2 pos;
	V2 dir;
	short speed;
	short size = 15;
};

struct Rect{
   short x, y, w, h;
};

// Stuff i shamlessly stole from rawdraw to have pretty text drawing
const unsigned short RawdrawFontCharMap[256] = {
	65535, 0, 8, 16, 24, 31, 41, 50, 51, 65535, 65535, 57, 66, 65535, 75, 83,
	92, 96, 100, 108, 114, 123, 132, 137, 147, 152, 158, 163, 169, 172, 178, 182, 
	65535, 186, 189, 193, 201, 209, 217, 226, 228, 232, 236, 244, 248, 250, 252, 253, 
	255, 261, 266, 272, 278, 283, 289, 295, 300, 309, 316, 318, 321, 324, 328, 331, 
	337, 345, 352, 362, 368, 375, 382, 388, 396, 402, 408, 413, 422, 425, 430, 435, 
	442, 449, 458, 466, 472, 476, 480, 485, 492, 500, 507, 512, 516, 518, 522, 525, 
	527, 529, 536, 541, 546, 551, 557, 564, 572, 578, 581, 586, 593, 595, 604, 610, 
	615, 621, 627, 632, 638, 642, 648, 653, 660, 664, 670, 674, 680, 684, 690, 694, 
	65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 
	65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 
	700, 703, 711, 718, 731, 740, 744, 754, 756, 760, 766, 772, 775, 777, 785, 787, 
	792, 798, 803, 811, 813, 820, 827, 828, 831, 833, 838, 844, 853, 862, 874, 880, 
	889, 898, 908, 919, 928, 939, 951, 960, 969, 978, 988, 997, 1005, 1013, 1022, 1030,
	1039, 1047, 1054, 1061, 1070, 1079, 1086, 1090, 1099, 1105, 1111, 1118, 1124, 1133, 1140, 1150,
	1159, 1168, 1178, 1189, 1198, 1209, 1222, 1231, 1239, 1247, 1256, 1264, 1268, 1272, 1277, 1281,
	1290, 1300, 1307, 1314, 1322, 1331, 1338, 1342, 1349, 1357, 1365, 1374, 1382, 1390, 1397, 65535, };

const unsigned char RawdrawFontCharData[1405] = {
	0x00, 0x09, 0x20, 0x29, 0x03, 0x23, 0x14, 0x8b, 0x00, 0x09, 0x20, 0x29, 0x04, 0x24, 0x13, 0x8c, 
	0x01, 0x21, 0x23, 0x14, 0x03, 0x09, 0x11, 0x9a, 0x11, 0x22, 0x23, 0x14, 0x03, 0x02, 0x99, 0x01, 
	0x21, 0x23, 0x09, 0x03, 0x29, 0x03, 0x09, 0x12, 0x9c, 0x03, 0x2b, 0x13, 0x1c, 0x23, 0x22, 0x11, 
	0x02, 0x8b, 0x9a, 0x1a, 0x01, 0x21, 0x23, 0x03, 0x89, 0x03, 0x21, 0x2a, 0x21, 0x19, 0x03, 0x14, 
	0x23, 0x9a, 0x01, 0x10, 0x21, 0x12, 0x09, 0x12, 0x1c, 0x03, 0xab, 0x02, 0x03, 0x1b, 0x02, 0x1a, 
	0x13, 0x10, 0xa9, 0x01, 0x2b, 0x03, 0x29, 0x02, 0x11, 0x22, 0x13, 0x8a, 0x00, 0x22, 0x04, 0x88, 
	0x20, 0x02, 0x24, 0xa8, 0x01, 0x10, 0x29, 0x10, 0x14, 0x0b, 0x14, 0xab, 0x00, 0x0b, 0x0c, 0x20, 
	0x2b, 0xac, 0x00, 0x28, 0x00, 0x02, 0x2a, 0x10, 0x1c, 0x20, 0xac, 0x01, 0x21, 0x23, 0x03, 0x09, 
	0x20, 0x10, 0x14, 0x8c, 0x03, 0x23, 0x24, 0x04, 0x8b, 0x01, 0x10, 0x29, 0x10, 0x14, 0x0b, 0x14, 
	0x2b, 0x04, 0xac, 0x01, 0x18, 0x21, 0x10, 0x9c, 0x03, 0x1c, 0x23, 0x1c, 0x10, 0x9c, 0x02, 0x22, 
	0x19, 0x22, 0x9b, 0x02, 0x2a, 0x02, 0x19, 0x02, 0x9b, 0x01, 0x02, 0xaa, 0x02, 0x22, 0x11, 0x02, 
	0x13, 0xaa, 0x11, 0x22, 0x02, 0x99, 0x02, 0x13, 0x22, 0x8a, 0x10, 0x1b, 0x9c, 0x10, 0x09, 0x20, 
	0x99, 0x10, 0x1c, 0x20, 0x2c, 0x01, 0x29, 0x03, 0xab, 0x21, 0x10, 0x01, 0x23, 0x14, 0x0b, 0x10, 
	0x9c, 0x00, 0x09, 0x23, 0x2c, 0x04, 0x03, 0x21, 0xa8, 0x21, 0x10, 0x01, 0x12, 0x03, 0x14, 0x2b, 
	0x02, 0xac, 0x10, 0x99, 0x10, 0x01, 0x03, 0x9c, 0x10, 0x21, 0x23, 0x9c, 0x01, 0x2b, 0x11, 0x1b, 
	0x21, 0x0b, 0x02, 0xaa, 0x02, 0x2a, 0x11, 0x9b, 0x04, 0x9b, 0x02, 0xaa, 0x9c, 0x03, 0xa9, 0x00, 
	0x20, 0x24, 0x04, 0x08, 0x9a, 0x01, 0x10, 0x1c, 0x04, 0xac, 0x01, 0x10, 0x21, 0x22, 0x04, 0xac, 
	0x00, 0x20, 0x24, 0x0c, 0x12, 0xaa, 0x00, 0x02, 0x2a, 0x20, 0xac, 0x20, 0x00, 0x02, 0x22, 0x24, 
	0x8c, 0x20, 0x02, 0x22, 0x24, 0x04, 0x8a, 0x00, 0x20, 0x21, 0x12, 0x9c, 0x00, 0x0c, 0x00, 0x20, 
	0x2c, 0x04, 0x2c, 0x02, 0xaa, 0x00, 0x02, 0x22, 0x20, 0x08, 0x22, 0x8c, 0x19, 0x9b, 0x19, 0x13, 
	0x8c, 0x20, 0x02, 0xac, 0x01, 0x29, 0x03, 0xab, 0x00, 0x22, 0x8c, 0x01, 0x10, 0x21, 0x12, 0x1b, 
	0x9c, 0x21, 0x01, 0x04, 0x24, 0x22, 0x12, 0x13, 0xab, 0x04, 0x01, 0x10, 0x21, 0x2c, 0x02, 0xaa, 
	0x00, 0x04, 0x14, 0x23, 0x12, 0x0a, 0x12, 0x21, 0x10, 0x88, 0x23, 0x14, 0x03, 0x01, 0x10, 0xa9, 
	0x00, 0x10, 0x21, 0x23, 0x14, 0x04, 0x88, 0x00, 0x04, 0x2c, 0x00, 0x28, 0x02, 0x9a, 0x00, 0x0c, 
	0x00, 0x28, 0x02, 0x9a, 0x21, 0x10, 0x01, 0x03, 0x14, 0x23, 0x22, 0x9a, 0x00, 0x0c, 0x20, 0x2c, 
	0x02, 0xaa, 0x00, 0x28, 0x10, 0x1c, 0x04, 0xac, 0x00, 0x20, 0x23, 0x14, 0x8b, 0x00, 0x0c, 0x02, 
	0x12, 0x21, 0x28, 0x12, 0x23, 0xac, 0x00, 0x04, 0xac, 0x04, 0x00, 0x11, 0x20, 0xac, 0x04, 0x00, 
	0x2a, 0x20, 0xac, 0x01, 0x10, 0x21, 0x23, 0x14, 0x03, 0x89, 0x00, 0x0c, 0x00, 0x10, 0x21, 0x12, 
	0x8a, 0x01, 0x10, 0x21, 0x23, 0x14, 0x03, 0x09, 0x04, 0x9b, 0x00, 0x0c, 0x00, 0x10, 0x21, 0x12, 
	0x02, 0xac, 0x21, 0x10, 0x01, 0x23, 0x14, 0x8b, 0x00, 0x28, 0x10, 0x9c, 0x00, 0x04, 0x24, 0xa8, 
	0x00, 0x03, 0x14, 0x23, 0xa8, 0x00, 0x04, 0x2c, 0x14, 0x1b, 0x24, 0xa8, 0x00, 0x01, 0x23, 0x2c, 
	0x04, 0x03, 0x21, 0xa8, 0x00, 0x01, 0x12, 0x1c, 0x12, 0x21, 0xa8, 0x00, 0x20, 0x02, 0x04, 0xac, 
	0x10, 0x00, 0x04, 0x9c, 0x01, 0xab, 0x10, 0x20, 0x24, 0x9c, 0x01, 0x10, 0xa9, 0x04, 0xac, 0x00, 
	0x99, 0x02, 0x04, 0x24, 0x2a, 0x23, 0x12, 0x8a, 0x00, 0x04, 0x24, 0x22, 0x8a, 0x24, 0x04, 0x03, 
	0x12, 0xaa, 0x20, 0x24, 0x04, 0x02, 0xaa, 0x24, 0x04, 0x02, 0x22, 0x23, 0x9b, 0x04, 0x09, 0x02, 
	0x1a, 0x01, 0x10, 0xa9, 0x23, 0x12, 0x03, 0x14, 0x23, 0x24, 0x15, 0x8c, 0x00, 0x0c, 0x03, 0x12, 
	0x23, 0xac, 0x19, 0x12, 0x9c, 0x2a, 0x23, 0x24, 0x15, 0x8c, 0x00, 0x0c, 0x03, 0x13, 0x2a, 0x13, 
	0xac, 0x10, 0x9c, 0x02, 0x0c, 0x02, 0x1b, 0x12, 0x1c, 0x12, 0x23, 0xac, 0x02, 0x0c, 0x03, 0x12, 
	0x23, 0xac, 0x02, 0x22, 0x24, 0x04, 0x8a, 0x02, 0x0d, 0x04, 0x24, 0x22, 0x8a, 0x02, 0x04, 0x2c, 
	0x25, 0x22, 0x8a, 0x02, 0x0c, 0x03, 0x12, 0xaa, 0x22, 0x02, 0x03, 0x23, 0x24, 0x8c, 0x11, 0x1c, 
	0x02, 0xaa, 0x02, 0x04, 0x14, 0x2b, 0x24, 0xaa, 0x02, 0x03, 0x14, 0x23, 0xaa, 0x02, 0x03, 0x14, 
	0x1a, 0x13, 0x24, 0xaa, 0x02, 0x2c, 0x04, 0xaa, 0x02, 0x03, 0x1c, 0x22, 0x23, 0x8d, 0x02, 0x22, 
	0x04, 0xac, 0x20, 0x10, 0x14, 0x2c, 0x12, 0x8a, 0x10, 0x19, 0x13, 0x9c, 0x00, 0x10, 0x14, 0x0c, 
	0x12, 0xaa, 0x01, 0x10, 0x11, 0xa8, 0x03, 0x04, 0x24, 0x23, 0x12, 0x8b, 0x18, 0x11, 0x9c, 0x21, 
	0x10, 0x01, 0x02, 0x13, 0x2a, 0x10, 0x9b, 0x11, 0x00, 0x04, 0x24, 0x2b, 0x02, 0x9a, 0x01, 0x0a, 
	0x11, 0x29, 0x22, 0x2b, 0x03, 0x1b, 0x02, 0x11, 0x22, 0x13, 0x8a, 0x00, 0x11, 0x28, 0x11, 0x1c, 
	0x02, 0x2a, 0x03, 0xab, 0x10, 0x1a, 0x13, 0x9d, 0x20, 0x00, 0x02, 0x11, 0x2a, 0x02, 0x13, 0x22, 
	0x24, 0x8c, 0x08, 0xa8, 0x20, 0x10, 0x11, 0xa9, 0x10, 0x29, 0x20, 0x21, 0x11, 0x98, 0x11, 0x02, 
	0x1b, 0x21, 0x12, 0xab, 0x01, 0x21, 0xaa, 0x12, 0xaa, 0x10, 0x20, 0x21, 0x19, 0x12, 0x18, 0x11, 
	0xaa, 0x00, 0xa8, 0x01, 0x10, 0x21, 0x12, 0x89, 0x02, 0x2a, 0x11, 0x1b, 0x03, 0xab, 0x01, 0x10, 
	0x21, 0x03, 0xab, 0x01, 0x10, 0x21, 0x12, 0x0a, 0x12, 0x23, 0x8b, 0x11, 0xa8, 0x02, 0x0d, 0x04, 
	0x14, 0x2b, 0x22, 0xac, 0x14, 0x10, 0x01, 0x1a, 0x10, 0x20, 0xac, 0x9a, 0x14, 0x15, 0x8d, 0x20, 
	0xa9, 0x10, 0x20, 0x21, 0x11, 0x98, 0x01, 0x12, 0x0b, 0x11, 0x22, 0x9b, 0x00, 0x09, 0x02, 0x28, 
	0x12, 0x13, 0x2b, 0x22, 0xac, 0x00, 0x09, 0x02, 0x28, 0x12, 0x22, 0x13, 0x14, 0xac, 0x00, 0x10, 
	0x11, 0x09, 0x11, 0x02, 0x28, 0x12, 0x13, 0x2b, 0x22, 0xac, 0x18, 0x11, 0x12, 0x03, 0x14, 0xab, 
	0x04, 0x02, 0x11, 0x22, 0x2c, 0x03, 0x2b, 0x10, 0xa9, 0x04, 0x02, 0x11, 0x22, 0x2c, 0x03, 0x2b, 
	0x01, 0x98, 0x04, 0x02, 0x11, 0x22, 0x2c, 0x03, 0x2b, 0x01, 0x10, 0xa9, 0x04, 0x02, 0x11, 0x22, 
	0x2c, 0x03, 0x2b, 0x01, 0x10, 0x11, 0xa8, 0x04, 0x02, 0x11, 0x22, 0x2c, 0x03, 0x2b, 0x08, 0xa8, 
	0x04, 0x02, 0x11, 0x22, 0x2c, 0x03, 0x2b, 0x00, 0x20, 0x11, 0x88, 0x00, 0x0c, 0x02, 0x2a, 0x00, 
	0x19, 0x10, 0x1c, 0x10, 0x28, 0x14, 0xac, 0x23, 0x14, 0x03, 0x01, 0x10, 0x29, 0x14, 0x15, 0x8d, 
	0x02, 0x2a, 0x02, 0x04, 0x2c, 0x03, 0x1b, 0x00, 0x99, 0x02, 0x2a, 0x02, 0x04, 0x2c, 0x03, 0x1b, 
	0x11, 0xa8, 0x02, 0x2a, 0x02, 0x04, 0x2c, 0x03, 0x1b, 0x01, 0x10, 0xa9, 0x02, 0x2a, 0x02, 0x04, 
	0x2c, 0x03, 0x1b, 0x08, 0xa8, 0x02, 0x2a, 0x12, 0x1c, 0x04, 0x2c, 0x00, 0x99, 0x02, 0x2a, 0x12, 
	0x1c, 0x04, 0x2c, 0x11, 0xa8, 0x02, 0x2a, 0x12, 0x1c, 0x04, 0x2c, 0x01, 0x10, 0xa9, 0x02, 0x2a, 
	0x12, 0x1c, 0x04, 0x2c, 0x28, 0x88, 0x00, 0x10, 0x21, 0x23, 0x14, 0x04, 0x08, 0x02, 0x9a, 0x04, 
	0x02, 0x24, 0x2a, 0x01, 0x10, 0x11, 0xa8, 0x02, 0x22, 0x24, 0x04, 0x0a, 0x00, 0x99, 0x02, 0x22, 
	0x24, 0x04, 0x0a, 0x11, 0xa8, 0x02, 0x22, 0x24, 0x04, 0x0a, 0x11, 0x28, 0x00, 0x99, 0x02, 0x22, 
	0x24, 0x04, 0x0a, 0x01, 0x10, 0x11, 0xa8, 0x01, 0x21, 0x24, 0x04, 0x09, 0x08, 0xa8, 0x01, 0x2b, 
	0x03, 0xa9, 0x01, 0x10, 0x21, 0x23, 0x14, 0x03, 0x09, 0x03, 0xa9, 0x01, 0x04, 0x24, 0x29, 0x11, 
	0xa8, 0x01, 0x04, 0x24, 0x29, 0x00, 0x99, 0x02, 0x04, 0x24, 0x2a, 0x01, 0x10, 0xa9, 0x01, 0x04, 
	0x24, 0x29, 0x08, 0xa8, 0x01, 0x02, 0x13, 0x1c, 0x13, 0x22, 0x29, 0x11, 0xa8, 0x00, 0x0c, 0x01, 
	0x11, 0x22, 0x13, 0x8b, 0x00, 0x0d, 0x00, 0x10, 0x21, 0x1a, 0x02, 0x22, 0x24, 0x8c, 0x02, 0x04, 
	0x24, 0x2a, 0x23, 0x12, 0x0a, 0x00, 0x99, 0x02, 0x04, 0x24, 0x2a, 0x23, 0x12, 0x0a, 0x11, 0xa8, 
	0x02, 0x04, 0x24, 0x2a, 0x23, 0x12, 0x0a, 0x01, 0x10, 0xa9, 0x02, 0x04, 0x24, 0x2a, 0x23, 0x12, 
	0x0a, 0x01, 0x10, 0x11, 0xa8, 0x02, 0x04, 0x24, 0x2a, 0x23, 0x12, 0x0a, 0x09, 0xa9, 0x02, 0x04, 
	0x24, 0x2a, 0x23, 0x12, 0x0a, 0x01, 0x10, 0x21, 0x89, 0x02, 0x1b, 0x02, 0x04, 0x2c, 0x12, 0x1c, 
	0x12, 0x2a, 0x13, 0x2b, 0x22, 0xab, 0x03, 0x04, 0x2c, 0x03, 0x12, 0x2a, 0x14, 0x15, 0x8d, 0x24, 
	0x04, 0x02, 0x22, 0x23, 0x1b, 0x00, 0x99, 0x24, 0x04, 0x02, 0x22, 0x23, 0x1b, 0x11, 0xa8, 0x24, 
	0x04, 0x02, 0x22, 0x23, 0x1b, 0x01, 0x10, 0xa9, 0x24, 0x04, 0x02, 0x22, 0x23, 0x1b, 0x09, 0xa9, 
	0x12, 0x1c, 0x00, 0x99, 0x12, 0x1c, 0x11, 0xa8, 0x12, 0x1c, 0x01, 0x10, 0xa9, 0x12, 0x1c, 0x09, 
	0xa9, 0x00, 0x2a, 0x11, 0x28, 0x02, 0x22, 0x24, 0x04, 0x8a, 0x02, 0x0c, 0x03, 0x12, 0x23, 0x2c, 
	0x01, 0x10, 0x11, 0xa8, 0x02, 0x04, 0x24, 0x22, 0x0a, 0x00, 0x99, 0x02, 0x04, 0x24, 0x22, 0x0a, 
	0x11, 0xa8, 0x02, 0x04, 0x24, 0x22, 0x0a, 0x01, 0x10, 0xa9, 0x02, 0x04, 0x24, 0x22, 0x0a, 0x01, 
	0x10, 0x11, 0xa8, 0x02, 0x04, 0x24, 0x22, 0x0a, 0x09, 0xa9, 0x19, 0x02, 0x2a, 0x9b, 0x02, 0x04, 
	0x24, 0x22, 0x0a, 0x04, 0xaa, 0x02, 0x04, 0x14, 0x2b, 0x24, 0x2a, 0x00, 0x99, 0x02, 0x04, 0x14, 
	0x2b, 0x24, 0x2a, 0x11, 0xa8, 0x02, 0x04, 0x14, 0x2b, 0x24, 0x2a, 0x01, 0x10, 0xa9, 0x02, 0x04, 
	0x14, 0x2b, 0x24, 0x2a, 0x09, 0xa9, 0x02, 0x03, 0x1c, 0x22, 0x23, 0x0d, 0x11, 0xa8, 0x00, 0x0c, 
	0x02, 0x11, 0x22, 0x13, 0x8a, 0x02, 0x03, 0x1c, 0x22, 0x23, 0x0d, 0x09, 0xa9, };

void DrawText(const short x_c, const short y_c, const char * text, short scale ){
	const unsigned char * lmap;
	float iox = (float)x_c; //x offset
	float ioy = (float)y_c; //y offset

	int place = 0;
	unsigned short index;
	int bQuit = 0;

	while( text[place] )
	{
		unsigned char c = text[place];
		switch(c)
		{
		case 9: // tab
			iox += 12 * scale;
			break;
		case 10: // linefeed
			iox = (float)x_c;
			ioy += 6 * scale;
			break;
		default:
			index = RawdrawFontCharMap[c];
			if( index == 65535 )
			{
				iox += 3 * scale;
				break;
			}

			lmap = &RawdrawFontCharData[index];
			short penx = 0, peny = 0;
			unsigned char start_seg = 1;
			do
			{
				unsigned char data = (*(lmap++));
				short x1 = (short)(((data >> 4) & 0x07)*scale + iox);
				short y1 = (short)((data        & 0x07)*scale + ioy);
				if( start_seg )
				{
					penx = x1;
					peny = y1;
					start_seg = 0;
					if( data & 0x08 )
						XDrawPoint(display, window, DefaultGC(display, screen),x1,y1);
						//CNFGTackPixel( x1, y1 );
				}
				else
				{
					//CNFGTackSegment( penx, peny, x1, y1 );
					XDrawLine(display, window, DefaultGC(display, screen),penx,peny,x1,y1);
					penx = x1;
					peny = y1;
				}
				if( data & 0x08 ) start_seg = 1;
				bQuit = data & 0x80;
			} while( !bQuit );

			iox += 3 * scale;
		}
		place++;
	}
}

// --------------------------------------------------------------

void draw_rect(const Rect rect,const bool is_white){
   if (is_white)
      XSetForeground(display, DefaultGC(display, screen), WhitePixel(display, screen));
   else
      XSetForeground(display, DefaultGC(display, screen), BlackPixel(display, screen));
   XFillRectangle(display, window, DefaultGC(display, screen), rect.x, rect.y, rect.w, rect.h);
}
// I wanted to use XFillArc but well, it doesn't want to be where i want it to so...
void DrawCircle(const short& x,const short& y,const short& radius){
   short x_ = 0;
   short y_ = radius;
   short d = 1-radius;

   XDrawLine(display,window,DefaultGC(display,screen),x-radius,y,x+radius,y);

   while(x_ < y_)
   {
      if(d<0)
      {
         d = d+2*x_+3;
         x_+=1;
      }
      else
      {
         d = d+2*(x_-y_)+5;
         x_+=1;
         y_-=1;
      }

      XDrawLine(display,window,DefaultGC(display,screen),x-x_,y+y_,x+x_,y+y_);
     	XDrawLine(display,window,DefaultGC(display,screen),x-x_,y-y_,x+x_,y-y_);
      XDrawLine(display,window,DefaultGC(display,screen),x-y_,y+x_,x+y_,y+x_);
      XDrawLine(display,window,DefaultGC(display,screen),x-y_,y-x_,x+y_,y-x_);
   }
}

const char* titles[] = {"PONG GAME","POG GAME"}; // Easter egg ;P

// ------ GLOBALS -------
	chrono::duration<double> delta;// used for delta
	const int W = 1024;
	const int H = 768;
	unsigned char P1oints;
	unsigned char P2oints;
	Player P1;
	Player P2; // implement simple bot for it
	Ball ball;
	bool in_menu = true;
	bool multiplayer = false;
	bool keycodes[256] = { 0 };
	const char* title = titles[rand() % 100 < 10];

// ---------------------

// ---- MENU STUFF ------
	float title_x = 0;
	float menu_ball_x = 1024 / 2,menu_ball_y = 768 / 2;
	float menu_y_velocity = 0;
	float gravity = 980;
// ----------------------

void MoveBall(const double& delta){
	//ball.pos += (ball.dir * ball.speed) * delta;
	ball.pos.x += (ball.dir.x * ball.speed) * delta;
	ball.pos.y += (ball.dir.y * ball.speed) * delta;

	if(ball.pos.y + ball.size >= H){      // BOTTOM
		ball.dir.y *= -1;
		ball.pos.y = H - ball.size - 1;}

	else if(ball.pos.y - ball.size <= 0){ // TOP
		ball.dir.y *= -1;
		ball.pos.y = 1 + ball.size;}

    // Going for P2
	if(ball.dir.x > 0){
		V2 pad_pos,dist;
		pad_pos.x = W- 60;
		pad_pos.y = P2.height + (P2.pad_length/2);

		dist.x = abs(ball.pos.x - pad_pos.x);
		dist.y = abs(ball.pos.y - pad_pos.y);

		V2 intersect = V2{dist.x - (20 + ball.size / 2), dist.y - (P2.pad_length/2 + ball.size / 2)};
		if(intersect.x <= 0 && intersect.y <= 0){
			if((ball.pos.y - ball.size < P2.height) || (ball.pos.y + ball.size) > (P2.height + P2.pad_length)){ //! Glitch with moving up or down while ball is colliding, FIX
				ball.dir.y *= -1;
			}
			else{
				ball.dir.x *= -1;
				ball.speed += 50;
			}
		}
	}
	 // Going for P1
	else{
		V2 pad_pos,dist;
		pad_pos.x = 60;
		pad_pos.y = P1.height + (P1.pad_length/2);

		dist.x = abs(ball.pos.x - pad_pos.x);
		dist.y = abs(ball.pos.y - pad_pos.y);

		V2 intersect = V2{dist.x - (20 + ball.size / 2), dist.y - (P1.pad_length/2 + ball.size / 2)};
		if(intersect.x <= 0 && intersect.y <= 0){
			if((ball.pos.y - ball.size < P1.height) || (ball.pos.y + ball.size) > (P1.height + P1.pad_length)){ //! Glitch with moving up or down while ball is colliding, FIX
				ball.dir.y *= -1;
			}
			else{
				ball.dir.x *= -1;
				ball.speed += 50;
			}
		}
	}
}

void handlekey(int keycode, bool press){
   if (keycode > 0 && keycode < 256)
      keycodes[keycode] = press;
}

void HandlePoints(){
	if(ball.pos.x > W){
		++P1oints;

		ball.pos.x = W/2; ball.pos.y = H/2;
		ball.dir = {-0.45,0.45};
		ball.speed = 900;
	}
	else if(ball.pos.x < 0){
		++P2oints;

		ball.pos.x = W/2; ball.pos.y = H/2;
		ball.dir = {0.45,0.45};
		ball.speed = 900;
	}
}

void game_update(){
	if(in_menu){
		title_x = lerp(title_x, W / 4 , 2.5 * delta.count()); // Title animation
		menu_y_velocity += gravity * delta.count();
		menu_ball_y += menu_y_velocity * delta.count();
		if((short)(menu_ball_y + 30) >= H){ menu_y_velocity *= -0.98; menu_ball_y = H - 31;}
		if(keycodes[SPACE]) in_menu = false;
	}
	else{
		if(keycodes[KY_W] && P1.next_height >= 0)
			P1.next_height -= PAD_SPEED;
		else if(keycodes[KY_S] && P1.next_height + P1.pad_length < H)
			P1.next_height += PAD_SPEED;

		if(keycodes[KY_UP] && P2.next_height >= 0)
			P2.next_height -= PAD_SPEED;
		else if(keycodes[KY_DOWN] && P2.next_height + P2.pad_length < H)
			P2.next_height += PAD_SPEED;

		MoveBall(delta.count());
		HandlePoints();
	}
}

void draw_game(){
	XSetForeground(display, DefaultGC(display, screen), WhitePixel(display, screen));
	if(in_menu){
		DrawText(title_x, 20, title, 20);
		DrawCircle((short)menu_ball_x,(short)menu_ball_y,26);
	}
	else{
			char p[3];
			sprintf(p,"%u",P1oints);
			DrawText(W/2 - 140,40,p,13);

			sprintf(p,"%u",P2oints);
			DrawText(W/2 + 140,40,p,13);

			// Player 1
			draw_rect((Rect){50,(short)P1.height,20,P1.pad_length},true);
			P1.height = lerp(P1.height,P1.next_height,5 * delta.count());
			
			// Player 2
			draw_rect((Rect){W-70,(short)P2.height,20,P2.pad_length},true);
			P2.height = lerp(P2.height,P2.next_height,5 * delta.count());

			DrawCircle((short)ball.pos.x,(short)ball.pos.y, ball.size);
			
			//MoveBall(delta.count());
			//HandePoints();
	}
}


int main(){
	XEvent e;

   display = XOpenDisplay(NULL);
   if (display == NULL) return 1;

   // Setting Window
	   screen = DefaultScreen(display);
	   window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, 640, 480, 1,
	                           	  BlackPixel(display, screen), WhitePixel(display, screen));
	   XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask );
	   XMapWindow(display, window);

		Atom wm_delete = XInternAtom( display, "WM_DELETE_WINDOW", 0 );
		XSetWMProtocols( display, window, &wm_delete, 1 );

	   XSizeHints *hints = XAllocSizeHints();
	   hints->flags = PMinSize | PMaxSize;
	   hints->min_width = W; hints->max_width = W;
	   hints->min_height = H; hints->max_height = H;
		
	   XSetWMSizeHints(display, window, hints, PMinSize | PMaxSize);
	   XSetWMNormalHints(display, window, hints);
	   
	   srand(time(0));
	   XStoreName(display,window,title);
   // ==================

	chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now(); 
	chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
	delta = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
	// Initialization
		P1.height = 50;P1.next_height = P1.height;
		P2.height = 50;P2.next_height = P2.height;

		ball.pos = {W/2,H/2};
		ball.dir = {-0.45,-0.45};
		ball.speed = 900;
	// ------------------
   while(true){
      XNextEvent(display, &e);
      if (e.type == Expose) {
      	//XClearWindow(display,window);
         draw_rect((Rect){ 0, 0, W, H }, BLACK);
         draw_game();
         game_update();
         
         usleep(4000 - delta.count()* 6000); //? it fixes flickering, dirty solution but well... added the delta because why not
         XClearArea(display, window, W-1, H-1, 1, 1, true); // X11 is so broken, it works with it only
         XFlush(display);
      }
      if (e.type == KeyPress) {
      	handlekey(e.xkey.keycode,true);
      }
      if (e.type == KeyRelease) {
      	handlekey(e.xkey.keycode,false);
      }
      if (e.type == ClientMessage)
         break;

		t1 = t2;
		t2 = chrono::high_resolution_clock::now();
		delta = chrono::duration_cast<chrono::duration<double>>(t2 -t1); // calc delta
   }

  	XCloseDisplay(display);
	return 0;
}