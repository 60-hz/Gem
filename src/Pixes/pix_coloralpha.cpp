////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) G�nther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_coloralpha.h"

CPPEXTERN_NEW(pix_coloralpha)

/////////////////////////////////////////////////////////
//
// pix_coloralpha
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_coloralpha :: pix_coloralpha()
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_coloralpha :: ~pix_coloralpha()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_coloralpha :: processImage(imageStruct &image)
{
    // process the image
    int datasize = image.xsize * image.ysize;
    unsigned char *pixel = image.data;

    while(datasize--)
    {
		float tempVal = (float)pixel[chRed] + (float)pixel[chGreen] + (float)pixel[chBlue];
		tempVal /= 3.f;
		pixel[chAlpha] = (unsigned char)tempVal;
		pixel += 4;
    }    
}

void pix_coloralpha :: processYUVImage(imageStruct &image)
{
    post("pix_coloralpha: YUV not yet implemented :-(");
}
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_coloralpha :: obj_setupCallback(t_class *)
{ }
