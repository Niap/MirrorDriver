> google关键字"MirrorDriver"，大部分的可以看到一些应用层的代码，但是驱动层的代码寥寥无几，实际上只有MirrorDriver VNC和Qemu的显卡驱动有相关代码。但实际上从微软的Sample Code修改一下就可以很快实现一个自己的MirrorDriver
## 微软Sample Code 里面的MirrorDriver
MirrorDriver在xp时代是最有效的转屏方式，原理是直接从gdi桌面混合器中直接获取图像数据。那么如果获取图像数据又有两种方式吗，一种是使用文件，另外一种是客户端先分配一段内存然后通过驱动程序让MirrorDriver在这段内存中写入图像数据。

目录结构：微软的SampleCode在WDK7600的\src\video\displays\mirror位置，需要注意的是，MirrorDriver分成三个部分一个是MiniPort驱动，对应的是mini文件夹，作用是实现一个小端口驱动的基本雏形，可以理解为引导程序，其次是MirrorDriver的dll部分，这个具体实现了MirrorDriver框架的一些回调，是整个驱动中最重要的部分，app则是生成一个启动\停止MirrorDriver的控制台程序。

安装方法：可以分别在mini、disp、app目录分别执行bld命令，生成三个文件mirror.dll、mirror.sys，还需要再app目录下找到mirror.inf，讲这三个文件放在同一个文件夹中，右键点击mirror.info，选择安装即可。
## 修改Sample Code
1、在文件driver.h中添加
```cpp
PVOID   pvTmpBuffer;                // ptr to MIRRSURF bits for screen surface
ULONG_PTR pMappedFile;
```
2、在DrvEnableSurface函数中 flHooks |= flGlobalHooks 后的代码替换成一下代码，实际上，EngCreateDeviceSurface和EngCreateBitmap从文档上看没有什么差别，都可以。
```cpp
mirrorsize = (ULONG)(ppdev->cxScreen * ppdev->cyScreen * BitsPerPel);
ppdev->pvTmpBuffer = EngMapFile(L"\\??\\c:\\video.dat", 
                    mirrorsize,
                    &ppdev->pMappedFile);
hsurf = (HSURF) EngCreateBitmap(sizl,
					ppdev->lDeltaScreen,
			  		ulBitmapType,
					0,
					(PVOID)(ppdev->pvTmpBuffer));
if (hsurf == (HSURF) 0){
    RIP("DISP DrvEnableSurface failed EngCreateBitmap\n");
    return(FALSE);
}
if (!EngAssociateSurface(hsurf, ppdev->hdevEng, flHooks)){
    RIP("DISRP DrvEnableSurface failed EngAssociateSurface\n");
    EngDeleteSurface(hsurf);
    return(FALSE);
}
return(hsurf);
```
3、参考源码，修改DrvCopyBits、DrvBitBlt、DrvTextOut等函数，直接调用底层Eng*函数，让驱动直接往下执行。
4、修改sreen.c文件，整体替换就行了。原理可能需要了解gdi内部绘图方式了。
5、编译安装重启后，驱动会在c盘下新建文件video.dat文件，文件内容就是桌面的镜像画面，实际上就是rgba数据

### 客户端实现
1、获取共享内存文件的内容的指针。

```cpp

PCHAR VideoMemory_GetSharedMemory(void)
{
   PCHAR pVideoMemory;
   HANDLE hMapFile, hFile; 
   hFile = CreateFile("c:\\video.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
   if(hFile && hFile != INVALID_HANDLE_VALUE){
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
```
2、生成一张bitmap图片，生成文件头，bitmap头，然后把图像数据拼接在后面，这里可以参考《windows 程序设计》关于dib那章的内容。
```cpp
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
```
