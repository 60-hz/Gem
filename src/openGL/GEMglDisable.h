 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2002 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM
  *	zmoelnig@iem.kug.ac.at
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  *  this file has been generated...
  * ------------------------------------------------------------------
  */

#ifndef INCLUDE_GEM_GLDISABLE_H_
#define INCLUDE_GEM_GLDISABLE_H_

#include "GemGLBase.h"

/*
 CLASS
	GEMglDisable
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glDisable( GLenum cap)"
 */

class GEM_EXTERN GEMglDisable : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglDisable, GemGLBase)

	public:
	  // Constructor
	  GEMglDisable (int, t_atom*); // CON

	protected:
	  // Destructor
	  virtual ~GEMglDisable ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	cap;		// VAR
	  virtual void	capMess(t_atom);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 capMessCallback (void*,t_symbol*,int,t_atom*);
};
#endif // for header file
