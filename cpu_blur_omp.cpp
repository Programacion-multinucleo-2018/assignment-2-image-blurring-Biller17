#include <iostream>
#include <string>
#include <cmath>
#include <time.h>
#include <omp.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#define BLURMATRIX 5

using namespace std;
// using namespace cv;





void blur(const cv::Mat& input, cv::Mat& output, int width, int height)
{
  omp_set_num_threads(5);

//checking for every pixel in the image
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      int convolutionDims = BLURMATRIX * BLURMATRIX;
      int red = 0;
      int green = 0;
      int blue = 0;


      int convolutionX, convolutionY;
      #pragma omp parallel for private (convolutionX, convolutionY) shared(input, output)
      //for that goes through the blur matrix area
      for(convolutionX = i - (int)floor(BLURMATRIX/2); convolutionX < i + (int)floor(BLURMATRIX/2); convolutionX++){
        for(convolutionY = j - (int)floor(BLURMATRIX/2); convolutionY < j + (int)floor(BLURMATRIX/2); convolutionY++){
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
		imagePath = "galaga.jpg";
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

  clock_t begin = clock();
  blur(input, output, input.cols, input.rows);
  clock_t end = clock();
  double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
  cout<<"Elapsed time: "<<elapsed_secs<<" seconds"<<endl;
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

// g++ -o CPU_blur_OMP CPU_blur_OMP.cpp -fopenmp -lopencv_core -lopencv_highgui -lopencv_imgproc
