/*
 *  pix_recordQT.cpp
 *  GEM_darwin
 *
 *  Created by chris clepper on 7/18/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */


#ifdef __APPLE__
#define HAVE_QUICKTIME
#endif

/* define HAVE_QUICKTIME in Base/configNT.h if you have quicktime-sdk installed */
#if defined HAVE_QUICKTIME

#include "pix_recordQT.h"
#include "Base/GemMan.h"
#include "Base/GemCache.h"

#ifdef __WIN32__
#include <io.h>
#include <stdio.h>
#include <QTML.h>
#include <Movies.h>
#include <QuicktimeComponents.h>
#include <Files.h>
#endif

#ifdef __APPLE__
#include <Quicktime/Quicktime.h>
#include <Carbon/Carbon.h>

#include <unistd.h> //needed for Unix file open() type functions
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 

#endif

CPPEXTERN_NEW_WITH_GIMME(pix_recordQT)

  /////////////////////////////////////////////////////////
//
// pix_recordQT
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_recordQT :: pix_recordQT(int argc, t_atom *argv)
{
 //cna ditch the offsets
  m_xoff = m_yoff = 0;
  m_width = m_height = 0;
  m_prevHeight = m_prevWidth = 0;
  if (argc == 4) {
    m_xoff = (int)atom_getfloat(&argv[0]);
    m_yoff = (int)atom_getfloat(&argv[1]);
    m_width = (int)atom_getfloat(&argv[2]);
    m_height = (int)atom_getfloat(&argv[3]);
  } else if (argc == 2) {
    m_width = (int)atom_getfloat(&argv[0]);
    m_height = (int)atom_getfloat(&argv[1]);
  } else if (argc != 0){
    error("GEM: pix_recordQT: needs 0, 2, or 4 values");
    m_xoff = m_yoff = 0;
    m_width = m_height = 128;
	 
  }

  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vert_pos"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("size"));
  
  m_outNumFrames = outlet_new(this->x_obj, 0);

  m_automatic = false;
  m_autocount = 0;
  m_filetype=0;
 // sprintf(m_pathname, "/Users/lincoln/Movies/temp");

  m_filename[0] = NULL;
    
  m_banged = false;
  

# ifdef __WIN32__
  // Initialize QuickTime Media Layer

  
  OSErr		err = noErr;
  if ((err = InitializeQTML(0))) {
    error("filmQT: Could not initialize quicktime: error %d\n", err);
    return;
  }
	
  // Initialize QuickTime
  if (err = EnterMovies()) {
    error("filmQT: Could not initialize quicktime: error %d\n", err);
    return;
  }
  post("pix_video: QT init done");
# endif // WINDOWS

  /* */
  //get list of codecs installed  -- useful later
	CodecNameSpecListPtr codecList;
	CodecNameSpec	codecName;
	int	i;
	int count;
	
	GetCodecNameList(&codecList,1);
	post("pix_recordQT : %i codecs installed",codecList->count);
	if (codecList->count < 64) count = codecList->count; else count = 64;
	for (i = 0; i < count; i++){
		codecName = codecList->list[i];
	//	post("pix_recordQT : codec %i %s %i ctype",i,codecName.typeName, codecName.cType);
		codecContainer[i].position = i;
		codecContainer[i].ctype = codecName.cType;
		codecContainer[i].codec = codecName.codec;
		}
  
  //initialize member variables
  stdComponent = NULL;
  hImageDesc = NULL;
  nFileRefNum = 0;
  nResID = movieInDataForkResID;
  m_recordStart = 0;
  m_recordStop = 0;
  m_recordSetup = 0;
  m_codecType = kJPEGCodecType;
  
  for(i = 0; i < count; i++){
		if (codecContainer[i].ctype == kJPEGCodecType) m_codec = codecContainer[i].codec;
  }
  post("pix_recordQT : pjpeg codec %i %i %i ctype",i,m_codecType, m_codec);
 // m_codec = (CodecComponent)65731;//65719;//65708; //this is pjpeg????
  m_codecSet = true;
  m_spatialQuality = codecNormalQuality; //codecHighQuality;
  m_codecQualitySet = true;
  m_dialog = 0;
  m_currentFrame = 0;
  
  #ifdef __APPLE__
  m_colorspace = GL_YUV422_GEM;
  #else
  m_colorspace = GL_BGRA_EXT;
  #endif
  
  m_firstRun = 1;
  
  m_ticks = 20;
  
 // post("pix_recordQT : anyCodec %d bestSpeedCodec %d bestFidelityCodec %d bestCompressionCodec %d",anyCodec,bestSpeedCodec,bestFidelityCodec,bestCompressionCodec);
   stdComponent = OpenDefaultComponent(StandardCompressionType,StandardCompressionSubType);
	
	if (stdComponent == NULL){
		post("pix_recordQT failed to open compressor component");
		return;
	}
	   
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_recordQT :: ~pix_recordQT()
{
	ComponentResult			compErr = noErr;

	post("pix_recordQT: deconstructor");
	if (stdComponent != NULL){
	compErr = CloseComponent(stdComponent);
	
	if (compErr != noErr) post("pix_recordQT : CloseComponent failed with error %d",compErr);

	}
//most likely a dumb thing to do
	/*
	#ifdef HAVE_QUICKTIME
# ifdef __WIN32__
	
	post("pix_video: exiting QT");
  // Initialize QuickTime
  ExitMovies();
  
  // Initialize QuickTime Media Layer
  TerminateQTML();
	
# endif // WINDOWS
#endif
*/

}

/////////////////////////////////////////////////////////
// Prepares QT for recording
//
/////////////////////////////////////////////////////////
void pix_recordQT :: setupQT() //this only needs to be done when codec info changes
{
	FSSpec		theFSSpec;
    OSErr		err = noErr;
    FSRef		ref;
	
	ComponentResult			compErr = noErr;

	m_recordSetup = 0; //if it fails then there is no setup
	
	//this mess should create and open a file for QT to use
	//probably should be a separate function
	//post("filename %s",m_filename);

	if (!m_filename[0]) {
        post("pix_recordQT:  no filename passed");
		return;
#ifdef __APPLE__
		} else {            
			err = ::FSPathMakeRef((UInt8*)m_filename, &ref, NULL);
			if (err == fnfErr) {
				// if the file does not yet exist, then let's create the file
				int fd;
                fd = open(m_filename, O_CREAT | O_RDWR, 0600);
                if (fd < 0){
                    post("pix_recordQT : problem with fd");
					return ;
					}
                        write(fd, " ", 1);
                        close(fd);
						err = FSPathMakeRef((UInt8*)m_filename, &ref, NULL);
						//post("pix_recordQT : made new file %s",m_filename);
			}

    

			

			if (err) {
				error("GEM: pix_recordQT: Unable to make file ref from filename %s", m_filename);
				return;
			}
			
			//err = ::FSsetCatalogInfo(&ref, kFSCatInfoSettableInfo, NULL);
			err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags, NULL, NULL, &theFSSpec, NULL);

			if (err != noErr){
					error("GEM: pix_recordQT: error %d in FSGetCatalogInfo()", err);
					return;
				}
		
		
			err = FSMakeFSSpec(theFSSpec.vRefNum, theFSSpec.parID, (UInt8*)m_filename, &theFSSpec);
			
			if (err != noErr && err != -37){
					error("GEM: pix_recordQT: error %d in FSMakeFSSpec()", err);
					return;
				}

		}
#else //win32 here
	} else {
		/*
		FILE *outfile;
		if ((outfile = fopen(m_filename, "")) == NULL) {
			post( "can't open %s", m_filename);
			return ;
		}
		fclose(outfile);
		*/
		c2pstr(m_filename);

		FSMakeFSSpec (0, 0L, (UInt8*)m_filename, &theFSSpec);
	//err = ::FSPathMakeRef((UInt8*)m_filename, &ref, NULL);
	if (err != noErr && err != -37){
					error("GEM: pix_recordQT: error %d in FSMakeFSSpec()", err);
					return;
				}
	

	}
