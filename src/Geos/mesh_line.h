/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A mesh_line

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) G�nther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM. zmoelnig@iem.kug.ac.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef INCLUDE_mesh_line_H_
#define INCLUDE_mesh_line_H_

#include "Base/GemShape.h"


#define MAXGRID 1000

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    mesh_line
    
    Creates a mesh_line

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN mesh_line : public GemShape
{
    CPPEXTERN_HEADER(mesh_line, GemShape)

    public:

        //////////
        // Constructor
    	mesh_line(t_floatarg sizeX);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~mesh_line();
        

    	//////////
    	// Do the rendering
    	virtual void 	renderShape(GemState *state);
      static void 	gridMessCallback(void *data, t_floatarg size);


		//////////
		// getStuff
		int 		gridX;
		float		xsize, xsize0;
		int 		alreadyInit;
		void		setGrid( int valueX);
		void		getTexCoords(void);
		float 		texCoords[MAXGRID][MAXGRID][2];
};

#endif	// for header file
