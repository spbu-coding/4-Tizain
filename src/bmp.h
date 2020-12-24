#ifndef _BMPH_
#define _BMPH_
#pragma pack(push,1)

typedef struct 
{
	unsigned short	bfType; 		 // ������� BMP ����� 
	unsigned int	bfSize;			 // ������ ����� � ������	
	unsigned short	bfReserved1;	 // �������������� ����, ������ ���� �������       
	unsigned short	bfReserved2;	 // �������������� ����, ������ ���� �������       
	unsigned int	bfOffBits;		 // �������� �� ������ ����������� 
}BITMAPFILEHEADER;
typedef struct 
{
    unsigned int biSize;			 // ������ ���������, ������ ���� ����� 40 ��� 3 ������ bmp �����
    int biWidth;					 // ������ �����������
    int biHeight;					 // ������ �����������
    unsigned short biPlanes;		 // ������ ���� ����� 1
    unsigned short biBitCount;		 // ����� ����� �� ������
    unsigned int biCompression;		 // ��� ������. 0 ���� �� ����
    unsigned int biSizeImage;		 // ������ �����������. ����� ���� ����� 0 ���� �� ����
    unsigned int biXPelsPerMeter;	 // ���������������� ����������
    unsigned int biYPelsPerMeter;	 // ���������������� ����������
    unsigned int biClrUsed;			 // ����� ���������� ������������ ��������� ����� ������
    unsigned int biClrImportant;	 // ����� �������� ������
}BITMAPINFOHEADER;
#endif