#endif    //APPLE 

	//create the movie from the file 
	err = CreateMovieFile(	&theFSSpec,
							FOUR_CHAR_CODE('TVOD'),
							smSystemScript,
							createMovieFileDeleteCurFile |
							createMovieFileDontCreateResFile,
							&nFileRefNum,
							&m_movie);

	if (err != noErr) {
		post("pix_recordQT : CreateMovieFile failed with error %d",err);
		return;
		}
	


	//give QT the dimensions of the image to compress
	m_srcRect.top = 0;
	m_srcRect.left = 0;
	m_srcRect.bottom = m_height;
	m_srcRect.right = m_width;
	

	if (m_colorspace == GL_YUV422_GEM){

	//give QT the length of each pixel row in bytes (2 for 4:2:2 YUV)
	m_rowBytes = m_width * 2;

		//give QT the length of each pixel row in bytes (2 for 4:2:2 YUV)
		m_rowBytes = m_width * 2;
		
		//m_srcGWorld = NULL;//probably a memory leak
		err = QTNewGWorldFromPtr(&m_srcGWorld,
								k422YpCbCr8CodecType,
								//k32ARGBPixelFormat,
								&m_srcRect,
								NULL,
								NULL,
								0,
								m_compressImage.data,
								m_rowBytes);
	
	}else{
			//give QT the length of each pixel row in bytes (4 for ARGB)
		m_rowBytes = m_width * 4;
		
		#ifdef __APPLE__
		
		//m_srcGWorld = NULL;//probably a memory leak
		err = QTNewGWorldFromPtr(&m_srcGWorld,
								k32ARGBPixelFormat,
								&m_srcRect,
								NULL,
								NULL,
								0,
								m_compressImage.data,
								m_rowBytes);

		#else
		err = QTNewGWorldFromPtr(&m_srcGWorld,
								k32RGBAPixelFormat,
								&m_srcRect,
								NULL,
								NULL,
								0,
								m_compressImage.data,
								m_rowBytes);
		#endif

	}
	
	if (err != noErr){
		post("pix_recordQT : QTNewGWorldFromPtr failed with error %d",err);
		return;
		}
	
	SetMovieGWorld(m_movie,m_srcGWorld,GetGWorldDevice(m_srcGWorld));
	
	track = NewMovieTrack(m_movie,FixRatio(m_srcRect.right, 1),FixRatio(m_srcRect.bottom, 1),kNoVolume);
	
	media = NewTrackMedia(track,VideoMediaType,600,NULL,0);
	
	//moved to constructor
	/*
	stdComponent = OpenDefaultComponent(StandardCompressionType,StandardCompressionSubType);
	
	if (stdComponent == NULL){
		post("pix_recordQT failed to open compressor component");
		return;
	}*/
	
	//if the settings aren't already set then go ahead and do them
	//if (!m_spatialQuality || !m_codecType || m_dialog ){
	if (m_dialog ){
	
		//close the component if already open
		if (stdComponent) compErr = CloseComponent(stdComponent);
	
		if (compErr != noErr) post("pix_recordQT : CloseComponent failed with error %d",compErr);
		
		//open a new component from scratch
		stdComponent = OpenDefaultComponent(StandardCompressionType,StandardCompressionSubType);
	
		if (stdComponent == NULL){
			post("pix_recordQT failed to open compressor component");
			return;
		}
		
		post("pix_recordQT : opening settings Dialog");
		compErr = SCRequestSequenceSettings(stdComponent);
	
		if (compErr != noErr) post("pix_recordQT : SCRequestSequenceSettings failed with error %d",compErr);
	
		compErr = SCGetInfo(stdComponent, scTemporalSettingsType, &TemporalSettings);
		compErr = SCGetInfo(stdComponent, scSpatialSettingsType, &SpatialSettings);
	
		if (compErr != noErr) post("pix_recordQT : SCGetInfo failed with error %d",compErr);
		
		m_codecType = SpatialSettings.codecType;
		m_depth = SpatialSettings.depth;
		m_spatialQuality = SpatialSettings.spatialQuality;
		m_codec = SpatialSettings.codec;
		
		post("pix_recordQT : Dialog returned SpatialSettings.codecType %d",SpatialSettings.codecType);
		post("pix_recordQT : Dialog returned SpatialSettings.codec %d",SpatialSettings.codec);
		post("pix_recordQT : Dialog returned SpatialSettings.depth %d",SpatialSettings.depth);
		post("pix_recordQT : Dialog returned SpatialSettings.spatialQuality %d",SpatialSettings.spatialQuality);
		post("pix_recordQT : Dialog returned TemporalSettings.temporalQualitye %d",TemporalSettings.temporalQuality);
		post("pix_recordQT : Dialog returned TemporalSettings.frameRate %d",TemporalSettings.frameRate);
		post("pix_recordQT : Dialog returned TemporalSettings.keyFrameRate %d",TemporalSettings.keyFrameRate);
		
		m_dialog = false; //don't keep doing it again
		
	}else{
	
	/*
		compErr = SCGetInfo(stdComponent, scTemporalSettingsType, &TemporalSettings);
		compErr = SCGetInfo(stdComponent, scSpatialSettingsType, &SpatialSettings);
		compErr = SCGetInfo(stdComponent, scDataRateSettingsType, &datarate);
	
		if (compErr != noErr) post("pix_recordQT : SCGetInfo failed with error %d",compErr);
	*/	
		//post("pix_recordQT : manually filling in codec info");
		//fill in manually
		SpatialSettings.codecType = m_codecType;
		SpatialSettings.codec = m_codec;
		SpatialSettings.depth = 0; //should choose best depth
		SpatialSettings.spatialQuality = m_spatialQuality;
		
		TemporalSettings.temporalQuality = m_spatialQuality;
		TemporalSettings.frameRate = 0;
		TemporalSettings.keyFrameRate = 0;
		
		/*
		post("pix_recordQT : manual returned SpatialSettings.codecType %d",SpatialSettings.codecType);
		post("pix_recordQT : manual returned SpatialSettings.codec %d",SpatialSettings.codec);
		post("pix_recordQT : manual returned SpatialSettings.depth %d",SpatialSettings.depth);
		post("pix_recordQT : manual returned SpatialSettings.spatialQuality %d",SpatialSettings.spatialQuality);
		post("pix_recordQT : manual returned TemporalSettings.temporalQualitye %d",TemporalSettings.temporalQuality);
		post("pix_recordQT : manual returned TemporalSettings.frameRate %d",TemporalSettings.frameRate);
		post("pix_recordQT : manual returned TemporalSettings.keyFrameRate %d",TemporalSettings.keyFrameRate);
		*/
		
	}
	
	//if (m_codecType == kJPEGCodecType)
	//post("pix_recordQT : SCSpatialSettings CodecType %d is p-jpeg",m_codecType);
	//m_codec = SpatialSettings.codec;
	//post("pix_recordQT : SCSpatialSettings Codec %s",m_codec);
	
	//post("pix_recordQT : SCSpatialSettings depth %d",m_depth);
	
	//if (m_spatialQuality == codecHighQuality) post("pix_recordQT : SCSpatialSettings SpatialQuality codecHighQuality");
	
	datarate.frameDuration = 33;
	
	compErr = SCSetInfo(stdComponent, scTemporalSettingsType, &TemporalSettings);
	compErr = SCSetInfo(stdComponent, scSpatialSettingsType, &SpatialSettings);
	compErr = SCSetInfo(stdComponent, scDataRateSettingsType, &datarate);
	
	if (compErr != noErr) post("pix_recordQT : SCSetInfo failed with error %d",compErr);
	
