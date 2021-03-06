#include <iostream>
#include <cstdio>
#include <cmath>
#include <chrono>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "common/common.h"
#include <cuda_runtime.h>

using namespace std;


//Adrian Biller A10018940 gpu blur code


//convolution matrix dimension *must be odd number*
#define BLURMATRIX 5
//nvcc -o test Mult2D_1D.cu -std=c++11


// input - input image one dimensional array
// ouput - output image one dimensional array
// width, height - width and height of the images
// colorWidthStep - number of color bytes (cols * colors)
// grayWidthStep - number of gray bytes
__global__ void blur_kernel(unsigned char* input, unsigned char* output, int width, int height, int colorWidthStep, int grayWidthStep)
{
	// 2D Index of current thread
	const int xIndex = blockIdx.x * blockDim.x + threadIdx.x;
	const int yIndex = blockIdx.y * blockDim.y + threadIdx.y;

	// Only valid threads perform memory I/O
	if ((xIndex < width) && (yIndex < height))
	{
		const int blur_tid = yIndex * colorWidthStep + (3 * xIndex);
		int convolutionDims = BLURMATRIX * BLURMATRIX;
		int red = 0;
		int green = 0;
		int blue = 0;



		// cout<<" posicion ("<<i<<","<<j<<")"<<endl;
		//for that goes through the blur matrix area
		for(int convolutionX = xIndex - BLURMATRIX/2; convolutionX < xIndex + BLURMATRIX/2; convolutionX++){
			for(int convolutionY = yIndex - BLURMATRIX/2; convolutionY < yIndex + BLURMATRIX/2; convolutionY++){
				if(convolutionX > 0 && convolutionX < width && convolutionY > 0 && convolutionY < height){
					int btid = convolutionY * grayWidthStep + (3 * convolutionX);
					//getting every value of the BLURMATRIX
					blue += input[btid];
					green += input[btid+1];
					red += input[btid+2];
				}

			}
		}
		//mean of all pixels around the selected pixel
		blue = blue/convolutionDims;
		green = green/convolutionDims;
		red = red/convolutionDims;
		// cout<<"("<<i<<","<<j<<")"<<endl;
		//asigning mean to target pixel

		output[blur_tid] = static_cast<unsigned char>(blue);
		output[blur_tid+1] = static_cast<unsigned char>(green);
		output[blur_tid+2] = static_cast<unsigned char>(red);
	}
}

void blur_image(const cv::Mat& input, cv::Mat& output)
{
	cout << "Input image step: " << input.step << " rows: " << input.rows << " cols: " << input.cols << endl;
	// Calculate total number of bytes of input and output image
	// Step = cols * number of colors
	size_t colorBytes = input.step * input.rows;
	size_t blurBytes = output.step * output.rows;

	unsigned char *d_input, *d_output;

	// Allocate device memory
	SAFE_CALL(cudaMalloc<unsigned char>(&d_input, colorBytes), "CUDA Malloc Failed");
	SAFE_CALL(cudaMalloc<unsigned char>(&d_output, blurBytes), "CUDA Malloc Failed");

	// Copy data from OpenCV input image to device memory
	SAFE_CALL(cudaMemcpy(d_input, input.ptr(), colorBytes, cudaMemcpyHostToDevice), "CUDA Memcpy Host To Device Failed");

	// Specify a reasonable block size
	const dim3 block(32, 32);

	// Calculate grid size to cover the whole image
	// const dim3 grid((input.cols + block.x - 1) / block.x, (input.rows + block.y - 1) / block.y);
	const dim3 grid((int)ceil((float)input.cols / block.x), (int)ceil((float)input.rows/ block.y));
	printf("blur_kernel<<<(%d, %d) , (%d, %d)>>>\n", grid.x, grid.y, block.x, block.y);

	// Launch the color conversion kernel
	blur_kernel <<<grid, block >>>(d_input, d_output, input.cols, input.rows, static_cast<int>(input.step), static_cast<int>(output.step));

	// Synchronize to check for any kernel launch errors
	SAFE_CALL(cudaDeviceSynchronize(), "Kernel Launch Failed");

	// Copy back data from destination device meory to OpenCV output image
	SAFE_CALL(cudaMemcpy(output.ptr(), d_output, blurBytes, cudaMemcpyDeviceToHost), "CUDA Memcpy Host To Device Failed");

	// Free the device memory
	SAFE_CALL(cudaFree(d_input), "CUDA Free Failed");
	SAFE_CALL(cudaFree(d_output), "CUDA Free Failed");
}

int main(int argc, char *argv[])
{
	string imagePath;

	if(argc < 2)
		imagePath = "small_img.jpg";
  	else
  		imagePath = argv[1];

	// Read input image from the disk
	cv::Mat input = cv::imread(imagePath, CV_LOAD_IMAGE_COLOR);

	if (input.empty())
	{
		cout << "Image Not Found!" << std::endl;
		cin.get();
		return -1;
	}

	//Create output image										Formato de imagen
	cv::Mat output(input.size(), input.type());
	// cv::Mat output(input.rows, input.cols, CV_8UC1);

	//Call the wrapper function
	auto start_cpu =  std::chrono::high_resolution_clock::now();
	blur_image(input, output);
	auto end_cpu =  std::chrono::high_resolution_clock::now();
	std::chrono::duration<float, std::milli> duration_ms = end_cpu - start_cpu;
	printf("Blurring of image time: %f ms\n", duration_ms.count());
	//Allow the windows to resize
	namedWindow("Input", cv::WINDOW_NORMAL);
	namedWindow("Output", cv::WINDOW_NORMAL);

	//Show the input and output
	imshow("Input", input);
	imshow("Output", output);

	//Wait for key press
	cv::waitKey();

	return 0;
}
