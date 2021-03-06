#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <zlib.h>

#include "Atlas.h"

Atlas::Atlas(){
	canvas=NULL;
	canvas_width=-1;
	canvas_height=-1;
}

Atlas::~Atlas(){
	// iterate over bitmaps and free img data
	for(const Bitmap &b:bitmap_list)
		delete[] b.bmp;
	// canvas
	delete[] canvas;
}

void Atlas::add(const Texture &texture){
	if(canvas!=NULL){
		// already compiled, can't add new textures
		return;
	}

	const int width=texture.get_width();
	const int height=texture.get_height();

	// get the image data from <texture>
	unsigned char *imgdata=new unsigned char[width*height*4];
	texture.get_bitmap(imgdata);

	// add to internal store (std::vector<Bitmap>)
	bitmap_list.push_back(Bitmap(width,height,imgdata,bitmap_list.size()));
}

// the main "arranging" function
// good luck making sense of this
void Atlas::compile(){
	if(canvas!=NULL){
		// already compiled
		return;
	}

	// reverse sort the images on size
	std::sort(bitmap_list.begin(),bitmap_list.end(),std::greater<Bitmap>());

	// decide the number of columns to generate
	const int colnum=sqrt(bitmap_list.size());

	// figure out avg bitmap width
	float avg_width=0.0f;
	for(const Bitmap &b:bitmap_list)
		avg_width+=(float)b.width/bitmap_list.size();

	int current_col=0;
	int row_first_y_insert=PADDING;
	bool new_row_flag=true;

	// iterate over the bitmaps
	for(Bitmap &b:bitmap_list){
		// decide if a new row should be started (above this one)
		if(current_col>=colnum){
			// only start new row if more items can't be put on current row
			// without making the enclosed area wider
			int enclosed_width=proposed_width();
			// try to place next item at end of this line
			b.ypos=row_first_y_insert;
			b.xpos=PADDING;
			// move right til fits()
			while(!fits(b))
				++b.xpos;
			if(proposed_width()>enclosed_width){
				// didn't work, abort abort
				current_col=0;
				new_row_flag=true;
			}
		}
		if(b.width>avg_width/2.0f)
			++current_col;

		if(new_row_flag){
			b.xpos=PADDING;
			b.ypos=PADDING;
			// move it up til it fits()
			while(!fits(b))
				++b.ypos;
			row_first_y_insert=b.ypos;
			new_row_flag=false;
		}
		else{
			b.ypos=row_first_y_insert;
			b.xpos=PADDING;
			// move it right til it fits()
			while(!fits(b))
				++b.xpos;
			// move it down till it doesn't fit, then back up one
			while(fits(b))
				--b.ypos;
			++b.ypos;
		}
	}

	// additional optimization: move everything as far left as possible
	bool changed=false;
	do{
		changed=false;
		for(Bitmap &b:bitmap_list){
			int before_x=b.xpos;
			// move it left, till it doesn't fit, then right one
			while(fits(b))
				--b.xpos;
			++b.xpos;
			if(b.xpos!=before_x)
				changed=true;
		}
	}while(changed);

	// actually write the bitmaps into their agreed upon locations
	finalize();
}

int Atlas::write(const char *output){
	if(canvas==NULL)
		compile();

	// construct header
	std::vector<unsigned short> header;

	// save the number of textures in the header
	header.push_back(bitmap_list.size());

	// save the dimension of the atlas bitmap in the header
	header.push_back(canvas_width);
	header.push_back(canvas_height);

	// need to process them in the order they were added,
	// which is not the current order
	for(int i=0;i<bitmap_list.size();++i){
		// find the bitmap with id <i>
		int index=-1;
		for(int j=0;j<bitmap_list.size();++j){
			if(bitmap_list[j].id==i){
				index=j;
				break;
			}
		}
		if(index==-1)
			return 0;

		// save the coordinates in the header
		header.push_back(bitmap_list[index].xpos);
		header.push_back(bitmap_list[index].ypos);
		header.push_back(bitmap_list[index].width);
		header.push_back(bitmap_list[index].height);
	}

	// create new buffer with header followed by canvas bitmap data
	const int combined_size=header.size()*sizeof(unsigned short)+get_canvas_size();
	unsigned char *combined=new unsigned char[combined_size];
	memcpy(combined,&header[0],header.size()*sizeof(unsigned short)); // copy the header
	memcpy(combined+(header.size()*sizeof(unsigned short)),canvas,get_canvas_size()); // copy canvas bitmap data

	// create new buffer for compressed data
	uLongf compressed_size=(combined_size*1.1)+12;
	Bytef *compressed=new Bytef[compressed_size];
	memset(compressed,0,compressed_size);

	// compress with zlib
	int result=compress(compressed,&compressed_size,combined,combined_size);
	if(result!=Z_OK)
		std::cout<<"compression failed!"<<std::endl;

	// no longer needed
	delete[] combined;

	// write to file
	std::ofstream out(output);
	if(!out){
		std::cout<<"error: could not open \""<<output<<"\" for writing"<<std::endl;
		delete[] compressed;
		return 0;
	}

	// write magic
	unsigned char magic[]={'J','R','W'};
	out.write((char*)magic,3);

	// write uncompressed size
	unsigned int cs=(unsigned int)combined_size;
	out.write((char*)&cs,sizeof(unsigned int));

	// write compressed data
	out.write((char*)compressed,compressed_size);

	delete[] compressed;

	return compressed_size+4+3;
}

