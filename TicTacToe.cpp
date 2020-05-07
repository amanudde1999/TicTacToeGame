// TIC TAC TOE Game to be played on an Arduino TFT Screen either against a computer or against a friend.
// 							¯\_(ツ)_/¯

#include <Arduino.h>
// core graphics library for displays (written by Adafruit)
#include <Adafruit_GFX.h>
// necessary for using a GFX font
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSerifBoldItalic9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
// LCD and SD card will communicate using the Serial Peripheral Interface (SPI)
// e.g., SPI is used to display images stored on the SD card
#include <SPI.h>
// needed for reading/writing to SD card
#include <SD.h>
#include <TouchScreen.h>
// Graphics library for MCU Friend 3.5" TFT LCD shield
#include <MCUFRIEND_kbv.h>
// make an object from MCUFRIEND_kbv class
MCUFRIEND_kbv tft;
//MCUFRIEND_kbv tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
#define SD_CS 10
// touch screen pins
#define YP A3
#define XM A2
#define YM 9
#define XP 8
// calibration data for the touch screen, minimum/maximum readings from the touch point
#define TS_MINX 100
#define TS_MINY 110
#define TS_MAXX 960
#define TS_MAXY 910
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// Assign human-readable names to some common 16-bit color values (UTFT color code)
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0xC618

TouchScreen ts = TouchScreen(XP,YP,XM,YM,300);


extern uint8_t circle[];
extern uint8_t x_bitmap[]; 

// This is a flag to determine if we have entered a single player mode (0) or a 
// 2 player mode (1)
int OneOrTwo = 0;

// Variable determining who's turn it is
int turn = 1;

// Flag variable to check if the game is over or if it should keep going
int endGame = 0;

// Declaring counts for the scoreboard
int numTies = 0;
int p1Wins = 0;
int p2Wins = 0;
/*
		 _UL__|_UM__|_UR__
		 _ML__|_MM__|_MR__
		  LL  |	LM  | LR

*/
// Initialise a flag to each part of the grid for player 1 and player 2
// 0 if untouched, 1 if touched by player 1 and 2 if touched by player 2
int Upperleft = 0;
int Uppermid = 0;
int Upperright = 0;
int Middleleft = 0;
int Middlemid = 0;
int Middleright = 0;
int Lowerleft = 0;
int Lowermid = 0;
int Lowerright = 0;

// Used as a flag to determine whether the square is taken up or not
// 1 if occupied by an X or an O , 0 otherwise 
int UL = 0;
int UM = 0;
int UR = 0;
int ML = 0;
int MM = 0;
int MR = 0;
int LL = 0;
int LM = 0;
int LR = 0;


// Initialise all the finite states we have using enumeration
enum {Main,OnePlayer,TwoPlayer} current;
// We start at the main menu
int current_state=Main;

///////////////////////////////////////////////// END OF GLOBAL VARIABLE DECLARATIONS ///////////////////////////////////////////////////////

