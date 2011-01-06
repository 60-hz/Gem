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
   
#ifndef INCLUDE_PLUGINS_RECORDV4L2_H_
#define INCLUDE_PLUGINS_RECORDV4L2_H_
   
#include "plugins/record.h"
   
#if defined HAVE_LINUX_VIDEODEV2_H
# define HAVE_VIDEO4LINUX2
# include <linux/videodev2.h>
#endif
 
/*---------------------------------------------------------------
 -------------------------------------------------------------------
  CLASS
  recordV4L2
    
  class for outputting video using a v4l2 loopback device
    
  KEYWORDS
  pix record movie
  
  DESCRIPTION
  
  -----------------------------------------------------------------*/
namespace gem { class GEM_EXPORT recordV4L2 : public record {
 public:
  
  //////////
  // Constructor
  
  recordV4L2();

  ////////
  // Destructor
  /* free what is apropriate */
  virtual ~recordV4L2();

#if defined HAVE_VIDEO4LINUX2

  //////////
  // close the movie file
  // stop recording, close the file and clean up temporary things
  virtual void close(void);

  //////////
  // open a movie up
  // open the recordV4L2 "filename" (think better about URIs ?)
  // returns TRUE if opening was successfull, FALSE otherwise 
  virtual bool open(const std::string filename);

  
  //////////
  // initialize the encoder
  // dummyImage provides meta-information (e.g. size) that must not change during the encoding cycle
  // (if it does, abort the recording session)
  // framedur is the duration of one frame in [ms]
  //   
  // returns TRUE if init was successfull, FALSE otherwise 
  virtual bool init(const imageStruct* dummyImage, const int framedur);

  //////////
  // compress and write the next frame
  /* this is the core-function of this class !!!!
   * when called it returns something depending on success
   * (what? the framenumber and -1 (0?) on failure?)
   */
  virtual bool putFrame(imageStruct*);

  virtual bool setCodec(const std::string name);

  /**
   * get a list of supported codecs (short-form names, e.g. "mjpa")
   */ 
  virtual std::vector<std::string>getCodecs(void);

 private:

  int m_fd;
  imageStruct m_image;
  bool m_init;
  int m_palette;

#endif /* V4L2 */
}; };

#endif	// for header file
