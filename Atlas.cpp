#include <stdio.h>

#include "Atlas.h"

Atlas::Atlas(){
	canvas=NULL;
}

Atlas::~Atlas(){
	// iterate over bitmaps and free img data
	for(const Bitmap &b:bitmap_list)
		delete[] b.bmp;
	// canvas
	delete[] canvas;
}

void Atlas::add(const Texture &texture){
	const int width=texture.get_width();
	const int height=texture.get_height();

	// get the image data from <texture>
	unsigned char *imgdata=new unsigned char[width*height*4];
	texture.get_bitmap(imgdata);

	// add to internal store (std::vector<Bitmap>)
	bitmap_list.push_back(Bitmap(width,height,imgdata));
}

int Atlas::write(const char *output)const{
	FILE *file=fopen(output,"wb");
	if(!file)
		return -1;

	fclose(file);

	return 0;
}