void setup() {
    init();
    Serial.begin(9600);

    //    tft.reset();             // hardware reset
    uint16_t ID = tft.readID();    // read ID from display
    Serial.print("ID = 0x");
    Serial.println(ID, HEX);
    if (ID == 0xD3D3) ID = 0x9481; // write-only shield
    //    ID = 0x9329;             // force ID

    tft.begin(ID);                 // LCD gets ready to work

}
// Full credit for this function: http://educ8s.tv/arduino-tic-tac-toe/
// The function allows us to convert the hex representation of bitmap images 
// to be able to be drawn on the TFT screnen
void drawBitmap(int16_t x, int16_t y,
 const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {

  int16_t i, j, byteWidth = (w + 7) / 8;
  uint8_t byte;

  for(j=0; j<h; j++) {
    for(i=0; i<w; i++) {
      if(i & 7) byte <<= 1;
      else      byte   = pgm_read_byte(bitmap + j * byteWidth + i / 8);
      if(byte & 0x80) tft.drawPixel(x+i, y+j, color);
    }
  }
}

/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
drawO function takes 2 parameters.
	x,y: coordinate pixels that determine where to draw 
		 the O 
	
It does not return any parameters

This will draw the O depending on the inputted coordinates
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void drawO(int x, int y){

  drawBitmap(x,y,circle,65,65,RED);
}

/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
drawX function takes 2 parameters.
	x,y: coordinate pixels that determine where to draw 
		 the X 
	
It does not return any parameters

This will draw the X depending on the inputted coordinates
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void drawX(int x, int y){

  drawBitmap(x,y,x_bitmap,65,65,BLUE);
}


/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
drawMainMenu function takes no parameters.

It also does not return any parameters

This will setup the title page
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void drawMainMenu(){
	tft.setRotation(0);
	tft.fillScreen(WHITE);
	tft.setTextColor(BLACK);
	tft.setCursor(75,20);
	tft.setTextSize(4);
	tft.print("TIC TAC");
	tft.setCursor(125,55);
	tft.print("TOE");
	tft.setTextSize(3);
	tft.fillRect(68,128,184,74,BLACK);
	tft.fillRect(70,130,180,70,GREEN);
	tft.setCursor(85,155);
	tft.print("1 PLAYER");
	tft.fillRect(68,218,184,74,BLACK);
	tft.fillRect(70,220,180,70,GREEN);
	tft.setCursor(85,245);
	tft.print("2 PLAYERS");
	for(int i =0; i < 4; i++){
		drawO(0,65*2*i);
		if (i == 3 ){
			break;
		}
		else{
			drawX(0,65*(2*i+1));
		}
	}
	drawX(65,390);
	drawO(130,390);
	drawX(195,390);
	drawO(255,390);
	for(int i=2; i >= 0; i--){
		drawX(255,65*(2*i+1));
		drawO(255,65*2*i);	
	}
}


/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
drawBoard function takes no parameters.

It also does not return any parameters

This will setup the board for the game
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void drawBoard(){
	// Draw the gridlines
	tft.fillRect(0,0,320,325,BLACK);
	// Vertical
	tft.fillRect(100,20,10,280,WHITE);
	tft.fillRect(200,20,10,280,WHITE);
	// Horizontal
	tft.fillRect(20,100,280,10,WHITE);
	tft.fillRect(20,200,280,10,WHITE);

}

/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
updateNumWins function takes no parameters.

It also does not return any parameters

This will update the score for player 1 and player 2's wins 
and the number of ties as the game progresses
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void updateNumWins(){
	tft.setTextSize(2);
	// Update Player 1's score
	tft.fillRect(180,350,145,20,WHITE);
	tft.setCursor(180,350);
	tft.print(p1Wins);
	// Update Player 2's score
	tft.fillRect(180,370,145,20,WHITE);
	tft.setCursor(180,370);
	tft.print(p2Wins);
	// Update the number of ties
	tft.fillRect(60,390,100,20,WHITE);
	tft.setCursor(60,390);
	tft.print(numTies);
}

/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
p1WinScreen function takes no parameters.

It also does not return any parameters

This will increase player 1's score, update it on the screen
and load the player 1 winning screen
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void p1WinScreen(){
	tft.setTextSize(6);
	tft.setTextColor(WHITE);
	tft.fillRect(0,0,320,325,RED);
	tft.setCursor(20,60);
	tft.print("PLAYER 1");
	tft.setCursor(80,130);
	tft.print("WINS!");
	tft.setCursor(100,190);
	tft.print(":D");
	tft.setTextColor(BLACK);
	// Increment the wins by 1 and update it to appear on the screen
	p1Wins++;
	updateNumWins();
	// Set to 1 since this round is over and nothing will happen
	// until the reset button is pressed
	endGame = 1;

}
/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
p2WinScreen function takes no parameters.

It also does not return any parameters

This will increase player 2's score, update it on the screen
and load the player 2 winning screen
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void p2WinScreen(){
	tft.setTextSize(6);
	tft.setTextColor(WHITE);
	tft.fillRect(0,0,320,325,BLUE);
	tft.setCursor(20,60);
	tft.print("PLAYER 2");
	tft.setCursor(80,130);
	tft.print("WINS!");
	tft.setCursor(100,190);
	tft.print(":D");
	tft.setTextColor(BLACK);
	p2Wins++;
	updateNumWins();
	endGame = 1;

}


/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
tieScreen function takes no parameters.

It also does not return any parameters

This will increase the count for number of ties, update it 
on the screen and load the tie screen
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */  
void tieScreen(){
	tft.setTextSize(9);
	tft.setTextColor(WHITE);
	tft.fillRect(0,0,320,325,BLACK);
	tft.setCursor(60,90);
	tft.print("TIE!");
	tft.setCursor(90,190);
	tft.print(":(");
	tft.setTextColor(BLACK);
	numTies++;
	updateNumWins();
	endGame = 1;


}


/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
printTurn function takes no parameters.

It also does not return any parameters

This will print who's turn it is on the screen 
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void printTurn(){
	// If it's player 1's turn
	tft.setTextSize(2);
	tft.setCursor(5,330);
	if (turn == 1){
		tft.fillRect(0,330,320,20,WHITE);
		tft.print("Player 1's turn!");

	}
	// If it's player 2's turn
	if (turn == 2){
		tft.fillRect(0,330,320,20,WHITE);
		tft.print("Player 2's turn!");

	}
}

/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
changeTurns function takes no parameters.

It also does not return any parameters

This will simply just change the value of the 
turn variable depending on who's turn it is when you use it
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void changeTurns(){
	if (turn == 2){
		turn = 1;
	}
	else if (turn == 1){
		turn = 2;
	}

}

/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
TwoPlayerMode function takes no parameters.

It also does not return any parameters

This will setup the player vs player gamemode
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void TwoPlayerMode(){
	tft.fillScreen(BLACK);
	tft.fillRect(0,325,320,155,WHITE);
	tft.fillRect(176,396,138,68,BLACK);
	tft.fillRect(180,400,130,60,CYAN);
	tft.setCursor(200,420);
	tft.print("RESET");
	tft.setTextSize(2);
	tft.setCursor(5,350);
	tft.print("Player 1 score: ");
	tft.setCursor(5,370);
	tft.print("Player 2 score: ");
	tft.setCursor(5,390);
	tft.print("Ties: ");
	drawBoard();
	printTurn();
	int flag = 0;
	while (flag != 1){
		TSPoint touch = ts.getPoint();
    	pinMode(YP,OUTPUT);
    	pinMode(XM,OUTPUT);
        if(touch.z > MINPRESSURE && touch.z < MAXPRESSURE){
            int16_t point_x = map(touch.x, TS_MINX, TS_MAXX,0,tft.width());
            int16_t point_y = map(touch.y, TS_MAXY, TS_MINY,0,tft.height());
            // Reset button
			if(point_x > 180 && point_x < 310 && point_y > 400 && point_y < 460){
				drawBoard();
				Upperleft = 0 ,Uppermid = 0, Upperright = 0,Middleleft = 0, Middlemid = 0,Middleright = 0;
				Lowerleft = 0, Lowermid = 0, Lowerright = 0;
				UL = 0, UM = 0,UR = 0,ML = 0,MM = 0,MR = 0,LL = 0,LM = 0,LR = 0;
				endGame = 0;
				turn = 1;
				printTurn();                
            }
            // Upper left grid
            else if ((point_x > 20 && point_x < 100 && point_y > 20 && point_y <  100) && (endGame == 0) && (UL == 0)){

            	if (turn == 1){
            		drawO(10,10);
            		Upperleft = 1;
            	}
            	else if (turn == 2){
            		drawX(10,10);
            		Upperleft = 2;
            	}
            	UL = 1;
            	changeTurns();
            	printTurn();

            }

            // Upper middle grid
            else if ((point_x > 110 && point_x < 200 && point_y > 20 && point_y < 100) && (endGame == 0) && (UM == 0)){
            	if (turn == 1){
            		drawO(125,10);
            		Uppermid = 1;
            	}
            	else if (turn == 2){
            		drawX(125,10);
            		Uppermid = 2;
            	}
            	UM = 1;
            	changeTurns();
            	printTurn();

            }
            // Upper right grid
            else if ((point_x > 210 && point_x < 300 && point_y > 20 && point_y < 100)&& (endGame == 0) && (UR == 0)){
            	if (turn == 1){
            		drawO(225,10);
            		Upperright = 1;
            	}
            	else if (turn == 2){
            		drawX(225,10);
            		Upperright = 2;
            	}
            	UR = 1;
            	changeTurns();
            	printTurn();

            
            }
            // Middle left grid
            else if ((point_x > 20 && point_x < 100 && point_y > 110 && point_y < 200) && (endGame == 0) && (ML == 0)){
            	if (turn == 1){
            		drawO(10,125);
            		Middleleft = 1;
            	}
            	else if (turn == 2){
            		drawX(10,125);
            		Middleleft = 2;
            	}
            	ML = 1;
            	changeTurns();
            	printTurn();

            }
            // Middle mid grid
        	else if ((point_x > 110 && point_x < 200 && point_y > 110 && point_y < 200) && (endGame == 0) && (MM == 0)){
        		if (turn == 1){
            		drawO(125,125);
            		Middlemid = 1;
            	}
            	else if (turn == 2){
            		drawX(125,125);
            		Middlemid = 2;
            	}
            	MM = 1;
            	changeTurns();
            	printTurn();

        	}
        	// Middle right grid
        	else if ((point_x > 210 && point_x < 300 && point_y > 110 && point_y < 200) && (endGame == 0) && (MR == 0)){
        		if (turn == 1){
            		drawO(225,125);
            		Middleright = 1;
            	}
            	else if (turn == 2){
            		drawX(225,125);
            		Middleright = 2;
            	}
            	MR = 1;
            	changeTurns();
            	printTurn();
        	}
        	// Lower left grid
        	else if ((point_x > 20 && point_x < 100 && point_y > 210 && point_y < 300) && (endGame == 0) && (LL == 0)){
        		if (turn == 1){
            		drawO(10,225);
            		Lowerleft = 1;
            	}
            	else if (turn == 2){
            		drawX(10,225);
            		Lowerleft = 2;
            	}
            	LL = 1;
            	changeTurns();
            	printTurn();
        	}
        	// Lower mid grid
        	else if ((point_x > 110 && point_x < 200 && point_y > 210 && point_y < 300) && (endGame == 0) && (LM == 0)){
        		if (turn == 1){
            		drawO(125,225);
            		Lowermid = 1;
            	}
            	else if (turn == 2){
            		drawX(125,225);
            		Lowermid = 2;
            	}
            	LM = 1;
            	changeTurns();
            	printTurn();

        	}
        	// Lower right grid
        	else if ((point_x > 210 && point_x < 300 && point_y > 210 && point_y < 300) && (endGame == 0) && (LR == 0)){
        		if (turn == 1){
            		drawO(225,225);
            		Lowerright = 1;
            	}
            	else if (turn == 2){
            		drawX(225,225);
            		Lowerright = 2;
            	}
            	LR = 1;
            	changeTurns();
            	printTurn();
			
			}
			// Using brute force algorithm to determine the winner, even better use an or || instead of ifs
			// and end result is either loading p1 or p2
			// Player 1 winning cases
			if ((Upperleft == 1) && (Uppermid == 1) && (Upperright == 1) && (endGame == 0)){
				p1WinScreen();
			}
			else if ((Middleleft == 1) && (Middlemid == 1) && (Middleright == 1) && (endGame == 0)){
				p1WinScreen();
			}
			else if ((Lowerleft == 1) && (Lowermid == 1) && (Lowerright == 1) && (endGame == 0)){
				p1WinScreen();
			}
			else if ((Upperleft == 1) && (Middleleft == 1) && (Lowerleft == 1) && (endGame == 0)){
				p1WinScreen();
			}
			else if ((Uppermid == 1) && (Middlemid == 1) && (Lowermid == 1) && (endGame == 0)){
				p1WinScreen();
			}
			else if ((Upperright == 1) && (Middleright == 1) && (Lowerright == 1) && (endGame == 0)){
				p1WinScreen();
			}
			else if ((Upperleft == 1) && (Middlemid == 1) && (Lowerright == 1) && (endGame == 0)){
				p1WinScreen();
			}
			else if ((Upperright == 1) && (Middlemid == 1) && (Lowerleft == 1) && (endGame == 0)){
				p1WinScreen();
			}

			// Player 2 winning cases
			else if ((Upperleft == 2) && (Uppermid == 2) && (Upperright == 2) && (endGame == 0)){
				p2WinScreen();
			}
			else if ((Middleleft == 2) && (Middlemid == 2) && (Middleright == 2) && (endGame == 0)){
				p2WinScreen();
			}
			else if ((Lowerleft == 2) && (Lowermid == 2) && (Lowerright == 2) && (endGame == 0)){
				p2WinScreen();
			}
			else if ((Upperleft == 2) && (Middleleft == 2) && (Lowerleft == 2) && (endGame == 0)){
				p2WinScreen();
			}
			else if ((Uppermid == 2) && (Middlemid == 2) && (Lowermid == 2) && (endGame == 0)){
				p2WinScreen();
			}
			else if ((Upperright == 2) && (Middleright == 2) && (Lowerright == 2) && (endGame == 0)){
				p1WinScreen();
			}
			else if ((Upperleft == 2) && (Middlemid == 2) && (Lowerright == 2) && (endGame == 0)){
				p2WinScreen();
			}
			else if ((Upperright == 2) && (Middlemid == 2) && (Lowerleft == 2) && (endGame == 0)){
				p2WinScreen();
			}
			// Tie case
			else if ((Upperleft != 0) && (Uppermid != 0) && (Upperright != 0) && (Middleleft != 0) && (Middlemid != 0) && (Middleright != 0) && (Lowerleft != 0) && (Lowermid != 0) && (Lowerright != 0) && (endGame == 0)){
				tieScreen();
			}

        }

	}

}

/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
	TO BE UPDATED LATER...

OnePlayerMode function takes no parameters.

It also does not return any parameters

This will setup the player to play against the CPU.
To play against the CPU, we generate a  random number from 0-8 to
determine where the computer will place an X and then check
if that spot is occupied or not using the flags initialized
above.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void OnePlayerMode(){

}

/* * * * * * * * * * *  * * * * * * * * * * * * * * * * *
processTouchScreen function takes 4 parameters.

It does not return any parameters

This will allows us to determine where to go when the 
touchscreen buttons are pressed using enumeration. 

NOTE: Further changes will me made to this function to allow
 	  it to be compatible with the single player mode
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 
void processTouchScreen(int x, int x2, int y, int y2) {

    TSPoint touch = ts.getPoint();

    pinMode(YP,OUTPUT);
    pinMode(XM,OUTPUT);

        if(touch.z > MINPRESSURE && touch.z < MAXPRESSURE){
            int16_t point_x = map(touch.x, TS_MINX, TS_MAXX,0,tft.width());
            int16_t point_y = map(touch.y, TS_MAXY, TS_MINY,0,tft.height());
            Serial.print("x = " );
            Serial.println(point_x);
            Serial.print("y = ");
            Serial.println(point_y);
            if (point_x > x && point_x < x2 && point_y > y && point_y < y2){

                if (current_state==Main) {
                    current_state=TwoPlayer;
                }
                // TO BE UPDATED: make a "QUIT" to main menu button
               // if (current_state == TwoPlayer){
                //	current_state = Main;
                //}
            }
        }
    

}


int main(){
	setup();
	while(true){
		// Using enumeration and the processTouchScreen function to determine which mode to load
		if (current_state == Main){
			drawMainMenu();
			while (current_state==Main) {
                processTouchScreen(70,250,220,290);
            }
		}
		if (current_state == TwoPlayer){
			TwoPlayerMode();
		}
		if (current_state == OnePlayer){
			OnePlayerMode();
		}
	}

}
