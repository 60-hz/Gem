////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.at
//
// Implementation file
//
//    Copyright (c) 2015 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "videoDECKLINK.h"
#include "plugins/PluginFactory.h"
#include "Gem/RTE.h"
#include "Gem/Exception.h"

#include <stdio.h>
#define MARK() printf("%s:%d\t%s\n", __FILE__, __LINE__, __FUNCTION__)

/* -LICENSE-START-
** Copyright (c) 2013 Blackmagic Design
**
** Permission is hereby granted, free of charge, to any person or organization
** obtaining a copy of the software and accompanying documentation covered by
** this license (the "Software") to use, reproduce, display, distribute,
** execute, and transmit the Software, and to prepare derivative works of the
** Software, and to permit third-parties to whom the Software is furnished to
** do so, all subject to the following:
**
** The copyright notices in the Software and this entire statement, including
** the above license grant, this restriction and the following disclaimer,
** must be included in all copies of the Software, in whole or in part, and
** all derivative works of the Software, unless such copies or derivative
** works are solely in the form of machine-executable object code generated by
** a source language processor.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
** SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
** FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
** ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
** -LICENSE-END-
*/
class DeckLinkCaptureDelegate : public IDeckLinkInputCallback
{
public:
  ULONG m_refCount;
  pthread_mutex_t m_mutex;

  unsigned long m_frameCount;
  BMDVideoInputFlags m_cfg_inputFlags;
  BMDPixelFormat m_cfg_pixelFormat;
  IDeckLinkInput*m_deckLinkInput;

  gem::plugins::videoDECKLINK*m_priv;

public:
  DeckLinkCaptureDelegate(gem::plugins::videoDECKLINK*parent,
                          IDeckLinkInput*dli)
    : IDeckLinkInputCallback()
    , m_refCount(0)
    , m_frameCount(0)
    , m_cfg_inputFlags(bmdVideoInputFlagDefault)
    , m_cfg_pixelFormat(bmdFormat8BitYUV)
    , m_deckLinkInput(dli)
    , m_priv(parent)
  {
    m_deckLinkInput->AddRef();
    pthread_mutex_init(&m_mutex, NULL);
    m_deckLinkInput->SetCallback(this);
    //result = m_deckLinkInput->EnableVideoInput(displayMode->GetDisplayMode(), m_cfg_pixelFormat, m_cfg_inputFlags);
    //m_deckLinkInput->DisableAudioInput();

  }

  ~DeckLinkCaptureDelegate()
  {
    m_deckLinkInput->Release();
    pthread_mutex_destroy(&m_mutex);
  }
  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv)
  {
    return E_NOINTERFACE;
  }
  virtual ULONG STDMETHODCALLTYPE AddRef(void)
  {
    pthread_mutex_lock(&m_mutex);
    m_refCount++;
    pthread_mutex_unlock(&m_mutex);

    return (ULONG)m_refCount;
  }
  virtual ULONG STDMETHODCALLTYPE  Release(void)
  {
    pthread_mutex_lock(&m_mutex);
    m_refCount--;
    pthread_mutex_unlock(&m_mutex);

    if (m_refCount == 0) {
      delete this;
      return 0;
    }

    return (ULONG)m_refCount;
  }
  virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(
    IDeckLinkVideoInputFrame*videoFrame, IDeckLinkAudioInputPacket*audioFrame)
  {
    IDeckLinkVideoFrame*rightEyeFrame = NULL;
    IDeckLinkVideoFrame3DExtensions*    threeDExtensions = NULL;
    void*frameBytes;
    void*audioFrameBytes;

    // Handle Video Frame
    if (videoFrame) {
#if 0
      // If 3D mode is enabled we retreive the 3D extensions interface which gives.
      // us access to the right eye frame by calling GetFrameForRightEye() .
      if ( (videoFrame->QueryInterface(IID_IDeckLinkVideoFrame3DExtensions,
                                       (void **) &threeDExtensions) != S_OK) ||
           (threeDExtensions->GetFrameForRightEye(&rightEyeFrame) != S_OK)) {
        rightEyeFrame = NULL;
      }
#endif
      if (threeDExtensions) {
        threeDExtensions->Release();
      }

      if (videoFrame->GetFlags() & bmdFrameHasNoInputSource) {
      } else {
        long w=videoFrame->GetWidth();
        long h=videoFrame->GetHeight();
        videoFrame->GetBytes(&frameBytes);
        m_priv->setFrame(videoFrame->GetWidth(),
                         videoFrame->GetHeight(),
                         GL_YUV422_GEM,
                         (unsigned char*)frameBytes);
      }

      if (rightEyeFrame) {
        rightEyeFrame->Release();
      }

      m_frameCount++;
    }

    return S_OK;
  }

  virtual HRESULT STDMETHODCALLTYPE
  VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events,
                          IDeckLinkDisplayMode*mode,
                          BMDDetectedVideoInputFormatFlags)
  {
    // This only gets called if bmdVideoInputEnableFormatDetection was set
    // when enabling video input
    HRESULT     result;
    char*       displayModeName = NULL;

    if (!(events & bmdVideoInputDisplayModeChanged)) {
      return S_OK;
    }

    mode->GetName((const char**)&displayModeName);

    if (displayModeName) {
      free(displayModeName);
    }

    if (m_deckLinkInput) {
      m_deckLinkInput->StopStreams();

      result = m_deckLinkInput->EnableVideoInput(mode->GetDisplayMode(),
               m_cfg_pixelFormat, m_cfg_inputFlags);
      if (result != S_OK) {
        goto bail;
      }

      m_deckLinkInput->StartStreams();
    }

bail:
    return S_OK;
  }
};
/*
** END of Blackmagic licensed code
*/


