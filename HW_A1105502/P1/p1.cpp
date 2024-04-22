#include <stdio.h>
#include <stdlib.h>

typedef long INT32;
typedef unsigned short int INT16;
typedef unsigned char U_CHAR;

#define UCH(x) ((int)(x))
#define GET_2B(array, offset) ((INT16)UCH(array[offset]) + \
                               (((INT16)UCH(array[offset + 1])) << 8))
#define GET_4B(array, offset) ((INT32)UCH(array[offset]) +               \
                               (((INT32)UCH(array[offset + 1])) << 8) +  \
                               (((INT32)UCH(array[offset + 2])) << 16) + \
                               (((INT32)UCH(array[offset + 3])) << 24))
#define FREAD(file, buf, sizeofbuf) \
   ((size_t)fread((void *)(buf), (size_t)1, (size_t)(sizeofbuf), (file)))

void generateArray(int coordinates[][2], int numPoints, int *outputArray)
{
   int i, j;
   // 初始化輸出陣列
   for (i = 0; i < 256; i++)
   {
      outputArray[i] = 0;
   }

   // 根據座標點設置對應的y值
   for (i = 0; i < numPoints - 1; i++)
   {
      int x1 = coordinates[i][0];
      int y1 = coordinates[i][1];
      int x2 = coordinates[i + 1][0];
      int y2 = coordinates[i + 1][1];

      // 計算斜率
      double slope = (double)(y2 - y1) / (double)(x2 - x1);

      // 計算兩點之間的y值
      for (j = x1; j <= x2; j++)
      {
         outputArray[j] = (int)(y1 + slope * (j - x1));
      }
   }
}

