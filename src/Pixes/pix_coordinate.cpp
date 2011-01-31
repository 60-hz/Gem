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
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_coordinate.h"

#include "Base/GemState.h"
#include "Base/GemPixUtil.h"

CPPEXTERN_NEW(pix_coordinate)

/////////////////////////////////////////////////////////
//
// pix_coordinate
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_coordinate :: pix_coordinate()
  : m_coords(NULL), m_rectcoords(NULL), m_numCoords(0),
    m_oldTexCoords(NULL), m_oldNumCoords(0)
{
    // the size inlet
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("coords"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_coordinate :: ~pix_coordinate()
{
    if(m_coords)delete [] m_coords;
    if(m_rectcoords)delete [] m_rectcoords;
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_coordinate :: render(GemState *state)
{
  pixBlock*img=NULL;
  state->get("pix", img);

  state->get("gl.tex.coords", m_oldTexCoords);
  state->get("gl.tex.numcoords", m_oldNumCoords);
  state->set("gl.tex.type", m_oldTexType);

  if (m_oldTexType && m_numCoords){
    state->set("gl.tex.numcoords", m_numCoords);

	if(m_oldTexType==2 && img!=NULL){ 
	  // since we are using rectangle-textures (state->texture==2), 
	  // we want to scale the coordinates by the image-dimensions if they are available
	  t_float xsize = (t_float)img->image.xsize;
	  t_float ysize = (t_float)img->image.ysize;

	  for (int i = 0; i <  m_numCoords; i++)
	    {
	      m_rectcoords[i].s = xsize*m_coords[i].s;
	      m_rectcoords[i].t = ysize*m_coords[i].t;
	    }
    state->set("gl.tex.coords", m_rectcoords);

	} else
    state->set("gl.tex.coords", m_coords);
  }
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_coordinate :: postrender(GemState *state)
{
  state->set("gl.tex.coords", m_oldTexCoords);
  state->set("gl.tex.numcoords", m_oldNumCoords);
  state->set("gl.tex.type", m_oldTexType);
}

/////////////////////////////////////////////////////////
// coordsMess
//
/////////////////////////////////////////////////////////
void pix_coordinate :: coordsMess(int argc, t_atom *argv)
{
    if (argc % 2)
    {
    	error("must have a even number of coordinates");
    	return;
    }
    
    int numVals = argc / 2;
    
    if (numVals != m_numCoords)
    {
      if(m_coords)delete [] m_coords;	    m_coords = NULL;
      if(m_rectcoords)delete[]m_rectcoords; m_rectcoords=NULL;
      m_numCoords = numVals;

      if (m_numCoords == 0)return;
      m_coords = new TexCoord[m_numCoords];
      m_rectcoords=new TexCoord[m_numCoords];
    }
    
    for (int i = 0; i < numVals; i++)
    {
    	m_coords[i].s = atom_getfloat(&argv[2 * i]);
    	m_coords[i].t = atom_getfloat(&argv[2 * i + 1]);
    }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_coordinate :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_coordinate::coordsMessCallback),
    	    gensym("coords"), A_GIMME, A_NULL);
}
void pix_coordinate :: coordsMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    GetMyClass(data)->coordsMess(argc, argv);
}


