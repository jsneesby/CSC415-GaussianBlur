/* 
* File:   gaussBlur.cpp
* Author: Jared Sneesby
*/

#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#include <cstdlib>
#include <sys/time.h>
#include "bitmap_image.hpp" //image library

using namespace std;

/* Function Definitons
/
*/
void gaussinBlurBMP(bitmap_image& image, int kernelRadius);
double timeDifference (struct timeval * start, struct timeval * end);
struct kernelElement {double value; rgb_store color;}; // structure to hold kernal element gaussian value and rgb colors


/* Main Method
/  --> takes terminal input for file name 
/  --> gets bitmap image from input name
/  --> passes file reference to gaussian blur function
/  --> saves file and outputs timing data
*/
int main(int argc, char** argv) {
	// check count of arguments
	if(argc < 2){
		fprintf(stdout,"\nEnter File Name:  <filename.bmp> \n", argv[0]);
		exit(0); //if there are not enough arguments, exit program
	}

	// Ron's timing vars
	struct timeval pg_start, pg_end, start, end; 
	gettimeofday(&pg_start, NULL);  // program start time check

	//taking string input and getting image
	string s = argv[1];
	bitmap_image image(s);  // -------------------------------------------------> add validation of file name, if not, exit program

	/* Image Effect
	/  --> take the given image and calculate new image with gaussian blur
	*/ 
	gettimeofday(&start, NULL);  //blur start
	gaussinBlurBMP(image, 3.);
	gettimeofday(&end, NULL); // blur end

	// save the image as a new file
	image.save_image("blurImage.bmp"); 
	gettimeofday(&pg_end, NULL); // program end


	// Print timing information (From Ron's Methods)
	printf("\nProgram Execution time: %.2f usec.\n", timeDifference(&pg_start, &pg_end));
	printf("Blur Execution time: %.2f usec.\n", timeDifference(&start, &end));
	double speedupPotential =  timeDifference(&pg_start, &pg_end)/(timeDifference(&pg_start, &pg_end)-timeDifference(&start, &end));
	printf("Potential Speedup: %.2f \n", speedupPotential);

	return 0;
}

double timeDifference (struct timeval * start, struct timeval * end)
{
	struct timeval diff;

	if (start->tv_sec == end->tv_sec) {
		diff.tv_sec = 0;
		diff.tv_usec = end->tv_usec - start->tv_usec;
	}
	else {
		diff.tv_usec = 1000000 - start->tv_usec;
		diff.tv_sec = end->tv_sec - (start->tv_sec + 1);
		diff.tv_usec += end->tv_usec;

		if (diff.tv_usec >= 1000000) {
			diff.tv_usec -= 1000000;
			diff.tv_sec += 1;
		}
	}

	return ((double)diff.tv_sec + (diff.tv_usec / 1.0e6)) * 1.0e6;
}


/* Gaussian Blur 
/  --> Main algorithm implimentation
/
/ @params --> image(bitmap_image): reference to image in main method
/         --> kernelRadius(int): desired radius of kernel  
/
/ @vars   --> image: reference to full sized bitmap image /////////////////////////////////////////////////-------------------------------> fill in all major params
/         --> kernelRadius: integer
/         --> kernelRadius: integer
/         --> kernelRadius: integer
/         --> kernelRadius: integer
/         --> kernelRadius: integer
/         --> kernelRadius: integer
*/
void gaussinBlurBMP(bitmap_image& image, int kernelRadius){

	int w = image.width(); 
	int h = image.height();
	int kernelSize = 2*kernelRadius+1; // width and height of kernel, assumes square kernel

	// Compute kernel size (2d set of vectors
	size_t kernel2dSize = kernelSize*kernelSize*sizeof(kernelElement) * sizeof(kernelElement);
	vector< vector<kernelElement> >kernel2d = (vector< vector<kernelElement> >) malloc(kernel2dSize);
	
	vector< vector<kernelElement> > kernel2d  (kernelSize, vector< vector<kernelElement>(kernelSize));
    //kernel2d (kernelSize, vector<kernelElement>(kernelSize));  // make the 2d kernel to hold surrounding pixel informatnion
	
	float sigma = kernelRadius/2.0; // calculate sigma
	double sum, redSum, greenSum, blueSum;  // sum of values within each kernel
	int currentX, currentY;
	unsigned char newRed, newGreen, newBlue;
	double kernelValue; //value of weight

	//Start Main Loop --------------------------------------------------------------------------------------------------------------------------------> add edge case algos 
	for (unsigned int x = 0 + kernelRadius ; x < w; ++x){ // move over width
		for (unsigned int y = 0 + kernelRadius; y < h; ++y){ // move over height
			sum = 0; //reset to 0 for each pixel

			/* kernel Calculation
			/  --> calculate the new color mapping for pixel (x,y)
			/  --> iterate over row and col of kernel corresponding to each pixel
			*/
			for (int i = 0; i < kernelSize ; i++){ // kernel row
				for (int j = 0; j < kernelSize; j++) { // kernel column
					/* Gaussian Mapping -- where the magic happens
					/  --> maps pixel within kernel to a value between 0->1 
					/  --> mapping uses e^-x to give outer pixels less significance
					/  --> directly within loops to avoid activation record overhead
					/  --> e^-x * e^-y = e^-(x+y)
					*/
					kernel2d[i][j].value =exp( -((((i - kernelRadius)/(sigma))*((i - kernelRadius)/(sigma)) + (((j - kernelRadius)/(sigma))*((j - kernelRadius)/(sigma))))/2.0) );

					// Gets appropriate pixel for manipulation -------------------------------------------------------------------------------------------------------------> get reference to pixel instead, no change will occur
					currentX = x-kernelRadius+i;
					currentY = y-kernelRadius+j;
					kernel2d[i][j].color = {image.red_channel(currentX , currentY), image.green_channel(currentX , currentY), image.blue_channel(currentX , currentY)};

					// adds this kernel value to kernal sum
					sum += kernel2d[i][j].value;
				}
			}
			
			redSum = 0, greenSum=0, blueSum=0;
			// normalize (map all values to add up to 1 using their ratio relative to the sum)
			for (int i = 0; i < kernelSize; i++){
				for (int j = 0; j < kernelSize; j++){
					kernel2d[i][j].value /= sum;

					// summing on double variable to keep small outside pixel influences
					redSum += kernel2d[i][j].color.red * kernel2d[i][j].value;
					greenSum += kernel2d[i][j].color.green * kernel2d[i][j].value;
					blueSum += kernel2d[i][j].color.blue * kernel2d[i][j].value;

					//trouble shoot: printf("RGB[%d, %d]: {%.2f %.2f %.2f} += %.7f x {%d %d %d} \n", i, j, redSum, greenSum, blueSum, kernel2d[i][j].value, kernel2d[i][j].color.red, kernel2d[i][j].color.green, kernel2d[i][j].color.blue);
					
				}
				
			}
			
			// change pixel in the image
			image.set_pixel(x, y, (unsigned char) redSum  , (unsigned char) greenSum , (unsigned char) blueSum) ;

		}
	}
}
