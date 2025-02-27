#pragma once
#include <cv.h>
#include "Common3D.h"

// ========================= ShiftMap 2D =========================
CvPoint* CreateCvPoint(int x, int y);

// Get label index
int GetLabel(CvPoint point, CvSize imageSize);

// Get pixel position from label
CvPoint GetPoint(int label, CvSize imageSize);

bool IsOutside(CvPoint point, CvSize imageSize); 

CvPoint GetMappedPoint(int pixel, int label, CvSize output, CvSize shiftSize);

CvPoint GetShift(int label, CvSize shiftSize);

void SetMapDataTerm(IplImage* map, int u, int v, double value);

// width & height are default to be 3 (so 9 labels in total)
// however just leave them here for flexibility
CvPoint GetMappedPointInitialGuess(int pixel, int label, CvSize output, CvSize shiftSize, CvMat* initialGuess);



// set a label to a label map
void SetLabel(CvPoint point, CvPoint shiftLabel, CvMat* labelMap);

// get a label from a label map
CvPoint GetLabel(CvPoint point, CvMat* labelMap);

// ========================= ShiftMap 3D =========================
Point3D GetMappedPoint3D(int pixel, int label, Volume3D output, Volume3D shiftSize);
