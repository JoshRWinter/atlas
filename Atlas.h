#ifndef ATLAS_H
#define ATLAS_H

#include <vector>

#include "Texture.h"
#include "Targa.h"

struct Bitmap{
	Bitmap(int w,int h,unsigned char *b):width(w),height(h),bmp(b){}

	int width;
	int height;
	unsigned char *bmp;
};

class Atlas{
public:
	Atlas();
	~Atlas();
	void add(const Texture&);
	int write(const char*)const; // save to file

private:
	std::vector<Bitmap> bitmap_list;
};

#endif // ATLAS_H
