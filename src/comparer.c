 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include "bmp.h"
 #define MAX_DIF_PIX 100

char *bmp_file_name_1;  
char *bmp_file_name_2; 
BITMAPFILEHEADER bf_1, bf_2;
BITMAPINFOHEADER bi_1, bi_2;

int console_read(int argc, char* argv[]) {  // функция обработки команды с консоли и ее проверки
	if (argc != 3) {
		printf("The needed command format: comparer.exe xxx.bmp yyy.bmp\n"); 	
		return -1;	
	}
	for (int i = 1; i < argc; i++) {
		/* переменная будет содержать 4 последних символа элементов массива argv для поиска вхождения в них части строки ".bmp"
		Далее применится strcmp при возможном отсутствии инициализации. Мне кажется здесь будет легче проинициализировать	
		эту переменную произвольным значением, чем обрабатывать NULL. И еще хочется избегать предупреждений вроде
		"potentially used without being initialized"...? */
		char *last_four = "init";		
		size_t len = strlen(argv[i]);
		if (len > 4) last_four = &argv[i][len-4];
		if (strcmp(last_four, ".bmp") == 0) {
            if (bmp_file_name_1 == NULL) bmp_file_name_1 = argv[i]; 
        	else  bmp_file_name_2 = argv[i]; 
		}
	}
	if ((bmp_file_name_1 == NULL) || (bmp_file_name_2 == NULL)) {
		printf("The names of input and output bmp files are needed\n"); 
		return -1;
	}
	if (strcmp(bmp_file_name_1,bmp_file_name_2)==0) {
		printf("The same names of input and output bmp files\n"); 		
		return -1;
	}
	return 0;
}

int error_test(void) { // проверка на ошибки файлов BMP
	char bmp_types[6][3] = {"BM", "BA", "CI", "CP", "IC", "PT"};
	int is_file1_bmp = 0, is_file2_bmp = 0;
    for(int i = 0; i < 6; i++) {
        if (memcmp(&bf_1.bfType, bmp_types[i], 2) == 0) is_file1_bmp = 1;
        if (memcmp(&bf_2.bfType, bmp_types[i], 2) == 0) is_file2_bmp = 1;
    }
	if (is_file1_bmp == 0 || bf_1.bfReserved1 != 0 || bf_1.bfReserved2 != 0) {
		printf("Not BMP file %s!\n", bmp_file_name_1); 
		return -1;
	}
	if (is_file2_bmp == 0 || bf_2.bfReserved1 != 0 || bf_2.bfReserved2 != 0) {
		printf("Not BMP file %s!\n", bmp_file_name_2); 
		return -1;
	}
	if (bi_1.biSize != 0x28) {
		printf("The file does not contain a 40-bytes BITMAPINFOHEADER structure. Not BMP version 3 in %s\n", bmp_file_name_1); 
		return -1;
	}
	if (bi_2.biSize != 0x28) {
		printf("The file does not contain a 40-bytes BITMAPINFOHEADER structure. Not BMP version 3 in %s\n", bmp_file_name_2); 
		return -1;
	}
	if (bi_1.biPlanes != 0x1) {
		printf("Not BMP file %s\n", bmp_file_name_1); 
		return -1;
	}
	if (bi_2.biPlanes != 0x1) {
		printf("Not BMP file %s\n", bmp_file_name_2); 
		return -1;
	}
	if (bi_1.biBitCount != 0x8 && bi_1.biBitCount != 0x18) {
		printf("Not BMP 8-bit or 24-bit in %s\n", bmp_file_name_1); 
		return -1;
	}
	if (bi_2.biBitCount != 0x8 && bi_2.biBitCount != 0x18) {
		printf("Not BMP 8-bit or 24-bit in %s\n", bmp_file_name_2); 
		return -1;
	}
	if (bi_1.biCompression != 0x0) {
		printf("File %s compressed!\n", bmp_file_name_1); 
		return -1;
	}
	if (bi_2.biCompression != 0x0) {
		printf("File %s compressed!\n", bmp_file_name_2); 
		return -1;
	}
	if (bi_1.biBitCount != bi_2.biBitCount) {
		printf("Not the same bits in files\n"); 
		return -1;
	}
	if (bi_1.biBitCount == 0x8) {
		if (bf_1.bfOffBits != bf_2.bfOffBits){
			printf("Different sizes of color palettes\n"); 
			return -1;
		}
	}
	if (bi_1.biWidth != bi_2.biWidth || abs(bi_1.biHeight) != abs(bi_2.biHeight)) {
		printf("Not the same sizes\n"); 
		return -1;
	}
	if (bi_1.biWidth < 1 || bi_1.biHeight == 0) {
		printf("Undocumented values for the dimensions of the file %s\n", bmp_file_name_1); 
		return -1;
	}
	if (bi_2.biWidth < 1 || bi_2.biHeight == 0) {
		printf("Undocumented values for the dimensions of the file %s\n", bmp_file_name_2); 
		return -1;
	}
	return 0;
}

