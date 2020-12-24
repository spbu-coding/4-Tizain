#include <stdlib.h>
#include <string.h>
#include "qdbmp.h"
#include "bmp.h"
/* #pragma pack(pop) */

char *infile, *outfile, *sposob;
/* Неудобно неинициализируя. Не могу неинициализированными передавать переменные в функцию и в дальнейшем, 
 если они так и не были инициализированы, чтобы сравнить названия входящего и исходящего файла приходится 
 проверять прежде на инициализацию.. Инициализировать очень хочется. */
FILE *inptr, *outptr;
BITMAPFILEHEADER bf;
BITMAPINFOHEADER bi;
                
int console_read(int argc, char* argv[]) {  // функция обработки команды с консоли
	for (int i = 1; i < argc; i++) {
		char *last_four = "init";
		size_t len = strlen(argv[i]);
		if (len > 4) last_four = &argv[i][len-4];
		if (strcmp (argv[i], "--mine")==0)	sposob = "mine";
		if (strcmp (argv[i], "--their")==0)	sposob = "their";
		if (strcmp (last_four, ".bmp") == 0) {	
			if (infile == NULL) infile = argv[i];
			else outfile = argv[i];
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
	int widthImage = bi.biWidth + (3 * bi.biWidth) % 4; 
	unsigned char bitmapImage[widthImage];
	fseek(inptr, (long)bf.bfOffBits, SEEK_SET);
	for (int i = 1; i < abs(bi.biHeight) + 1; i++) {
		fread(bitmapImage, widthImage, 1, inptr);
		fwrite(&bitmapImage, widthImage, 1, outptr);
	}
	return 0;
}

int mine_24(void) { // самостоятельная обработка bmp-файлов с 24-битными пиксельными изображениями
	int widthImage = 3 * bi.biWidth + (bi.biWidth % 4);
	unsigned char b;
	unsigned char *bitmapImage = (unsigned char*)malloc(widthImage * sizeof(unsigned char));
	int ColorWidthPix = 3 * bi.biWidth; 
	fseek(inptr, (long)bf.bfOffBits, SEEK_SET);
	for (int j = 1; j < abs(bi.biHeight) + 1; j++) {
		fread(bitmapImage, widthImage, 1, inptr);
		unsigned int c = 0;	
		for (int i = 1; i < widthImage + 1; i++) {
			if (i <= ColorWidthPix)  {
				b = ~ bitmapImage[c];
				memcpy(&bitmapImage[c],&b,1); 
			}
			c++;			
		} 
		fwrite(bitmapImage, widthImage, 1, outptr);
		}
	free(bitmapImage);
	return 0;
}

int main(int argc, char* argv[]) {
	if (argc == 1) return 0;
	if (argc != 4) {
		printf("The needed command format: converter.exe --mine (or --their) xxx.bmp yyy.bmp\n");		
		return -1;	
	}
	console_read(argc, argv);    // Чтение команды ввода с консоли и обработка ошибок в этой команде
	if (sposob == NULL) printf("It needs --mine or --their\n"); 
	if (infile == NULL || outfile == NULL) printf("The names of input and output bmp files are needed\n"); 
	if (sposob == NULL || infile == NULL || outfile == NULL) return -1;
	if (infile != NULL && outfile != NULL && strcmp(infile,outfile)==0) {
		printf("The same names of input and output bmp files\n"); 
		return -1;
	}
    inptr = fopen(infile, "rb");   // Открытие bmp-файла на чтение
    if (inptr == NULL) {
        printf("Could not open %s.\n", infile);
        return -1;
    }
	// чтение из файла bmp-заголовков 
	fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);
	// проверка файла на несоответствия
	char bmp_types[6][3] = {"BM", "BA", "CI", "CP", "IC", "PT"};
	int is_file_bmp = 0;
    for(int i = 0; i < 6; i++) {
        if (memcmp(&bf.bfType, bmp_types[i], 2) == 0) is_file_bmp = 1;
    }
	if (is_file_bmp == 0 || bf.bfReserved1 != 0 || bf.bfReserved2 != 0) {
		printf("Not BMP file!\n"); 
		fclose(inptr);
		return -1;
	}
	if (bi.biSize != 0x28) {
		printf("The file does not contain a 40-bytes BITMAPINFOHEADER structure. Not BMP version 3\n"); 
		fclose(inptr);
		return -1;
	}
	if (bi.biPlanes != 0x1) {
		printf("Not BMP file\n"); 
		fclose(inptr);
		return -1;
	}
	if (bi.biBitCount != 0x8 && bi.biBitCount != 0x18) {
		printf("Not BMP 8-bit or 24-bit\n"); 
		fclose(inptr);
		return -1;
	}
	if (bi.biCompression != 0x0) {
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
	if (bi.biBitCount == 0x8) mine_8();
	if (bi.biBitCount == 0x18) mine_24();
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