int Atlas::write_tga(const char *output){
	if(canvas==NULL)
		compile();

	// construct tga header
	unsigned short header[9]={
		0x0000,
		0x0002,
		0x0000,
		0x0000,
		0x0000,
		0x0000,
		(unsigned short)canvas_width,
		(unsigned short)canvas_height,
		0x0820
	};

	// open the output file in write mode
	std::ofstream out(output);
	if(!out){
		std::cout<<"couldn't open "<<output<<" for writing"<<std::endl;
		return 0;
	}

	// write the header
	out.write((char*)header,18);
	// write the bitmap data
	out.write((char*)canvas,canvas_width*canvas_height*4);
	return 18+(canvas_width*canvas_height*4);
}

// insert <b> into the canvas at [b->xpos, b->ypos]
void Atlas::insert(const Bitmap &b){
	if(canvas==NULL)
		return;

	if(!fits(b)){
		std::cout<<"does not fit"<<std::endl;
		return;
	}

	// for each row in <b.bmp>
	for(int row=0;row<b.height;++row){
		const int dest_row_index=(row+b.ypos)*canvas_width;
		const int dest_col_index=b.xpos;
		const int src_row_index=row*b.width;

		// copy the row from source to dest
		memcpy(canvas + ((dest_row_index+dest_col_index)*4), b.bmp + (src_row_index*4), b.width*4);
	}
}

bool Atlas::fits(const Bitmap &b)const{
	for(const Bitmap &x:bitmap_list){
		if(&x==&b)
			continue; // skip self

		if(b.collide(x))
			return false;
	}

	if(!b.contained())
		return false;

	return true;
}

// finalizes the arrangement created by this->compile
// creates the canvas and writes the bmps to their intended location
void Atlas::finalize(){
	// allocate canvas
	canvas_width=proposed_width();
	canvas_height=proposed_height();
	canvas=new unsigned char[canvas_width*canvas_height*4];
	memset(canvas,0,canvas_width*canvas_height*4);

	// write the bitmaps
	for(const Bitmap &bmp:bitmap_list)
		insert(bmp);
}

// determine the width of the canvas based on its current state
int Atlas::proposed_width()const{
	int w=0;
	for(const Bitmap &b:bitmap_list){
		if(b.xpos==-1||b.ypos==-1) // <b> hasn't been evaluated yet
			continue;

		if(b.xpos+b.width+PADDING>w)
			w=b.xpos+b.width+PADDING;
	}

	return w;
}

// determine the height of the canvas based on its current state
int Atlas::proposed_height()const{
	int h=0;
	for(const Bitmap &b:bitmap_list){
		if(b.xpos==-1||b.ypos==-1) // <b> hasn't been evaluated yet
			continue;

		if(b.ypos+b.height+PADDING>h)
			h=b.ypos+b.height+PADDING;
	}

	return h;
}

bool Bitmap::contained()const{
	return xpos>=PADDING&&ypos>=PADDING;
}

bool Bitmap::collide(const Bitmap &other)const{
	if(other.xpos==-1||other.ypos==-1)
		return false; // <other> has not yet been placed in the atlas, ignore it

	return xpos+width+PADDING>other.xpos && xpos-PADDING<other.xpos+other.width &&
	ypos+height+PADDING>other.ypos && ypos-PADDING<other.ypos+other.height;
}

bool Bitmap::operator>(const Bitmap &b)const{
	return size()>b.size();
}
