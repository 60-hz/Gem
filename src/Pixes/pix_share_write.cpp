/*
 *  pix_share_write.cpp
 *  GEM_darwin
 *
 *  Created by lincoln on 9/29/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "pix_share_write.h"
#include "Gem/Image.h"
#include "Gem/State.h"
#include "Gem/Exception.h"

#include <errno.h>
#include <stdio.h>

#ifdef _MSC_VER
# define snprintf _snprintf
#endif


CPPEXTERN_NEW_WITH_GIMME(pix_share_write);

int hash_str2us(std::string s)
{
  /*
  def self.rs( str, len=str.length )
    a,b = 63689,378551
    hash = 0
    len.times{ |i|
      hash = hash*a + str[i]
      a *= b
    }
    hash & SIGNEDSHORT
  end
  */

  int result=0;
  int a=63689;
  int b=378551;


  if(s.length()<1) {
    return -1;
  }

  unsigned int i=0;
  for(i=0; i<s.length(); i++) {
    result=result*a+s[i];
    a *= b;
  }

  return ((unsigned short)(result) & 0x7FFFFFFF);
}

/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_share_write :: pix_share_write(int argc, t_atom*argv) :
#if USE_SHM
  shm_id(0), shm_addr(NULL),
#endif
  m_size(0),
  m_outlet(0)
{
#if USE_SHM
  memset(&shm_desc, 0, sizeof(shm_desc));
#elif defined _WIN32
#else
  error("Gem has been compiled without shared memory support!")
#endif
  if(argc<1) {
    //~ throw(GemException("no ID given"));
  } else {
    int err  = getShm(argc, argv);

    switch(err) {
    case 0:
      break;
    case 1:
      throw(GemException("no valid size given"));
      break;
    case 2:
      throw(GemException("given size < 0"));
      break;
    case 3:
      throw(GemException("no valid dimensions given"));
      break;
    case 4:
      throw(GemException("<color> must be one of: 4,2,1,RGBA,YUV,Grey"));
      break;
    case 5:
      throw(GemException("arguments: <id> <width> <height> <color>"));
      break;
    case 6:
      throw(GemException("couldn't get shared memory"));
      break;
    case 7:
      throw(GemException("no ID given"));
      break;
    case 8:
      throw(GemException("invalid ID..."));
      break;
    default:
      throw(GemException("unknown error"));
      break;
    }
  }
  m_outlet = outlet_new(this->x_obj,0);
}

pix_share_write :: ~pix_share_write()
{
  freeShm();
  outlet_free(m_outlet);
}


void pix_share_write :: freeShm()
{
#ifdef _WIN32
  if ( shm_addr ) {
    UnmapViewOfFile( shm_addr );
  }
  if ( m_MapFile ) {
    CloseHandle( m_MapFile );
  }
  m_MapFile = NULL;
#elif USE_SHM
  if(shm_addr) {
    if (shmdt(shm_addr) == -1) {
      error("shmdt failed at %p", shm_addr);
    }
  }
  if(shm_id>0) {
    if (shmctl(shm_id,IPC_STAT, &shm_desc) != -1) {
      if(shm_desc.shm_nattch<=0) {
        if (shmctl(shm_id,IPC_RMID, &shm_desc) == -1) {
          error("shmctl remove failed for %d", shm_id);
        }
      }
    }
  }
  shm_id=0;
#endif /* _WIN32, USE_SHM */
  shm_addr = NULL;
}