int main()
{
   FILE *input_file = 0;
   FILE *output_file = 0;

   U_CHAR bmpfileheader[14] = {0};
   U_CHAR bmpinfoheader[40] = {0};

   // U_CHAR bmpinfoheader[40] = { 0 } ;

   INT32 FileSize = 0;
   INT32 bfOffBits = 0;
   INT32 headerSize = 0;
   INT32 biWidth = 0;
   INT32 biHeight = 0;
   INT16 biPlanes = 0;
   INT16 BitCount = 0;
   INT32 biCompression = 0;
   INT32 biImageSize = 0;
   INT32 biXPelsPerMeter = 0, biYPelsPerMeter = 0;
   INT32 biClrUsed = 0;
   INT32 biClrImp = 0;

   U_CHAR *data, *new_data, color_table[1024];
   int i, j, k, temp;
   int histo_table[256] = {0};
   int specify_histo_table[256] = {0};
   int inverse_histo_table[256] = {0};

   /* �}���ɮ� */
   if ((input_file = fopen("Fig3.23(a).bmp", "rb")) == NULL)
   {
      fprintf(stderr, "File can't open.\n");
      exit(0);
   }

   FREAD(input_file, bmpfileheader, 14);
   FREAD(input_file, bmpinfoheader, 40);

   if (GET_2B(bmpfileheader, 0) == 0x4D42) /* 'BM' */
      fprintf(stdout, "BMP file.\n");
   else
   {
      fprintf(stdout, "Not bmp file.\n");
      exit(0);
   }

   FileSize = GET_4B(bmpfileheader, 2);
   bfOffBits = GET_4B(bmpfileheader, 10);
   headerSize = GET_4B(bmpinfoheader, 0);
   biWidth = GET_4B(bmpinfoheader, 4);
   biHeight = GET_4B(bmpinfoheader, 8);
   biPlanes = GET_2B(bmpinfoheader, 12);
   BitCount = GET_2B(bmpinfoheader, 14);
   biCompression = GET_4B(bmpinfoheader, 16);
   biImageSize = GET_4B(bmpinfoheader, 20);
   biXPelsPerMeter = GET_4B(bmpinfoheader, 24);
   biYPelsPerMeter = GET_4B(bmpinfoheader, 28);
   biClrUsed = GET_4B(bmpinfoheader, 32);
   biClrImp = GET_4B(bmpinfoheader, 36);

   printf("FileSize = %ld \n"
          "DataOffset = %ld \n"
          "HeaderSize = %ld \n"
          "Width = %ld \n"
          "Height = %ld \n"
          "Planes = %d \n"
          "BitCount = %d \n"
          "Compression = %ld \n"
          "ImageSize = %ld \n"
          "XpixelsPerM = %ld \n"
          "YpixelsPerM = %ld \n"
          "ColorsUsed = %ld \n"
          "ColorsImportant = %ld \n",
          FileSize, bfOffBits, headerSize, biWidth, biHeight, biPlanes,
          BitCount, biCompression, biImageSize, biXPelsPerMeter, biYPelsPerMeter, biClrUsed, biClrImp);

   if (BitCount != 8)
   {
      fprintf(stderr, "Not a 8-bit file.\n");
      fclose(input_file);
      exit(0);
   }

   FREAD(input_file, color_table, 1024);

   //
   data = (U_CHAR *)malloc(((biWidth * 1 + 3) / 4 * 4) * biHeight * 1);
   if (data == NULL)
   {
      fprintf(stderr, "Insufficient memory.\n");
      fclose(input_file);
      exit(0);
   }

   //
   fseek(input_file, bfOffBits, SEEK_SET);
   FREAD(input_file, data, ((biWidth * 1 + 3) / 4 * 4) * biHeight * 1);
   //
   fclose(input_file);

   // Process the file
   for (i = 0; i < biHeight; i++)
   {
      k = i * ((biWidth * 1 + 3) / 4 * 4);
      for (j = 0; j < biWidth; j++)
      {
         histo_table[data[k]]++;
         k = k + 1;
      }
   }

   for (i = 1; i < 256; i++)
      histo_table[i] = histo_table[i - 1] + histo_table[i];

   temp = histo_table[255];
   for (i = 0; i < 256; i++)
      histo_table[i] = histo_table[i] * 255 / temp;

   int arr[6][2] = {{0, 0}, {5, 70000}, {28, 8000}, {180, 0}, {205, 6000}, {255, 0}};
   int *tempArray = (int *)malloc(256 * sizeof(int)); // 生成指定的直方圖
   generateArray(arr, 6, tempArray);
   for (i = 0; i < 256; i++)
   {
      specify_histo_table[i] = tempArray[i];
   }

   for (i = 1; i < 256; i++)
      specify_histo_table[i] = specify_histo_table[i - 1] + specify_histo_table[i];

   temp = specify_histo_table[255];
   for (i = 0; i < 256; i++)
      specify_histo_table[i] = specify_histo_table[i] * 255 / temp;

   for (i = 0; i < 256; i++)
   {
      inverse_histo_table[specify_histo_table[i]] = i; // inverse
   }

   for (i = 0; i < 256; i++)
   {
      // 當指定的直方圖為0時，將其設置為前一個值 => 生成遞增函數
      if (inverse_histo_table[i] == 0)
      {
         inverse_histo_table[i] = inverse_histo_table[i - 1];
      }
   }

   for (i = 0; i < 256; i++)
   {
      printf("%d,", specify_histo_table[i]);
      printf("%d\n", inverse_histo_table[i]);
   }

   // Process the file
   for (i = 0; i < biHeight; i++)
   {
      k = i * ((biWidth * 1 + 3) / 4 * 4);
      for (j = 0; j < biWidth; j++)
      {
         data[k] = inverse_histo_table[histo_table[data[k]]]; // 將原始直方圖的值作為索引，取得指定直方圖的值，再取得反向直方圖的值
         k = k + 1;
      }
   }

   //
   /* �}�ҷs�ɮ� */
   if ((output_file = fopen("p1.bmp", "wb")) == NULL)
   {
      fprintf(stderr, "Output file can't open.\n");
      exit(0);
   }

   fwrite(bmpfileheader, sizeof(bmpfileheader), 1, output_file);
   fwrite(bmpinfoheader, sizeof(bmpinfoheader), 1, output_file);

   fwrite(color_table, 1024, 1, output_file);

   fwrite(data, ((biWidth * 1 + 3) / 4 * 4) * biHeight * 1, 1, output_file);

   fclose(output_file);

   return 0;
}