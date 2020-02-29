/******************************Module*Header*******************************\
*
*                           *******************
*                           * GDI SAMPLE CODE *
*                           *******************
*
* Module Name: screen.c
*
* Initializes the GDIINFO and DEVINFO structures for DrvEnablePDEV.
*
* Copyright (c) 1992-1998 Microsoft Corporation
\**************************************************************************/

#include "driver.h"

#define SYSTM_LOGFONT {16,7,0,0,700,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,VARIABLE_PITCH | FF_DONTCARE,L"System"}
#define HELVE_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_STROKE_PRECIS,PROOF_QUALITY,VARIABLE_PITCH | FF_DONTCARE,L"MS Sans Serif"}
#define COURI_LOGFONT {12,9,0,0,400,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_STROKE_PRECIS,PROOF_QUALITY,FIXED_PITCH | FF_DONTCARE, L"Courier"}

// This is the basic devinfo for a default driver.  This is used as a base and customized based
// on information passed back from the miniport driver.

const DEVINFO gDevInfoFrameBuffer = {
    0 , /* Graphics capabilities         */
    SYSTM_LOGFONT,    /* Default font description */
    HELVE_LOGFONT,    /* ANSI variable font description   */
    COURI_LOGFONT,    /* ANSI fixed font description          */
    0,                /* Count of device fonts          */
    0,                /* Preferred DIB format          */
    8,                /* Width of color dither          */
    8,                /* Height of color dither   */
    0,
    GCAPS2_SYNCTIMER |
    GCAPS2_SYNCFLUSH                /* Default palette to use for this device */
};

// This is default palette from Win 3.1

#define NUMPALCOLORS 256
#define NUMPALRESERVED 20

/******************************Public*Routine******************************\
* bInitPDEV
*
* Determine the mode we should be in based on the DEVMODE passed in.
* For mirrored devices we don't bother querying the miniport.
*
\**************************************************************************/

