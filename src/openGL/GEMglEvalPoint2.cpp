////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// Implementation file
//
// Copyright (c) 2002 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM
//	zmoelnig@iem.kug.ac.at
//  For information on usage and redistribution, and for a DISCLAIMER
//  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
//
//  this file has been generated...
////////////////////////////////////////////////////////

#include "GEMglEvalPoint2.h"

CPPEXTERN_NEW_WITH_TWO_ARGS ( GEMglEvalPoint2 , t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT)

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglEvalPoint2 :: GEMglEvalPoint2	(t_floatarg arg0=0, t_floatarg arg1=0) :
		i((GLint)arg0), 
		j((GLint)arg1)
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("i"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("j"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglEvalPoint2 :: ~GEMglEvalPoint2 () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}
//////////////////
// extension check
bool GEMglEvalPoint2 :: isRunnable(void) {
  if(GLEW_VERSION_1_1)return true;
  error("your system does not support OpenGL-1.1");
  return false;
}
/////////////////////////////////////////////////////////
// Render
//
void GEMglEvalPoint2 :: render(GemState *state) {
	glEvalPoint2 (i, j);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglEvalPoint2 :: iMess (t_float arg1) {	// FUN
	i = (GLint)arg1;
	setModified();
}

void GEMglEvalPoint2 :: jMess (t_float arg1) {	// FUN
	j = (GLint)arg1;
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglEvalPoint2 :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, (t_method)&GEMglEvalPoint2::iMessCallback,  	gensym("i"), A_DEFFLOAT, A_NULL);
	 class_addmethod(classPtr, (t_method)&GEMglEvalPoint2::jMessCallback,  	gensym("j"), A_DEFFLOAT, A_NULL);
}

void GEMglEvalPoint2 :: iMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->iMess ( (t_float)    arg0);
}
void GEMglEvalPoint2 :: jMessCallback (void* data, t_floatarg arg0){
	GetMyClass(data)->jMess ( (t_float)    arg0);
}