namespace
{
IDeckLinkDisplayMode*getDisplayMode(IDeckLinkInput*dli,
                                    const std::string&formatname, int formatnum)
{
  IDeckLinkDisplayModeIterator*dmi = NULL;
  IDeckLinkDisplayMode*displayMode = NULL;
  int count=formatnum;
  if(S_OK == dli->GetDisplayModeIterator(&dmi)) {
    while(S_OK == dmi->Next(&displayMode)) {
      if (formatnum<0 && formatname.empty()) {
        // we don't care for the format; accept the first one
        break;
      }

      // if we have set the format name, check that
      if(!formatname.empty()) {
        const char*dmn = NULL;
        if (S_OK == displayMode->GetName(&dmn)) {
          bool found=(formatname == dmn);
          verbose(1, "[GEM:videoDECKLINK] checking format '%s'", dmn);
          free((void*)dmn);
          if(found) {
            break;
          }
        }
      }
      // else check the format index
      if(formatnum>=0 && 0 == count) {
        break;
      }
      --count;

      displayMode->Release();
      displayMode=NULL;
    }
    dmi->Release();
  }
  return displayMode;
}
};




using namespace gem::plugins;

REGISTER_VIDEOFACTORY("decklink", videoDECKLINK);

videoDECKLINK::videoDECKLINK(void)
  : video()
  , m_name(std::string("decklink"))
  , m_devname   (std::string("")), m_devnum   (-1)
  , m_formatname(std::string("")), m_formatnum(-1)
  , m_dlIterator(NULL)
  , m_dl(NULL)
  , m_dlInput(NULL)
  , m_displayMode(NULL)
  , m_dlConfig(NULL)
  , m_connectionType(bmdVideoConnectionSDI)
  , m_dlCallback(NULL)
{
  IDeckLinkIterator*dli = CreateDeckLinkIteratorInstance();
  if(!dli) {
    throw(GemException("DeckLink: unable to initialize Framework"));
  }
  dli->Release();

  m_pixBlock.image.xsize = 64;
  m_pixBlock.image.ysize = 64;
  m_pixBlock.image.setCsizeByFormat(GL_RGBA);
  m_pixBlock.image.reallocate();
}

videoDECKLINK::~videoDECKLINK(void)
{
  close();
}

void videoDECKLINK::close(void)
{
  stop();
  if(m_displayMode) {
    m_displayMode->Release();
    m_displayMode=0;
  }
  if(m_dlConfig) {
    m_dlConfig->Release();
    m_dlConfig=0;
  }

  if(m_dlInput) {
    m_dlInput->DisableAudioInput();
    m_dlInput->DisableVideoInput();
    m_dlInput->Release();
    m_dlInput=0;
  }
  if(m_dlCallback) {
    m_dlCallback->Release();
    m_dlCallback=0;
  }
  if(m_dl) {
    m_dl->Release();
    m_dl=0;
  }
  if(m_dlIterator) {
    m_dlIterator->Release();
    m_dlIterator=0;
  }
}

