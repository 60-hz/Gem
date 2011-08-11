/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  access symbols of the RTE

  Copyright (c) 2010 IOhannes m zmoelnig. forum::f�r::uml�ute. IEM. zmoelnig@iem.kug.ac.at
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/
#ifndef INCLUDE_GEM_RTE_SYMBOL_H_
#define INCLUDE_GEM_RTE_SYMBOL_H_


#include "Gem/ExportDef.h"
#include <string>

struct _symbol;
struct _atom;
namespace gem {
  namespace RTE {
    GEM_EXTERN class Symbol {
    private:
      class PIMPL;
      PIMPL*m_pimpl;
      
    public:
      Symbol(void);
      Symbol(const gem::RTE::Symbol&a);
      Symbol(const std::string&name);
      Symbol(const struct _symbol*name);
      
      virtual ~Symbol(void);

      virtual Symbol&operator=(const Symbol&);
      virtual Symbol&operator=(const std::string&);
      virtual Symbol&operator=(const struct _symbol*);
      virtual Symbol&setSymbol(const unsigned int, const struct _atom*);
#if 0
      virtual std::string getString(void);
      virtual struct _symbol*getRTESymbol(void);
#endif
    };
  };
};
#endif /*  INCLUDE_GEM_RTE_SYMBOL_H_ */
