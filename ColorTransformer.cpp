﻿// ColorTransformer.cpp
#include "ColorTransformer.h"

// Hàm nhận vào một ảnh, thay đổi độ sáng của ảnh này và lưu kết quả vào ảnh mới
int ColorTransformer::ChangeBrighness(const Mat& sourceImage, Mat& destinationImage, short b) 
{
	if (sourceImage.data == nullptr)
		return 0;
	
	//Lấy width, height, srcChannels của sourceImage
	int width = sourceImage.cols, height = sourceImage.rows, 
	srcchannels = sourceImage.channels();
	
	//Khởi tạo ma trận destinationImage
	destinationImage = Mat(height, width, CV_8UC3);
	
	int dstchannels = destinationImage.channels();
	
	//Mảng lookup với lookup[i] gồm 256 phần tử chứa giá trị của độ sáng i đã được tăng bởi b
	//Nếu i + b < 0 thì lookup[i] = 0
	//Nếu i + b > 255 thì lookup[i] = 255
	uchar lookup[256];
	for (int i = 0; i < 256; i++)
		lookup[i] = min(255, max(0, i + b));

	//Tạo destinationImage tương ứng với giá trị R, G, B tra trong lookup
	for (int y = 0; y < height; y++)
	{
		const uchar* srcpRows = sourceImage.ptr<uchar>(y);
		uchar* dstpRows = destinationImage.ptr<uchar>(y);

		for (int x = 0; x < width; x++, srcpRows += srcchannels, dstpRows += dstchannels)
		{
			uchar B = srcpRows[0];
			uchar G = srcpRows[1];
			uchar R = srcpRows[2];

			dstpRows[0] = lookup[B];
			dstpRows[1] = lookup[G];
			dstpRows[2] = lookup[R];
		}
	}
	if (destinationImage.data == nullptr)
		return 0;
	else
		return 1;
}

// Hàm nhận vào một ảnh, thay đổi độ tương phản của ảnh này và lưu kết quả vào ảnh mới
int ColorTransformer::ChangeContrast(const Mat& sourceImage, Mat& destinationImage, float c) 
{
	if (sourceImage.data == nullptr)
		return 0;
	
	//Lấy width, height, srcChannels của sourceImage
	int width = sourceImage.cols, height = sourceImage.rows, srcchannels = sourceImage.channels();
	
	//Khởi tạo ma trận destinationImage
	destinationImage = Mat(height, width, CV_8UC3);
	
	int dstchannels = destinationImage.channels();
	
	//Mảng lookup với lookup[i] gồm 256 phần tử chứa giá trị của độ sáng i đã được tăng bởi c
	//Nếu i * c < 0 thì lookup[i] = 0
	//Nếu i * c > 255 thì lookup[i] = 255
	uchar lookup[256];
	for (int i = 0; i < 256; i++)
		lookup[i] = min(255, max(0, i * c));

	//Tạo destinationImage tương ứng với giá trị R, G, B tra trong lookup
	for (int y = 0; y < height; y++)
	{
		const uchar* srcpRows = sourceImage.ptr<uchar>(y);
		uchar* dstpRows = destinationImage.ptr<uchar>(y);

		for (int x = 0; x < width; x++, srcpRows += srcchannels, dstpRows += dstchannels)
		{
			uchar B = srcpRows[0];
			uchar G = srcpRows[1];
			uchar R = srcpRows[2];

			dstpRows[0] = lookup[B];
			dstpRows[1] = lookup[G];
			dstpRows[2] = lookup[R];
		}
	}
	if (destinationImage.data == nullptr)
		return 0;
	else
		return 1;
}