bool videoDECKLINK::open(gem::Properties&props)
{
  BMDVideoInputFlags flags = bmdVideoInputFlagDefault;
  BMDPixelFormat pixformat = bmdFormat8BitYUV;
  const std::string formatname=(("auto"==m_formatname)
                                || ("automatic" == m_formatname))?"":m_formatname;

  //if(m_devname.empty())return false;
  close();

  IDeckLinkIterator*m_dlIterator = CreateDeckLinkIteratorInstance();
  if(m_dlIterator) {
    setProperties(props);

    if(m_devnum<0 && m_devname.empty()) {
      // TODO: automatic device detection, based on input and mode
      while (m_dlIterator->Next(&m_dl) == S_OK) {
        m_dlInput=NULL;
        if (S_OK == m_dl->QueryInterface(IID_IDeckLinkInput, (void**)&m_dlInput)) {
          // check whether this device supports the selected format
          m_displayMode=getDisplayMode(m_dlInput, formatname, m_formatnum);
          if(m_displayMode) {
            // supported!
            break;
          }
          m_dlInput->Release();
        }
        m_dlInput=NULL;
      }
    } else { // user requested device (via name or index)
      int deviceCount=0;
      while (m_dlIterator->Next(&m_dl) == S_OK) {
        if(m_devnum == deviceCount) {
          break;
        }
        if(!m_devname.empty()) {
          char*deckLinkName = NULL;
          if(S_OK == m_dl->GetDisplayName((const char**)&deckLinkName)) {
            if (m_devname == deckLinkName) {
              free(deckLinkName);
              break;
            }
            free(deckLinkName);
          }
          if(S_OK == m_dl->GetModelName((const char**)&deckLinkName)) {
            if (m_devname == deckLinkName) {
              free(deckLinkName);
              break;
            }
            free(deckLinkName);
          }
        }
        m_dl->Release();
        m_dl=NULL;
        ++deviceCount;
      }
      m_dlInput=NULL;
      if(m_dl) {
        if (S_OK == m_dl->QueryInterface(IID_IDeckLinkInput, (void**)&m_dlInput)) {
          // check whether this device supports the selected format
          m_displayMode=getDisplayMode(m_dlInput, formatname, m_formatnum);
        } else {
          m_dlInput=NULL;
        }
      }
    }
  }
  if(!m_displayMode) {
    goto bail;
  }
  if (m_formatnum<0 && formatname.empty()) {
    // no format specified; try auto-detection
    IDeckLinkAttributes*dlAttribs=0;
    bool formatDetectionSupported = false;
    if (S_OK == m_dl->QueryInterface(IID_IDeckLinkAttributes,
                                     (void**)&dlAttribs)) {
      if (S_OK == dlAttribs->GetFlag(BMDDeckLinkSupportsInputFormatDetection,
                                     &formatDetectionSupported)) {
        if(formatDetectionSupported) {
          flags|=bmdVideoInputEnableFormatDetection;
        }
      }
    }
    if(dlAttribs) {
      dlAttribs->Release();
    }
  }

  BMDDisplayModeSupport displayModeSupported;
  if (S_OK != m_dlInput->DoesSupportVideoMode(
        m_displayMode->GetDisplayMode(),
        pixformat,
        flags,
        &displayModeSupported,
        NULL)) {
    goto bail;
  }
  if (displayModeSupported == bmdDisplayModeNotSupported) {
    goto bail;
  }
  if(S_OK != m_dl->QueryInterface (IID_IDeckLinkConfiguration,
                                   (void**)&m_dlConfig)) {
    m_dlConfig=NULL;
  }

  if(m_dlConfig) {
    m_dlConfig->SetInt(bmdDeckLinkConfigVideoInputConnection,
                       m_connectionType);
  }

  m_dlCallback = new DeckLinkCaptureDelegate(this, m_dlInput);
  if(S_OK != m_dlInput->EnableVideoInput(m_displayMode->GetDisplayMode(),
                                         pixformat, flags)) {
    goto bail;
  }

  return true;

bail:
  close();
  return false;
}

bool videoDECKLINK::start(void)
{
  return (m_dlInput && (S_OK == m_dlInput->StartStreams()));
}
bool videoDECKLINK::stop(void)
{
  if(m_dlInput) {
    m_dlInput->StopStreams();
  }
  return true;
}


pixBlock*videoDECKLINK::getFrame(void)
{
  m_mutex.lock();
  return &m_pixBlock;
}
void videoDECKLINK::setFrame(unsigned int w, unsigned int h, GLenum format,
                             unsigned char*data)
{
  m_mutex.lock();
  m_pixBlock.image.xsize=w;
  m_pixBlock.image.ysize=h;
  m_pixBlock.image.setCsizeByFormat(format);
  m_pixBlock.image.reallocate();

  m_pixBlock.image.fromUYVY(data);
  m_pixBlock.newimage=true;

  m_mutex.unlock();
}

void videoDECKLINK::releaseFrame(void)
{
  m_pixBlock.newimage=false;
  m_mutex.unlock();
}


