//============================================================================
// Name        : MultiMediaP4_krw55.cpp
// Author      : Kevin Wright
// NetID	   : krw55
// Description : Takes an Image and performs uniform quantization on it.
//============================================================================

#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <stdio.h>
#include <string.h>

using namespace std;

class Image{
public:
	bool ImportFile(const char*);
	void ExportFile(int[], int, const char*);
	double Quantize(const char* ,int);

private:
	int xMax, yMax; //x and y sizes for the data
	int maxDepth; // max depth of initial image
	int *pixel; //pointer to future array of the input data
};

bool Image::ImportFile(const char* fileName){
	string word; //for pulling values from the input image
	int intIn;
	int lnCnt = 0;	// counter for tracking that the program stays within the image.
	xMax = 0; yMax = 0; maxDepth = 0;
	pixel = new int [512 * 512]; //initializing the input array
	ifstream imgIn;	//opening the input image

	imgIn.open (fileName);
	if (imgIn.is_open()){	// if the image is able to open, continue. otherwise return false.
		while (lnCnt < 6)	//loop through the header of the image file
		{
			imgIn >> word;
		   	lnCnt ++;
		}
		//cout << "\n";
	    imgIn >> xMax;	//take the X and Y sizes of the image from the file
	   	imgIn >> yMax;
	    lnCnt = 0;
	    imgIn >> maxDepth; // takes in the max depth of the image from the file
	   	//cout << "creating an array of size " << xMax << " x " << yMax << ", with a max depth of "<< maxDepth <<";\n";
	   	for (int x = 0; x < xMax; x++){ // reading through the body of the file and putting it into the array
	   		for (int y = 0; y < yMax; y++){
	   			imgIn >> intIn;
		    	pixel[x + y * 512] = intIn;
		    	//cout << "x:" << x << " y:" << y << " = " << intIn << endl;
		    }
	   	}
	   	imgIn.close();
	   	return true;
	}
	else{
		return false;
	}
}

void Image::ExportFile(int out[512 * 512], int maxD, const char* fileName){ //makes output images, requires array of data, max Depth, and a name for the file
	ofstream imgOut;

	cout << "creating output image : " << fileName << endl;
	imgOut.open (fileName);
	imgOut << "P2\n# " << fileName <<" created by Kevin Wright.\n"; //creates header for the new image
	imgOut << xMax << "  " << yMax << '\n'; //put image size / depth info into image
	imgOut << maxD << '\n';
	int lnCnt = 0;int xCnt = 0;int yCnt = 0;	//create counters
	while( lnCnt <= ((xMax * yMax))){ //loop through the whole size of the image
		lnCnt ++;
		if (yCnt < yMax){	//loop through y dimension
			imgOut << out[xCnt + yCnt * 512]<< "  ";
			yCnt ++;
		}
		else{	// when y dimension exceeds the size of the image, drop down to the next line
			xCnt ++;
			yCnt = 0;
			imgOut << out[xCnt + yCnt * 512] << "  ";
			yCnt ++;
		}
	}
	//cout << "output done; Closing file.\n";
	imgOut.close();
}

double Image::Quantize(const char* fileName, int newDepth){ // body of the program returns MSE and runs ExportImage, needs prefix of the file name and
															//the new depth of the image to be created
	char outName[100]; //final output name for the ExportFile function to use
	const char* outEnEnd = "_EncodedOut.pgma"; // file extensions to append to the given prefix
	const char* outDeEnd = "_DecodedOut.pgma";
	const char* errorEnd = "_Error.pgma";

	double mse; //Mean Squared Error, the output of this function
	double curError; //error of current step using mse
	double totalError = 0; //sum of the error for all steps

	double difference; //used for creating error image
	double maxDif = -1;

	double normalize; //used in MSE for the output image

	int *outEn = new int [512 * 512]; // array for the output image encoded to new max value
	int *outDe = new int [512 * 512]; // array for the output image decoded to the original max value
	int *error = new int [512 * 512]; //array for the error image

	int oldDepth = maxDepth;

	double midVal = (double(maxDepth) / newDepth) / 2; //finding the midpoint of the gap between 2 discrete points

	for (int x = 0; x < xMax; x++){ //loop through input data set
		for (int y = 0; y < yMax; y++){
			normalize = pixel[x + y * 512]; //finding value for Encoded image
			normalize = (normalize / maxDepth) * newDepth;
			outEn[x + y * 512] = normalize; //put the encoded pixel values into an array

			difference = abs(pixel[x + y * 512] - (double(outEn[x + y * 512]) / newDepth) * oldDepth + midVal);
			error[x + y * 512] = difference; //calculating the Absoulte value difference for output image, as individual MSE steps
			if (difference > maxDif)		// exceed the max depth of the image.
				maxDif = difference; //to be used as the depth of the error image

			outDe[x + y * 512] = ((double(outEn[x + y * 512]) / newDepth) * oldDepth) + midVal; //takes encoded values and returns it to the range of the original maxDepth
			curError = pow(((((double(outEn[x + y * 512]) / newDepth) * oldDepth) + midVal) - pixel[x + y * 512]),2); //current step of MSE
			totalError += curError; //Summation of MSE steps so far
		}
	}
	strcpy(outName, fileName); //copies body of the name to the output name var
	strcat(outName, errorEnd);	//appends the file extension
	ExportFile(error,int(maxDif), outName); //creates error image
	delete[] (error); //deletes dynamically assigned array

	strcpy(outName, fileName);
	strcat(outName, outEnEnd);
	ExportFile(outEn, newDepth ,outName); //creates encoded image of given max depth
	delete[](outEn); //deletes dynamically assigned array

	strcpy(outName, fileName);
	strcat(outName, outDeEnd);
	ExportFile(outDe, oldDepth ,outName); //creates output image of original max depth
	delete[](outDe); //deletes dynamically assigned array

	mse = (double(1)/ (xMax * yMax)) * totalError; //last step of finding mse

	delete[] (pixel); //deletes dynamically assigned array

	return mse;
}

int main() {
	Image quant10;
	Image quant2;

	bool foundFile;
	double meanSqError;

	cout << "for: size 10\n";
	foundFile = quant10.ImportFile("baboon.pgma");
	if (foundFile){ //if the file was found, use it
		meanSqError = quant10.Quantize("baboon10",10); //body of program, creates output images and returns MSE
		cout << "MSE = " << meanSqError << endl;
	}
	else{
		cout << "Unable to find Image : baboon.pgma\n";
	}

	cout << "\nfor: size 2\n";
	foundFile = quant2.ImportFile("baboon.pgma");
	if (foundFile){ //if the file was found, use it
		meanSqError = quant2.Quantize("baboon2",2); //body of program, creates output images and returns MSE
		cout << "MSE = " << meanSqError << endl;
	}
	else{
		cout << "Unable to find Image : baboon.pgma\n";
	}

	return 0;
}

