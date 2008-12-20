// ImgTifFile.cpp: implementation of the CImgTifFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImgTifFile.h"
#include <math.h>

extern "C" {
#define __INT32__
#include "tiff/tiffio.h"
}

#define CVT(x)			(((x) * 255L) / ((1L<<16)-1))
#define	SCALE(x)		(((x)*((1L<<16)-1))/255)
#define CalculateLine(width,bitdepth)	(((width * bitdepth) + 7) / 8)
#define CalculatePitch(line)	(line + 3 & ~3)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImgTifFile::CImgTifFile()
{

}

CImgTifFile::~CImgTifFile()
{

}

BOOL CImgTifFile::Open(LPCTSTR szFile)
{
	//Comment this line if you need more information on errors
	TIFFSetErrorHandler(NULL);	//<Patrick Hoffmann>
	
	//Open file and fill the TIFF structure
	// m_tif = TIFFOpen(imageFileName,"rb");
	FILE	*fp = _tfopen(szFile, _T("rb"));
	if (!fp)
		return false;
	stringex	strFileA;
	ConvertStr(szFile, -1, strFileA);

	TIFF* m_tif = TIFFFdOpen((int)fp, strFileA.c_str(), "rb");
	
	uint32 height=0;
	uint32 width=0;
	uint16 bitspersample=1;
	uint16 samplesperpixel=1;
	uint32 rowsperstrip=(DWORD)-1;
	uint16 photometric=0;
	uint16 compression=1;
	uint16 orientation=ORIENTATION_TOPLEFT; //<vho>
	int		bitcount;
	int		clrUsed;
	uint32 x, y;
	float resolution;//, offset;
	BOOL isRGB;
	BYTE *bits;		//pointer to source data
	BYTE *bits2;	//pointer to destination data
	
	//check if it's a tiff file
	if (!m_tif)
	{
		// throw "Error encountered while opening TIFF file";
		return false;
	}

	// <Robert Abram> - 12/2002 : get NumFrames directly, instead of looping
	// info.nNumFrames=0;
	// while(TIFFSetDirectory(m_tif,(uint16)info.nNumFrames)) info.nNumFrames++;
	// info.nNumFrames = TIFFNumberOfDirectories(m_tif);
	
	if (!TIFFSetDirectory(m_tif, (uint16)0))
		throw "Error: page not present in TIFF file";			
	
	//get image info
	TIFFGetField(m_tif, TIFFTAG_COMPRESSION, &compression);
	//	if (compression == COMPRESSION_LZW)
	//		throw "LZW compression is no longer supported due to Unisys patent enforcement";			
	
	TIFFGetField(m_tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(m_tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(m_tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	TIFFGetField(m_tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	TIFFGetField(m_tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);   
	TIFFGetField(m_tif, TIFFTAG_PHOTOMETRIC, &photometric);
	TIFFGetField(m_tif, TIFFTAG_ORIENTATION, &orientation);
	
	if (TIFFGetField(m_tif, TIFFTAG_XRESOLUTION, &resolution))	m_lpImgData->SetXDPI((long)resolution);
	if (TIFFGetField(m_tif, TIFFTAG_YRESOLUTION, &resolution))	m_lpImgData->SetYDPI((long)resolution);
	// if (TIFFGetField(m_tif, TIFFTAG_XPOSITION, &offset))	info.xOffset = (long)offset;
	// if (TIFFGetField(m_tif, TIFFTAG_YPOSITION, &offset))	info.yOffset = (long)offset;

	if (rowsperstrip > height){
		rowsperstrip = height;
		TIFFSetField(m_tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
	}

	isRGB = (bitspersample >= 8) &&
		(photometric == PHOTOMETRIC_RGB) ||
		(photometric == PHOTOMETRIC_YCBCR) ||
		(photometric == PHOTOMETRIC_SEPARATED) ||
		(photometric == PHOTOMETRIC_LOGL) ||
		(photometric == PHOTOMETRIC_LOGLUV);

	if (isRGB)
	{
		bitcount = 24;
	}
	else
	{
		if ((photometric == PHOTOMETRIC_MINISBLACK) || (photometric == PHOTOMETRIC_MINISWHITE))
		{
			if	(bitspersample == 1)
			{
				bitcount = 1;		//B&W image
				clrUsed = 2;
			}
			else if (bitspersample == 4)
			{
				bitcount = 4;		//16 colors gray scale
				clrUsed = 16;
			}
			else
			{
				bitcount = 8;		//gray scale
				clrUsed = 256;
			}
		}
		else if (bitspersample == 4)
		{
			bitcount = 4;			// 16 colors
			clrUsed = 16;
		}
		else
		{
			bitcount = 8;			//256 colors
			clrUsed = 256;
		}
	}

	m_lpImgData->Create(width, height, bitcount, clrUsed);	//image creation
// 
// #if CXIMAGE_SUPPORT_ALPHA
// 		if (samplesperpixel==4) AlphaCreate();	//add alpha support for 32bpp tiffs
// 		if (samplesperpixel==2 && bitspersample==8) AlphaCreate();	//add alpha support for 8bpp + alpha
// #endif //CXIMAGE_SUPPORT_ALPHA

	if (isRGB)
	{
		// Read the whole image into one big RGBA buffer using
		// the traditional TIFFReadRGBAImage() API that we trust.
		uint32* raster;		// retrieve RGBA image
		uint32 *row;

		raster = (uint32*)_TIFFmalloc(width * height * sizeof (uint32));
		if (raster == NULL)
		{
			// throw "No space for raster buffer";
			return false;
		}

		// Read the image in one chunk into an RGBA array
		if(!TIFFReadRGBAImage(m_tif, width, height, raster, 1))
		{
			_TIFFfree(raster);
			// throw "Corrupted TIFF file!";
			return false;
		}

		// read the raster lines and save them in the DIB
		// with RGB mode, we have to change the order of the 3 samples RGB
		row = &raster[0];
		bits2 = m_lpImgData->GetImgData();
		for (y = 0; y < height; y++)
		{
			bits = bits2;
			for (x = 0; x < width; x++)
			{
				*bits++ = (BYTE)TIFFGetB(row[x]);
				*bits++ = (BYTE)TIFFGetG(row[x]);
				*bits++ = (BYTE)TIFFGetR(row[x]);
// #if CXIMAGE_SUPPORT_ALPHA
// 				if (samplesperpixel==4) AlphaSet(x,y,(BYTE)TIFFGetA(row[x]));
// #endif //CXIMAGE_SUPPORT_ALPHA
			}
			row += width;
			bits2 += m_lpImgData->GetRowWidthBytes();
		}
		_TIFFfree(raster);
	}
	else
	{
		RGBQUAD *pal;
		pal=(RGBQUAD*)calloc(256, sizeof(RGBQUAD));
		if (pal==NULL)
		{
			// throw "Unable to allocate TIFF palette";
			return false;
		}

		// set up the colormap based on photometric	
		switch(photometric)
		{
		case PHOTOMETRIC_MINISBLACK:	// bitmap and greyscale image types
		case PHOTOMETRIC_MINISWHITE:
			if (bitspersample == 1)
			{
				// Monochrome image
				if (photometric == PHOTOMETRIC_MINISBLACK) {
					pal[1].rgbRed = pal[1].rgbGreen = pal[1].rgbBlue = 255;
				} else {
					pal[0].rgbRed = pal[0].rgbGreen = pal[0].rgbBlue = 255;
				}
			}
			else
			{
				// need to build the scale for greyscale images
				if (photometric == PHOTOMETRIC_MINISBLACK) {
					for (DWORD i=0; i<clrUsed; i++){
						pal[i].rgbRed = pal[i].rgbGreen = pal[i].rgbBlue = (BYTE)(i*(255/(clrUsed-1)));
					}
				}
				else
				{
					for (DWORD i=0; i<clrUsed; i++)
					{
						pal[i].rgbRed = pal[i].rgbGreen = pal[i].rgbBlue = (BYTE)(255-i*(255/(clrUsed-1)));
					}
				}
			}
			break;
		case PHOTOMETRIC_PALETTE:	// color map indexed
			uint16 *red;
			uint16 *green;
			uint16 *blue;
			TIFFGetField(m_tif, TIFFTAG_COLORMAP, &red, &green, &blue); 

			// Is the palette 16 or 8 bits ?
			BOOL Palette16Bits = FALSE;
			int n=1<<bitspersample;
			while (n-- > 0)
			{
				if (red[n] >= 256 || green[n] >= 256 || blue[n] >= 256) {
					Palette16Bits=TRUE;
					break;
				}
			}
			
			// load the palette in the DIB
			for (int i = (1 << bitspersample) - 1; i >= 0; i--) {
				if (Palette16Bits) {
					pal[i].rgbRed =(BYTE) CVT(red[i]);
					pal[i].rgbGreen = (BYTE) CVT(green[i]);
					pal[i].rgbBlue = (BYTE) CVT(blue[i]);           
				} else {
					pal[i].rgbRed = (BYTE) red[i];
					pal[i].rgbGreen = (BYTE) green[i];
					pal[i].rgbBlue = (BYTE) blue[i];        
				}
			}
			break;
		}
		m_lpImgData->SetPalette(clrUsed, pal);	//palette assign
		free(pal);

		// read the tiff lines and save them in the DIB
		uint32 nrow;
		uint32 ys;
		int line = CalculateLine(width, bitspersample * samplesperpixel);
		long bitsize= TIFFStripSize(m_tif);
		//verify bitsize: could be wrong if StripByteCounts is missing.
		if (bitsize > (long)(m_lpImgData->GetBitMapInfoHeader()->biSizeImage*samplesperpixel))
			bitsize = m_lpImgData->GetBitMapInfoHeader()->biSizeImage*samplesperpixel;

		int tiled_image = TIFFIsTiled(m_tif);
		uint32 tw, tl;
		BYTE* tilebuf;
		if (tiled_image)
		{
			TIFFGetField(m_tif, TIFFTAG_TILEWIDTH, &tw);
			TIFFGetField(m_tif, TIFFTAG_TILELENGTH, &tl);
			rowsperstrip = tl;
			bitsize = TIFFTileSize(m_tif) * (int)(1+width/tw);
			tilebuf = (BYTE*)malloc(TIFFTileSize(m_tif));
		}
		
		bits = (BYTE*)malloc(bitsize);
		
		for (ys = 0; ys < height; ys += rowsperstrip)
		{
			nrow = (ys + rowsperstrip > height ? height - ys : rowsperstrip);

			if (tiled_image)
			{
				uint32 imagew = TIFFScanlineSize(m_tif);
				uint32 tilew  = TIFFTileRowSize(m_tif);
				int iskew = imagew - tilew;
				uint8* bufp = (uint8*) bits;
				
				uint32 colb = 0;
				for (uint32 col = 0; col < width; col += tw) {
					if (TIFFReadTile(m_tif, tilebuf, col, ys, 0, 0) < 0){
						free(tilebuf);
						free(bits);
						// throw "Corrupted tiled TIFF file!";
						return false;
					}

					if (colb + tw > imagew)
					{
						uint32 owidth = imagew - colb;
						uint32 oskew = tilew - owidth;
						TileToStrip(bufp + colb, tilebuf, nrow, owidth, oskew + iskew, oskew );
					} else {
						TileToStrip(bufp + colb, tilebuf, nrow, tilew, iskew, 0);
					}
					colb += tilew;
				}
				
			} else {
				if (TIFFReadEncodedStrip(m_tif, TIFFComputeStrip(m_tif, ys, 0), bits, nrow * line) == -1) {
					free(bits);
					// throw "Corrupted TIFF file!";
					return false;
				}
			}
			
			for (y = 0; y < nrow; y++)
			{
				long offset=(nrow-y-1)*line;
				if (bitspersample==16) for (DWORD xi=0;xi<width;xi++) bits[xi+offset]=bits[xi*2+offset+1];
				if (samplesperpixel==1) { //simple 8bpp image
					LPBYTE	pbyImage = m_lpImgData->GetImgData();
					memcpy(pbyImage + m_lpImgData->GetRowWidthBytes() * (height - ys - nrow + y),
						bits + offset, m_lpImgData->GetRowWidthBytes());
				} else if (samplesperpixel==2) { //8bpp image with alpha layer
					int xi=0;
					int ii=0;
					int yi=height-ys-nrow+y;
					while (ii<line){
						m_lpImgData->SetPixelIndex(xi, yi, bits[ii*samplesperpixel+offset]);
// #if CXIMAGE_SUPPORT_ALPHA
// 						AlphaSet(xi,yi,bits[ii*samplesperpixel+offset+1]);
// #endif //CXIMAGE_SUPPORT_ALPHA
						ii++;
						xi++;
						if (xi>=(int)width){
							yi--;
							xi=0;
						}
					}
				} else { //photometric==PHOTOMETRIC_CIELAB
					if (bitcount!=24){ //fix image
						m_lpImgData->Create(width, height, 24, 0);
// #if CXIMAGE_SUPPORT_ALPHA
// 						if (samplesperpixel==4) AlphaCreate();
// #endif //CXIMAGE_SUPPORT_ALPHA
					}
					
					int xi=0;
					int ii=0;
					int yi=height-ys-nrow+y;
					RGBQUAD c;
					int l,a,b;
					double p,cx,cy,cz,cr,cg,cb;
					while (ii<line){
						l=bits[ii*samplesperpixel+offset];
						a=bits[ii*samplesperpixel+offset+1];
						b=bits[ii*samplesperpixel+offset+2];
						if (a>127) a-=256;
						if (b>127) b-=256;
						// lab to xyz
						p = (l/2.55 + 16) / 116.0;
						cx = pow( p + a * 0.002, 3);
						cy = pow( p, 3);
						cz = pow( p - b * 0.005, 3);
						// white point
						cx*=0.95047;
						//cy*=1.000;
						cz*=1.0883;
						// xyz to rgb
						cr =  3.240479 * cx - 1.537150 * cy - 0.498535 * cz;
						cg = -0.969256 * cx + 1.875992 * cy + 0.041556 * cz;
						cb =  0.055648 * cx - 0.204043 * cy + 1.057311 * cz;
						
						if ( cr > 0.00304 ) cr = 1.055 * pow(cr,0.41667) - 0.055;
						else            cr = 12.92 * cr;
						if ( cg > 0.00304 ) cg = 1.055 * pow(cg,0.41667) - 0.055;
						else            cg = 12.92 * cg;
						if ( cb > 0.00304 ) cb = 1.055 * pow(cb,0.41667) - 0.055;
						else            cb = 12.92 * cb;
						
						c.rgbRed  =(BYTE)max(0,min(255,(int)(cr*255)));
						c.rgbGreen=(BYTE)max(0,min(255,(int)(cg*255)));
						c.rgbBlue =(BYTE)max(0,min(255,(int)(cb*255)));
						
						m_lpImgData->SetPixelColor(xi, yi, c);
// #if CXIMAGE_SUPPORT_ALPHA
// 						AlphaSet(xi,yi,bits[ii*samplesperpixel+offset+3]);
// #endif //CXIMAGE_SUPPORT_ALPHA
						ii++;
						xi++;
						if (xi>=(int)width){
							yi--;
							xi=0;
						}
					}
				}
			}
		}
		free(bits);
		if (tiled_image)
			free(tilebuf);
// 
// 		switch(orientation)
// 		{
// 		case ORIENTATION_TOPRIGHT: /* row 0 top, col 0 rhs */
// 			Mirror();
// 			break;
// 		case ORIENTATION_BOTRIGHT: /* row 0 bottom, col 0 rhs */
// 			Flip();
// 			Mirror();
// 			break;
// 		case ORIENTATION_BOTLEFT: /* row 0 bottom, col 0 lhs */
// 			Flip();
// 			break;
// 		case ORIENTATION_LEFTTOP: /* row 0 lhs, col 0 top */
// 			RotateRight();
// 			Mirror();
// 			break;
// 		case ORIENTATION_RIGHTTOP: /* row 0 rhs, col 0 top */
// 			RotateLeft();
// 			break;
// 		case ORIENTATION_RIGHTBOT: /* row 0 rhs, col 0 bottom */
// 			RotateLeft();
// 			Mirror();
// 			break;
// 		case ORIENTATION_LEFTBOT: /* row 0 lhs, col 0 bottom */
// 			RotateRight();
// 			break;
// 		}
	}

	if (m_tif)
		TIFFClose(m_tif);
	return TRUE;
}

BOOL CImgTifFile::Save(LPCTSTR szFile)
{
	return false;
}

void CImgTifFile::TileToStrip(uint8* out, uint8* in,	uint32 rows, uint32 cols, int outskew, int inskew)
{
	while (rows-- > 0) {
		uint32 j = cols;
		while (j-- > 0)
			*out++ = *in++;
		out += outskew;
		in += inskew;
	}
}
