/******************************Module*Header*******************************\
*
*                           *******************
*                           * GDI SAMPLE CODE *
*                           *******************
*
* Module Name: driver.h
*
* contains prototypes for the frame buffer driver.
*
* Copyright (c) 1992-1998 Microsoft Corporation
\**************************************************************************/

#define DBG 1

#include "stddef.h"

#include <stdarg.h>

#pragma warning(push)
#pragma warning(disable: 4200 4201 4214)

#include "windef.h"
#include "wingdi.h"
#include "winddi.h"
#include "devioctl.h"
#include "ntddvdeo.h"

#pragma warning(pop)    // C4200: nonstandard extension used : 
                        //        zero-sized array in struct/union
                        // C4201: nonstandard extension used:
                        //        nameless struct/union
                        // C4214: nonstandard extension used:
                        //        bit field types other than int

#include "debug.h"

typedef struct  _PDEV
{
    HANDLE  hDriver;                    // Handle to \Device\Screen
    HDEV    hdevEng;                    // Engine's handle to PDEV
    HSURF   hsurfEng;                   // Engine's handle to surface
    HPALETTE hpalDefault;               // Handle to the default palette for device.
    PBYTE   pjScreen;                   // This is pointer to base screen address
    ULONG   cxScreen;                   // Visible screen width
    ULONG   cyScreen;                   // Visible screen height
    POINTL  ptlOrg;                     // Where this display is anchored in
                                        //   the virtual desktop.
    ULONG   ulMode;                     // Mode the mini-port driver is in.
    LONG    lDeltaScreen;               // Distance from one scan to the next.
    ULONG   cScreenSize;                // size of video memory, including
                                        // offscreen memory.
    PVOID   pOffscreenList;             // linked list of DCI offscreen surfaces.
    FLONG   flRed;                      // For bitfields device, Red Mask
    FLONG   flGreen;                    // For bitfields device, Green Mask
    FLONG   flBlue;                     // For bitfields device, Blue Mask
    ULONG   cPaletteShift;              // number of bits the 8-8-8 palette must
                                        // be shifted by to fit in the hardware
                                        // palette.
    ULONG   ulBitCount;                 // # of bits per pel 8,16,24,32 are only supported.
    POINTL  ptlHotSpot;                 // adjustment for pointer hot spot
    VIDEO_POINTER_CAPABILITIES PointerCapabilities; // HW pointer abilities
    PVIDEO_POINTER_ATTRIBUTES pPointerAttributes; // hardware pointer attributes
    DWORD   cjPointerAttributes;        // Size of buffer allocated
    BOOL    fHwCursorActive;            // Are we currently using the hw cursor
    PALETTEENTRY *pPal;                 // If this is pal managed, this is the pal
    BOOL    bSupportDCI;                // Does the miniport support DCI?

    PVOID   pvTmpBuffer;                // ptr to MIRRSURF bits for screen surface

    ULONG_PTR pMappedFile;

} PDEV, *PPDEV;

typedef struct _MIRRSURF {
    PPDEV   *pdev;
    ULONG   cx;               
    ULONG   cy;
    ULONG   lDelta;
    ULONG   ulBitCount;
    BOOL    bIsScreen;

} MIRRSURF, *PMIRRSURF;

DWORD getAvailableModes(HANDLE, PVIDEO_MODE_INFORMATION *, DWORD *);
BOOL bInitPDEV(PPDEV, PDEVMODEW, GDIINFO *, DEVINFO *);
BOOL bInitSURF(PPDEV, BOOL);
BOOL bInitPaletteInfo(PPDEV, DEVINFO *);
BOOL bInitPointer(PPDEV, DEVINFO *);
BOOL bInit256ColorPalette(PPDEV);
VOID vDisablePalette(PPDEV);
VOID vDisableSURF(PPDEV);

#define MAX_CLUT_SIZE (sizeof(VIDEO_CLUT) + (sizeof(ULONG) * 256))

//
// Determines the size of the DriverExtra information in the DEVMODE
// structure passed to and from the display driver.
//

#define DRIVER_EXTRA_SIZE 0

#define DLL_NAME                L"framebuf"   // Name of the DLL in UNICODE
#define STANDARD_DEBUG_PREFIX   "FRAMEBUF: "  // All debug output is prefixed
#define ALLOC_TAG               'bfDD'        // Four byte tag (characters in
                                              // reverse order) used for memory
                                              // allocations



#define ROP3_NOP            0xAA
#define ROP3_SRCCOPY        (SRCCOPY >> 16)     // 0xCC
#define ROP3_SRCPAINT       (SRCPAINT >> 16)    // 0xEE
#define ROP3_SRCAND         (SRCAND >> 16)      // 0x88
#define ROP3_SRCINVERT      (SRCINVERT >> 16)   // 0x66
#define ROP3_SRCERASE       (SRCERASE >> 16)    // 0x44
#define ROP3_NOTSRCCOPY     (NOTSRCCOPY >> 16)  // 0x33
#define ROP3_NOTSRCERASE    (NOTSRCERASE >> 16) // 0x11
#define ROP3_MERGECOPY      (MERGECOPY >> 16)   // 0xC0
#define ROP3_MERGEPAINT     (MERGEPAINT >> 16)  // 0xBB
#define ROP3_PATCOPY        (PATCOPY >> 16)     // 0xF0
#define ROP3_PATPAINT       (PATPAINT >> 16)    // 0xFB
#define ROP3_PATINVERT      (PATINVERT >> 16)   // 0x5A
#define ROP3_DSTINVERT      (DSTINVERT >> 16)   // 0x55
#define ROP3_BLACKNESS      (BLACKNESS >> 16)   // 0x00
#define ROP3_WHITENESS      (WHITENESS >> 16)   // 0xFF


#define ROP4_SRCCOPY       (ROP3_SRCCOPY | (ROP3_SRCCOPY << 8))       // 0xCCCC
#define ROP4_NOTSRCCOPY    (ROP3_NOTSRCCOPY | (ROP3_NOTSRCCOPY << 8)) // 0x3333
#define ROP4_NOP           (ROP3_NOP | (ROP3_NOP << 8))               // 0xAAAA
#define ROP4_PATCOPY       (ROP3_PATCOPY | (ROP3_PATCOPY << 8))       // 0xFOFO
#define ROP4_BLACKNESS     (ROP3_BLACKNESS | (ROP3_BLACKNESS << 8))   // 0x0000
#define ROP4_WHITENESS     (ROP3_WHITENESS | (ROP3_WHITENESS << 8))   // 0xFFFF
