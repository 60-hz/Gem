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

#ifndef INCLUDE_GEM_GLGETINTEGERV_H_
#define INCLUDE_GEM_GLGETINTEGERV_H_

#include "GemGLBase.h"

/*
  CLASS
  GEMglGetIntegerv
  KEYWORDS
  openGL	0
  DESCRIPTION
  wrapper for the openGL-function
  "glGetIntegerv( GLenum pname, GLint *params)"
*/

class GEM_EXTERN GEMglGetIntegerv : public GemGLBase
{
  CPPEXTERN_HEADER(GEMglGetIntegerv, GemGLBase)

    public:
  // Constructor
  GEMglGetIntegerv (int,t_atom*);	// CON
 protected:
  // Destructor
  virtual ~GEMglGetIntegerv ();
  // check extensions
  virtual bool isRunnable(void);

  // Do the rendering
  virtual void	render (GemState *state);
  // variable
  GLenum pname;
  virtual void	pnameMess(t_atom);	// FUN

  t_atom m_alist[16];

 private:
  // we need one inlet
  t_inlet *m_inlet;
  // The outlet
  t_outlet    	*m_outlet;

  // static member functions
  static void    pnameMessCallback (void*,t_symbol*,int,t_atom*);
};
#endif // for header file