// Hàm tính lược đồ màu tổng quát cho ảnh bất kỳ
int ColorTransformer::CalcHistogram(const Mat& sourceImage, Mat& histMatrix) 
{
	if (sourceImage.data == NULL)
	{
		return 0;
	}

	// Thông số ảnh sourceImage
	int rows = sourceImage.rows;
	int cols = sourceImage.cols;
	int nChannels = sourceImage.channels();

	// Khởi tạo ma trận histogram (nChannel x 256), giá trị mặc định là 0
	histMatrix = Mat(nChannels, 256, CV_32SC1, Scalar(0));

	// duyệt qua các dòng pixel của ảnh sourceImage
	for (int y = 0; y < rows; y++)
	{
		// lấy con trỏ đầu dòng của sourceImage
		const uchar* pRow = sourceImage.ptr<uchar>(y);

		for (int x = 0; x < cols; x++, pRow += nChannels)
		{
			for (int k = 0; k < nChannels; k++)
			{
				// lấy con trỏ đầu dòng (tương ứng từng kênh) của histMatrix
				uint* pHistRow = histMatrix.ptr<uint>(k);
				pHistRow[pRow[k]]++;
			}
		}
	}

	return 1;
}

// Hàm cân bằng lược đồ màu tổng quát cho ảnh bất kỳ
int ColorTransformer::HistogramEqualization(const Mat& sourceImage, Mat& destinationImage) 
{
	if (sourceImage.data == NULL)
		return 0;

	Mat histMatrix;
	CalcHistogram(sourceImage, histMatrix);

	int width = sourceImage.cols, height = sourceImage.rows;
	int srcChannels = sourceImage.channels();
	cout << srcChannels << endl;

	//Khởi tạo kích thước destinationImage bằng với sourceImage
	if (srcChannels == 1)
		destinationImage = Mat(height, width, CV_8UC1);
	else
		destinationImage = Mat(height, width, CV_8UC3);

	//Khởi tạo ma trận T kích thước (srcChannels, 256)
	vector<vector<long long>> T(srcChannels, vector<long long>(256));

	//Tính T[x] = T[x - 1] + H[x] ứng với từng channel
	for (int y = 0; y < srcChannels; y++) {
		uint* pHistRow = histMatrix.ptr<uint>(y);
		T[y][0] = int(pHistRow[0]);
		for (int x = 1; x < 256; x++) {
			T[y][x] = T[y][x - 1] + pHistRow[x];
		}
	}

	//Chuẩn hóa T về [0, 255]
	for (int y = 0; y < srcChannels; y++) {
		uchar* pHistRow = histMatrix.ptr<uchar>(y);
		int maxVal = T[y][255];
		for (int x = 0; x < 256; x++) {
			T[y][x] = uchar(((1.0 * T[y][x]) / maxVal) * 255.0);
		}
	}

	//Map độ sáng x của từng pixel của sourceImage ứng với T[x] vào destinationImage
	for (int y = 0; y < height; y++) {
		const uchar* pSrcRow = sourceImage.ptr<uchar>(y);
		uchar* pDstRow = destinationImage.ptr<uchar>(y);
		for (int x = 0; x < width; x++, pSrcRow += srcChannels, pDstRow += srcChannels) {
			if (srcChannels == 1) {
				pDstRow[0] = T[0][pSrcRow[0]];
			}
			else {
				pDstRow[0] = T[0][pSrcRow[0]];
				pDstRow[1] = T[1][pSrcRow[1]];
				pDstRow[2] = T[2][pSrcRow[2]];
			}
		}
	}

	return 1;
}

