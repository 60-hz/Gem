 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2002 IOhannes m zmoelnig. forum::für::umläute. IEM
  *	zmoelnig@iem.kug.ac.at
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  *  this file has been generated...
  * ------------------------------------------------------------------
  */

#ifndef INCLUDE_GEM_GLCLEARINDEX_H_
#define INCLUDE_GEM_GLCLEARINDEX_H_

#include "GemGLBase.h"

/*
 CLASS
	GEMglClearIndex
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glClearIndex( GLfloat c)"
 */

class GEM_EXTERN GEMglClearIndex : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglClearIndex, GemGLBase);

	public:
	  // Constructor
	  GEMglClearIndex (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglClearIndex ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLfloat	c;		// VAR
	  virtual void	cMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 cMessCallback (void*, t_floatarg);
};
#endif // for header file
