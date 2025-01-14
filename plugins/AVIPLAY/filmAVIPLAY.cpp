////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_LIBAVIPLAY

#include "filmAVIPLAY.h"
#include "plugins/PluginFactory.h"
#include "Gem/Properties.h"
using namespace gem::plugins;

REGISTER_FILMFACTORY("aviplay", filmAVIPLAY);

#include <unistd.h>
#include <time.h>

/////////////////////////////////////////////////////////
//
// filmAVIPLAY
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

filmAVIPLAY :: filmAVIPLAY(void) :
  m_wantedFormat(GEM_RGBA),
  m_fps(-1.0),
  m_numFrames(-1), m_numTracks(-1),
  m_curFrame(-1), m_curTrack(-1),
  m_readNext(false), m_newfilm(false),
  m_avifile(NULL),
  m_avistream(NULL),
  m_aviimage(NULL),
  m_rawdata(NULL),
  m_rawlength(0)
{
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
filmAVIPLAY :: ~filmAVIPLAY(void)
{
  close();
  if(m_rawdata) {
    delete[]m_rawdata;
  }
}
void filmAVIPLAY :: close(void)
{
  if(m_aviimage) {
    m_aviimage->Release();
  }
  m_aviimage = 0;

  if (m_avistream) {
    m_avistream->StopStreaming();
  }
  m_avistream = 0;

  if (m_avifile) {
    delete m_avifile;
  }
  m_avifile = 0;
}

/////////////////////////////////////////////////////////
// open the file
//
/////////////////////////////////////////////////////////
bool filmAVIPLAY :: open(const std::string&filename,
                         const gem::Properties&wantProps)
{
  double d;
  if(wantProps.get("colorspace", d) && d>0) {
    m_wantedFormat=d;
  }
  // how do we close the avifile ??? automagically ?
  if (!(m_avifile = CreateIAviReadFile(filename.c_str()))) {
    goto unsupported;
  }
  while(!m_avifile->IsOpened()) {
    struct timeval sleep;
    sleep.tv_sec=0;
    sleep.tv_usec=500;/*500us*/
    select(0,0,0,0,&sleep);
  }
  if (!m_avifile->IsValid()) {
    goto unsupported;
  }
  m_numTracks = m_avifile->VideoStreamCount();
  if (m_numTracks<1) {
    return false;
  }
  if (m_curTrack>=m_numTracks) {
    m_curTrack = 0;
  }
  try {
    int curtrack = m_curTrack;
    if(curtrack<0) {
      curtrack = 0;
    }
    m_avistream=m_avifile->GetStream(curtrack,AviStream::Video);
  } catch (const char* string) {
    m_avistream = 0;
  }
  if (!m_avistream) {
    goto unsupported;
  }
  if (m_avistream->StartStreaming()==-1) {
    goto unsupported;
  }
  m_numFrames = m_avistream->GetLength();
  m_curFrame = -1;
  if (1) {
    avm::StreamInfo *l_info = m_avistream->GetStreamInfo();
    m_image.image.xsize = l_info->GetVideoWidth();
    m_image.image.ysize = l_info->GetVideoHeight();
    m_fps= l_info->GetFps();
  }
  m_image.image.setCsizeByFormat(m_wantedFormat);
  if (!(m_image.image.xsize*m_image.image.ysize*m_image.image.csize)) {
    goto unsupported;
  }
  m_readNext=true;
  m_newfilm = true;
  return true;

unsupported:
  close();
  return false;
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
pixBlock* filmAVIPLAY :: getFrame()
{
  if (!m_avistream) {
    return 0;
  }
  if (!m_readNext) {
    return &m_image;
  }
  if(m_aviimage) {
    m_aviimage->Release();
    m_aviimage = 0;
  }
  /* for MPEGs ReadFrame() will return 0 only when errors occur
   * other formats return 0 all the time (and -1 on file end)
   */
  m_aviimage = m_avistream->GetFrame(
                 true); // this might crash sometimes...
  if (m_aviimage) {
    int format = m_aviimage->Format();
    m_rawdata=m_aviimage->Data();
    m_image.image.setCsizeByFormat(m_wantedFormat);
    switch(format) {
    case IMG_FMT_RGB24:
      m_image.image.fromRGB (m_rawdata);
      break;
    case IMG_FMT_RGB32:
      m_image.image.fromRGBA(m_rawdata);
      break;
    case IMG_FMT_BGR24:
      m_image.image.fromBGR (m_rawdata);
      break;
    case IMG_FMT_BGR32:
      m_image.image.fromBGRA(m_rawdata);
      break;
    case IMG_FMT_Y800 :
    case IMG_FMT_Y8   :
      m_image.image.fromGray(m_rawdata);
      break;
    case IMG_FMT_UYVY :
      m_image.image.fromUYVY(m_rawdata);
      break;
    case IMG_FMT_YUY2 :
      m_image.image.fromYUY2(m_rawdata);
      break;
    case IMG_FMT_YVYU :
      m_image.image.fromYVYU(m_rawdata);
      break;
    case IMG_FMT_YV12 :
      m_image.image.fromYV12(m_rawdata);
      break;
    case IMG_FMT_BGR16: // it seems like this was RGB16
    default:
      m_image.image.fromRGB16(m_rawdata);
      break;
    }
    m_image.newimage=1;
    if (m_newfilm) {
      m_image.newfilm=1;
    }
    m_newfilm=false;
    m_image.image.upsidedown=true;
    m_readNext=false;
    return &m_image;
  }
  return 0;
}

film::errCode filmAVIPLAY :: changeImage(int imgNum, int trackNum)
{
  if (!m_avistream) {
    return film::FAILURE;
  }
  m_avistream->Seek(imgNum);
  m_readNext=true;
  return film::SUCCESS;
}


///////////////////////////////
// Properties
bool filmAVIPLAY::enumProperties(gem::Properties&readable,
                                 gem::Properties&writeable)
{
  readable.clear();
  writeable.clear();

  gem::any value;
  value=0.;
  readable.set("fps", value);
  readable.set("frames", value);
  readable.set("width", value);
  readable.set("height", value);

  return false;
}

void filmAVIPLAY::setProperties(gem::Properties&props)
{
}

void filmAVIPLAY::getProperties(gem::Properties&props)
{
  std::vector<std::string> keys=props.keys();
  gem::any value;
  double d;
  unsigned int i=0;
  for(i=0; i<keys.size(); i++) {
    std::string key=keys[i];
    props.erase(key);
    if("fps"==key) {
      d=m_fps;
      value=d;
      props.set(key, value);
    }
    if("frames"==key && m_numFrames>=0) {
      d=m_numFrames;
      value=d;
      props.set(key, value);
    }
    if("tracks"==key && m_numTracks>=0) {
      d=m_numTracks;
      value=d;
      props.set(key, value);
    }
    if("width"==key) {
      d=m_image.image.xsize;
      value=d;
      props.set(key, value);
    }
    if("height"==key) {
      d=m_image.image.ysize;
      value=d;
      props.set(key, value);
    }
  }
}
#endif // AVIPLAY