#ifdef __APPLE__
	compErr = SCCompressSequenceBegin(stdComponent,GetPortPixMap(m_srcGWorld),&m_srcRect,&hImageDesc);
#else
	compErr = SCCompressSequenceBegin(stdComponent,m_srcGWorld->portPixMap,&m_srcRect,&hImageDesc);
#endif
	if (compErr != noErr) {
		post("pix_recordQT : SCCompressSequenceBegin failed with error %d",compErr);
		return;
		}
	
	err = BeginMediaEdits(media);
	if (err != noErr) {
		post("pix_recordQT : BeginMediaEdits failed with error %d",err);
		return;
		}
	

	//this will show that everything is OK for recording
	m_recordSetup = 1;
	
	//set the previous dimensions for the sanity check during compression
	m_prevWidth = m_width;
	m_prevHeight = m_height;
	
	//reset frame counter for new movie file
	m_currentFrame = 0;

	post("pix_recordQT: setup end");

}



//
// stops recording into the QT movie
//
void pix_recordQT :: stopRecording()
{
	ComponentResult			compErr = noErr;
	OSErr					err;
	
	
	err = EndMediaEdits(media);
	if (err != noErr) post("pix_recordQT : EndMediaEdits failed with error %d",err);
	
	err = InsertMediaIntoTrack(track,0,0,GetMediaDuration(media),0x00010000);
	if (err != noErr) post("pix_recordQT : InsertMediaIntoTrack failed with error %d",err);

	err = AddMovieResource(m_movie,nFileRefNum,&nResID,NULL);
	if (err != noErr) post("pix_recordQT : AddMovieResource failed with error %d",err);
	
	err = CloseMovieFile(nFileRefNum);
	if (err != noErr) post("pix_recordQT : CloseMovieFile failed with error %d",err);
	
	DisposeMovie(m_movie);
	DisposeGWorld(m_srcGWorld);
	m_srcGWorld = NULL;
		
	compErr = SCCompressSequenceEnd(stdComponent);
	
	if (compErr != noErr) post("pix_recordQT : SCCompressSequenceEnd failed with error %d",compErr);
	
	/*moved to destructor
	compErr = CloseComponent(stdComponent);
	
	if (compErr != noErr) post("pix_recordQT : CloseComponent failed with error %d",compErr);
	
	*/
	m_recordStop = 0;
	m_recordSetup = 0;
	m_recordStart = 0; //just to be sure
	
	m_currentFrame = 0; //reset the frame counter?
	
	m_firstRun = 1;
	
	outlet_float(m_outNumFrames,m_currentFrame);
	
	post("pix_recordQT : movie written to %s",m_filename);

}

