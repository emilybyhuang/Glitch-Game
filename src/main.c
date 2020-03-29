#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

short int grid[10][10];//each time it draws, make grid occupied by the colour value
int sideLength = 24;
volatile int pixel_buffer_start = 0xC8000000; // global variable
                      //red   yellow   green    cyan    blue   magenta
short int colour[6] = {0xF800, 0xFFE0, 0x07E0, 0x07FF, 0x001F, 0xF81F};
short int backgroundColour = 0x0000;
int wall[8] = {0, 0, 9, 9, 0 ,0 ,9 ,9 };//-1 if not taken
int middleOpening[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
int dir[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
short int wallColour[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
int direction = 0;
int opening = 0;
bool movePlayerSignal = false; // =true if to move
int prevPlayerX, prevPlayerY;
int keyValue;

short int colourWall = 0, playerColour = 0x07E0, white = 0xFFFF;
short int occupiedCol[10] = {0xFFFF};
short int occupiedRow[10] = {0xFFFF};
int topWall = 0, bottomWall = 9, counter = 0, playerX = 4, playerY = 4, currentKeyValue;
//int random_eight = 0;

void clearVerticalWall(int col);
void clearHorizontalWall(int row);
void wallLeftRight(int colToColour,int middleOpening, short int wallColour);
void wallUpDown(int rowToColour,int middleOpening, short int wallColour);
void clear_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void draw_grid(int a, int b, short int gridColour);
void draw_background();
//void randomBars(int direction, int opening, int colour_wall);
void randomBars();
void speed_adjust(int startValue);
void movePlayerDown();
void movePlayerUp();
void movePlayerLeft();
void movePlayerRight();

int main(void){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();
    draw_background(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
	clear_screen();
	draw_background();
    for(int i = 0; i < 8; i++){
        middleOpening[i] = rand()%8+1;
        wallColour[i] = colour[rand()%6];
        dir[i] = rand()%4;
    }

    //for 1 wall at a time
    direction = dir[rand() % 8];
    opening = middleOpening[rand() % 8];
    colourWall = wallColour[rand() % 8];  

    while (1){   	
		//random_eight = rand() % 8;
	    //direction = dir[random_eight];
	    //opening = middleOpening[random_eight];
	    //colour_wall = wallColour[random_eight];  
        speed_adjust(5000);
	    volatile int *key_ptr = (int *)0xFF200050;
    	int key_value = *key_ptr;
		// value == 1 is right; value == 2 is down; value == 3 is up; value == 4 is left;
		
        if(movePlayerSignal == true && key_value == 0){
            draw_grid(prevPlayerX, prevPlayerY, white);
            draw_grid(playerX, playerY, playerColour);
            movePlayerSignal = false;
        }else draw_grid(playerX, playerY, playerColour);
        if(!movePlayerSignal){
            keyValue = *key_ptr;
            if(key_value == 1) {
                movePlayerRight();
            } else if(key_value == 2) {
                movePlayerDown();
            } else if(key_value == 4) {
                movePlayerUp();
            } else if(key_value == 8) {
                movePlayerLeft();
            }
        }
        
		wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
    }
}



void movePlayerDown(){
        prevPlayerX = playerX;
        prevPlayerY = playerY;
        movePlayerSignal = true;
        playerY += 1;
        if(playerY == 10) playerY = 9;
}

void movePlayerUp(){
        prevPlayerX = playerX;
        prevPlayerY = playerY;
        movePlayerSignal = true;
        playerY -= 1;
        if(playerY == -1) playerY = 0;
}

void movePlayerRight(){
        prevPlayerX = playerX;
        prevPlayerY = playerY;
        movePlayerSignal = true;
        playerX += 1;
        if(playerX == 10) playerX = 9;
}

void movePlayerLeft(){
        prevPlayerX = playerX;
        prevPlayerY = playerY;
        movePlayerSignal = true;
        playerX -= 1;
        if(playerX == -1) playerX = 0;
}

void speed_adjust(int startValue){
    int count = startValue;
    while(count > 0){
        count--;
    }
    randomBars();
}


void randomBars(){
//void randomBars(int direction, int opening, int colourWall){
    if(counter!=0){
        for(int i = 0; i < 10; i++){
            if(occupiedCol[i] != 0xFFFF)clearVerticalWall(i);
            if(occupiedRow[i] != 0xFFFF)clearHorizontalWall(i);
        }
    }
    counter++;  
    if(direction == 0) { // From top
        wallUpDown(topWall, opening, colourWall);
        topWall += 1;
    } else if(direction == 1) { // From bottom
        wallUpDown(bottomWall, opening, colourWall);
        bottomWall -= 1;
    } else if(direction == 2) { // From left
        wallLeftRight(topWall, opening, colourWall);
        topWall += 1;
    } else if(direction == 3) { // From right
        wallLeftRight(bottomWall, opening, colourWall);
        bottomWall -= 1;
    }
    
    if(topWall == 10) {
        topWall = 0;
        direction = dir[rand() % 8];
        opening = middleOpening[rand() % 8];
        colourWall = wallColour[rand() % 8];
    } else if(bottomWall == -1) {
        bottomWall = 9;
        direction = dir[rand() % 8];
        opening = middleOpening[rand() % 8];
        colourWall = wallColour[rand() % 8];
    }
}

void clear_screen(){
    for(int j = 0; j < 240; j++){
        for(int i = 0; i < 320; i++){
            plot_pixel(i, j, backgroundColour);
        }
    }
}

void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *) 0xFF203020;//beginning addr of DMA controller
    register int status;
    *pixel_ctrl_ptr = 1;//request to synchronize with VGA timing: sets S = 1 in status register
    status = *(pixel_ctrl_ptr + 3);//get S
    while((status & 0x1)!=0){//check is S = 1?
        status = *(pixel_ctrl_ptr + 3);//if S = 1, update status variable and keep checking
    }//exit while loop when S = 0(which is when DMA controller finsihed transfering pixels)
}

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void draw_grid(int a, int b, short int gridColour){
    grid[a][b] = gridColour;
    //i, j is which index of grid to draw
    int xMin = a*sideLength;
    int yMin = b*sideLength;
    int xMax = ((a+1)*sideLength)-1;
    int yMax = ((b+1)*sideLength)-1;
    int x, y;
    
	for(x = xMin; x < xMax; x++){
    	for(y = yMin; y < yMax; y++){
            if(x % 24 == 0 || x % 24 == 23 || y % 24 == 0 || y % 24 == 23){
                //draw black
                plot_pixel(x, y, 0x0000);
            }else{
                //draw white
                plot_pixel(x,y,gridColour);
            }
        }
    }
}

void draw_background(){
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            draw_grid(i, j, 0xFFFF);
        }
    }
}

//col, row because uses x, y 
void wallLeftRight(int colToColour, int middleOpening, short int wallColour){
    occupiedCol[colToColour] = wallColour;
    //opening is from middleOpening - 1 to middleOpening + 1
    for(int row = 0; row < 10; row ++){
        if(row!=middleOpening && row!=middleOpening+1 && row!=middleOpening-1){
            draw_grid(colToColour, row, wallColour);
        }
    }
}

void wallUpDown(int rowToColour, int middleOpening, short int wallColour){
    occupiedRow[rowToColour] = wallColour;
    //opening is from middleOpening - 1 to middleOpening + 1
    for(int col = 0; col < 10; col++){
        if(col!=middleOpening && col!=middleOpening+1 && col!=middleOpening-1){
            draw_grid(col, rowToColour, wallColour);
        }
    }   
}

void clearVerticalWall(int col){
    occupiedCol[col] = white;
    for(int i = 0; i < 10; i++){
        draw_grid(col, i, white);
    }
}

void clearHorizontalWall(int row){
    occupiedRow[row] = white;
    for(int i = 0; i < 10; i++){
        draw_grid(i, row, white);
    }
}

