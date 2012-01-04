/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  read in a model file

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  Copyright (c) Günther Geiger. geiger@epy.co.at
  Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_MODEL_H_
#define _INCLUDE__GEM_GEOS_MODEL_H_

#include "Base/GemBase.h"
#include "model_loader.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  model

  read in a model file

  DESCRIPTION

  Inlet for a list - "model"

  "open" - the RGB model to set the object to

  -----------------------------------------------------------------*/
class GEM_EXTERN model : public GemBase
{
  CPPEXTERN_HEADER(model, GemBase);

 public:

  //////////
  // Constructor
  model(t_symbol *filename);

 protected:

  //////////
  // Destructor
  virtual ~model(void);

  //////////
  // When an open is received
  virtual void	openMess(const std::string&filename);

  //////////
  // When a rescale is received
  virtual void	rescaleMess(bool state);
  //////////
  // When a reverse is received
  virtual void	reverseMess(int state);
  //////////
  // Which texture type (linear, spheric)
  virtual void	textureMess(int state);

  //////////
  // Set smoothing factor
  virtual void	smoothMess(t_float fsmooth);

  //////////
  // Set material mode
  virtual void    materialMess(int material);

  //////////
  // Set material mode
  virtual void    groupMess(int group);

  //////////
  virtual void	cleanModel(void);
  //////////
  virtual void    buildList(void);

  //////////
  virtual void	render(GemState *state);
  //////////
  virtual void	startRendering(void);

  //////////
  GLMmodel *m_model;
  GLint	  	m_dispList;

	//////////
	// Should we rescale the model when loaded
	// Default is yes
	bool		m_rescaleModel;

	GLfloat m_smooth;
	int     m_material;

	int     m_flags;

  int     m_group;

  bool    m_rebuild;
  float		m_currentH, m_currentW;

  //////////////
  // how to texture
  glmtexture_t m_textype;
};

#endif	// for header file
