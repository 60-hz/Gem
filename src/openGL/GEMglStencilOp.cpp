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

#include "GEMglStencilOp.h"

CPPEXTERN_NEW_WITH_GIMME ( GEMglStencilOp )

/////////////////////////////////////////////////////////
//
// GEMglViewport
//
/////////////////////////////////////////////////////////
// Constructor
//
GEMglStencilOp :: GEMglStencilOp	(int argc, t_atom*argv) :
		fail(0), 
		zfail(0), 
		zpass(0)
{
  if(3==argc){failMess(argv[0]); zfailMess(argv[1]); zpassMess(argv[2]);}else if(argc) GemException("invalid number of arguments");

	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("fail"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("zfail"));
	m_inlet[2] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("zpass"));
}
/////////////////////////////////////////////////////////
// Destructor
//
GEMglStencilOp :: ~GEMglStencilOp () {
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
inlet_free(m_inlet[2]);
}

/////////////////////////////////////////////////////////
// Render
//
void GEMglStencilOp :: render(GemState *state) {
	glStencilOp (fail, zfail, zpass);
}

/////////////////////////////////////////////////////////
// Variables
//
void GEMglStencilOp :: failMess (t_atom arg) {	// FUN
	fail = (GLenum)getGLdefine(&arg);
	setModified();
}

void GEMglStencilOp :: zfailMess (t_atom arg) {	// FUN
	zfail = (GLenum)getGLdefine(&arg);
	setModified();
}

void GEMglStencilOp :: zpassMess (t_atom arg) {	// FUN
	zpass = (GLenum)getGLdefine(&arg);
	setModified();
}


/////////////////////////////////////////////////////////
// static member functions
//

void GEMglStencilOp :: obj_setupCallback(t_class *classPtr) {
	 class_addmethod(classPtr, (t_method)&GEMglStencilOp::failMessCallback,  	gensym("fail"), A_GIMME, A_NULL);
	 class_addmethod(classPtr, (t_method)&GEMglStencilOp::zfailMessCallback,  	gensym("zfail"), A_GIMME, A_NULL);
	 class_addmethod(classPtr, (t_method)&GEMglStencilOp::zpassMessCallback,  	gensym("zpass"), A_GIMME, A_NULL);
};

void GEMglStencilOp :: failMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	if(argc==1)GetMyClass(data)->failMess ( argv[0]);
}
void GEMglStencilOp :: zfailMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	if(argc==1)GetMyClass(data)->zfailMess ( argv[0]);
}
void GEMglStencilOp :: zpassMessCallback (void* data, t_symbol*, int argc, t_atom*argv){
	if(argc==1)GetMyClass(data)->zpassMess ( argv[0]);
}
