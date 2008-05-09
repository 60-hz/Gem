////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2002-2004 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM
//	zmoelnig@iem.kug.ac.at
//  For information on usage and redistribution, and for a DISCLAIMER
//  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
//
//  this file has been generated...
////////////////////////////////////////////////////////

#include "GEMglReportError.h"
#include "Base/GemGLUtil.h"


CPPEXTERN_NEW( GEMglReportError)

/////////////////////////////////////////////////////////
//
// GEMglReportError
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglReportError :: GEMglReportError	(void) {
  m_outlet = outlet_new(this->x_obj, &s_float);
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglReportError :: ~GEMglReportError () {
  outlet_free(m_outlet);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglReportError :: render(GemState *state) {
  GLenum err=glReportError();  
  outlet_float(m_outlet, (t_float)err);
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglReportError :: obj_setupCallback(t_class *classPtr) {
	 class_addanything(classPtr, GEMglReportError::bangMessCallback);

};
void GEMglReportError :: bangMessCallback (void* data){
	GetMyClass(data)->render(NULL);
}
