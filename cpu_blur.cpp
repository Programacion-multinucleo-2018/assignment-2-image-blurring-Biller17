#include <iostream>
#include <string>
#include <cmath>
#include <time.h>
#include <chrono>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

//Adrian Biller A10018940 cpu blur code



//convolution matrix dimension *must be odd number*
#define BLURMATRIX 5

using namespace std;
// using namespace cv;





void blur(const cv::Mat& input, cv::Mat& output, int width, int height)
{


//checking for every pixel in the image
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      int convolutionDims = BLURMATRIX * BLURMATRIX;
      int red = 0;
      int green = 0;
      int blue = 0;



      // cout<<" posicion ("<<i<<","<<j<<")"<<endl;
      //for that goes through the blur matrix area
      for(int convolutionX = i - floor(BLURMATRIX/2); convolutionX < i + floor(BLURMATRIX/2); convolutionX++){
        for(int convolutionY = j - floor(BLURMATRIX/2); convolutionY < j + floor(BLURMATRIX/2); convolutionY++){
          if(convolutionX > 0 && convolutionX < width && convolutionY > 0 && convolutionY < height){
            //getting every value of the BLURMATRIX
            blue += input.at<cv::Vec3b>(convolutionY, convolutionX)[0];
            green += input.at<cv::Vec3b>(convolutionY, convolutionX)[1];
            red += input.at<cv::Vec3b>(convolutionY, convolutionX)[2];
          }

        }
      }
      //mean of all pixels around the selected pixel
      blue = floor(blue/convolutionDims);
      green = floor(green/convolutionDims);
      red = floor(red/convolutionDims);
      // cout<<"("<<i<<","<<j<<")"<<endl;
      //asigning mean to target pixel
      output.at<cv::Vec3b>(j, i)[0] = blue;
      output.at<cv::Vec3b>(j, i)[1] = green;
      output.at<cv::Vec3b>(j, i)[2] = red;
    }
  }
}



int main(int argc, char *argv[])
{
  srand(time(NULL));
	string imagePath;

	if(argc < 2)
		imagePath = "big_img.jpg";
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
  cv::Mat output(input.rows, input.cols, input.type());

	//Call the wrapper function



  auto start_cpu =  std::chrono::high_resolution_clock::now();
  blur(input, output, input.cols, input.rows);
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
