#include <vld.h>
#include <vldapi.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>

#define CHANGE_QUEUE_SIZE          50000


PCHAR VideoMemory_GetSharedMemory(void)
{
   PCHAR pVideoMemory;
   HANDLE hMapFile, hFile; 
   
   hFile = CreateFile("c:\\video.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

   if(hFile && hFile != INVALID_HANDLE_VALUE)
   {
       hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);

       if(hMapFile && hMapFile != INVALID_HANDLE_VALUE)
       {
           pVideoMemory = (PCHAR) MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
    
           CloseHandle(hMapFile);
       }

       CloseHandle(hFile);
   }
   
   return pVideoMemory;
}

VOID cropWindow(PCHAR cropedData,PCHAR myframebuffer,int bufferWidth,int bufferHeight,RECT rect){
	int cropedWidth = rect.right-rect.left;
	int cropedHeight = rect.bottom-rect.top ; 
	int line = 0;
	int offset = 0;
	for(line=bufferHeight-rect.bottom;line<bufferHeight-rect.top;line++){
		offset = line*bufferWidth*4+rect.left*4 ;
		memcpy(cropedData,myframebuffer + offset,(rect.right-rect.left)*4);
		cropedData+=(rect.right-rect.left)*4;
	}
}


void cropWindow(LPBYTE lpBits,PCHAR myframebuffer,int bufferWidth,int bufferHeight,RECT rect){
	int line = 0;
	int offset = 0;
	for(line=bufferHeight-rect.bottom;line<bufferHeight-rect.top;line++){
		offset = line*bufferWidth*4+rect.left*4 ;
		memcpy(lpBits,myframebuffer + offset,(rect.right-rect.left)*4);
		lpBits+=(rect.right-rect.left)*4;
	}
}

void captureScreenFrame(int width,int height,RECT cropRegin, PCHAR myframebuffer,int index)
{

	BITMAPINFOHEADER infoHeader;
	BITMAPFILEHEADER fileHeader;
	
	const int cropReginWidth = cropRegin.right -cropRegin.left;
	const int cropReginHeight = cropRegin.bottom -cropRegin.top;

	const int bits = 32;
	const int bitPerPexcel = 4;
	const int imageSize = width*height*bitPerPexcel;
	const int headerSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

	
	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = cropReginWidth ;
	infoHeader.biHeight = cropReginHeight ;
	infoHeader.biPlanes = 1 ;
	infoHeader.biBitCount = (WORD) bits ;
	infoHeader.biCompression = BI_RGB ;
	infoHeader.biSizeImage = imageSize;
	infoHeader.biXPelsPerMeter = 0 ;
	infoHeader.biYPelsPerMeter = 0 ;
	infoHeader.biClrUsed = (bits <= 8) ? 1<<bits : 0;
	infoHeader.biClrImportant = 0 ;


	

	//位图文件头结构
	

	fileHeader.bfType=0x4D42;//set the attribute of BITMAPFILEHEADER
    fileHeader.bfSize=headerSize+imageSize;
    fileHeader.bfReserved1=0;
    fileHeader.bfReserved2=0;
    fileHeader.bfOffBits=headerSize;
	
	PCHAR imageData = (PCHAR)malloc(imageSize);
	cropWindow(imageData,myframebuffer,width,height,cropRegin);
	//memcpy(lpBits, myframebuffer, width*height*4);


	CHAR fileName[20];
	sprintf(fileName,"c://caps//%d.bmp",index);
	HANDLE pFile = CreateFile(fileName,
		GENERIC_WRITE|GENERIC_READ,          
        FILE_SHARE_READ,
        NULL,               
        CREATE_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL);

	ULONG written;
	WriteFile(pFile,&fileHeader,sizeof(BITMAPFILEHEADER),&written,NULL);
	WriteFile(pFile,&infoHeader,sizeof(BITMAPINFOHEADER),&written,NULL);
	WriteFile(pFile,imageData,imageSize,&written,NULL);
	CloseHandle(pFile);
	free(imageData);
}

int main(){
	PCHAR pVideoMemory = VideoMemory_GetSharedMemory();
	HWND hwnd = FindWindow(TEXT("Notepad"), NULL);
	ShowWindow(hwnd,SW_SHOW);
	RECT rect;
	int i=0;
	while(i<200){
		if(GetWindowRect(hwnd,&rect)){
			captureScreenFrame(1024,768,rect,pVideoMemory,i);
		}else{
			ULONG errorCode = GetLastError();
			return 1;
		}
		Sleep(33);
		i++;
	}
	
	return 0;
}