void pix_recordQT :: compressFrame()
{
	OSErr					err;

	Handle					compressedData; //data to put in QT mov
	
	ComponentResult			compErr = noErr;

	short					syncFlag; //flag for keyframes
	

	
	//this times the render length to give QT a better idea about the actual framerate of the movie
	//the goal is to provide improved playback using internal QT playback routines
	
	#ifdef __APPLE__
	//fakes the first run time
	if (m_firstRun){
	
	  ::Microseconds(&startTime);
		m_firstRun = 0;
	
	}
	::Microseconds(&endTime);
	
	seconds = (float)(endTime.lo - startTime.lo) / 1000000.f;
	
	m_ticks = (int)(600 * seconds);
	
	if (m_ticks < 20) m_ticks = 20;
	
	#endif //timers
	
	//post("pix_recordQT : frame compression took %f seconds %d ticks", seconds, m_ticks );
	

	//post("pix_recordQT: compressing frame");
//apparently on OSX there is no member portPixMap in a GWorld so a function is used instead
#ifdef __APPLE__

	compErr = SCCompressSequenceFrame(	stdComponent,
										GetPortPixMap(m_srcGWorld),
										&m_srcRect,
										&compressedData,
										&dataSize,
										&syncFlag);
#else //Windows
	compErr = SCCompressSequenceFrame(	stdComponent,
										m_srcGWorld->portPixMap,
										&m_srcRect,
										&compressedData,
										&dataSize,
										&syncFlag);
#endif
	if (compErr != noErr) post("pix_recordQT : SCCompressSequenceFrame failed with error %d",compErr);
										 
	err = AddMediaSample(media,
							compressedData,
							0,
							dataSize,
							m_ticks, //this should not be a fixed value but vary with framerate
							(SampleDescriptionHandle)hImageDesc,
							1,
							syncFlag,
							NULL);
							
	if (err != noErr) post("pix_recordQT : AddMediaSample failed with error %d",err);
	
	#ifdef __APPLE__
	::Microseconds(&startTime);
					
	#endif //timer																		
																																															
	m_currentFrame++;
	
	outlet_float(m_outNumFrames,m_currentFrame);
	
}



