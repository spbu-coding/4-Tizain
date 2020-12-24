 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include <string.h>

# define MAX_DIF_PIX 100

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

char *infile_1 = "in";
char *infile_2 = "in"; 
BITMAPFILEHEADER bf_1, bf_2;
BITMAPINFOHEADER bi_1, bi_2;



int console_read(int argc, char* argv[]) {  // функция обработки команды с консоли и ее проверки

	if (argc != 3) {
		printf("The needed command format: comparer.exe xxx.bmp yyy.bmp\n"); 	
		return -1;	
	}
	
	for (int i = 1; i < argc; i++) {
		char *last_four = "nope";
		size_t len = strlen(argv[i]);
		if (len > 4) last_four = &argv[i][len-4];
		if (strcmp(last_four, ".bmp") == 0)	
			if (strcmp(infile_1,"in")==0) infile_1 = argv[i];
			else infile_2 = argv[i];
	}
	if (strcmp(infile_1,"in")==0 || strcmp(infile_2,"in")==0) printf("The names of input and output bmp files are needed\n"); 
	if (strcmp(infile_1,infile_2)==0) printf("The same names of input and output bmp files\n"); 
	if (strcmp(infile_1,"in")==0 || strcmp(infile_2,"in")==0 || strcmp(infile_1,infile_2)==0) return -1;
	return 0;
}


int error_test(void) { // проверка на ошибки файлов BMP

	if (bf_1.bfType1 != 'B' || bf_1.bfType2 != 'M' || bf_1.bfReserved1 != 0 || bf_1.bfReserved2 != 0) {
		printf("Not BMP file %s!\n", infile_1); 
		return -1;
	}
	if (bf_2.bfType1 != 'B' || bf_2.bfType2 != 'M' || bf_2.bfReserved1 != 0 || bf_2.bfReserved2 != 0) {
		printf("Not BMP file %s!\n", infile_2); 
		return -1;
	}
	if (bi_1.biSize != 40) {
		printf("Not BMP version 3 in %s\n", infile_1); 
		return -1;
	}
	if (bi_2.biSize != 40) {
		printf("Not BMP version 3 in %s\n", infile_2); 
		return -1;
	}
	if (bi_1.biPlanes != 1) {
		printf("Not BMP file %s\n", infile_1); 
		return -1;
	}
	if (bi_2.biPlanes != 1) {
		printf("Not BMP file %s\n", infile_2); 
		return -1;
	}
	if (bi_1.biBitCount != 8 && bi_1.biBitCount != 24) {
		printf("Not BMP 8-bit or 24-bit in %s\n", infile_1); 
		return -1;
	}
	if (bi_2.biBitCount != 8 && bi_2.biBitCount != 24) {
		printf("Not BMP 8-bit or 24-bit in %s\n", infile_2); 
		return -1;
	}
	if (bi_1.biCompression != 0) {
		printf("File %s compressed!\n", infile_1); 
		return -1;
	}
	if (bi_2.biCompression != 0) {
		printf("File %s compressed!\n", infile_2); 
		return -1;
	}
	if (bi_1.biBitCount != bi_2.biBitCount) {
		printf("Not the same bits in files\n"); 
		return -1;
	}
	if (bi_1.biWidth != bi_2.biWidth || abs(bi_1.biHeight) != abs(bi_2.biHeight)) {
		printf("Not the same sizes\n"); 
		return -1;
	}
	if (bi_1.biWidth < 1 || bi_1.biHeight == 0) {
		printf("Undocumented values for the dimensions of the file %s\n", infile_1); 
		return -1;
	}
	if (bi_2.biWidth < 1 || bi_2.biHeight == 0) {
		printf("Undocumented values for the dimensions of the file %s\n", infile_2); 
		return -1;
	}
	return 0;
}


int bmp_8_comparer(FILE *ptr_1, FILE *ptr_2, int *different_pixels) { // сравнение bmp-файлов с 8-битной цветовой палитрой

/*	size_t pallbyte = bf.bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER);
	unsigned char ColorPallete[pallbyte];
	fread(&ColorPallete, sizeof(ColorPallete), 1, inptr);
	for (size_t i = 0; i < pallbyte; i++)	
		if 	((i + 1) % 4 != 0) ColorPallete[i] = ~ ColorPallete[i];
	fwrite(&ColorPallete, sizeof(ColorPallete), 1, outptr);
*/
	int WidthImage = bi_1.biWidth + (3 * bi_1.biWidth) % 4; // сколько байт на самом деле в строке
	unsigned char bitmapImage_1[WidthImage], bitmapImage_2[WidthImage];
	long int infile_ptr1, infile_ptr2;

	for (int j = 1; j < abs(bi_1.biHeight) + 1; j++) {
			// Если высота отрицательная сравнивание начинается с последней строки
		if (bi_1.biHeight > 0) infile_ptr1 = (long)(bf_1.bfOffBits + WidthImage * (j - 1)); 
		else infile_ptr1 = (long)(bf_1.bfOffBits + WidthImage * (abs(bi_1.biHeight) + 1 - j)); 
		if (bi_2.biHeight > 0) infile_ptr2 = (long)(bf_2.bfOffBits + WidthImage * (j - 1)); 
		else infile_ptr2 = (long)(bf_2.bfOffBits + WidthImage * (abs(bi_2.biHeight) + 1 - j));		
		
		fseek(ptr_1, infile_ptr1, SEEK_SET); // нахождение текущего расположения пикселей
		fread(bitmapImage_1, WidthImage, 1, ptr_1);    // считываем строку в массив
		fseek(ptr_2, infile_ptr2, SEEK_SET); 		
		fread(bitmapImage_2, WidthImage, 1, ptr_2);
		for (int i = 1; i < WidthImage + 1; i++) {     // для каждых байт строки
			if (i <= bi_1.biWidth)   {                     // если только они содержат значимые данные
				if (bitmapImage_1[i-1] != bitmapImage_2[i-1]) {    // происходит сравнивание 
					*different_pixels = *different_pixels + 1;         // Идет счет количества разных байт пикселов, если они появляются
					if (*different_pixels < MAX_DIF_PIX + 1) {    // если этих разных байт меньше равно 100
						fprintf(stderr, "%d %d\n", j, i); 	    // Вывод пиксела: номер строки  номер столбца			 						
					}
					else return 0;
				}			
			}
		}
	}
	return 0;
}

