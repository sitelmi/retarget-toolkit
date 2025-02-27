#include "StdAfx.h"
#include "EnergyFunction.h"


// this function is the interface with the paper
int ShifmapDataFunction(CvPoint point, CvPoint shift, IplImage* saliency)
{
	CvScalar value = cvGet2D(saliency, point.y + shift.y, point.x + shift.x);
	return value.val[0] + value.val[1] + value.val[2];
}

int dataFunctionShiftmap(int pixel, int label, void *extraData)
{	
	 
	ForDataFunction *data = (ForDataFunction *) extraData;
	// position of output pixel
	CvPoint pixelPoint = GetPoint(pixel, data->outWidth, data->outHeight);
	// position of input pixel used to map to output pixel
	CvPoint labelPoint = GetPoint(label, data->inWidth, data->inHeight);

	CvScalar value = cvGet2D(data->saliency, labelPoint.y, labelPoint.x);
	double saliency = value.val[0] + value.val[1] + value.val[2]; 
	//saliency = saliency * 1000000;
 
	//printf("energy: %f, pixel %i label %i \n", saliency, pixel, label);
	return saliency;
	//if(pixelPoint.x == data->outWidth - 1 && labelPoint.x == data->inWidth - 1 && pixelPoint.y == labelPoint.y)
	//	return saliency;
	//else
	//	if(pixelPoint.x == 0 && labelPoint.x == 0 && pixelPoint.y == labelPoint.y)
	//	return saliency;
	//else
	//	return saliency + 100;


	
}

//double smoothFunctionShiftmap(int pixel1, int pixel2, int label1, int label2, void* extraData)
//{
//	ForSmoothFunction *data = (ForSmoothFunction *) extraData;
//
//	// position of output pixel
//	CvPoint pixelPoint1 = GetPoint(pixel1, data->outWidth, data->outHeight);
//	// position of input pixel used to map to output pixel
//	CvPoint labelPoint1 = GetPoint(label1, data->inWidth, data->inHeight);
//	// position of output pixel
//	CvPoint pixelPoint2 = GetPoint(pixel2, data->outWidth, data->outHeight);
//	// position of input pixel used to map to output pixel
//	CvPoint labelPoint2 = GetPoint(label2, data->inWidth, data->inHeight);
//
//	// just calculate the diff of neighbor in original image
//	// in this way, if 2 labels are indeed neighbors in original image
//	// the cost will be zero
//	double energy = 0;
//	
//	// color term
//	energy += SquareColorDifference(cvPoint(labelPoint1.x, labelPoint1.y + 1), cvPoint(labelPoint2.x, labelPoint2.y + 1), data->image);
//	energy += SquareColorDifference(cvPoint(labelPoint1.x, labelPoint1.y - 1), cvPoint(labelPoint2.x, labelPoint2.y - 1), data->image);
//	energy += SquareColorDifference(cvPoint(labelPoint1.x + 1, labelPoint1.y), cvPoint(labelPoint2.x + 1, labelPoint2.y), data->image);
//	energy += SquareColorDifference(cvPoint(labelPoint1.x - 1, labelPoint1.y), cvPoint(labelPoint2.x - 1, labelPoint2.y), data->image);
//	
//	// gradient different term 
//	energy += SquareColorDifference(cvPoint(labelPoint1.x, labelPoint1.y + 1), cvPoint(labelPoint2.x, labelPoint2.y + 1), data->gradient);
//	energy += SquareColorDifference(cvPoint(labelPoint1.x, labelPoint1.y - 1), cvPoint(labelPoint2.x, labelPoint2.y - 1), data->gradient);
//	energy += SquareColorDifference(cvPoint(labelPoint1.x + 1, labelPoint1.y), cvPoint(labelPoint2.x + 1, labelPoint2.y), data->gradient);
//	energy += SquareColorDifference(cvPoint(labelPoint1.x - 1, labelPoint1.y), cvPoint(labelPoint2.x - 1, labelPoint2.y), data->gradient);
//	
//	return energy;
//}

bool IsCorrectMap(CvPoint pixel1, CvPoint pixel2, CvPoint label1, CvPoint label2)
{
	if(pixel1.x == label1.x && pixel1.y == label1.y && pixel2.x == label2.x && pixel2.y == label2.y)
		return true;
	return false;
}

