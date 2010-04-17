/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	- file handling with Gem

    Copyright (c) 2010 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM. zmoelnig@iem.kug.ac.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/



#ifndef INCLUDE_GEMFILES_H_
#define INCLUDE_GEMFILES_H_

#include <string>
#include <vector>


namespace gem {
  
  namespace files {

    std::vector<std::string>getFilenameListing(const std::string pattern);

  };
};

#endif /* INCLUDE_GEMFILES_H_ */