int bmp_24_comparer(FILE *ptr_1, FILE *ptr_2, int *different_pixels) { // сравнение bmp-файлов с 24-битными пиксельными изображениями

	int WidthImage = 3 * bi_1.biWidth + (bi_1.biWidth % 4);  // сколько байт на самом деле в строке (в 3 раза больше чем значимых пикселей плюс пиксели некратные 4)
	unsigned char bitmapImage_1[WidthImage], bitmapImage_2[WidthImage];  // массив по числу количества байт в строке
	int ColorWidthPix = 3 * bi_1.biWidth;    // сколько тех байт, что описывают пиксели в строке, их в 3 раза больше, чем значимых пикселей
	long int infile_ptr1, infile_ptr2;   // это будут указатели на места в файлах, с которых будет считывание байт
	// т.к. нам нужно отслеживать не байты, а пикселы, а для них как известно выделяется 3 байта на 1 пиксел в строке (для 24 битных), 
	int current_pixel_y = 0;	     // то вводим переменную, в которой будем учитывать именно пиксел, а не байт

	for (int j = 1; j < abs(bi_1.biHeight) + 1; j++) {  // прогоняем каждую строку в таблице пикселей
//		current_pixel_x = (j-1)/3 + 1;   // текущий номер строки пиксела 
			// Если высота отрицательная сравнивание начинается с последней строки
		if (bi_1.biHeight > 0) infile_ptr1 = (long)(bf_1.bfOffBits + WidthImage * (j - 1)); 
		else infile_ptr1 = (long)(bf_1.bfOffBits + WidthImage * (abs(bi_1.biHeight) + 1 - j)); 
		if (bi_2.biHeight > 0) infile_ptr2 = (long)(bf_2.bfOffBits + WidthImage * (j - 1)); 
		else infile_ptr2 = (long)(bf_2.bfOffBits + WidthImage * (abs(bi_2.biHeight) + 1 - j));		
		
		fseek(ptr_1, (long)(bf_1.bfOffBits + WidthImage * (j - 1)), SEEK_SET); // нахождение текущего расположения пикселей
		fread(bitmapImage_1, WidthImage, 1, ptr_1);    // считываем строку в массив
		fseek(ptr_2, (long)(bf_2.bfOffBits + WidthImage * (j - 1)), SEEK_SET); 		
		fread(bitmapImage_2, WidthImage, 1, ptr_2);
		for (int i = 1; i < WidthImage + 1; i++) {     // для каждых байт строки
			if (i <= ColorWidthPix)   {                     // если только они содержат значимые данные
				if (bitmapImage_1[i-1] != bitmapImage_2[i-1]) {    // происходит сравнивание 
					if (current_pixel_y != (i-1)/3 + 1) {          // текущий номер пиксела сравнивается с предыдущим и только если не равен ему
						current_pixel_y = (i-1)/3 + 1;
						*different_pixels = *different_pixels + 1;         // Идет счет количества разных байт пикселов, если они появляются
						if (*different_pixels < MAX_DIF_PIX + 1) {    // если этих разных байт меньше равно 100
							fprintf(stderr, "%d %d\n", j, current_pixel_y); 	// Вывод пиксела: номер строки  номер столбца			 						
						}
					else return 0;
					}
				}			
			}
		} 
	}
	return 0;
}



int main(int argc, char* argv[]) {

	if (argc == 1) { // чтобы не выводило лишних ошибок при сборке
		printf("\n");
		return 0;
	} 
	if (console_read(argc, argv) != 0) return -1;  // Чтение команды ввода с консоли и обработка ошибок в этой команде

	FILE *inptr_1, *inptr_2;

    inptr_1 = fopen(infile_1, "rb");   // Открытие 1 bmp-файла на чтение
    if (inptr_1 == NULL) {
        printf("Could not open %s.\n", infile_1);
        return -1;
    }
	inptr_2 = fopen(infile_2, "rb");   // Открытие 2 bmp-файла на чтение
    if (inptr_2 == NULL) {
        printf("Could not open %s.\n", infile_2);
        return -1;
    }
	
	// чтение из файлов bmp-заголовков 
	fread(&bf_1, sizeof(BITMAPFILEHEADER), 1, inptr_1);
    fread(&bi_1, sizeof(BITMAPINFOHEADER), 1, inptr_1);
	fread(&bf_2, sizeof(BITMAPFILEHEADER), 1, inptr_2);
    fread(&bi_2, sizeof(BITMAPINFOHEADER), 1, inptr_2);

	if (error_test() != 0) {   // проверка BMP-файлов на несоответствие
		fclose(inptr_1);
		fclose(inptr_2);
		return -1;
	}

	int different_pixels = 0;   // количество разных байт пикселов
				// вызов функций сравнения для 8 или 24 битных изображений
	if (bi_1.biBitCount == 8) bmp_8_comparer(inptr_1, inptr_2, &different_pixels);
	if (bi_1.biBitCount == 24) bmp_24_comparer(inptr_1, inptr_2, &different_pixels);

	fclose(inptr_1);
	fclose(inptr_2);

	return different_pixels;  // Ноль возвращается только если все пиксели изображений совпали
}