int smoothFunctionShiftmap(int pixel1, int pixel2, int label1, int label2, void* extraData)
{	

	
	//if(label1 == label2 + 1 && label1 != 1)
	//{
	//	printf("test");
	//}
	ForSmoothFunction *data = (ForSmoothFunction *) extraData;

	// position of output pixel
	CvPoint pixelPoint1 = GetPoint(pixel1, data->outWidth, data->outHeight);
	// position of input pixel used to map to output pixel
	CvPoint labelPoint1 = GetPoint(label1, data->inWidth, data->inHeight);
	// position of output pixel
	CvPoint pixelPoint2 = GetPoint(pixel2, data->outWidth, data->outHeight);
	// position of input pixel used to map to output pixel
	CvPoint labelPoint2 = GetPoint(label2, data->inWidth, data->inHeight);

	 
	//return 0;
	//if(IsCorrectMap(pixelPoint1, pixelPoint2, labelPoint1, labelPoint2))
	////if(IsCorrectMap(pixelPoint1, pixelPoint2, labelPoint1, labelPoint2) && pixelPoint1.y != 0 && pixelPoint2.y != 0)
	//	return 1;
	//else
	//	return 100;
	//// check neighbor
	//if(!IsNeighbor(labelPoint1, labelPoint2))
	//	return 100;
	if(labelPoint1.x == labelPoint2.x && labelPoint2.y == labelPoint2.y)
	{
		 
		return 600; // prevent mapping into 1 pixel
	}
	if(IsCorrectMap(pixelPoint1, pixelPoint2, labelPoint1, labelPoint2))
	{
		return 0;
	}
	if(!IsNeighbor(pixelPoint1, pixelPoint2))
		return 0;
	//return 50;
	// pre-compute variables:
	CvPoint neighbor1 = GetNeighbor(pixelPoint1, pixelPoint2, labelPoint1); // neighbor of label1
	CvPoint neighbor2 = GetNeighbor(pixelPoint2, pixelPoint1, labelPoint2); // neighbor of label2

	int energy = 0;
	// color different term:
	// for pixel1:
	
	energy += SquareColorDifference(labelPoint1, neighbor2, data->image);
	energy += SquareColorDifference(labelPoint2, neighbor1, data->image);
  

	// gradient different term 
	energy += 2 * SquareColorDifference(labelPoint2, neighbor1, data->gradient);
	energy += 2 * SquareColorDifference(labelPoint1, neighbor2, data->gradient);
	
  
	//if(IsCorrectMap(pixelPoint1, pixelPoint2, labelPoint1, labelPoint2))
	//printf("energy: %f %i %i -> %i %i and %i %i -> %i %i\n", energy, labelPoint1.x, labelPoint1.y, pixelPoint1.x, pixelPoint1.y, labelPoint2.x, labelPoint2.y, pixelPoint2.x, pixelPoint2.y);

 
	return  energy;
}

CvScalar ColorDiffenrece(CvPoint point1, CvPoint point2, IplImage* image)
{
	if(!IsInsideImage(point1, image->width, image->height) || !IsInsideImage(point2, image->width, image->height))
		return cvScalar(0,0,0,0);

	CvScalar value1 = cvGet2D(image, point1.y, point1.x);
	CvScalar value2 = cvGet2D(image, point2.y, point2.x);
	return cvScalar(value1.val[0] - value2.val[0], value1.val[1] - value2.val[1], value1.val[2] - value2.val[2]);
}

int SquareColorDifference(CvPoint point1, CvPoint point2, IplImage* image)
{
	if(!IsInsideImage(point1, image->width, image->height) || !IsInsideImage(point2, image->width, image->height))
		return 1000000;

	CvScalar value1 = cvGet2D(image, point1.y, point1.x);
	CvScalar value2 = cvGet2D(image, point2.y, point2.x);

	return SquareDifference(value1, value2);
}

int SquareDifference(CvScalar value1, CvScalar value2)
{
	return pow(value1.val[0]-value2.val[0], 2) + pow(value1.val[1]-value2.val[1], 2) + pow(value1.val[2]-value2.val[2], 2);
}

bool IsNeighbor(CvPoint point1, CvPoint point2)
{
	if(abs(point1.x - point2.x) <= 1 && abs(point2.y - point1.y) <=1)
		return true;
	else
		return false;
}

CvPoint GetNeighbor(CvPoint pixel1, CvPoint pixel2, CvPoint label)
{
	return cvPoint(label.x - pixel1.x + pixel2.x, label.y - pixel1.y + pixel2.y);
}

bool IsInsideImage(CvPoint point, int width, int height)
{
	if(point.x < 0 || point.y < 0 || point.x + 1 > width || point.y + 1 > height)
		return false;
	return true;
}