/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_recordQT :: render(GemState *state)
{
	//check if state exists
	if (state->image){
		m_pixBlock = state->image;
		m_compressImage = m_pixBlock->image;
		m_height = m_pixBlock->image.ysize;
		m_width = m_pixBlock->image.xsize;
			

  if (m_automatic || m_banged) {
 
    m_autocount++;
    
	
	
	
	//record
	if (m_recordStart) {
		//if setupQT() has not been run do that first
		if (!m_recordSetup) setupQT();
		
		//should check if the size has changed or else we will freak the compressor's trip out
		if (m_width == m_prevWidth && m_height == m_prevHeight) {
			//go ahead and grab a frame if everything is ready to go
			if (m_recordSetup) 
				if (m_automatic && state->image->newimage) {
					compressFrame();
					}
					else{
						if (m_banged) {
							compressFrame();
							m_banged = false;
							}
					}
			//	post("grabbing frame");

			}else{
				post("pix_recordQT: movie dimensions changed prev %dx%d now %dx%d stopping recording",m_prevWidth,m_prevHeight,m_width,m_height);
				m_recordStop = 1;
				m_prevWidth = m_width;
				m_prevHeight = m_height; //go ahead and change dimensions
			}
	}
	
	//if recording is stopped and everything is setup then stop recording
	if (m_recordStop){
		//guard against someone not setting up QT beforehand
		if (!m_recordSetup)	return;
		stopRecording();
	}
	

  }
  }
}


