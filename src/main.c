#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int grid[10][10];
int sideLength = 24;
volatile int pixel_buffer_start = 0xC8000000; // global variable
short int colour[4] = {0xF800, 0x07E0, 0x001F, 0xAAAA};
//direction: 0:left 1:top 2:right 3:bottom 


// code for subroutines (not shown)
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

void draw_grid(int a, int b){
    //i, j is which index of grid to draw
    int xMin = a*sideLength;
    int yMin = b*sideLength;
    int xMax = ((a+1)*sideLength)-1;
    int yMax = ((b+1)*sideLength)-1;
    int x, y;
	short int black = 0x0000, white = 0xFFFF;
    for(x = xMin; x < xMax; x++){
        for(y = yMin; y < yMax; y++){
            if(x % 24 == 0 || x % 24 == 23 || y % 24 == 0 || y % 24 == 23){
                //draw black
                plot_pixel(x, y, black);
            }else{
                //draw white
                plot_pixel(x,y,white);
            }
        }
    }
}

void drawBackground(){
    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++){
            draw_grid(i, j);
        }
    }
}


void wallFromLeft(int middleOpening, short int wallColour){
    //opening is from middleOpening - 1 to middleOpening + 1
    //using dir, decide where to draw
    
}

void wallFromTop(int middleOpening, short int wallColour){
    //opening is from middleOpening - 1 to middleOpening + 1
    //using dir, decide where to draw
    
}

void wallFromRight(int middleOpening, short int wallColour){
    //opening is from middleOpening - 1 to middleOpening + 1
    //using dir, decide where to draw
    
}

void wallFromBottom(int middleOpening, short int wallColour){
    //opening is from middleOpening - 1 to middleOpening + 1
    //using dir, decide where to draw
    
}

void randomBars(){
    //generate a bar with a random opening, random colour, coming from a random direction
    int middleOpening = rand()%8+1;
    int wallColour = colour[rand()%3];
    int dir = rand()%4;
    switch(dir){
        case 0:
            wallFromLeft(middleOpening, wallColour);
            break;
        case 1:
            wallFromTop(middleOpening, wallColour);
            break;
        case 2:
            wallFromRight(middleOpening, wallColour);
            break;
        case 3:
            wallFromBottom(middleOpening, wallColour);
            break; 
    }
    
    
}

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
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer

    while (1){   
        wait_for_vsync(); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
        drawBackground();
    }
}

