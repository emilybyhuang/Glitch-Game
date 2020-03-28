#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int grid[10][10];
int sideLength = 24;
volatile int pixel_buffer_start = 0xC8000000; // global variable
short int colour[4] = {0xF800, 0x07E0, 0x001F, 0xAAAA};
//direction: 0:left 1:top 2:right 3:bottom 
short int backgroundColour = 0x0000;
int wall[8] = {0, 0, 9, 9, 0 ,0 ,9 ,9 };//-1 if not taken
int middleOpening[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
int dir[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
int wallColour[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

void wallLeftRight(int colToColour,int middleOpening, short int wallColour);
void wallUpDown(int rowToColour,int middleOpening, short int wallColour);
void clear_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void draw_grid(int a, int b, short int gridColour);
void draw_background();
void randomBars();

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

    for(int i = 0; i < 8; i++){
        middleOpening[i] = rand()%8+1;
        wallColour[i] = colour[rand()%3];
        dir[i] = rand()%4;
    }

    while (1){   
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
        draw_background();
        randomBars();
    }
}

void randomBars(){
    clear_screen();
    draw_background();
    //generate a bar with a random opening, random colour, coming from a random direction
    
    //find unoccupied wall element: take 1st one for now
    wallUpDown(wall[0], middleOpening[0], wallColour[0]);
    wall[0] += 1; 
    wallUpDown(wall[2], middleOpening[2], wallColour[2]);
    wall[2] -= 1;
    wallLeftRight(wall[1], middleOpening[1], wallColour[1]);
    wall[1] += 1;
    wallLeftRight(wall[3], middleOpening[3], wallColour[3]);
	wall[3] -= 1;
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
    //opening is from middleOpening - 1 to middleOpening + 1
    for(int row = 0; row < 10; row ++){
		for(int col = 0; col < 10; col++){
            if(col == colToColour){
                if(row!=middleOpening && row!=middleOpening+1 && row!=middleOpening-1){
                    draw_grid(col, row, wallColour);
                }
            }//else draw_grid(col, row, 0xFFFF);//fill the rest in with white
        }   
    }
}

void wallUpDown(int rowToColour, int middleOpening, short int wallColour){
    //opening is from middleOpening - 1 to middleOpening + 1
    for(int row = 0; row < 10; row ++){
        for(int col = 0; col < 10; col++){
            if(row == rowToColour){
                if(col!=middleOpening && col!=middleOpening+1 && col!=middleOpening-1){
                    draw_grid(col, row, wallColour);
                }
            }//else draw_grid(col, row, 0xFFFF);//fill the rest in with white
        }   
    }
}
