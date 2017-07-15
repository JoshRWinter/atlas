#include <iostream>
#include <string.h>
#include <ctype.h>

#include "Atlas.h"

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
	int bytes=atlas.write(argv[1]);
	if(bytes>=0)
		std::cout<<"successfully wrote "<<bytes<<" bytes to \""<<argv[1]<<"\""<<std::endl;
	else
		std::cout<<"error: could not open output file \""<<argv[1]<<"\" in write mode"<<std::endl;

	return 0;
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
