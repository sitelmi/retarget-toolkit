//////////////////////////////////////////////////////////////////////////////
// Motion Guided Shift-Map for Video Retargeting
//////////////////////////////////////////////////////////////////////////////
//
// Its graph labeling formulation:
// For every frame, we design a 2d grid graph to represent it. Every node in 
// the graph corresponds to a pixel in the frame and is connected to 4 nieg-
// boring nodes. The labels are the shift-offset of the spatial coordinates
// of the pixel from source to target.  
// Similar to original shift-map, the objective energy function of graph-cut
// consists of 2 terms:
// 1. Data terms: sum_p D(p,l_p)
// 2. Smoothness terms: sum_{p,q} V_{pq}(l_p,l_q)
// Instead of directly applying shift-map on individual frames, whose output
// is not motion consistent, we introduce the motion-consistent shift-map for
// video retargeting. The idea of motion-consistency is to ensure that the 
// temporal changes are as similar as possible to the source. We encode the 
// data term to ensure such mostion-consistency.
// Given the shift-map of previous frame, we use it to constrain the graph-cut
// of current frame. For every spatial location $p$ in target, we have their
// shifts (l_{t-1}(p),l_{t}(p)) in both previous and current frame. The temporal
// change in the target is |I_{t-1}(p+l_{t-1}(p))-I_{t}(p+l_{t}(p))|. By fixing 
// the mapping in the previous frame, the temporal change of this location in 
// the source is |I_{t-1}(p+l_{t-1}(p))-I_{t}(p+l_{t-1}(p))|. So the similarity
// of the temporal change at this location between target and source can be 
// decided by |I_{t}(p+l_{t-1}(p))-I_{t}(p+l_{t}(p))|.


#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "../Common/picture.h"
#include "../Common/utils.h"
#include "../GCoptimization/GCoptimization.h"

using namespace std;

/*
 * data structure for saliency data term
 */
struct ForDataFn
{
	Picture *src;
	gradient2D *gradient;
	Matrix *saliency;
	int *assignments;
	imageSize previous_size;
	imageSize target_size;
};

/*
 * data structure for motion-guided data term
 */
struct ForMGDataFn
{
	Picture *src;
	gradient2D *gradient;
	Matrix *saliency;
	int *pre_assignments;
	imageSize previous_size;
	imageSize target_size;
	Picture *pre;
};

/*
 * data structure for smoothness term
 */
struct ForSmoothFn
{
	Picture *src;
	gradient2D *gradient;
	imageSize target_size;
	imageSize previous_size;
	float alpha;
	float beta;
};

/*
 * Saliency-based data term calculation
 * (used for first frame and shot boundaries)
 */
double dataFn(int p, int l, void *data)
{
	ForDataFn *myData = (ForDataFn *) data;

	int x = p % myData->target_size.width;
	int y = p / myData->target_size.width;

	double cost = 0.0;
	
	// pixel rearrangement: 
	// keep the leftmost/rightmost columns
	/*
	if (x==0 && l!=0)
		cost = 100000*MAX_COST_VALUE;
	if (x==myData->target_size.width-1 && 
		l!=myData->src->GetWidth()-myData->target_size.width)
		cost = 100000*MAX_COST_VALUE;
	*/
	

	// motion saliency
	// TODO: improve motion saliency calculation
	cost = 0.01/(0.01+10000*myData->saliency->Get(y+1,x+l+1));
	cost += 0.01/(0.01+10000*myData->gradient->dx->Get(y+1,x+l+1));
	cost += 0.01/(0.01+10000*myData->gradient->dy->Get(y+1,x+l+1));
	
	return cost;
}

/*
 * Motion-guided data term calculation
 */
double MGDataFn(int p, int l, void *data)
{
	ForMGDataFn *myData = (ForMGDataFn *) data;

	int x = p % myData->target_size.width;
	int y = p / myData->target_size.width;

	double cost = 0.0;
	// Assign motion guided data energy 
	// preserve temporal consistency
	int cur_match = x+l;
	int pre_match = x+myData->pre_assignments[p];
	cost += pow(myData->pre->GetPixelIntensity(cur_match,y).r-
				myData->pre->GetPixelIntensity(pre_match,y).r,2.0);
	cost += pow(myData->pre->GetPixelIntensity(cur_match,y).g-
				myData->pre->GetPixelIntensity(pre_match,y).g,2.0);		
	cost += pow(myData->pre->GetPixelIntensity(cur_match,y).b-
				myData->pre->GetPixelIntensity(pre_match,y).b,2.0);

	// TODO: whether integrated with saliency?
	//cost += 0.01/(0.01+10000*myData->saliency->Get(y+1,x+l+1));
	//cost += 0.01/(0.01+10000*(myData->gradient->dx->Get(y+1,x+l+1)));
	//cost += 0.01/(0.01+10000*(myData->gradient->dy->Get(y+1,x+l+1)));
	
	return cost;
}


