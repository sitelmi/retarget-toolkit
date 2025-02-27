#include "stdafx.h"
#include "VideoSequence.h"

// Testing
void TestCreateSequence()
{
	VideoSequence sequence;
	vector<char*> frameList(2);
	
	frameList[0] = "Ha ha ha ha";
	frameList[1] = "Ha ha ha ha 2";
	 
	sequence.frameList = &frameList;
}

void TestLoadSaveSequence()
{
	VideoSequence sequence;
	vector<char*> frameList(2);
	
	frameList[0] = "Ha ha ha ha";
	frameList[1] = "Ha ha ha ha 2";
	 
	sequence.frameList = &frameList;
	SaveVideoSequenceToFile(&sequence, "test.tst");
	VideoSequence* sequ = LoadSequenceFromFile("test.tst");
	sequ->frameList = 0;
}

// Load a frame from memory
IplImage* LoadFrame(VideoSequence* videoSequence, int frame_number)
{
	char* filename = (*videoSequence->frameList)[frame_number];
	return cvLoadImage(filename);
}

void SaveVideoSequenceToFile(VideoSequence* sequence, char* fileName)
{
	ofstream out(fileName, ios::binary);
	int count = sequence->frameList->size();	
	out.write((char*)&count, sizeof(int));
	
	for(int i = 0; i < count; i++)
	{		 
		char* name = (*(sequence->frameList))[i];
		int length = strlen(name);
		out.write((char*)&length, sizeof(int));
		out.write(name, sizeof(char) * length);
	}
	
    out.close();
}

VideoSequence* LoadSequenceFromFile(char* filename)
{
	ifstream in(filename, ios::binary);
	VideoSequence* sequence = new VideoSequence();
	int count;
	in.read((char*)&count, sizeof(int));

	vector<char*>* frameList = new vector<char*>(count);
	
	for(int i = 0; i < count; i++)
	{
		int length;		
		in.read((char*)&length, sizeof(int));		
		char* name = (char*) malloc(length * sizeof(char));
		// malloc will allocate more than necessary sometime
		name[length] = '\0';
		in.read(name, sizeof(char) * length);
		(*frameList)[i] = name;
		//strncpy((*frameList)[i], name, length);		
	}
	sequence->frameList = frameList;
	return sequence;
}

ShotInfo* LoadShotFromFile(char* fileName)
{	
	ifstream in(fileName, ios::binary);

	ShotInfo* shotInfo = new ShotInfo();
	
	
	in.read((char*)&(shotInfo->shotCount), sizeof(int));
	int shotCount = shotInfo->shotCount;

	int test = sizeof(long);
	test = sizeof(Shot);

	shotInfo->shotList = (Shot*)malloc(shotCount * sizeof(Shot));
	Shot* shot = shotInfo->shotList;
	for(int i = 0; i < shotCount; i++)
	{
		in.read((char*)&(shot->subShotCount), sizeof(int));
		int subShotCount = shot->subShotCount;				
		
		shot->subShotList = (SubShot*) malloc(subShotCount * sizeof(SubShot));
		
		SubShot* subShot = shot->subShotList;
		for(int j = 0; j < subShotCount; j++)
		{
			// writing each subShot
			in.read((char*)&(subShot->shotType), sizeof(int));
			in.read((char*)&(subShot->start), sizeof(int));
			in.read((char*)&(subShot->end), sizeof(int));
			subShot++;
		}
		shot++;
	}	

	return shotInfo;
}

void SaveShotToFile(ShotInfo* shotInfo, char* fileName)
{
	ofstream out(fileName, ios::binary);
	
	int shotCount = shotInfo->shotCount;
	// number of shot
	out.write((char*)&shotCount, sizeof(int));
	
	Shot* shot = shotInfo->shotList;
	for(int i = 0; i < shotCount; i++)
	{
		int subShotCount = shot->subShotCount;
		// number of subShot
		out.write((char*)&subShotCount, sizeof(int));
		
		SubShot* subShot = shot->subShotList;
		for(int j = 0; j < subShotCount; j++)
		{
			// writing each subShot
			out.write((char*)&(subShot->shotType), sizeof(int));
			out.write((char*)&(subShot->start), sizeof(int));
			out.write((char*)&(subShot->end), sizeof(int));
			subShot++;
		}
		shot++;
	}

    out.close();
}

void TestSaveLoadShot()
{
	ShotInfo* shotInfo = new ShotInfo();
	SubShot subShot[10];
	for(int i = 0; i < 10; i++)
	{
		subShot[i].end = 20;
		subShot[i].start = 1;
		subShot[i].shotType = VCSHOT_PAN;
	}
	Shot shot[10];
	for(int i = 0; i < 10; i++)
	{
		shot[i].subShotCount = 10;
		shot[i].subShotList = subShot;
	}
	shotInfo->shotList = shot;
	shotInfo->shotCount = 10;

	SaveShotToFile(shotInfo, "test.txt");
	ShotInfo* shotInfo2 = LoadShotFromFile("test.txt");
	Shot* shot2 = shotInfo2->shotList;
	for(int i = 0; i < shotInfo2->shotCount; i++)
	{
		SubShot* subShot2 = shot2->subShotList;
		for(int j = 0; j < shot2->subShotCount; j++)
		{			
			int end = subShot2->end;
			subShot2++;
		}
		shot2++;
	}
}