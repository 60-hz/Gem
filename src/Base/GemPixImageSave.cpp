////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "GemPixImageSave.h"
#include "Gem/RTE.h"
#include "Base/GemFiles.h"

#include "plugins/imagesaver.h"

namespace gem {
  static std::map<std::string, std::string>*s_extension2mime=NULL;

  static void build_extension2mime(void) {
    if(NULL==s_extension2mime)
      s_extension2mime = new  std::map<std::string, std::string>;
    std::map<std::string, std::string>&e2m=*s_extension2mime;

    // avoid the use of '*/x-*' mimetypes as the "x-" is actually private
    // if a file-extension matches multiple mimetypes, chose the most obvious
    e2m["bmp"] ="image/bmp";
    e2m["cmx"] ="image/x-cmx";
    e2m["cod"] ="image/cis-cod";
    e2m["gif"] ="image/gif";
    e2m["ico"] ="image/x-icon";
    e2m["ief"] ="image/ief";
    e2m["jfif"]="image/pipeg";
    e2m["jpeg"]="image/jpeg";
    //e2m["jpeg"]="image/pjpeg";
    e2m["jpe"] ="image/jpeg";
    e2m["jpg"] ="image/jpeg";
    e2m["pbm"] ="image/x-portable-bitmap";
    e2m["pgm"] ="image/x-portable-graymap";
    e2m["png"] ="image/png";
    //e2m["png"]="image/x-png";
    e2m["pnm"] ="image/x-portable-anymap";
    e2m["ppm"] ="image/x-portable-pixmap";
    e2m["ras"] ="image/x-cmu-raster";
    e2m["rgb"] ="image/x-rgb";
    e2m["svg"] ="image/svg+xml";
    e2m["tiff"]="image/tiff";
    e2m["tif"] ="image/tiff";
    e2m["xbm"] ="image/x-xbitmap";
    e2m["xpm"] ="image/x-xpixmap";
    e2m["xwd"] ="image/x-xwindowdump";
  }


  static const std::string imgName2Mime(const std::string filename) {
    std::string empty;
    if(NULL==s_extension2mime) {
      build_extension2mime();
    }
    if(s_extension2mime) {
      std::map<std::string, std::string>::iterator it=s_extension2mime->find(gem::files::getExtension(filename, true));
      if(s_extension2mime->end() != it) {
        return it->second;
      }
      return empty;
    }
  }

  class PixImageSaver : public gem::plugins::imagesaver {
  private:
    static PixImageSaver*s_instance;
    std::vector<gem::plugins::imagesaver*>m_savers;
    std::vector<std::string>m_ids;

    PixImageSaver(void) {
      gem::PluginFactory<gem::plugins::imagesaver>::loadPlugins("image");
      std::vector<std::string>available_ids=gem::PluginFactory<gem::plugins::imagesaver>::getIDs();
      if(available_ids.size()>0) {
        startpost("Image saving support:");
        int i;
        for(i=0; i<available_ids.size(); i++) {
          startpost(" %s", available_ids[i].c_str());
        }
        endpost();
      }

      addSaver(available_ids, "QT");
      addSaver(available_ids, "magick");
      addSaver(available_ids);
    }
    bool addSaver( std::vector<std::string>available, std::string ID=std::string(""))
    {
      int i=0;
      int count=0;

      std::vector<std::string>id;
      if(!ID.empty()) {
        // if requested 'cid' is in 'available' add it to the list of 'id's
        if(std::find(available.begin(), available.end(), ID)!=available.end()) {
          id.push_back(ID);
        } else {
          // request for an unavailable ID
          verbose(2, "backend '%s' unavailable", ID.c_str());
          return false;
        }
      } else {
        // no 'ID' given: add all available IDs
        id=available;
      }

      for(i=0; i<id.size(); i++) {
        std::string key=id[i];
        verbose(2, "trying to add '%s' as backend", key.c_str());
        if(std::find(m_ids.begin(), m_ids.end(), key)==m_ids.end()) {
          // not yet added, do so now!
          gem::plugins::imagesaver*saver=
            gem::PluginFactory<gem::plugins::imagesaver>::getInstance(key); 
          if(NULL==saver)break;
          m_ids.push_back(key);
          m_savers.push_back(saver);
          count++;
          verbose(2, "added backend#%d '%s' @ 0x%x", m_savers.size()-1, key.c_str(), saver);
        }
      }
      return (count>0);
    }

  public:
    virtual ~PixImageSaver(void) {
      int i;
      for(i=0; i<m_savers.size(); i++) {
        delete m_savers[i];
        m_savers[i]=NULL;
      }
    }

    virtual bool save(const imageStruct&img, const std::string&filename, const std::string&mimetype, const gem::Properties&props) {
      std::multimap<float, int>priorities;
      std::multimap<float,int>::reverse_iterator rit;
      int i;
      for(i=0; i<m_savers.size(); i++) {
        float prio=m_savers[i]->estimateSave(img, filename, mimetype, props);
        priorities.insert( std::multimap<float, int>::value_type(prio, i));
      }

      for(rit=priorities.rbegin(); rit != priorities.rend(); rit++) {
        float prio=rit->first;
        int index=rit->second;
        verbose(2, "trying saver[%d]=%s @ %f", index, m_ids[index].c_str(), prio);
        if(m_savers[i]->save(img, filename, mimetype, props))
          return true;
      }

      return false;
    }

    static PixImageSaver*getInstance(void) {
      if(NULL==s_instance) {
        s_instance=new PixImageSaver();
      }
      return s_instance;
    }

  }; };

gem::PixImageSaver*gem::PixImageSaver::s_instance=NULL;

/***************************************************************************
 *
 * mem2image - Save an image to a file
 *
 ***************************************************************************/
GEM_EXTERN int mem2image(imageStruct* image, const char *filename, const int type)
{
  gem::PixImageSaver*piximagesaver=gem::PixImageSaver::getInstance();
  if(piximagesaver) {
    std::string fname=filename;
    std::string mimetype=gem::imgName2Mime(fname);
    gem::Properties props;
    props.set("quality", (float)type);

    if(piximagesaver->save(*image, filename, mimetype, props)) {
      return (1);
    }
  }
  error("GEM: Unable to save image to '%s'", filename);
  return (0);
}