int pix_share_write :: getShm(int argc,t_atom*argv)
{
  int fake = 0;

  size_t size=0;
  int    xsize=1;
  int    ysize=1;
  unsigned int color=GEM_RGBA;

  if(argc<1) {
    return 7;
  }
#ifdef _WIN32
  if ( shm_addr ) {
    UnmapViewOfFile( shm_addr );
  }
  if ( m_MapFile ) {
    CloseHandle( m_MapFile );
  }
  if(A_FLOAT==argv->a_type) {
    snprintf(m_fileMappingName, MAXPDSTRING-1,
             "gem_pix_share-FileMappingObject_%g", atom_getfloat(argv));
  } else if (A_SYMBOL==argv->a_type) {
    snprintf(m_fileMappingName, MAXPDSTRING-1,
             "gem_pix_share-FileMappingObject_%s", atom_getsymbol(argv)->s_name);
  }

#elif USE_SHM
  if(shm_id>0) {
    freeShm();
  }
  if(A_FLOAT==argv->a_type) {
    char buf[MAXPDSTRING];
    snprintf(buf, MAXPDSTRING-1, "%g", atom_getfloat(argv));
    buf[MAXPDSTRING-1]=0;
    fake = hash_str2us(buf);
  } else if(A_SYMBOL==argv->a_type) {
    fake = hash_str2us(atom_getsymbol(argv)->s_name);
  }
  if(fake<=0) {
    return 8;
  }
#else
  return -1;
#endif /* _WIN32, USE_SHM */

  argc--;
  argv++;

  switch(argc) {
  case 1: { /* just the size */
    if(A_FLOAT!=argv->a_type) {
      return 1;
    }
    int ssize=atom_getint(argv);
    if(ssize<0) {
      return 2;
    }
    size=ssize;
  }
  break;
  case 2: { /* x*y; assume GL_RGBA */
    if((A_FLOAT!=(argv+0)->a_type)||(A_FLOAT!=(argv+1)->a_type)) {
      return 3;
    }
    xsize=atom_getint(argv);
    ysize=atom_getint(argv+1);
  }
  break;
  case 3: {
    if((A_FLOAT!=(argv+0)->a_type)||(A_FLOAT!=(argv+1)->a_type)) {
      return 3;
    }
    xsize=atom_getint(argv);
    ysize=atom_getint(argv+1);
    if(A_FLOAT==(argv+2)->a_type) {
      int csize=atom_getint(argv+2);
      switch(csize) {
      case 1:
        color = GEM_GRAY;
        break;
      case 2:
        color = GEM_YUV;
        break;
      case 4:
        color = GEM_RGBA;
        break;
      default:
        return 4;
        break;
      }
    } else { // the 4th argument is a symbol: either "RGBA", "YUV" or "Grey"
      char c=atom_getsymbol(argv+2)->s_name[0];
      switch(c) {
      case 'G':
      case 'g':
        color = GEM_GRAY;
        break;
      case 'Y':
      case 'y':
        color = GEM_YUV;
        break;
      case 'R':
      case 'r':
        color = GEM_RGBA;
        break;
      default:
        return 4;
        break;
      }
    }
  }
  break;
  default:
    return 5;
  }

  if (xsize <= 0 || ysize <= 0) {
    return 3;
  }

  imageStruct dummy;
  dummy.setCsizeByFormat(color);

  m_size = (size)?(size):(xsize * ysize * dummy.csize);

  verbose(1, "%dx%dx%d: %d",
          xsize,ysize,dummy.csize, m_size);

#ifdef _WIN32
  size_t segmentSize=m_size+sizeof(t_pixshare_header);

  m_MapFile = CreateFileMapping(
                INVALID_HANDLE_VALUE,    // use paging file
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                (segmentSize & 0xFFFFFFFF00000000) >>
                32,         // maximum object size (high-order DWORD)
                segmentSize & 0xFFFFFFFF,         // maximum object size (low-order DWORD)
                m_fileMappingName);      // name of mapping object

  if (m_MapFile == NULL) {
    error("Could not create file mapping object %s - error %ld.",
          m_fileMappingName, GetLastError());
    return -1;
  }

  shm_addr = (unsigned char*) MapViewOfFile(
               m_MapFile,   // handle to map object
               FILE_MAP_ALL_ACCESS, // read/write permission
               0,
               0,
               segmentSize);

  if ( !shm_addr ) {
    error("Could not get a view of file %s - error %ld",m_fileMappingName,
          GetLastError());
    return -1;
  } else {
    verbose(0,"File mapping object %s successfully created.",
            m_fileMappingName);
  }

#elif USE_SHM

  /* get a new segment with the size specified by the user
   * OR an old segment with the size specified in its header
   * why: if somebody has already created the segment with our key
   * we want to reuse it, even if its size is smaller than we requested
   */
  errno=0;
  shm_id = shmget(fake,m_size+sizeof(t_pixshare_header), IPC_CREAT | 0666);

  if((shm_id<0) && (EINVAL==errno)) {
    errno=0;
    // the segment already exists, but is smaller than we thought!
    int id = shmget(fake,sizeof(t_pixshare_header),0666);
    if(id>0) { /* yea, we got it! */
      t_pixshare_header*h=(t_pixshare_header*)shmat(id,NULL,0666);
      if (!shm_addr || shm_addr==(void *)-1) {
        shm_addr=NULL;
        return 8;
      }
      /* read the size of the blob from the shared segment */
      if(h&&h->size) {
        error("someone was faster: only got %d bytes instead of %d",
              h->size, m_size);
        m_size=h->size;

        /* so free this shm-segment before we re-try with a smaller size */
        shmdt(h);

        /* now get the shm-segment with the correct size */
        shm_id = shmget(fake,m_size+sizeof(t_pixshare_header), IPC_CREAT | 0666);
      }
    }
  }

  if(shm_id>0) {
    /* now that we have a shm-segment, get the pointer to the data */
    shm_addr = (unsigned char*)shmat(shm_id,NULL,0666);
    if (!shm_addr || shm_addr==(void *)-1) {
      shm_addr=NULL;
      return 8;
    }

    if(shmctl(shm_id,IPC_STAT,&shm_desc)<0) {
      return 8;
    }
    /* write the size into the shm-segment */
    t_pixshare_header *h=(t_pixshare_header *)shm_addr;
    h->size = (shm_desc.shm_segsz-sizeof(t_pixshare_header));

    verbose(1, "shm:: id(%d) segsz(%d) cpid (%d) mem(%p)",
            shm_id,shm_desc.shm_segsz,shm_desc.shm_cpid, shm_addr);
  } else {
    error("couldn't get shm_id: error %d", errno);
    return -1; // AV : added because i'm usure of what value is returned when we get this error...
  }
#endif /* _WIN32, SHM */
  return 0;
}