/////////////////////////////////////////////////////////
// sizeMess
//
/////////////////////////////////////////////////////////
void pix_recordQT :: sizeMess(int width, int height)
{
  m_width = width;
  m_height = height;
}

/////////////////////////////////////////////////////////
// posMess
//
/////////////////////////////////////////////////////////
void pix_recordQT :: posMess(int x, int y)
{
  m_xoff = x;
  m_yoff = y;
}

/////////////////////////////////////////////////////////
// dialogMess
//
/////////////////////////////////////////////////////////
void pix_recordQT :: dialogMess()
{
	//if recording is going do not open the dialog
  if (!m_recordStart) {
		post("pix_recordQT : opening compression dialog");
		m_dialog = true;
		setupQT();
  }else{
		post("pix_recordQT : cannot open compression dialog while recording");
  }
}

/////////////////////////////////////////////////////////
// spits out a list of installed codecs and stores them
//
/////////////////////////////////////////////////////////
void pix_recordQT :: getCodecList()
{
	  //get list of codecs installed  -- useful later
	CodecNameSpecListPtr codecList;
	CodecNameSpec	codecName;
	int	i;
	int count;
	
	GetCodecNameList(&codecList,1);
	post("pix_recordQT : %i codecs installed",codecList->count);
	if (codecList->count < 64) count = codecList->count; else count = 64;
	for (i = 0; i < count; i++){
		codecName = codecList->list[i];
		post("pix_recordQT : codec %i %s %i ctype %d",i,codecName.typeName, codecName.cType,codecName.codec);
		codecContainer[i].position = i;
		codecContainer[i].ctype = codecName.cType;
		
		}
}


/////////////////////////////////////////////////////////
// deals with the name of a codec
//
/////////////////////////////////////////////////////////
void pix_recordQT :: codecMess(int argc, t_atom *argv)
{

	char codecName[80];

	//might be nice to allow both a symbol corresponding to the codecType and a number from the list
	if (argc) {
		if (argv->a_type == A_SYMBOL) {
			atom_string(argv++, codecName, 80);
			argc--;
		}
     }
	
	if (!strncmp(codecName,"jpeg",4)) {
		//have to put the right things in here
		m_codecType = kJPEGCodecType;
		m_codec = (CodecComponent)65719;//65708; //this is pjpeg?!? 
		post("pix_recordQT : kJPEGCodecType");
	}
	//do the same for these
	if (!strcmp(codecName,"animation")) post("pix_recordQT : kAnimationCodecType");
	if (!strncmp(codecName,"yuv2",4)) post("pix_recordQT : kComponentVideoCodecType");
	if (!strncmp(codecName,"yuvu",4)) post("pix_recordQT : kComponentVideoSigned");
	if (!strncmp(codecName,"raw",3)) post("pix_recordQT : kRawCodecType");
	if (!strncmp(codecName,"dvc",3)) post("pix_recordQT : kDVCNTSCCodecType");
	if (!strncmp(codecName,"dvcp",4)) post("pix_recordQT : kDVCPALCodecType");
	if (!strncmp(codecName,"y420",4)) post("pix_recordQT : kYUV420CodecType");
	post("pix_recordQT : codecName %s",codecName);

}


