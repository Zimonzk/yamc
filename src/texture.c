#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>

void loadBMP_custom(const char * imagepath, GLuint textureID, unsigned int blockid){
	static char first = 1;

	SDL_Log("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file) {
		SDL_Log("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
		getchar();
		return;
	} else {
		SDL_Log("Opened texture file");
	}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){
		SDL_Log("Not a correct BMP file\n");
		fclose(file);
		return;
	}
	// A BMP files always begins with "BM"
	if ( header[0]!='B' || header[1]!='M' ){
		SDL_Log("Not a correct BMP file\n");
		fclose(file);
		return;
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1E])!=0  )         {SDL_Log("Not a correct BMP file\n");    fclose(file); return;}
	if ( *(int*)&(header[0x1C])!=24 )         {SDL_Log("Not a correct BMP file\n");    fclose(file); return;}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0) {
		imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
		SDL_Log("guessed image size");
	}
	if (dataPos==0) {
		dataPos=54; // The BMP header is done that way
		SDL_Log("guessed dataPos");
	}
	// Create a buffer
	data = malloc(sizeof(char) * imageSize);
	if(!data) {
		SDL_Log("unable to allocate image memory");
		return;
	}

	fseek(file, dataPos, SEEK_SET);

	SDL_Log("Image size: %u", imageSize);
	// Read the actual data from the file into the buffer
	if(fread(data,1,imageSize,file) != imageSize) {
		SDL_Log("couldn't read");
		return;
	}

	//dumpBMPdat(data, imageSize);

	// Everything is in memory now, the file wan be closed
	fclose (file);

	// Create one OpenGL texture
	//GLuint textureID;
	//glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

	if(first) {
		//glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, width, height, layerCount);
		// Give the image to OpenGL
		//glTexStorage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, height, 2);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, height, 2, 0, GL_BGR, GL_UNSIGNED_BYTE, 0);
		first = 0;
	}
	//glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, layerCount, GL_RGBA, GL_UNSIGNED_BYTE, texels);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, blockid - 1, width, height, 1, GL_BGR, GL_UNSIGNED_BYTE, data);


	// OpenGL has now copied the data. Free our own version
	free(data);

	// Poor filtering, or ...
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// ... nice trilinear filtering.
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);

	SDL_Log("Finished loading texture bmp");

	// Return the ID of the texture we just created
	//return textureID;
}

GLuint load_bmp_gui(const char * imagepath){
	GLuint textureID;
	SDL_Log("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file) {
		SDL_Log("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
		getchar();
		return 0;
	} else {
		SDL_Log("Opened texture file");
	}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){
		SDL_Log("Not a correct BMP file\n");
		fclose(file);
		return 0;
	}
	// A BMP files always begins with "BM"
	if ( header[0]!='B' || header[1]!='M' ){
		SDL_Log("Not a correct BMP file\n");
		fclose(file);
		return 0;
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1E])!=0  )         {SDL_Log("Not a correct BMP file\n");    fclose(file); return 0;}
	if ( *(int*)&(header[0x1C])!=24 )         {SDL_Log("Not a correct BMP file\n");    fclose(file); return 0;}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0) {
		imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
		SDL_Log("guessed image size");
	}
	if (dataPos==0) {
		dataPos=54; // The BMP header is done that way
		SDL_Log("guessed dataPos");
	}
	// Create a buffer
	data = malloc(sizeof(char) * imageSize);
	if(!data) {
		SDL_Log("unable to allocate image mamory");
		return 0;
	}

	fseek(file, dataPos, SEEK_SET);

	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);

	//dumpBMPdat(data, imageSize);
	//SDL_Log("WHS %u %u %u", width, height, imageSize);

	// Everything is in memory now, the file wan be closed
	fclose (file);

	// Create one OpenGL texture
	glGenTextures(1, &textureID);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Give the data to the currently bound GL_TEXTURE_2D
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	free(data);

	// Poor filtering, or ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// ... nice trilinear filtering.
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);

	SDL_Log("Finished loading texture bmp");

	// Return the ID of the texture we just created
	return textureID;
}

void dumpBMPdat(const unsigned char *data, size_t imageSize) {
	size_t n;
	SDL_Log("Dumping image data");
	for(n=0;n < imageSize/3; n = n +3) {
		SDL_Log("b: %i, g: %i, r: %i\n", data[n], data[n+1], data[n+2]);
	}
}
