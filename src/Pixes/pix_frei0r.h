/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    apply a Frei0r effect to an image

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM. zmoelnig@iem.kug.ac.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef INCLUDE_PIX_FREI0R_H_
#define INCLUDE_PIX_FREI0R_H_

#include "Base/GemPixObj.h"

#ifndef DONT_WANT_FREI0R

#include "frei0r.h"

#endif /* DONT_WANT_FREI0R */



/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_frei0r
    
    load a Frei0r-plugin and use it do process the image-data

KEYWORDS
    pix
    
DESCRIPTION
   
-----------------------------------------------------------------*/

class GEM_EXTERN pix_frei0r : public GemPixObj
{
    CPPEXTERN_HEADER(pix_frei0r, GemPixObj)

    public:

    //////////
    // Constructor
    pix_frei0r(t_symbol*s);
    	
 protected:
    	
    //////////
    // Destructor
    virtual ~pix_frei0r();

    //////////
    // Do the processing
    void 	processRGBAImage(imageStruct &image);

    imageStruct m_image;
    class F0RPlugin;
    F0RPlugin*m_plugin;

    bool m_canopen;
    std::vector<t_inlet*>m_inlet;

    void parmMess(int param, t_atom*value);
    void parmMess(std::string param, t_atom*value);

    void openMess(t_symbol*name);
    void closeMess(void);

 private:
    static void parmCallback(void *data, t_symbol*s, int argc, t_atom*argv);
    static void openCallback(void *data, t_symbol*s);

 public:
    static bool loader(t_canvas *canvas, const std::string classname);
};

#endif	// for header file
