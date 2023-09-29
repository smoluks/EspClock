/*--------------------- MATRIX GPIO CONFIG  -------------------------*/
#define R1_PIN 25
#define G1_PIN 27
#define B1_PIN 26
#define R2_PIN 14
#define G2_PIN 13
#define B2_PIN 12
#define A_PIN 2
#define B_PIN 0
#define C_PIN 17
#define D_PIN 5
#define E_PIN -1
#define LAT_PIN 4
#define OE_PIN 18
#define CLK_PIN 16

#undef USE_GFX_ROOT
#undef NO_GFX

#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another

// Color definitions
#define BLACK565    0x0000
#define BLUE565     0x001F
#define RED565      0xF800
#define GREEN565    0x07E0
#define CYAN565     0x07FF
#define MAGENTA565  0xF81F
#define YELLOW565   0xFFE0 
#define WHITE565    0xFFFF

void hub75_init();
void hub75_loop();