void pix_share_write :: render(GemState *state)
{
  if (!state) {
    return;
  }
  pixBlock*img=NULL;
  state->get(GemState::_PIX, img);
  if(!img) {
    return;
  }

#ifndef _WIN32
  if(shm_id>0) {
#elif USE_SHM
  if(m_MapFile) {
#else
  if(0) {
#endif /* _WIN32 */
    imageStruct *pix = &img->image;
    size_t size=pix->xsize*pix->ysize*pix->csize;

    if (!shm_addr) {
      t_atom atom;
      error("no shmaddr");
      SETFLOAT(&atom, -1);
      outlet_anything(m_outlet, gensym("error"), 1, &atom);
      return;
    }

    if (size<=m_size) {
      t_pixshare_header *h=(t_pixshare_header *)shm_addr;
      h->size =m_size;
      h->xsize=pix->xsize;
      h->ysize=pix->ysize;
      h->format=pix->format;
      h->upsidedown=pix->upsidedown;
      memcpy(shm_addr+sizeof(t_pixshare_header),pix->data,size);
    } else {
      error("input image too large: %dx%dx%d=%d>%d",
            pix->xsize, pix->ysize, pix->csize,
            pix->xsize*pix->ysize*pix->csize,
            m_size);
    }
  }
}

void pix_share_write :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr,
                  reinterpret_cast<t_method>(&pix_share_write::setMessCallback),
                  gensym("set"), A_GIMME, A_NULL);
}

void pix_share_write :: setMessCallback(void *data, t_symbol* s, int argc,
                                        t_atom *argv)
{
  if(argc) {
    int err  = 0;
    //GetMyClass(data)->freeShm();
    err = GetMyClass(data)->getShm(argc, argv);
    if(err) {
      GetMyClass(data)->error("couldn't get new shared memory block! %d", err);
    }
    t_atom atom;
    SETFLOAT(&atom, (t_float) err);
    outlet_anything(GetMyClass(data)->m_outlet, gensym("error"), 1, &atom);
  } else {
    GetMyClass(data)->error("no args given!");
  }
}
