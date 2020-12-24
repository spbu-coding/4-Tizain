#ifndef _BMPH_
#define _BMPH_
#pragma pack(push,1)

typedef struct 
{
	unsigned short	bfType; 		 // Символы BMP файла 
	unsigned int	bfSize;			 // Размер файла в байтах	
	unsigned short	bfReserved1;	 // Неиспользуемое поле, должно быть нулевым       
	unsigned short	bfReserved2;	 // Неиспользуемое поле, должно быть нулевым       
	unsigned int	bfOffBits;		 // Смещение до самого изображения 
}BITMAPFILEHEADER;
typedef struct 
{
    unsigned int biSize;			 // Размер заголовка, должен быть равен 40 для 3 версии bmp файла
    int biWidth;					 // Ширина изображения
    int biHeight;					 // Высота изображения
    unsigned short biPlanes;		 // Должно быть равно 1
    unsigned short biBitCount;		 // Число битов на пиксел
    unsigned int biCompression;		 // Тип сжатия. 0 если не сжат
    unsigned int biSizeImage;		 // Размер изображения. Может быть равен 0 если не сжат
    unsigned int biXPelsPerMeter;	 // Предпочтительное разрешение
    unsigned int biYPelsPerMeter;	 // Предпочтительное разрешение
    unsigned int biClrUsed;			 // Число фактически используемых элементов карты цветов
    unsigned int biClrImportant;	 // Число значимых цветов
}BITMAPINFOHEADER;
#endif