/*
 * Compute color difference between two mapped points
 */
double ColorDiff(Picture *src, int x1, int y1, 
				 int x2, int y2, int l1, int l2)
{
	double diff = 0.0;
	int width = src->GetWidth();
	int height = src->GetHeight(); 
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	//printf("ColorDiff: pixels: (%d,%d) and (%d,%d) labels: %d and %d\n",x2,y2+l2,x1+x_offset,y1+l1+y_offset,l2,l1);
	if (x2+l2>=0 && x2+l2<width && x1+l1+x_offset>=0 && x1+l1+x_offset<width)
	{
		//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
		diff += pow((double)src->GetPixel(x2+l2,y2).r-src->GetPixel(x1+l1+x_offset,y1+y_offset).r,2);
		diff += pow((double)src->GetPixel(x2+l2,y2).g-src->GetPixel(x1+l1+x_offset,y1+y_offset).g,2);
		diff += pow((double)src->GetPixel(x2+l2,y2).b-src->GetPixel(x1+l1+x_offset,y1+y_offset).b,2);
	} else
	{
		diff += MAX_COST_VALUE;
	}
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	return diff;
}

/*
 * Compute gradient difference between two mapped points
 */
double GradientDiff(gradient2D *gradient, int x1, int y1, 
					int x2, int y2, int l1, int l2)
{
	double diff = 0.0;
	int width = gradient->dx->NumOfCols();
	int height = gradient->dx->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width)
	{
		diff += pow((gradient->dx->Get(y2,x2+l2)-gradient->dx->Get(y1+y_offset,x1+l1+x_offset)),2)+
				pow((gradient->dy->Get(y2,x2+l2)-gradient->dy->Get(y1+y_offset,x1+l1+x_offset)),2);
	} else
	{	
		diff += MAX_COST_VALUE;
	}

	return diff;
}

/*
 * Smoothness term calculation
 */
double smoothFn(int p1, int p2, int l1, int l2, void *data)
{
	ForSmoothFn *myData = (ForSmoothFn *) data;
	double cost = 0.0;
	//printf("smoothFn: calculating image gradient...\n");
	int width = myData->target_size.width;
	int height = myData->target_size.height;
	//printf("smoothFn: image width=%d and height=%d\n",width,height);
	int x1 = p1 % width;
	int y1 = p1 / width;

	int x2 = p2 % width;
	int y2 = p2 / width;
	//printf("smoothFn: (%d,%d) and (%d,%d)\n",x1,y1,x2,y2);

	if (abs(x1-x2)<=1 && abs(y1-y2)<=1)
	{		
		cost += myData->alpha*ColorDiff(myData->src, x1, y1, x2, y2, l1, l2);
		cost += myData->beta*GradientDiff(myData->gradient, x1+1, y1+1, x2+1, y2+1, l1, l2);
	}
	else 
	{
		cost = MAX_COST_VALUE;
	}

	//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);
	return cost;
}

/*
 * Function to generate and save retargeted image
 * using shift-map labels
 */
void SaveRetargetPicture(int *labels, Picture *src,int width, int height, char *name)
{
	Picture *result = new Picture(width,height);
	result->SetName(name);
	
	int x, y;
	pixelType pixel;
	for ( int  i = 0; i < width*height; i++ )
	{
		x = i % width;
		y = i / width;
		//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
		pixel.r = src->GetPixel(x+labels[i],y).r;
		pixel.g = src->GetPixel(x+labels[i],y).g;
		pixel.b = src->GetPixel(x+labels[i],y).b;
		result->SetPixel(x,y,pixel);
	}

	result->Save(result->GetName());
	delete result;
}

/*
 * Function to find the optimal shift-map
 */
