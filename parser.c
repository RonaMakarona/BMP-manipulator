#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef unsigned char char8;

struct BITMAP_header {
	char name[2]; // BM
	uint32_t size;
	int garbage; // ?
	uint32_t image_offset; //offset from where the image starts in the file
};

struct DIB_header {
	uint32_t header_size;
	int32_t width;
	int32_t height;
	uint16_t colorplanes;
	uint16_t bitsperpixel;
	uint32_t compression;
	uint32_t image_size;
	uint32_t temp[4];
};


struct RGB
{
	char8 blue;
	char8 green;
	char8 red;
};

struct Image {
	struct RGB** rgb;
	int height;
	int width;
};

void readint(FILE* f, int* val, int size)
{
	unsigned char buf[4];
	fread(buf, size, 1, f);
	*val = 0;
	for (int i = size - 1; i >= 0; i--)
		*val += (buf[i] << (8 * i));
}

void writeint(FILE* f, int val, int size)
{
	unsigned char buf[4];
	for (int i = size - 1; i >= 0; i--)
		buf[i] = (val >> 8 * i) & 0xff;
	fwrite(buf, size, 1, f);
}

struct Image readImage(FILE *fp, int height, int width) {
	struct Image pic;

	pic.rgb = (struct RGB**)malloc(height * sizeof(void*)); // pointer to a row  of rgb data (pixels)
	pic.height = height;
	pic.width = width;

	for (int i = height-1; i >=0 ; i--)
	{
		pic.rgb[i] = (struct RGB*)malloc(width * sizeof(struct RGB)); // allocating a row of pixels
		fread(pic.rgb[i], width, sizeof(struct RGB), fp);
	}
	
	return pic;
}

void freeImage(struct Image pic) {
	for (int i = pic.height -1; i>= 0; i--)
	{
		free(pic.rgb[i]);
	}
	free(pic.rgb);
}

void createImage(struct BITMAP_header header, struct DIB_header dibheader, struct Image pic) {
	FILE* fpw = fopen("new.bmp", "wb");
	if (fpw == NULL) {
		return 1;
	}

	fwrite(header.name, 2, 1, fpw);
	fwrite(&header.size, 3 * sizeof(int), 1, fpw);

	fwrite(&dibheader, sizeof(struct DIB_header), 1, fpw);

	int count = 0;
	for (int i = pic.height - 1; i >= 0; i--) {
		fwrite(pic.rgb[count], pic.width, sizeof(struct RGB), fpw);
		count++;
	}

	fclose(fpw);
}

int openbmpfile() {
	FILE* fp = fopen("C:\\Users\\litva\\Downloads\\snail.bmp", "rb"); // read binary
	if (fp == NULL) {
		return 1;
	}

	struct BITMAP_header header;
	struct DIB_header dibheader;

	fread(header.name, 2, 1, fp); //BM
	readint(fp, &header.size, 4);
	readint(fp, &header.garbage, 4);
	readint(fp, &header.image_offset, 4);

	printf("First two characters: %c%c\n", header.name[0], header.name[1]);
	if ((header.name[0] != 'B') || (header.name[1] != 'M')) {
		fclose(fp);
		return 1;
	}

	printf("Size: %d\n", header.size);
	printf("Offset: %d\n", header.image_offset);

	readint(fp, &dibheader.header_size, 4);
	readint(fp, &dibheader.width, 4);
	readint(fp, &dibheader.height, 4);
	readint(fp, &dibheader.colorplanes, 2);
	readint(fp, &dibheader.bitsperpixel, 2);
	readint(fp, &dibheader.compression, 4);
	readint(fp, &dibheader.image_size, 4);
	readint(fp, &dibheader.temp[0], 4);
	readint(fp, &dibheader.temp[1], 4);
	readint(fp, &dibheader.temp[2], 4);
	readint(fp, &dibheader.temp[3], 4);

	char* buf = malloc(dibheader.image_size);
	if (!buf)
		return 0;
	fread(buf, 1, dibheader.image_size, fp);

	printf("Header size: %d\nWidth: %d\nHeight: %d\nColor planes: %d\nBits per pixel: %d\nCompression: %d\nImage size: %d\n",
		dibheader.header_size, dibheader.width, dibheader.height, dibheader.colorplanes, dibheader.bitsperpixel,
		dibheader.compression, dibheader.image_size);

	if ((dibheader.header_size != 40) || (dibheader.compression != 0) || (dibheader.bitsperpixel != 24)) {
		fclose(fp);
		return 1;
	}

	fseek(fp, header.image_offset, SEEK_SET);
	struct Image image = readImage(fp, dibheader.height, dibheader.width);
	createImage(header, dibheader, image);

	fclose(fp);
	freeImage(image);

	return 0;
}

int main() {
	openbmpfile();
}