std::vector<std::string>videoDECKLINK::enumerate(void)
{
  std::vector<std::string>result;
  IDeckLinkIterator*dli = CreateDeckLinkIteratorInstance();
  if(dli) {
    IDeckLink*deckLink = NULL;
    while (dli->Next(&deckLink) == S_OK) {
      char*deckLinkName = NULL;
      HRESULT res = deckLink->GetDisplayName((const char**)&deckLinkName);
      if (res == S_OK) {
        result.push_back(std::string(deckLinkName));
        free(deckLinkName);
      }
      deckLink->Release();
    }
    dli->Release();
  }
  return result;
}

bool videoDECKLINK::setDevice(int ID)
{
  m_devname.clear();
  m_devnum=ID;
  return true;
}
bool videoDECKLINK::setDevice(const std::string&device)
{
  m_devname=device;
  return true;
#if 0
  m_devname.clear();
  const std::string prefix="decklink://";
  if (!device.compare(0, prefix.size(), prefix)) {
    m_devname=device.substr(prefix.size());
    return true;
  }
  return false;
#endif
}
bool videoDECKLINK::enumProperties(gem::Properties&readable,
                                   gem::Properties&writeable)
{
  std::string dummy_s;
  int dummy_i=0;
  readable.clear();
  writeable.clear();

  readable.set("width", m_pixBlock.image.xsize);
  readable.set("height", m_pixBlock.image.ysize);

  dummy_s="auto";
  writeable.set("format", dummy_s);
  writeable.set("connection", dummy_s);

  return true;
}
void videoDECKLINK::setProperties(gem::Properties&props)
{
  std::vector<std::string>keys=props.keys();
  int i=0;
  for(i=0; i<keys.size(); i++) {
    const std::string key =keys[i];
    if("format" == key) {
      std::string s;
      double d;
      switch(props.type(key)) {
      case gem::Properties::STRING:
        if(props.get(key, s)) {
          m_formatnum =-1;
          m_formatname=s;
        }
        break;
      case gem::Properties::DOUBLE:
        if(props.get(key, d)) {
          m_formatnum =(int)d;
          m_formatname="";
        }
        break;
      }
    }
    if("connection" == key) {
      BMDVideoConnection vconn = m_connectionType;
      std::string s;
      double d;
      switch(props.type(key)) {
      case gem::Properties::STRING:
        if(props.get(key, s)) {
          if      ("SDI"        == s) {
            vconn=bmdVideoConnectionSDI;
          } else if ("HDMI"       == s) {
            vconn=bmdVideoConnectionHDMI;
          } else if ("OpticalSDI" == s) {
            vconn=bmdVideoConnectionOpticalSDI;
          } else if ("Component"  == s) {
            vconn=bmdVideoConnectionComponent;
          } else if ("Composite"  == s) {
            vconn=bmdVideoConnectionComposite;
          } else if ("SVideo"     == s) {
            vconn=bmdVideoConnectionSVideo;
          }
        }
        break;
      case gem::Properties::DOUBLE:
        if(props.get(key, d)) {
          int idx =(int)d;
          switch(idx) {
          default:
          case 0:
            vconn=bmdVideoConnectionSDI;
            break;
          case 1:
            vconn=bmdVideoConnectionHDMI;
            break;
          case 2:
            vconn=bmdVideoConnectionOpticalSDI;
            break;
          case 3:
            vconn=bmdVideoConnectionComponent;
            break;
          case 4:
            vconn=bmdVideoConnectionComposite;
            break;
          case 5:
            vconn=bmdVideoConnectionSVideo;
            break;
          }
        }
        break;
      }
      if(m_dlConfig && (m_connectionType != vconn)) {
        m_dlConfig->SetInt(bmdDeckLinkConfigVideoInputConnection, vconn);
      }
      m_connectionType = vconn;
    }
  }
  m_props=props;
}
void videoDECKLINK::getProperties(gem::Properties&props)
{
  std::vector<std::string>keys=props.keys();
  unsigned int i;
  for(i=0; i<keys.size(); i++) {
    if("width"==keys[i]) {
      props.set(keys[i], m_pixBlock.image.xsize);
    }
    if("height"==keys[i]) {
      props.set(keys[i], m_pixBlock.image.ysize);
    }
  }
}

std::vector<std::string>videoDECKLINK::dialogs(void)
{
  std::vector<std::string>result;
  return result;
}
bool videoDECKLINK::provides(const std::string&name)
{
  return (name==m_name);
}
std::vector<std::string>videoDECKLINK::provides(void)
{
  std::vector<std::string>result;
  result.push_back(m_name);
  return result;
}
const std::string videoDECKLINK::getName(void)
{
  return m_name;
}


#ifdef MUST_DLOPEN_DECKLINKAPI
# include <DeckLinkAPIDispatch.cpp>
#endif
