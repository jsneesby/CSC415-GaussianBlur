/* 
* File:   gaussBlur.cpp
* Author: Jared Sneesby
* Linux Compile: g++ -o <testBuild> -O3 GaussianBlur.cpp -std=c++11
* Cgywin Compile: g++ -o <testBuild> -O3 GaussianBlur.cpp -std=c++0x
* Run: ./<testBuild> <theImage.bmp> <Blur Radius: 1-9>
*/

#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#include <cstdlib>
#include <sys/time.h>
#include "bitmap_image.hpp" //image library

using namespace std;

/* Function Definitons */
double gaussianFilterValue(int& i, int& j, int& kernelRadius,float& sigma);
bitmap_image gaussinBlurBMP(bitmap_image& image, int kernelRadius);
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
	if(argc != 3){
		fprintf(stdout,"\nEnter File Name:  <filename.bmp> <Kernel Radius: integer [1,9]> \n", argv[0]);
		exit(0); //if there are not enough arguments, exit program
	}

	// Ron's timing vars
	struct timeval pg_start, pg_end, start, end; 
	gettimeofday(&pg_start, NULL);  // program start time check

	//taking string input and getting image
	string s = argv[1];
	bitmap_image image(s); 

	int kernelRadius = (int)(argv[2][0] - '0');

	/* Image Effect
	/  --> take the given image and calculate new image with gaussian blur
	*/ 
	gettimeofday(&start, NULL);  //blur start
	bitmap_image newImage(gaussinBlurBMP(image, kernelRadius));
	gettimeofday(&end, NULL); // blur end

	// save the image as a new file
	newImage.save_image("blurImage.bmp"); 
	gettimeofday(&pg_end, NULL); // program end


	// Print timing information (From Ron's Methods)
	printf("\nProgram Execution time: %.7f sec.\n", timeDifference(&pg_start, &pg_end));
	printf("Blur Execution time: %.7f sec.\n", timeDifference(&start, &end));
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

	return ((double)diff.tv_sec + (diff.tv_usec / 1.0e6)) ;//* 1.0e6;
}

