#ifndef _INCLUDE_GEMPLUGIN__VIDEOVNC_VIDEOVNC_H_
#define _INCLUDE_GEMPLUGIN__VIDEOVNC_VIDEOVNC_H_

#include "plugins/video.h"
#include "Gem/Image.h"

#include <rfb/rfbclient.h>

namespace gem { namespace plugins {
    class GEM_EXPORT videoVNC : public video {
 private:
   std::string m_name; // 'vnc'
   std::string m_devname;
   pixBlock m_pixBlock;
   rfbClient*m_client;
   Properties m_props;
   static void frameBufferCallback(rfbClient *client, int x, int y, int w, int h);
   void frameBufferUpdated(rfbClient *client, int x, int y, int w, int h);
 public:
   videoVNC(void);

   virtual ~videoVNC(void);

   virtual bool open(gem::Properties&props);
   virtual pixBlock *getFrame(void);

   virtual std::vector<std::string>enumerate(void);
   virtual bool	setDevice(int ID);
   virtual bool	setDevice(const std::string);


   virtual bool enumProperties(gem::Properties&readable,
			       gem::Properties&writeable);
   virtual void setProperties(gem::Properties&props);
   virtual void getProperties(gem::Properties&props);

   virtual std::vector<std::string>dialogs(void);
   // for pix_video: query whether this backend provides access to this class of devices
   // (e.g. "dv")
   virtual bool provides(const std::string);
   // get a list of all provided devices
   virtual std::vector<std::string>provides(void);

   // get's the name of the backend (e.g. "v4l")
   virtual const std::string getName(void);

   virtual bool isThreadable(void) {return false; return true;}
   virtual bool reset(void) {return true;}
   virtual void releaseFrame(void) {}
   virtual bool grabAsynchronous(bool) {return false; return true;}
   virtual bool	dialog(std::vector<std::string>names=std::vector<std::string>()) {return false;}
   virtual bool	    	setColor(int) {return false;}


   virtual void close(void);
   virtual bool start(void) {return true;};
   virtual bool stop(void)  {return true;};
};
};}; // namespace

#endif	// for header file
