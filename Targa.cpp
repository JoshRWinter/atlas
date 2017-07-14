#include <stdio.h>

#include "Targa.h"

Targa::Targa(const char *filename){
	// open the file
	FILE *file=fopen(filename,"rb");
	if(!file)
		throw TextureErrorNotFound();

	fclose(file);

	width=0;
	height=0;
	data=NULL;
}

Targa::~Targa(){
	delete[] data;
}

void Targa::get_bitmap(unsigned char *bmp)const{
}