int *GridGraph_GraphCut(listPyramidType *src, int level, int t, int *pre_assignments, imageSize &target_size, 
						imageSize &previous_size, int num_labels, float alpha, float beta, char *target_name)
{
	Picture *cur_frame, *pre_frame;
	if (t==0)
	{
		cur_frame = src->Lists[level].GetPicture(0);
		pre_frame = src->Lists[level].GetPicture(0);
	}
	else
	{
		pre_frame = src->Lists[level].GetPicture(1);
		cur_frame = src->Lists[level].GetPicture(1);
	}
	// set up the needed data to pass to function for the data costs
	double tdt;
	Matrix *saliency = FrameDifference(cur_frame,pre_frame,tdt,0.0);
	gradient2D *gradient = Gradient(cur_frame);


	int num_pixels = target_size.width*target_size.height;
	int *result = new int[num_pixels];   // stores result of optimization

	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(target_size.width,
																  target_size.height,
																  num_labels);


		// TODO: replace the hardcode of gradient threoshold
		// for shot boundary
		// data terms comes from function pointer
		if (t==0 || tdt>10000)
		{
			ForDataFn toDataFn;
			toDataFn.src = cur_frame;
			toDataFn.gradient = gradient;
			toDataFn.saliency = saliency;
			toDataFn.target_size = target_size;
			toDataFn.previous_size = previous_size;
			gc->setDataCost(&dataFn,&toDataFn);
		}
		else
		{
			Picture *pre_frame = src->Lists[level].GetPicture(0);
			saliency = FrameDifference(cur_frame,pre_frame,tdt,0.0);

			ForMGDataFn toDataFn;
			toDataFn.src = cur_frame;
			toDataFn.pre = pre_frame;
			toDataFn.gradient = gradient;
			toDataFn.saliency = saliency;
			toDataFn.pre_assignments = pre_assignments;
			toDataFn.target_size = target_size;
			toDataFn.previous_size = previous_size;

			gc->setDataCost(&MGDataFn,&toDataFn);
		}

		// smoothness comes from function pointer
		ForSmoothFn toSmoothFn;
		toSmoothFn.src = cur_frame;
		
		toSmoothFn.gradient = gradient;
		toSmoothFn.previous_size = previous_size;
		toSmoothFn.target_size = target_size;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&smoothFn, &toSmoothFn);

		printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion(1);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("After optimization energy is %f\n",gc->compute_energy());

		// obtain the labels
		int assign_idx, assignment;
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			result[i] = gc->whatLabel(i);
		}		

		// upsampling shift-map from low-resolution
		// to high-resolution
		double ratio = pow(2.0,level);
		int *up_result = SimpleUpsamplingMap(result,target_size,ratio);

		// generate and save retargeted images
		// from source and shift-map
		Picture *up_frame;
		if (t==0)
			up_frame = src->Lists[0].GetPicture(0);
		else
			up_frame = src->Lists[0].GetPicture(1);
		SaveRetargetPicture(up_result,up_frame,target_size.width*ratio,
							target_size.height*ratio,target_name);

		if (t>0)
			delete [] pre_assignments;
		delete gradient->dx;
		delete gradient->dy;
		delete gradient;
		delete [] up_result;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	return result;

}

/*
 * main entry of the grogram
 */ 
int main(int argc, char **argv)
{
	PictureList *input = NULL;
	int width, height, time;
	listPyramidType *vpyramid = NULL;
	int num_pixels, num_labels;
	int *assignments = NULL;	// store shift labels of every pixel
	imageSize target_size, previous_size;

	if (argc<6)
	{
		cout << "Usage: mg_shift_map <input_folder> <alpha> <beta> <ratio> <output_folder>" << endl;
		return 0;
		//default parameters
	}

	// load input video
	vector<string> filenames = Get_FrameNames(argv[1], 
											  PICTURE_FRAME_EXT);

	int level = 2; // gpyramid->Levels-1
	for (int t = 0; t < filenames.size(); t++)
	{
		cout << "Processing " << t << "th frames ..." << endl;
		if (t>0)
			input = new PictureList(argv[1],t-1,t);
		else
			input = new PictureList(argv[1],t,t+1);
		vpyramid = ListPyramid(input,level+1);

		width = vpyramid->Lists[level].GetPicture(1)->GetWidth();
		height = vpyramid->Lists[level].GetPicture(1)->GetHeight();
		num_pixels = ceil(width*atof(argv[4]))*height;
			
		num_labels = width-ceil(width*atof(argv[4]))+1;
		width = ceil(width*atof(argv[4]));

		target_size.width = width;
		target_size.height = height;
		previous_size.width = 0;
		previous_size.height = 0;	

		// smoothness and data costs are set up using functions
		char framename[512] = {'\0'};
		char buf[512] = {'\0'};

		strcat(framename,argv[5]);
		sprintf(buf,"%04d.ppm",t);
		strcat(framename,buf);
		
		
		assignments = GridGraph_GraphCut(vpyramid,level,t,assignments,target_size,previous_size,
										 num_labels,atof(argv[2]),atof(argv[3]),framename);

		//delete framename;
		//delete buf;
		delete input;
		delete [] vpyramid->Lists;
		delete vpyramid;
	}

	
	delete [] assignments;

	return 1;

}

/////////////////////////////////////////////////////////////////////////////////