void pix_recordQT :: fileMess(int argc, t_atom *argv)
{

//if recording is going do not accept a new file name
//on OSX changing the name while recording won't have any effect 
//but it will give the wrong message at the end if recording
if (m_recordStart) return;

//  char *extension = ".mov";
  if (argc) {
    if (argv->a_type == A_SYMBOL) {
      atom_string(argv++, m_pathname, 80);
      argc--;
      sprintf(m_filename, "%s", m_pathname);
    }
    if (argc>0)
      m_filetype = atom_getint(argv);
  }

  m_autocount = 0;
  setModified();

post("pix_recordQT : filename %s",m_filename);

}

void pix_recordQT :: csMess(int format){
	if(format && format != m_colorspace){
		m_colorspace=format;
		post("pix_film: colorspace change will take effect the next time you load a film");
	}
}


/////////////////////////////////////////////////////////
// cleanImage
//
/////////////////////////////////////////////////////////
/*
void pix_recordQT :: cleanImage()
{
  // release previous data
  if (m_originalImage)
    {
      delete m_originalImage;
      m_originalImage = NULL;
    }
}
*/

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void pix_recordQT :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator((t_newmethod)_classpix_recordQT,gensym("pix_record"),A_DEFSYM,A_NULL);
  class_addmethod(classPtr, (t_method)&pix_recordQT::fileMessCallback,
		  gensym("file"), A_GIMME, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_recordQT::autoMessCallback,
		  gensym("auto"), A_FLOAT, A_NULL);
  class_addbang(classPtr, (t_method)&pix_recordQT::bangMessCallback);

  class_addmethod(classPtr, (t_method)&pix_recordQT::sizeMessCallback,
		  gensym("size"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_recordQT::posMessCallback,
		  gensym("vert_pos"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_recordQT::recordMessCallback,
		  gensym("record"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_recordQT::dialogMessCallback,
		  gensym("dialog"),  A_NULL);
  class_addmethod(classPtr, (t_method)&pix_recordQT::codeclistMessCallback,
		  gensym("codeclist"),  A_NULL);
	class_addmethod(classPtr, (t_method)&pix_recordQT::codecMessCallback,
		  gensym("codec"), A_GIMME, A_NULL);
	class_addmethod(classPtr, (t_method)&pix_recordQT::colorspaceCallback,
		  gensym("colorspace"), A_SYMBOL, A_NULL);
}

void pix_recordQT :: fileMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  GetMyClass(data)->fileMess(argc, argv);
}
void pix_recordQT :: autoMessCallback(void *data, t_floatarg on)
{
  GetMyClass(data)->m_automatic=(on!=0);
}
void pix_recordQT :: bangMessCallback(void *data)
{
  GetMyClass(data)->m_banged=true;
}

void pix_recordQT :: sizeMessCallback(void *data, t_floatarg width, t_floatarg height)
{
  GetMyClass(data)->sizeMess((int)width, (int)height);
}
void pix_recordQT :: posMessCallback(void *data, t_floatarg x, t_floatarg y)
{
  GetMyClass(data)->posMess((int)x, (int)y);
}

void pix_recordQT :: recordMessCallback(void *data, t_floatarg on)
{
	if (!(!(int)on)) {
		GetMyClass(data)->m_recordStart=1;
		GetMyClass(data)->m_recordStop=0;
		post("pix_video: recording on!");
	}else{
		GetMyClass(data)->m_recordStart=0;
		GetMyClass(data)->m_recordStop=1;
		}
	//setModified();
}

void pix_recordQT :: dialogMessCallback(void *data)
{
	GetMyClass(data)->dialogMess();
}

void pix_recordQT :: codeclistMessCallback(void *data)
{
	GetMyClass(data)->getCodecList();
}

void pix_recordQT :: codecMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  GetMyClass(data)->codecMess(argc, argv);
 // if (s->s_name == kJPEGCodecType) post("pix_recordQT : photo-jpeg codec"); else post("pix_recordQT : not photo-jpeg");
}

void pix_recordQT :: colorspaceCallback(void *data, t_symbol *state)
{
  GetMyClass(data)->csMess(getPixFormat(state->s_name));
}

#endif // HAVE_QUICKTIME
