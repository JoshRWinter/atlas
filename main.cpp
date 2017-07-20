#include <iostream>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Atlas.h"

static void calc_size(int,std::string&);
static void usage(const char*);
static const char *fileext(const char*);
static bool icompare(const char*,const char*);

int main(int argc,char **argv){
	// usage string
	if(argc<3){
		usage(argv[0]);
		return 1;
	}

	Atlas atlas;

	// iterate over input files
	for(int i=2;i<argc;++i){
		// figure out the file ext
		const char *const ext=fileext(argv[i]);

		try{
			// atlas supports targa (.tga) images for now
			if(icompare(ext,"tga")){
				// construct Targa object
				Targa tga(argv[i]); // might throw TextureError
				atlas.add(tga);
			}
			else{
				// unrecognized
				std::cout<<"error: unrecognized \""<<ext<<"\" file"<<std::endl;
				std::cout<<"aborting..."<<std::endl;
				return 1;
			}
		}
		catch(const TextureError &te){
			std::cout<<"error on \""<<argv[i]<<"\": "<<te.what()<<std::endl;
			std::cout<<"aborting..."<<std::endl;
			return 1;
		}
	}

	// write to output file
	const char *const output_ext=fileext(argv[1]);
	int bytes;
	if(icompare(output_ext,"tga"))
		// write atlas to tga file
		bytes=atlas.write_tga(argv[1]);
	else
		bytes=atlas.write(argv[1]);

	// display write info
	std::string formatted,formatted_uncompressed;
	int uncompressed=atlas.get_canvas_size();
	calc_size(bytes,formatted);
	calc_size(uncompressed,formatted_uncompressed);
	if(bytes>=0)
		std::cout<<"successfully wrote "<<atlas.get_count()<<" images ("<<formatted_uncompressed<<" bmp / "<<formatted<<" output) to \""<<argv[1]<<"\""<<std::endl;
	else
		std::cout<<"error: could not open output file \""<<argv[1]<<"\" in write mode"<<std::endl;

	return 0;
}

static void calc_size(int bytes,std::string &size){
	char s[100];
	if(bytes>1048576)
		sprintf(s,"%.2f MB",(double)bytes/1048576);
	else
		sprintf(s,"%.2f KB",(double)bytes/1024);
	size=s;
}

static void usage(const char *name){
	std::cout<<"Usage:\n"<<name<<" output_file file1.tga [file2.tga ...]"<<std::endl;
}

static const char *fileext(const char *name){
	const int len=strlen(name);
	for(int i=len-1;i>=0;--i)
		if(name[i]=='.')
			return name+i+1;

	return name;
}

// portable strcasecmp
static bool icompare(const char *a,const char *b){
	int len=strlen(a);
	if(strlen(b)!=len)
		return false;

	for(int i=0;i<len;++i){
		if(tolower(a[i])!=tolower(b[i]))
			return false;
	}

	return true;
}
