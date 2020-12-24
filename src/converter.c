 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include <string.h>
  #include "qdbmp.h"

#pragma pack(push,1)
typedef struct 
{
	unsigned char	bfType1,bfType2;			//Символы BM 
	unsigned int	bfSize;			//Размер файла в байтах	
	unsigned short	bfReserved1;	       
	unsigned short	bfReserved2;	     
	unsigned int	bfOffBits;		//Смещение до самого изображения 
}BITMAPFILEHEADER;

typedef struct 
{
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    unsigned int biXPelsPerMeter;
    unsigned int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
}BITMAPINFOHEADER;
#pragma pack(pop)

FILE *inptr, *outptr;
BITMAPFILEHEADER bf;
BITMAPINFOHEADER bi;

                // функция обработки команды с консоли
int console_read(int argc, char* argv[], char **infile_t, char **outfile_t, char **sposob_t) {  
	for (int i = 1; i < argc; i++) {
		char *last_four = "nope";
		size_t len = strlen(argv[i]);
		if (len > 4) last_four = &argv[i][len-4];
		if (strcmp (argv[i], "--mine")==0)	*sposob_t = "mine";
		if (strcmp (argv[i], "--their")==0)	*sposob_t = "their";
		if (strcmp (last_four, ".bmp") == 0) {	
			if (strcmp (*infile_t,"in")==0) *infile_t = argv[i];
			else *outfile_t = argv[i];
		}
	}
	return 0;
}

int mine_8(void) {  // самостоятельная обработка bmp-файлов с 8-битной цветовой палитрой

	size_t pallbyte = bf.bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER);
	unsigned char ColorPallete[pallbyte];
	fread(&ColorPallete, sizeof(ColorPallete), 1, inptr);
	for (size_t i = 0; i < pallbyte; i++)	
		if 	((i + 1) % 4 != 0) ColorPallete[i] = ~ ColorPallete[i];
	fwrite(&ColorPallete, sizeof(ColorPallete), 1, outptr);

	int WidthImage = bi.biWidth + (3 * bi.biWidth) % 4; 
	unsigned char bitmapImage[WidthImage];

	fseek(inptr, (long)bf.bfOffBits, SEEK_SET);
	for (int i = 1; i < abs(bi.biHeight) + 1; i++) {
		fread(bitmapImage, WidthImage, 1, inptr);
		fwrite(&bitmapImage, WidthImage, 1, outptr);
	}
	return 0;
}

int mine_24(void) { // самостоятельная обработка bmp-файлов с 24-битными пиксельными изображениями

	int WidthImage = 3 * bi.biWidth + (bi.biWidth % 4);
	unsigned char bitmapImage[WidthImage];
	int ColorWidthPix = 3 * bi.biWidth; 

	fseek(inptr, (long)bf.bfOffBits, SEEK_SET);

	for (int j = 1; j < abs(bi.biHeight) + 1; j++) {
		fread(bitmapImage, WidthImage, 1, inptr);
		unsigned int c = 0;	
		for (int i = 1; i < WidthImage + 1; i++) {
			if (i <= ColorWidthPix)  
				bitmapImage[c] = bitmapImage[c] ^ 255u;
			c++;			
		} 
		fwrite(&bitmapImage, WidthImage, 1, outptr);
		}
	return 0;
}


int main(int argc, char* argv[]) {

	if (argc == 1) return 0;
	if (argc != 4) {
		printf("The needed command format: converter.exe --mine (or --their) xxx.bmp yyy.bmp\n");		
		return -1;	
	}
	char *infile = "in";
	char *outfile = "out"; 
	char *sposob = "no";

	console_read(argc, argv, &infile, &outfile, &sposob);    // Чтение команды ввода с консоли и обработка ошибок в этой команде
	if (strcmp(sposob,"no")==0) printf("It needs --mine or --their\n"); 
	if (strcmp(infile,"in")==0 || strcmp(outfile,"out")==0) printf("The names of input and output bmp files are needed\n"); 
	if (strcmp(infile,outfile)==0) printf("The same names of input and output bmp files\n"); 
	if (strcmp(sposob,"no")==0 || strcmp(infile,"in")==0 || strcmp(outfile,"out")==0 || strcmp(infile,outfile)==0) return -1;

    inptr = fopen(infile, "rb");   // Открытие bmp-файла на чтение
    if (inptr == NULL) {
        printf("Could not open %s.\n", infile);
        return -1;
    }
	
	// чтение из файла bmp-заголовков 
	fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

	// проверка файла на несоответствия
	if (bf.bfType1 != 'B' || bf.bfType2 != 'M' || bf.bfReserved1 != 0 || bf.bfReserved2 != 0) {
		printf("Not BMP file!\n"); 
		fclose(inptr);
		return -1;
	}
	if (bi.biSize != 40) {
		printf("Not BMP version 3\n"); 
		fclose(inptr);
		return -1;
	}
	if (bi.biPlanes != 1) {
		printf("Not BMP file\n"); 
		fclose(inptr);
		return -1;
	}
	if (bi.biBitCount != 8 && bi.biBitCount != 24) {
		printf("Not BMP 8-bit or 24-bit\n"); 
		fclose(inptr);
		return -1;
	}
	if (bi.biCompression != 0) {
		printf("File compressed!\n"); 
		fclose(inptr);
		return -2;
	}
   
	if (strcmp(sposob,"their")==0)	goto Label_1;    

	// открытие другого файла на запись
    outptr = fopen(outfile, "wb");
    if (outptr == NULL){
        fclose(inptr);
        printf("Could not create %s.\n", outfile);
        return -1;
    }

	 // запись в другой файл bmp-заголовков 
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

	// вызов функций конвертации в негатив для 8 или 24 битных изображений
	if (bi.biBitCount == 8) mine_8();
	if (bi.biBitCount == 24) mine_24();

	// закрытие файлов
	fclose(outptr);
	Label_1:
	fclose(inptr);
	
	if (strcmp(sposob,"their")==0) {  // вызов чужого способа 
		if (bi.biBitCount == 8) {
			printf("Aliens cant work with 8-bit image\n");
			return -3;
		}
		char *anargv[3] = {"negative", infile, outfile};
		//negative(количество элементов в массиве anargv, сам массив)
		negative ( sizeof(anargv)/sizeof(anargv[0]),  anargv );
	}		

	return 0;
}