int bmp_comparer(FILE *ptr_1, FILE *ptr_2, int *different_pixels) { // сравнение bmp-файлов с 24-битными пиксельными изображениями
	int widthImage, colorWidthPix;
	if (bi_1.biBitCount == 0x8) {     // обработка для 8-битных изображений
		long int infile_ptr = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); // байт до начала палитры
		size_t pallbyte = bf_1.bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER); // размер палитры байт 
		unsigned char ColorPallete_1[pallbyte], ColorPallete_2[pallbyte]; // здесь будут значения палитр двух файлов
		fseek(ptr_1, infile_ptr, SEEK_SET);  // устанавливаем указатель на место, с которого читается палитра
		fread(&ColorPallete_1, sizeof(ColorPallete_1), 1, ptr_1);
		fseek(ptr_2, infile_ptr, SEEK_SET);
		fread(&ColorPallete_2, sizeof(ColorPallete_2), 1, ptr_2);
		for (size_t i = 0; i < pallbyte; i++)	{  // перебор палитр цветов на сравнение
			if 	((i + 1) % 4 != 0) { // каждый четвертый байт не рассматриваем, он д.б. равен 0 и не используется
				if (memcmp(&ColorPallete_1[i],&ColorPallete_2[i],1) != 0) {
					printf("Not the same color palettes");
					*different_pixels = -1;
					return -1;
				}
			}
		}
	    /* в случае ширины изображения некратного четырем в файле существуют добавочные до кратности байты заполненные 
	    произвольной информацией. Определим число байт в строке пиксельного изображения при ширине любой кратности */
		widthImage = bi_1.biWidth + (3 * bi_1.biWidth) % 4; 
		colorWidthPix = bi_1.biWidth; // байт в строке столько же, сколько пикселей
	} else {           // для 24-битных изображений              
		widthImage = 3 * bi_1.biWidth + (bi_1.biWidth % 4);  // байт в 3 раза больше чем значимых пикселей плюс пиксели некратные 4
		colorWidthPix = 3 * bi_1.biWidth;    // байт, что описывают пиксели в строке в 3 раза больше, чем значимых пикселей
	}
	unsigned char *bitmapImage_1 = (unsigned char*) malloc(widthImage * sizeof(unsigned char));
	unsigned char *bitmapImage_2 = (unsigned char*) malloc(widthImage * sizeof(unsigned char));
	long int infile_ptr1, infile_ptr2;   // это будут указатели на места в файлах, с которых будет считывание байт
	// т.к. нам нужно отслеживать не байты, а пикселы, а для них как известно выделяется 3 байта на 1 пиксел в строке (для 24 битных), 
	int current_pixel_y = 0;	     // то вводим переменную, в которой будем учитывать именно пиксел, а не байт
	for (int j = 1; j < abs(bi_1.biHeight) + 1; j++) {  // прогоняем каждую строку в таблице пикселей
		// Если высота отрицательная сравнение начинается с последней строки
		if (bi_1.biHeight < 0) infile_ptr1 = (long)(bf_1.bfOffBits + widthImage * (j - 1)); 
		else infile_ptr1 = (long)(bf_1.bfOffBits + widthImage * (abs(bi_1.biHeight) + 1 - j)); 
		if (bi_2.biHeight < 0) infile_ptr2 = (long)(bf_2.bfOffBits + widthImage * (j - 1)); 
		else infile_ptr2 = (long)(bf_2.bfOffBits + widthImage * (abs(bi_2.biHeight) + 1 - j));		
		fseek(ptr_1, infile_ptr1, SEEK_SET); // нахождение текущего расположения пикселей
		/* в тестировании участвовал 24-битный файл размером 3000х2000 пикселей, мне показалось пиксельное изображение 
		займет много места в памяти (6 млн.пикселей, ок.18мб * здесь еще 2 массива), поэтому тогда подумал сделать построчное считывание */
		fread(bitmapImage_1, widthImage, 1, ptr_1);    // считываем строку в массив
		fseek(ptr_2, infile_ptr2, SEEK_SET); 		
		fread(bitmapImage_2, widthImage, 1, ptr_2);
		for (int i = 1; i < widthImage + 1; i++) {     // для каждых байт строки
			if (i <= colorWidthPix)   {                     // если только они содержат значимые данные
				if (memcmp(&bitmapImage_1[i-1],&bitmapImage_2[i-1],1) != 0)  {    // происходит сравнивание 
					if (bi_1.biBitCount == 0x18) {
						if (current_pixel_y != (i-1)/3 + 1) {          // текущий номер пиксела сравнивается с предыдущим и только если не равен ему
							current_pixel_y = (i-1)/3 + 1;
							*different_pixels = *different_pixels + 1;         // Идет счет количества разных байт пикселов, если они появляются
							if (*different_pixels < MAX_DIF_PIX + 1) {    // если этих разных байт меньше равно 100
								fprintf(stderr, "%d %d\n", j, current_pixel_y); 	// Вывод пиксела: номер строки  номер столбца			 						
							} else return 0;
						}
					}
					if (bi_1.biBitCount == 0x8) {
						*different_pixels = *different_pixels + 1;         // Идет счет количества разных байт пикселов, если они появляются
						if (*different_pixels < MAX_DIF_PIX + 1) {    // если этих разных байт меньше равно 100
							fprintf(stderr, "%d %d\n", j, i); 	    // Вывод пиксела: номер строки  номер столбца			 						
						} else return 0;
					}
				}			
			}
		} 
	}
	free(bitmapImage_1);
	free(bitmapImage_2);
	return 0;
}

int main(int argc, char* argv[]) {
	if (argc == 1) { // чтобы не выводило лишних ошибок при сборке
		printf("\n");
		return 0;
	} 
	if (console_read(argc, argv) != 0) return -1;  // Чтение команды ввода с консоли и обработка ошибок в этой команде
	FILE *inptr_1, *inptr_2;
    inptr_1 = fopen(bmp_file_name_1, "rb");   // Открытие 1 bmp-файла на чтение
    if (inptr_1 == NULL) {
        printf("Could not open %s.\n", bmp_file_name_1);
        return -1;
    }
	inptr_2 = fopen(bmp_file_name_2, "rb");   // Открытие 2 bmp-файла на чтение
    if (inptr_2 == NULL) {
        printf("Could not open %s.\n", bmp_file_name_2);
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
				// вызов функции сравнения
	bmp_comparer(inptr_1, inptr_2, &different_pixels);
	fclose(inptr_1);
	fclose(inptr_2);
	return different_pixels;  // Ноль возвращается только если все пиксели изображений совпали
}