// Hàm vẽ lược đồ màu tổng quát của ảnh bất kỳ
int ColorTransformer::DrawHistogram(const Mat& histMatrix, Mat& histImage) 
{
	Mat histMatrix_cl = histMatrix.clone(); //clone histMatrix
	
	if (!Normalize(histMatrix_cl)) //chuẩn hoá ma trận histogram
		return 0;

	int height = 300;
	int width = 260;
	uint32_t* pHistData = (uint32_t*)histMatrix_cl.data;
	
	//the histogram images array
	Mat* histImageArray = new Mat[histMatrix_cl.rows];

	if (histMatrix_cl.rows == 1) {
		Point_<int> text_pos = Point_<int>(90, 20); //vị trí của chữ trên ảnh histogram
		char name[10] = "GRAYSCALE"; 
		histImageArray[0] = Mat(300, 260, CV_8UC1, Scalar(0)); //khởi tạo ma trận ảnh cho grayscale
		putText(histImageArray[0],
			name,
			text_pos,
			FONT_HERSHEY_PLAIN,
			0.8,
			Scalar(255, 255, 255),
			1,
			LINE_8);
		histImage = Mat(height, width, CV_8UC1);
	}
	else { //tương tự như trên
		char name[4][2] = { "B","G","R" };
		Point_<int> text_pos = Point_<int>(120, 20);
		for (int i = 0; i < histMatrix_cl.rows; i++) { //khởi tạo ma trận ảnh histogram cho mỗi kênh màu
			histImageArray[i] = Mat(300, 260, CV_8UC3, Scalar(0));
			putText(histImageArray[i],
				name[i],
				text_pos,
				FONT_HERSHEY_PLAIN,
				0.8,
				Scalar(255, 255, 255),
				1,
				LINE_8);
		}
		histImage = Mat(height,width*3,CV_8UC3);
	}

	//vẽ đồ thị histogram
	for (int i = 0; i < histMatrix_cl.rows; i++) {
		for (int j = 0; j < histMatrix_cl.cols; j++) {
				int index = i * histMatrix_cl.cols + j;
				uchar value = pHistData[index]; 
				Point_<int> begin = Point_<int>(index % 256, height); //pixel bắt đầu 
				Point_<int> end = Point_<int>(index % 256, height - value); //pixel đích
				if (i == 0) {
					line(histImageArray[i], begin, end, Scalar(255, 0, 0), 1);
				}
				else if (i == 1) {
					line(histImageArray[i], begin, end, Scalar(0, 255, 0), 1);
				}
				else if (i == 2) {
					line(histImageArray[i], begin, end, Scalar(0, 0, 255), 1);
				}
		}
	}
	//ghép các ảnh histogram của mỗi kênh màu vào 1 ảnh duy nhất
	for (int i = 0; i < histMatrix_cl.rows; i++) {
		histImageArray[i].copyTo(histImage(Rect(i*width, 0, width, height)));
	}

	return 1;
}
//Hàm chuẩn hoá các giá trị trong histogram matrix về khoảng 0-255
//this function normalize the histogram matrix values into range (0-255)
int ColorTransformer::Normalize(Mat& histMatrix)
{
	if (histMatrix.empty()) { //kiểm tra histMatrix
		return 0;
	}

	int height = histMatrix.rows;
	int width = histMatrix.cols;
	uint32_t* pHistData = (uint32_t*)histMatrix.data;
	int max = 0;

	//tìm giá trị max
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int index = i * width+ j;
			if (pHistData[index] > max)
				max = pHistData[index];
		}
	}

	//chuẩn hoá
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int index = i * width + j;
			pHistData[index] = (uchar)(255 * (float)pHistData[index] / (float)max);
		}
	}

	return 1;
}

// Hàm so sánh hai ảnh
float ColorTransformer::CompareImage(const Mat& image1, Mat& image2) 
{
	//nếu một trong hai ảnh đưa vào không hợp lệ thì trả về -1 (tức là sai).
	//Vì các giá trị nguyên không âm sẽ là giá trị trả về độ chênh lệnh giữa hai lược đồ màu.
	if (image1.empty() || image2.empty())
		return -1;
	
	//Gọi hàm để tạo hai histogram pHist1,pHist2 từ hai ảnh image1 và image2
	Mat pHist1, pHist2;
	CalcHistogram(image1, pHist1);
	CalcHistogram(image2, pHist2);

	//kích thước của từng ảnh.
	int size1 = image1.cols * image1.rows;
	int size2 = image2.cols * image2.rows;
	
	//khởi tạo chênh lệch = 0
	float chenh = 0;
	for (int y = 0; y < pHist1.rows; y++)
	{
		const unsigned int* pRow1 = pHist1.ptr<unsigned int>(y);
		const unsigned int* pRow2 = pHist2.ptr<unsigned int>(y);
		for (int x = 0; x < pHist1.cols; x++, pRow1++, pRow2++)
		{
			//công thức tính độ chênh lệnh là tổng độ chênh lệch của từng phần tử
			//tại vị trí như nhau giữa hai lược đồ màu.
			//từng phần tử chia cho kích thước của ảnh là để đưa hai histogram về chung thang đo từ [0,1]
			chenh += abs((pRow1[0] / float(size1)) - (pRow2[0] / float(size2)));
		}		
	}	return chenh;
}

ColorTransformer::ColorTransformer() 
{
}

ColorTransformer::~ColorTransformer() 
{
}
