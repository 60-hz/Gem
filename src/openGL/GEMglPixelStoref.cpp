////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2002 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM
//    this file has been generated automatically
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "GEMglPixelStoref.h"

CPPEXTERN_NEW_WITH_TWO_ARGS (GEMglPixelStoref , t_symbol*, A_DEFSYMBOL, t_floatarg, A_DEFFLOAT)

/////////////////////////////////////////////////////////
//
// GEMglPixelStoref
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
GEMglPixelStoref :: GEMglPixelStoref(t_symbol* arg1=0, t_floatarg arg2=0) :
             		pname((GLenum)arg1),
		param((GLfloat)arg2)
{
	m_inlet[0] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("pname"));
	m_inlet[1] = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("param"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
GEMglPixelStoref :: ~GEMglPixelStoref(){
inlet_free(m_inlet[0]);
inlet_free(m_inlet[1]);
}
/////////////////////////////////////////////////////////
// Render
//
/////////////////////////////////////////////////////////
void GEMglPixelStoref :: render(GemState *state)
{ glPixelStoref(pname, param); }


/////////////////////////////////////////////////////////
// set my variables
/////////////////////////////////////////////////////////

void GEMglPixelStoref :: pnameMess (int arg1) {
	pname = (GLenum)arg1;
	setModified();
}


void GEMglPixelStoref :: paramMess (t_float arg1) {
	param = (GLfloat)arg1;
	setModified();
}



/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////

void GEMglPixelStoref :: obj_setupCallback(t_class *classPtr) {
        class_addcreator((t_newmethod)_classGEMglPixelStoref,gensym("glPixelStoref"),A_NULL);

	class_addmethod(classPtr, (t_method)&GEMglPixelStoref::pnameMessCallback, gensym("pname"), A_NULL);
	class_addmethod(classPtr, (t_method)&GEMglPixelStoref::paramMessCallback, gensym("param"), A_NULL);
}


void GEMglPixelStoref :: pnameMessCallback (   void* data, t_symbol*    arg0) {
	GetMyClass(data)->pnameMess (getGLdefine(arg0->s_name));
}
void GEMglPixelStoref :: paramMessCallback (   void* data, t_floatarg    arg0) {
	GetMyClass(data)->paramMess ( (t_int)    arg0);
}
