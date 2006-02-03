/* -----------------------------------------------------------------

GEM - Graphics Environment for Multimedia

Load an digital video (like AVI, Mpeg, Quicktime) into a pix block 
(OS independant parent-class)

Copyright (c) 1997-1999 Mark Danks. mark@danks.org
Copyright (c) G�nther Geiger. geiger@epy.co.at
Copyright (c) 2001-2003 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM. zmoelnig@iem.kug.ac.at
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/
   
#ifndef INCLUDE_PIXES_RECORDQT4L_H_
#define INCLUDE_PIXES_RECORDQT4L_H_
   
#include "Pixes/record.h"
   
#include <quicktime.h>
#include <colormodels.h>
   
/*---------------------------------------------------------------
 -------------------------------------------------------------------
  CLASS
  recordQT4L
    
  class for recording video-streams into a qt4l-movie
    
  KEYWORDS
  pix record movie
  
  DESCRIPTION
  
  -----------------------------------------------------------------*/
class GEM_EXTERN recordQT4L : public record {
 public:
  
  //////////
  // Constructor
  
  /* initialize the recordQT4Lloader
   *
   * set the default colour-space to format (like GL_RGBA)
   * if format==0, the default is set by the recordQT4Lloader
   * (for instance: the fastest colour-space)
   */
  recordQT4L(int x, int y, int width, int height);

  ////////
  // Destructor
  /* free what is apropriate */
  virtual ~recordQT4L();

#ifdef HAVE_LIBQUICKTIME
  //////////
  // open a movie up
  /* open the recordQT4L "filename" (think better about URIs ?)
   */
  /* returns TRUE if opening was successfull, FALSE otherwise */
  virtual bool open(char *filename);
  //////////
  // close the movie file
  /* stop recording, close the file and clean up temporary things */
  virtual void close(void);


  //////////
  // compress and write the next frame
  /* this is the core-function of this class !!!!
   * when called it returns something depending on success
   * (what? the framenumber and -1 (0?) on failure?)
   */
  virtual int putFrame(imageStruct*);

  virtual bool setCodec(char*name);
  virtual bool setCodec(int  num);

  virtual int getNumCodecs();
  virtual char*getCodecName(int n);
  virtual char*getCodecDescription(int n);

 private:
  quicktime_t *m_qtfile;
  char        m_compressor[8];
  double      m_fps;
  int          m_jpeg_quality;

  imageStruct m_image;
#else
  virtual int putFrame(imageStruct*){return (-1);}
#endif
};

#endif	// for header file