/* Gaussian Mapping -- where the magic happens
/  --> maps pixel within kernel to a value between 0->1 
/  --> mapping uses e^-x to give outer pixels less significance
/  --> directly within loops to avoid activation record overhead
/  --> e^-x * e^-y = e^-(x+y)
*/
double gaussianFilterValue(int& i, int& j, int& kernelRadius,float& sigma){
	return exp( -((((i - kernelRadius)/(sigma))*((i - kernelRadius)/(sigma)) + (((j - kernelRadius)/(sigma))*((j - kernelRadius)/(sigma))))/2.0) );
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
bitmap_image gaussinBlurBMP(bitmap_image& image, int kernelRadius){
	// need to do calculations off of different image, not original (otherwise everything is a bit off)
	//bitmap_image theImage = image;
	const unsigned int w = image.width(); 
	const unsigned int  h = image.height();
	
	bitmap_image newImage(w, h);


	int kernelSize = 2*kernelRadius+1; // width and height of kernel, assumes square kernel
	float sigma = kernelRadius/2.0; // calculate sigma
	double sum, redSum, greenSum, blueSum;  // sum of values within each kernel
	int currentX, currentY;
	unsigned char newRed, newGreen, newBlue;

	vector< vector<kernelElement> > kernel2d  (kernelSize, vector<kernelElement>(kernelSize));
	vector< vector<kernelElement> > kernel2dBoundry  (kernelSize, vector<kernelElement>(kernelSize));

	sum=0;
	// make the 2d kernel to hold surrounding pixel weights --> done outside loop because it will not change
	for (int i = 0; i < kernelSize; i++){
		for (int j = 0; j < kernelSize; j++){
			kernel2d[i][j].value = gaussianFilterValue(i, j, kernelRadius, sigma);

			// adds this kernel value to kernal sum
			sum += kernel2d[i][j].value;
		}
	}
	// normalize (map all values to add up to 1 using their ratio relative to the sum)
	for (int i = 0; i < kernelSize; i++){
		for (int j = 0; j < kernelSize; j++){
			kernel2d[i][j].value /= sum;
		}
	}

	// -------------------------------------------------------------Start Main Area-------------------------------------------------------------------------------------//
	for (int x = 0; x < w; ++x){ // move over width
		for (int y = 0; y < h; ++y){ // move over height
			sum = 0, redSum = 0, greenSum=0, blueSum=0; //reset sums to 0 for each pixel

			/* Kernel Calculation
			/  --> calculate the new color mapping for pixel (x,y)
			/  --> iterate over row and col of kernel corresponding to each pixel
			*/

			// Gets appropriate pixel for manipulation (takes care of boundry cases)
			if(x-kernelRadius<0 || x+kernelRadius>w || y-kernelRadius<0 || y+kernelRadius>h ){  //if first or last elements of kernal will be out of bounds on any side
				// make the 2d kernel to hold surrounding pixel weights --> re-done inside loop where some values be out of bounds
				for (int i = 0; i < kernelSize; i++){
					for (int j = 0; j < kernelSize; j++){
						if(x-kernelRadius+i<0 || x-kernelRadius+i>w || y-kernelRadius+j<0 || y-kernelRadius+j>h ){//where there is no pixel in image for current (i,j)
							kernel2dBoundry[i][j].color = {0,0,0};
							kernel2dBoundry[i][j].value = 0.0;
							sum += 0.0;
						}
						else{ 
							kernel2dBoundry[i][j].value = gaussianFilterValue(i, j, kernelRadius, sigma);
							// adds this kernel value to kernal sum
							sum += kernel2dBoundry[i][j].value;
							
							// calculate kernel mapping
							currentX = x-kernelRadius+i;
							currentY = y-kernelRadius+j;
							kernel2dBoundry[i][j].color = {image.red_channel(currentX , currentY), image.green_channel(currentX , currentY), image.blue_channel(currentX , currentY)};
						}
					}
				}
				// normalize (map all values to add up to 1 using their ratio relative to the sum)
				for (int i = 0; i < kernelSize; i++){
					for (int j = 0; j < kernelSize; j++){
						kernel2dBoundry[i][j].value /= sum;
					}
				}

				// sum the values of special case kernel
				for (int i = 0; i < kernelSize ; i++){ // kernel row
					for (int j = 0; j < kernelSize; j++) { // kernel column	
						// summing on double variable to keep small outside pixel influences
						redSum += kernel2dBoundry[i][j].color.red * kernel2dBoundry[i][j].value;
						greenSum += kernel2dBoundry[i][j].color.green * kernel2dBoundry[i][j].value;
						blueSum += kernel2dBoundry[i][j].color.blue * kernel2dBoundry[i][j].value;
					}
				}
				newImage.set_pixel(x, y, (unsigned char) redSum  , (unsigned char) greenSum , (unsigned char) blueSum) ;
			}
			else{ // all elements of kernel will be in bounds
				for (int i = 0; i < kernelSize ; i++){ // kernel row
					for (int j = 0; j < kernelSize; j++) { // kernel column		
						currentX = x-kernelRadius+i;
						currentY = y-kernelRadius+j;
						kernel2d[i][j].color = {image.red_channel(currentX , currentY), image.green_channel(currentX , currentY), image.blue_channel(currentX , currentY)};

						// summing on double variable to keep small outside pixel influences
						redSum += kernel2d[i][j].color.red * kernel2d[i][j].value;
						greenSum += kernel2d[i][j].color.green * kernel2d[i][j].value;
						blueSum += kernel2d[i][j].color.blue * kernel2d[i][j].value;
						//troubleshoot: printf("RGB[%d, %d]: {%.2f %.2f %.2f} += %.7f x {%d %d %d} \n", i, j, redSum, greenSum, blueSum, kernel2d[i][j].value, kernel2d[i][j].color.red, kernel2d[i][j].color.green, kernel2d[i][j].color.blue);
					}
				}
			}
			// change pixel in the image
			newImage.set_pixel(x, y, (unsigned char) redSum  , (unsigned char) greenSum , (unsigned char) blueSum) ;
		}
	}
	return newImage;
	//-----------------------------------------------------------------Main Area End-------------------------------------------------------------------------------------//
}