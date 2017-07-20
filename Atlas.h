#ifndef ATLAS_H
#define ATLAS_H

#include <vector>
#include <algorithm>

#include "Texture.h"
#include "Targa.h"

#define PADDING 0 // padding in between textures

// used internally in class Atlas
struct Bitmap{
	Bitmap(int w,int h,unsigned char *b,int unique)
	:width(w),height(h),xpos(-1),ypos(-1),bmp(b),id(unique){}
	bool contained()const;
	bool collide(const Bitmap&)const;
	bool operator>(const Bitmap&)const;
	int size()const{return height;}

	int id;
	int width;
	int height;
	int xpos; // xpos in the final canvas
	int ypos;
	unsigned char *bmp;
};

class Atlas{
public:
	Atlas();
	~Atlas();
	void add(const Texture&);
	void compile();
	int write(const char*); // save to file
	int write_tga(const char*); // save to tga file
	int get_count()const{return bitmap_list.size();}
	int get_canvas_size()const{return canvas_width*canvas_height*4;}

private:
	void insert(const Bitmap&);
	bool fits(const Bitmap&)const;
	void finalize();
	int proposed_width()const;
	int proposed_height()const;

	std::vector<Bitmap> bitmap_list;
	unsigned char *canvas;
	int canvas_width;
	int canvas_height;
};

#endif // ATLAS_H