BOOL bInitPDEV(
PPDEV ppdev,
DEVMODEW *pDevMode,
GDIINFO *pGdiInfo,
DEVINFO *pDevInfo)
{
    ULONG red, green, blue;
    INT i;

    //
    // Fill in the GDIINFO data structure with the information returned from
    // the kernel driver.
    //
    
    ppdev->ulMode = 0;
    ppdev->cxScreen = pDevMode->dmPelsWidth;
    ppdev->cyScreen = pDevMode->dmPelsHeight;
    ppdev->ulBitCount = pDevMode->dmBitsPerPel;
    ppdev->lDeltaScreen = 0;
    
    ppdev->flRed = 0x00FF0000;
    ppdev->flGreen = 0x000FF00;
    ppdev->flBlue = 0x00000FF;
    
    pGdiInfo->ulVersion    = GDI_DRIVER_VERSION;
    pGdiInfo->ulTechnology = DT_RASDISPLAY;
    pGdiInfo->ulHorzSize   = 0;
    pGdiInfo->ulVertSize   = 0;

    pGdiInfo->ulHorzRes        = ppdev->cxScreen;
    pGdiInfo->ulVertRes        = ppdev->cyScreen;
    pGdiInfo->ulPanningHorzRes = 0;
    pGdiInfo->ulPanningVertRes = 0;
    pGdiInfo->cBitsPixel       = 8;
    pGdiInfo->cPlanes          = 1; 
    pGdiInfo->ulVRefresh       = 1;       // not used
    pGdiInfo->ulBltAlignment   = 1;     // We don't have accelerated screen-
                                        //   to-screen blts, and any
                                        //   window alignment is okay

    pGdiInfo->ulLogPixelsX = pDevMode->dmLogPixels;
    pGdiInfo->ulLogPixelsY = pDevMode->dmLogPixels;

    pGdiInfo->flTextCaps = TC_RA_ABLE;

    pGdiInfo->flRaster = 0;           // flRaster is reserved by DDI

    pGdiInfo->ulDACRed   = 8;
    pGdiInfo->ulDACGreen = 8;
    pGdiInfo->ulDACBlue  = 8;

    pGdiInfo->ulAspectX    = 0x24;    // One-to-one aspect ratio
    pGdiInfo->ulAspectY    = 0x24;
    pGdiInfo->ulAspectXY   = 0x33;

    pGdiInfo->xStyleStep   = 1;       // A style unit is 3 pels
    pGdiInfo->yStyleStep   = 1;
    pGdiInfo->denStyleStep = 3;

    pGdiInfo->ptlPhysOffset.x = 0;
    pGdiInfo->ptlPhysOffset.y = 0;
    pGdiInfo->szlPhysSize.cx  = 0;
    pGdiInfo->szlPhysSize.cy  = 0;

    // RGB and CMY color info.

    pGdiInfo->ciDevice.Red.x = 6700;
    pGdiInfo->ciDevice.Red.y = 3300;
    pGdiInfo->ciDevice.Red.Y = 0;
    pGdiInfo->ciDevice.Green.x = 2100;
    pGdiInfo->ciDevice.Green.y = 7100;
    pGdiInfo->ciDevice.Green.Y = 0;
    pGdiInfo->ciDevice.Blue.x = 1400;
    pGdiInfo->ciDevice.Blue.y = 800;
    pGdiInfo->ciDevice.Blue.Y = 0;
    pGdiInfo->ciDevice.AlignmentWhite.x = 3127;
    pGdiInfo->ciDevice.AlignmentWhite.y = 3290;
    pGdiInfo->ciDevice.AlignmentWhite.Y = 0;

    pGdiInfo->ciDevice.RedGamma = 20000;
    pGdiInfo->ciDevice.GreenGamma = 20000;
    pGdiInfo->ciDevice.BlueGamma = 20000;

    pGdiInfo->ciDevice.Cyan.x = 1750;
    pGdiInfo->ciDevice.Cyan.y = 3950;
    pGdiInfo->ciDevice.Cyan.Y = 0;
    pGdiInfo->ciDevice.Magenta.x = 4050;
    pGdiInfo->ciDevice.Magenta.y = 2050;
    pGdiInfo->ciDevice.Magenta.Y = 0;
    pGdiInfo->ciDevice.Yellow.x = 4400;
    pGdiInfo->ciDevice.Yellow.y = 5200;
    pGdiInfo->ciDevice.Yellow.Y = 0;

    // No dye correction for raster displays.

    pGdiInfo->ciDevice.MagentaInCyanDye = 0;
    pGdiInfo->ciDevice.YellowInCyanDye = 0;
    pGdiInfo->ciDevice.CyanInMagentaDye = 0;
    pGdiInfo->ciDevice.YellowInMagentaDye = 0;
    pGdiInfo->ciDevice.CyanInYellowDye = 0;
    pGdiInfo->ciDevice.MagentaInYellowDye = 0;

    pGdiInfo->ulDevicePelsDPI = 0;   // For printers only
    pGdiInfo->ulPrimaryOrder = PRIMARY_ORDER_CBA;

    // Note: this should be modified later to take into account the size
    // of the display and the resolution.

    pGdiInfo->ulHTPatternSize = HT_PATSIZE_4x4_M;

    pGdiInfo->flHTFlags = HT_FLAG_ADDITIVE_PRIMS;

    // Fill in the basic devinfo structure

    *pDevInfo = gDevInfoFrameBuffer;

    pGdiInfo->ulNumColors = 20;
    pGdiInfo->ulNumPalReg = 256;

    pGdiInfo->ulHTOutputFormat = HT_FORMAT_32BPP;
    pDevInfo->iDitherFormat = BMF_32BPP;



    // create remaining palette entries, simple loop to create uniformly
    // distributed color values.

    red = 0, green = 0, blue = 0;
    
    pDevInfo->hpalDefault = EngCreatePalette(PAL_BITFIELDS, 0, NULL, 0xFF0000, 0xFF00, 0xFF);
  

    
    
    return(TRUE);
}



