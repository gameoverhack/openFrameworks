#include "ofQuickTimePlayer.h"
#include "ofUtils.h"

#ifndef TARGET_LINUX
#ifdef  OF_VIDEO_PLAYER_QUICKTIME

bool  	createMovieFromPath(char * path, Movie &movie);
bool 	createMovieFromPath(char * path, Movie &movie){
    
	Boolean 	isdir			= false;
	OSErr 		result 			= 0;
	FSSpec 		theFSSpec;
    
	short 		actualResId 	= DoTheRightThing;
    
#ifdef TARGET_WIN32
    result = NativePathNameToFSSpec (path, &theFSSpec, 0);
    if (result != noErr) {
        ofLogError("ofQuickTimePlayer") << "createMovieFromPath(): couldn't load movie, NativePathNameToFSSpec failed: OSErr " << result;
        return false;
    }
    
#endif
    
#ifdef TARGET_OSX
    FSRef 		fsref;
    result = FSPathMakeRef((const UInt8*)path, &fsref, &isdir);
    if (result) {
        ofLogError("ofQuickTimePlayer") << "createMovieFromPath(): couldn't load movie, FSPathMakeRef failed: OSErr " << result;
        return false;
    }
    result = FSGetCatalogInfo(&fsref, kFSCatInfoNone, NULL, NULL, &theFSSpec, NULL);
    if (result) {
        ofLogError("ofQuickTimePlayer") << "createMovieFromPath(): couldn't load movie, FSGetCatalogInfo failed: OSErr ", result;
        return false;
    }
#endif
    
	short movieResFile;
	result = OpenMovieFile (&theFSSpec, &movieResFile, fsRdPerm);
	if (result == noErr) {
        
		short   movieResID = 0;
		result = NewMovieFromFile(&movie, movieResFile, &movieResID, (unsigned char *) 0, newMovieActive, (Boolean *) 0);
		if (result == noErr){
			CloseMovieFile (movieResFile);
		} else {
			ofLogError("ofQuickTimePlayer") << "createMovieFromPath(): couldn't load movie, NewMovieFromFile failed: OSErr " << result;
			return false;
		}
	} else {
		ofLogError("ofQuickTimePlayer") << "createMovoeFromPath(): couldn't load movie, OpenMovieFile failed: OSErr " << result;
		return false;
	}
    
	return true;
}

bool    createFSSpecFromPath(char * path, FSSpec& theFSSpec);
bool    createFSSpecFromPath(char * path, FSSpec& theFSSpec){
    Boolean 	isdir			= false;
	OSErr 		result 			= 0;
    
	short 		actualResId 	= DoTheRightThing;
    
#ifdef TARGET_WIN32
    result = NativePathNameToFSSpec (path, &theFSSpec, 0);
    if (result != noErr) {
        ofLogError() << "NativePathNameToFSSpec failed with error: " << result << " for " << path << endl;
        return false;
    }
    
#endif
    
#ifdef TARGET_OSX
    FSRef 		fsref;
    result = FSPathMakeRef((const UInt8*)path, &fsref, &isdir);
    if (result) {
        ofLogError() << "FSPathMakeRef failed with error: " << result << " for " << path << endl;
        return false;
    }
    result = FSGetCatalogInfo(&fsref, kFSCatInfoNone, NULL, NULL, &theFSSpec, NULL);
    if (result) {
        ofLogError() << "FSGetCatalogInfo failed with error: " << result << " for " << path << endl;
        return false;
    }
#endif
	return true;
}

bool  	createMovieFromPath(char * path, Movie &movie, FSSpec& theFSSpec, short& movieResFile, short& movieResID);
bool 	createMovieFromPath(char * path, Movie &movie, FSSpec& theFSSpec, short& movieResFile, short& movieResID){
    
	OSErr 		result 			= 0;
    
    createFSSpecFromPath(path, theFSSpec);
	
	result = OpenMovieFile (&theFSSpec, &movieResFile, fsRdPerm);
	if (result == noErr) {
        
		
		result = NewMovieFromFile(&movie, movieResFile, &movieResID, (unsigned char *) 0, newMovieActive, (Boolean *) 0);
        if (result == noErr){
			CloseMovieFile (movieResFile);
		} else {
			ofLog(OF_LOG_ERROR,"NewMovieFromFile failed %d", result);
			return false;
		}
	} else {
		ofLog(OF_LOG_ERROR,"OpenMovieFile failed %d", result);
		return false;
	}
    
	return true;
}

//--------------------------------------------------------------
bool createMovieFromURL(string urlIn,  Movie &movie){
	char * url = (char *)urlIn.c_str();
	Handle urlDataRef;

	OSErr err;

	urlDataRef = NewHandle(strlen(url) + 1);
	if ( ( err = MemError()) != noErr){
		ofLogError("ofQuickTimePlayer") << "createMovieFromURL(): couldn't create url handle from \"" << urlIn << "\": OSErr " << err;
		return false;
	}

	BlockMoveData(url, *urlDataRef, strlen(url) + 1);

	err = NewMovieFromDataRef(&movie, newMovieActive,nil, urlDataRef, URLDataHandlerSubType);
	DisposeHandle(urlDataRef);

	if(err != noErr){
		ofLogError("ofQuickTimePlayer") << "createMovieFromURL(): couldn't load url \"" << urlIn << "\": OSErr " << err;
		return false;
	}else{
		return true;
	}

	return false;
}

//--------------------------------------------------------------
OSErr 	DrawCompleteProc(Movie theMovie, long refCon);
OSErr 	DrawCompleteProc(Movie theMovie, long refCon){

	ofQuickTimePlayer * ofvp = (ofQuickTimePlayer *)refCon;

	ofvp->bHavePixelsChanged = true;
	return noErr;
}

//---------------------------------------------------------------------------
ofQuickTimePlayer::ofQuickTimePlayer (){

	//--------------------------------------------------------------
   #if defined(TARGET_WIN32) || defined(TARGET_OSX)
    //--------------------------------------------------------------
    	moviePtr	 				= NULL;
    	allocated 					= false;
        offscreenGWorld				= NULL;
	//--------------------------------------------------------------
	#endif
	//--------------------------------------------------------------
    filePath                    = "";
	bLoaded 					= false;
	width 						= 0;
	height						= 0;
	speed 						= 1;
	bStarted					= false;
	nFrames						= 0;
	bPaused						= true;
	currentLoopState			= OF_LOOP_NORMAL;
}

//---------------------------------------------------------------------------
ofQuickTimePlayer::~ofQuickTimePlayer(){

	closeMovie();
    clearMemory();
    
	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------
		if(allocated)	delete[] offscreenGWorldPixels;
		if ((offscreenGWorld)) DisposeGWorld((offscreenGWorld));
	//--------------------------------------
	#endif
	//--------------------------------------

}

//---------------------------------------------------------------------------
unsigned char * ofQuickTimePlayer::getPixels(){
	return pixels.getPixels();
}

//---------------------------------------------------------------------------
ofPixelsRef ofQuickTimePlayer::getPixelsRef(){
	return pixels;
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::update(){

	if (bLoaded == true){

		//--------------------------------------------------------------
		#ifdef OF_VIDEO_PLAYER_QUICKTIME
		//--------------------------------------------------------------

			// is this necessary on windows with quicktime?
			#ifdef TARGET_OSX
				// call MoviesTask if we're not on the main thread
				if ( CFRunLoopGetCurrent() != CFRunLoopGetMain() )
				{
					//ofLog( OF_LOG_NOTICE, "not on the main loop, calling MoviesTask") ;
					MoviesTask(moviePtr,0);
				}
			#else
				// on windows we always call MoviesTask
				MoviesTask(moviePtr,0);
			#endif

		//--------------------------------------------------------------
		#endif
		//--------------------------------------------------------------
	}

	// ---------------------------------------------------
	// 		on all platforms,
	// 		do "new"ness ever time we idle...
	// 		before "isFrameNew" was clearning,
	// 		people had issues with that...
	// 		and it was badly named so now, newness happens
	// 		per-idle not per isNew call
	// ---------------------------------------------------

	if (bLoaded == true){

		bIsFrameNew = bHavePixelsChanged;
		if (bHavePixelsChanged == true) {
			bHavePixelsChanged = false;
		}
	}

}

//---------------------------------------------------------------------------
bool ofQuickTimePlayer::isFrameNew(){
	return bIsFrameNew;
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::close(){
	closeMovie();
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::closeMovie(){

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	if (bLoaded == true){

	    DisposeMovie (moviePtr);
		DisposeMovieDrawingCompleteUPP(myDrawCompleteProc);

        filePath = "";
		moviePtr = NULL;

    }

   	//--------------------------------------
	#endif
    //--------------------------------------

	bLoaded = false;

}

//--------------------------------------
#ifdef OF_VIDEO_PLAYER_QUICKTIME
//--------------------------------------

void ofQuickTimePlayer::createImgMemAndGWorld(){
    
	Rect movieRect;
    MacSetRect(&movieRect, 0, 0, width, height);
    
    switch(internalPixelFormat){
        case OF_PIXELS_MONO:
        {
            offscreenGWorldPixels = new unsigned char[1 * width * height + 8];
            pixels.allocate(width, height, OF_IMAGE_GRAYSCALE);
            // For k8IndexedGrayPixelFormat quicktime uses a reversed black and white color table ie., black for white, and 
            // white for black...rather than reverse every frame after decompression we can provide a custom color table
            // thanks to the unwieldy and ancient: http://www.cs.cmu.edu/afs/cs/project/cmcl/link.iwarp/member/OldFiles/tomstr/Mac2/Michigan/mac.bin/hypercard/xcmd/TIFFWindow%20:c4/tiffinfo.c
            
            // make a new color table
            CTabHandle grayCTab = (CTabHandle) NewHandle((256 * sizeof(ColorSpec)) + 10);
            (*grayCTab)->ctSeed = GetCTSeed();
            (*grayCTab)->ctFlags = 0;
            (*grayCTab)->ctSize = 255;
            RGBColor rgb;
            // invert the default color table
            for(int i = 0; i < 256; i++){
                rgb.red = rgb.green = rgb.blue = (65535 / (255)) * i;
                (*grayCTab)->ctTable[i].value = i; /* this must be filled in... */
                (*grayCTab)->ctTable[i].rgb = rgb;
            }
            QTNewGWorldFromPtr (&(offscreenGWorld), k8IndexedPixelFormat, &(movieRect), grayCTab, NULL, 0, (pixels.getPixels()), 1 * width);
            DisposeCTable(grayCTab);
            break;
        }
        case OF_PIXELS_RGB:
        {
            offscreenGWorldPixels = new unsigned char[3 * width * height + 24];
            pixels.allocate(width, height, OF_IMAGE_COLOR);
            QTNewGWorldFromPtr (&(offscreenGWorld), k24RGBPixelFormat, &(movieRect), NULL, NULL, 0, (pixels.getPixels()), 3 * width);
            break;
        }
        case OF_PIXELS_RGBA:
        {
            offscreenGWorldPixels = new unsigned char[4 * width * height + 32];
            pixels.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
            QTNewGWorldFromPtr (&(offscreenGWorld), k32RGBAPixelFormat, &(movieRect), NULL, NULL, 0, (pixels.getPixels()), 4 * width);
            break;
        }
        case OF_PIXELS_BGRA:
        {
            offscreenGWorldPixels = new unsigned char[4 * width * height + 32];
            pixels.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
            QTNewGWorldFromPtr (&(offscreenGWorld), k32BGRAPixelFormat, &(movieRect), NULL, NULL, 0, (pixels.getPixels()), 4 * width);
            break;
        }
//        case OF_PIXELS_ABGR:
//        {
//            offscreenGWorldPixels = new unsigned char[4 * width * height + 32];
//            pixels.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
//            QTNewGWorldFromPtr (&(offscreenGWorld), k32ABGRPixelFormat, &(movieRect), NULL, NULL, 0, (pixels.getPixels()), 4 * width);
//            break;
//        }
//        case OF_PIXELS_ARGB:
//        {
//            offscreenGWorldPixels = new unsigned char[4 * width * height + 32];
//            pixels.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
//            QTNewGWorldFromPtr (&(offscreenGWorld), k32ARGBPixelFormat, &(movieRect), NULL, NULL, 0, (pixels.getPixels()), 4 * width);
//            break;
//        }
        case OF_PIXELS_2YUV:
        {
#if !defined (TARGET_OSX) && !defined (GL_APPLE_rgb_422)
            MacSetRect(&movieRect, 0, 0, width*2, height); // this makes it look correct but we lose some of the performance gains
            SetMovieBox(moviePtr, &(movieRect));
            //width = width / 2; // this makes it go really fast but we only get 'half-resolution'...
            offscreenGWorldPixels = new unsigned char[4 * width * height + 32];
            pixels.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
            QTNewGWorldFromPtr (&(offscreenGWorld), k2vuyPixelFormat, &(movieRect), NULL, NULL, 0, (pixels.getPixels()), 4 * width);
#else
            // this works perfectly on Mac platform!
            offscreenGWorldPixels = new unsigned char[2 * width * height + 32];
            pixels.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
            QTNewGWorldFromPtr (&(offscreenGWorld), k2vuyPixelFormat, &(movieRect), NULL, NULL, 0, (pixels.getPixels()), 2 * width);
#endif
            
            break;
        }
//        case OF_PIXELS_RGB565:
//        {
//            offscreenGWorldPixels = new unsigned char[3 * width * height + 16];
//            pixels.allocate(width, height, OF_IMAGE_COLOR);
//            QTNewGWorldFromPtr (&(offscreenGWorld), k16BE565PixelFormat, &(movieRect), NULL, NULL, 0, (pixels.getPixels()), 3 * width);
//            break;
//        }
    }

	LockPixels(GetGWorldPixMap(offscreenGWorld));

    // from : https://github.com/openframeworks/openFrameworks/issues/244
    // SetGWorld do not seems to be necessary for offscreen rendering of the movie
    // only SetMovieGWorld should be called
    // if both are called, the app will crash after a few ofVideoPlayer object have been deleted

	#ifndef TARGET_WIN32
        SetGWorld (offscreenGWorld, NULL);
	#endif
	SetMovieGWorld (moviePtr, offscreenGWorld, nil);

}

//--------------------------------------
#endif
//--------------------------------------

//---------------------------------------------------------------------------
bool ofQuickTimePlayer::loadMovie(string name){

    
	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

		initializeQuicktime();			// init quicktime
		closeMovie();					// if we have a movie open, close it
		bLoaded 				= false;	// try to load now
        filePath                = name;

    // from : https://github.com/openframeworks/openFrameworks/issues/244
    // http://developer.apple.com/library/mac/#documentation/QuickTime/RM/QTforWindows/QTforWindows/C-Chapter/3BuildingQuickTimeCa.html
    // Apple's documentation *seems* to state that a Gworld should have been set prior to calling NewMovieFromFile
    // So I set a dummy Gworld (1x1 pixel) before calling createMovieFromPath
    // it avoids crash at the creation of objet ofVideoPlayer after a previous ofVideoPlayer have been deleted

    #ifdef TARGET_WIN32
        if (width != 0 && height != 0){
            pixels.clear();
            delete [] offscreenGWorldPixels;
        }
        width = 1;
        height = 1;
        createImgMemAndGWorld();
    #endif


		if( name.substr(0, 7) == "http://" || name.substr(0,7) == "rtsp://" ){
			if(! createMovieFromURL(name, moviePtr) ) return false;
		}else{
			name 					= ofToDataPath(name);
			if( !createMovieFromPath((char *)name.c_str(), moviePtr) ) return false;
		}

		bool bDoWeAlreadyHaveAGworld = false;
		if (width != 0 && height != 0){
			bDoWeAlreadyHaveAGworld = true;
		}
		Rect 				movieRect;
		GetMovieBox(moviePtr, &(movieRect));
		if (bDoWeAlreadyHaveAGworld){
			// is the gworld the same size, then lets *not* de-allocate and reallocate:
			if (width == movieRect.right &&
				height == movieRect.bottom){
				SetMovieGWorld (moviePtr, offscreenGWorld, nil);
			} else {
				width 	= movieRect.right;
				height 	= movieRect.bottom;
				pixels.clear();
				delete [] offscreenGWorldPixels;
				if ((offscreenGWorld)) DisposeGWorld((offscreenGWorld));
				createImgMemAndGWorld();
			}
		} else {
			width	= movieRect.right;
			height 	= movieRect.bottom;
			createImgMemAndGWorld();
		}

		if (moviePtr == NULL){
			return false;
		}

		//----------------- callback method
	    myDrawCompleteProc = NewMovieDrawingCompleteUPP (DrawCompleteProc);
		SetMovieDrawingCompleteProc (moviePtr, movieDrawingCallWhenChanged,  myDrawCompleteProc, (long)this);

		// ------------- get the total # of frames:
		nFrames				= 0;
		TimeValue			curMovieTime;
		curMovieTime		= 0;
		TimeValue			duration;

		//OSType whichMediaType	= VIDEO_TYPE; // mingw chokes on this
		OSType whichMediaType	= FOUR_CHAR_CODE('vide');

		short flags				= nextTimeMediaSample + nextTimeEdgeOK;

		while( curMovieTime >= 0 ) {
			nFrames++;
			GetMovieNextInterestingTime(moviePtr,flags,1,&whichMediaType,curMovieTime,0,&curMovieTime,&duration);
			flags = nextTimeMediaSample;
		}
		nFrames--; // there's an extra time step at the end of themovie

		// ------------- get some pixels in there ------
		GoToBeginningOfMovie(moviePtr);
		SetMovieActiveSegment(moviePtr, -1,-1);
		MoviesTask(moviePtr,0);

		bStarted 				= false;
		bLoaded 				= true;
		bPlaying 				= false;
		bHavePixelsChanged 		= false;
		speed 					= 1;

		return true;

	//--------------------------------------
	#endif
	//--------------------------------------

}

//--------------------------------------------------------
void ofQuickTimePlayer::start(){

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	if (bLoaded == true && bStarted == false){
		SetMovieActive(moviePtr, true);

		//------------------ set the movie rate to default
		//------------------ and preroll, so the first frames come correct

		TimeValue timeNow 	= 	GetMovieTime(moviePtr, 0);
		Fixed playRate 		=	GetMoviePreferredRate(moviePtr); 		//Not being used!

		PrerollMovie(moviePtr, timeNow, X2Fix(speed));
		SetMovieRate(moviePtr,  X2Fix(speed));
		setLoopState(currentLoopState);

		// get some pixels in there right away:
		MoviesTask(moviePtr,0);
		bHavePixelsChanged = true;

		bStarted = true;
		bPlaying = true;
	}

	//--------------------------------------
	#endif
	//--------------------------------------
}

//--------------------------------------------------------
void ofQuickTimePlayer::play(){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "play(): movie not loaded";
		return;
	}

	bPlaying = true;
	bPaused = false;

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	if (!bStarted){
	 	start();
	}else {
		// ------------ lower level "startMovie"
		// ------------ use desired speed & time (-1,1,etc) to Preroll...
		TimeValue timeNow;
	   	timeNow = GetMovieTime(moviePtr, nil);
		PrerollMovie(moviePtr, timeNow, X2Fix(speed));
		SetMovieRate(moviePtr,  X2Fix(speed));
		MoviesTask(moviePtr, 0);
	}

	//--------------------------------------
	#endif
	//--------------------------------------

	//this is if we set the speed first but it only can be set when we are playing.
	setSpeed(speed);

}

//--------------------------------------------------------
void ofQuickTimePlayer::stop(){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "stop(): movie not loaded";
		return;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	StopMovie (moviePtr);
	SetMovieActive (moviePtr, false);
	bStarted = false;

	//--------------------------------------
	#endif
	//--------------------------------------

	bPlaying = false;
}

//--------------------------------------------------------
void ofQuickTimePlayer::setVolume(float volume){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "setVolume(): movie not loaded";
		return;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	SetMovieVolume(moviePtr, volume*255);

	//--------------------------------------
	#endif
	//--------------------------------------
}

//--------------------------------------------------------
float ofQuickTimePlayer::getVolume(){
    float volume = 0.0f;
	if( !isLoaded() ){
		ofLog(OF_LOG_ERROR, "ofQuickTimePlayer: movie not loaded!");
		return volume;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

    volume = GetMovieVolume(moviePtr)/256.0;
    return volume;

	//--------------------------------------
	#endif
	//--------------------------------------
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::setPan(float pan){
	if( !isLoaded() ){
		ofLog(OF_LOG_ERROR, "ofQuickTimePlayer: movie not loaded!");
		return;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	SetMovieAudioBalance(moviePtr, pan, 0);

	//--------------------------------------
	#endif
	//--------------------------------------
}

//---------------------------------------------------------------------------
float ofQuickTimePlayer::getPan(){
    float pan = 0.0f;
	if( !isLoaded() ){
		ofLog(OF_LOG_ERROR, "ofQuickTimePlayer: movie not loaded!");
		return pan;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	GetMovieAudioBalance(moviePtr, &pan, 0);
	return pan;

	//--------------------------------------
	#endif
	//--------------------------------------
}

//--------------------------------------------------------
void ofQuickTimePlayer::setLoopState(ofLoopType state){

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

		if( isLoaded() ){

			TimeBase myTimeBase;
			long myFlags = 0L;
			myTimeBase = GetMovieTimeBase(moviePtr);
			myFlags = GetTimeBaseFlags(myTimeBase);
			switch (state) {
				case OF_LOOP_NORMAL:
					myFlags |= loopTimeBase;
					myFlags &= ~palindromeLoopTimeBase;
					SetMoviePlayHints(moviePtr, hintsLoop, hintsLoop);
					SetMoviePlayHints(moviePtr, 0L, hintsPalindrome);
					break;
				case OF_LOOP_PALINDROME:
					myFlags |= loopTimeBase;
					myFlags |= palindromeLoopTimeBase;
					SetMoviePlayHints(moviePtr, hintsLoop, hintsLoop);
					SetMoviePlayHints(moviePtr, hintsPalindrome, hintsPalindrome);
					break;
				case OF_LOOP_NONE:
					default:
					myFlags &= ~loopTimeBase;
					myFlags &= ~palindromeLoopTimeBase;
					SetMoviePlayHints(moviePtr, 0L, hintsLoop |
					hintsPalindrome);
					break;
			}
			SetTimeBaseFlags(myTimeBase, myFlags);

		}

	//--------------------------------------
	#endif
	//--------------------------------------

	//store the current loop state;
	currentLoopState = state;

}

//---------------------------------------------------------------------------
ofLoopType ofQuickTimePlayer::getLoopState(){
	return currentLoopState;
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::setPosition(float pct){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "setPosition(): movie not loaded";
		return;
	}

 	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	 	TimeRecord tr;
	 	tr.base 		= GetMovieTimeBase(moviePtr);
		long total 		= GetMovieDuration(moviePtr );
		long newPos 	= (long)((float)total * pct);
		SetMovieTimeValue(moviePtr, newPos);
		MoviesTask(moviePtr,0);

	//--------------------------------------
	#endif
	//--------------------------------------

}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::setFrame(int frame){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "setFrame(): movie not loaded";
		return;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	// frame 0 = first frame...

	// this is the simple way...
	//float durationPerFrame = getDuration() / getTotalNumFrames();

	// seems that freezing, doing this and unfreezing seems to work alot
	// better then just SetMovieTimeValue() ;

	if (!bPaused) SetMovieRate(moviePtr, X2Fix(0));

	// this is better with mpeg, etc:
	double frameRate = 0;
	double movieTimeScale = 0;
	MovieGetStaticFrameRate(moviePtr, &frameRate);
	movieTimeScale = GetMovieTimeScale(moviePtr);

	if (frameRate > 0){
		double frameDuration = 1 / frameRate;
		TimeValue t = (TimeValue)(frame * frameDuration * movieTimeScale);
		SetMovieTimeValue(moviePtr, t);
		MoviesTask(moviePtr, 0);
	}

   if (!bPaused) SetMovieRate(moviePtr, X2Fix(speed));

   //--------------------------------------
    #endif
   //--------------------------------------

}

//---------------------------------------------------------------------------
float ofQuickTimePlayer::getDuration(){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "getDuration(): movie not loaded";
		return 0.0;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

		return (float) (GetMovieDuration (moviePtr) / (double) GetMovieTimeScale (moviePtr));

	//--------------------------------------
	#endif
	//--------------------------------------

}

//---------------------------------------------------------------------------
float ofQuickTimePlayer::getPosition(){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "getPosition(): movie not loaded";
		return 0.0;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

		long total 		= GetMovieDuration(moviePtr);
		long current 	= GetMovieTime(moviePtr, nil);
		float pct 		= ((float)current/(float)total);
		return pct;

	//--------------------------------------
	#endif
	//--------------------------------------

}

//---------------------------------------------------------------------------
int ofQuickTimePlayer::getCurrentFrame(){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "getCurrentFrame(): movie not loaded";
		return 0;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	int frame = 0;

	// zach I think this may fail on variable length frames...
	float pos = getPosition();

	float  framePosInFloat = ((float)getTotalNumFrames() * pos);
	int    framePosInInt = (int)framePosInFloat;
	float  floatRemainder = (framePosInFloat - framePosInInt);
	if (floatRemainder > 0.5f) framePosInInt = framePosInInt + 1;
	//frame = (int)ceil((getTotalNumFrames() * getPosition()));
	frame = framePosInInt;

	return frame;

	//--------------------------------------
	#endif
	//--------------------------------------
}

//---------------------------------------------------------------------------
bool ofQuickTimePlayer::setPixelFormat(ofPixelFormat pixelFormat){
    if(pixelFormat == OF_PIXELS_RGB565){
        ofLogWarning() << "Pixel format not yet supported. Defaulting to OF_PIXELS_RGB";
        return false;
    }
    internalPixelFormat = pixelFormat;
    return true;
}

//--------------------------------------------------------------------
ofPixelFormat ofQuickTimePlayer::getPixelFormat(){
	return internalPixelFormat;
}
#ifdef TARGET_OSX
//---------------------------------------------------------------------------
vector<string> ofQuickTimePlayer::getAudioDevices(){
    
    if(qtAudioDeviceList.size() == 0){
        
        // taken from http://www.rawmaterialsoftware.com/viewtopic.php?t=9837&p=61685
        // and http://stackoverflow.com/questions/4575408/audioobjectgetpropertydata-to-get-a-list-of-input-devices
        
        AudioObjectPropertyAddress  propertyAddress;
        AudioObjectID               *deviceIDs;
        UInt32                      propertySize;
        int                         numDevices;
        
        propertyAddress.mSelector = kAudioHardwarePropertyDevices;
        propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
        propertyAddress.mElement = kAudioObjectPropertyElementMaster;
        
        if(AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize) == noErr){
            
            numDevices = propertySize / sizeof(AudioDeviceID);
            deviceIDs = (AudioDeviceID *)calloc(numDevices, sizeof(AudioDeviceID));
            
            AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize, deviceIDs);
            
            AudioObjectPropertyAddress      deviceAddress;
            char                            deviceName[64];
            char                            manufacturerName[64];
            
            for(int idx=0; idx<numDevices; idx++){
                
                // check for device name
                propertySize = sizeof(deviceName);
                deviceAddress.mSelector = kAudioDevicePropertyDeviceName;
                deviceAddress.mScope = kAudioObjectPropertyScopeGlobal;
                deviceAddress.mElement = kAudioObjectPropertyElementMaster;
                
                AudioObjectGetPropertyData(deviceIDs[idx], &deviceAddress, 0, NULL, &propertySize, deviceName);
                
                // check for device manufacturer
                propertySize = sizeof(manufacturerName);
                deviceAddress.mSelector = kAudioDevicePropertyDeviceManufacturer;
                deviceAddress.mScope = kAudioObjectPropertyScopeGlobal;
                deviceAddress.mElement = kAudioObjectPropertyElementMaster;
                
                AudioObjectGetPropertyData(deviceIDs[idx], &deviceAddress, 0, NULL, &propertySize, manufacturerName);
                
                // check for device uid
                CFStringRef uidString;
                propertySize = sizeof(uidString);
                deviceAddress.mSelector = kAudioDevicePropertyDeviceUID;
                deviceAddress.mScope = kAudioObjectPropertyScopeGlobal;
                deviceAddress.mElement = kAudioObjectPropertyElementMaster;
                
                AudioObjectGetPropertyData(deviceIDs[idx], &deviceAddress, 0, NULL, &propertySize, &uidString);
                
                // check for device with inputs
                deviceAddress.mSelector   = kAudioDevicePropertyStreams;
                deviceAddress.mScope = kAudioDevicePropertyScopeInput;
                UInt32 inputDataSize = 0;
                int inputStreams = 0;
                
                AudioObjectGetPropertyDataSize(deviceIDs[idx], &deviceAddress, 0, NULL, &inputDataSize);
                
                inputStreams = inputDataSize / sizeof(AudioStreamID);
                
                // check for device with inputs
                deviceAddress.mSelector   = kAudioDevicePropertyStreams;
                deviceAddress.mScope = kAudioDevicePropertyScopeOutput;
                UInt32 outputDataSize = 0;
                int outputStreams = 0;
                
                AudioObjectGetPropertyDataSize(deviceIDs[idx], &deviceAddress, 0, NULL, &outputDataSize);
                
                outputStreams = outputDataSize / sizeof(AudioStreamID);
                
                qtAudioDevice qtDevice;
                qtDevice.deviceName = deviceName;
                qtDevice.deviceManufacturer = manufacturerName;
                qtDevice.deviceID = idx;
                qtDevice.internalDeviceID = deviceIDs[idx];
                qtDevice.deviceUID = uidString;
                qtDevice.inputStreamCount = inputStreams;
                qtDevice.outputStreamCount = outputStreams;
                
                qtAudioDeviceList.push_back(qtDevice.deviceName);
                qtAudioDeviceMap.insert(pair<string, qtAudioDevice>(qtDevice.deviceName, qtDevice));
                
                ofLogNotice()   << "QT Audio deviceID [" << qtDevice.deviceID
                << "] name: " << qtDevice.deviceName
                << " manufacturer: " << qtDevice.deviceManufacturer
                << " internal ID: " << qtDevice.internalDeviceID
                << " UID: " << qtDevice.deviceUID
                << " input streams: " << qtDevice.inputStreamCount
                << " output streams " << qtDevice.outputStreamCount;
            }
            
        }else{
            ofLogError() << "Can't access audio device lists";
        }
    }else{
        
        for(int i = 0; i < qtAudioDeviceList.size(); i++){
            qtAudioDevice qtDevice = qtAudioDeviceMap[qtAudioDeviceList[i]];
            ofLogNotice()   << "QT Audio deviceID [" << qtDevice.deviceID
            << "] name: " << qtDevice.deviceName
            << " manufacturer: " << qtDevice.deviceManufacturer
            << " internal ID: " << qtDevice.internalDeviceID
            << " UID: " << qtDevice.deviceUID
            << " input streams: " << qtDevice.inputStreamCount
            << " output streams " << qtDevice.outputStreamCount;
        }
        
    }
    
    return qtAudioDeviceList;
    
}

//---------------------------------------------------------------------------
static string getTrackMediaTypeAsString(OSType trackType){
    switch (trackType) {
        case VideoMediaType:
            return "VideoMediaType";
            break;
        case SoundMediaType:
            return "SoundMediaType";
            break;
        case TextMediaType:
            return "TextMediaType";
            break;
        case BaseMediaType:
            return "BaseMediaType";
            break;
        case MPEGMediaType:
            return "MPEGMediaType";
            break;
        case MusicMediaType:
            return "MusicMediaType";
            break;
        case TimeCodeMediaType:
            return "TimeCodeMediaType";
            break;
        case SpriteMediaType:
            return "SpriteMediaType";
            break;
        case FlashMediaType:
            return "FlashMediaType";
            break;
        case MovieMediaType:
            return "MovieMediaType";
            break;
        case TweenMediaType:
            return "TweenMediaType";
            break;
        case ThreeDeeMediaType:
            return "ThreeDeeMediaType";
            break;
        case SkinMediaType:
            return "SkinMediaType";
            break;
        case HandleDataHandlerSubType:
            return "HandleDataHandlerSubType";
            break;
        case PointerDataHandlerSubType:
            return "PointerDataHandlerSubType";
            break;
        case NullDataHandlerSubType:
            return "NullDataHandlerSubType";
            break;
        case ResourceDataHandlerSubType:
            return "ResourceDataHandlerSubType";
            break;
        case URLDataHandlerSubType:
            return "URLDataHandlerSubType";
            break;
        case AliasDataHandlerSubType:
            return "AliasDataHandlerSubType";
            break;
        case WiredActionHandlerType:
            return "WiredActionHandlerType";
            break;
        case kQTQuartzComposerMediaType:
            return "kQTQuartzComposerMediaType";
            break;
        case TimeCode64MediaType:
            return "TimeCode64MediaType";
            break;
    }
}

//---------------------------------------------------------------------------
int ofQuickTimePlayer::getAudioTrackList(){
    
    AudioChannelLayout *layout = NULL;
    int trackIndex;
//    cout << "track count " << GetMovieTrackCount(moviePtr) << endl;
    for(trackIndex = 1; trackIndex <= GetMovieTrackCount(moviePtr); trackIndex++){
        UInt32 size = 0;
        
        Track track = GetMovieIndTrackType(moviePtr, trackIndex, SoundMediaType, movieTrackMediaType | movieTrackEnabledOnly);
    
        OSType trackType;
        GetMediaHandlerDescription(GetTrackMedia(track), &trackType, nil, nil);
        
        if(track == nil) continue;
        
        ofLogNotice() << "trackindex: " << trackIndex << " " << getTrackMediaTypeAsString(trackType) << endl;
        
        QTGetTrackPropertyInfo(track, kQTPropertyClass_Audio, kQTAudioPropertyID_ChannelLayout, nil, &size, nil);
        
        layout = (AudioChannelLayout*)calloc(1, size);
        
        QTGetTrackProperty(track, kQTPropertyClass_Audio, kQTAudioPropertyID_ChannelLayout, size, layout, nil );
        
        for(int i = 0; i < layout->mNumberChannelDescriptions; i++){
            ofLogNotice() << "trackindex: " << trackIndex << " channel: " << i << " assigned to: " << getAudioChannelAsString(layout->mChannelDescriptions[i].mChannelLabel) << " == " << layout->mChannelDescriptions[i].mChannelLabel;
        }
        break;
    }
    
    if(layout != NULL) free(layout);
    
    if(trackIndex == GetMovieTrackCount(moviePtr)) trackIndex = -1;
    
    // so this actually returns the first audio track
    // which is a bit dodgy, but will work for now
    return trackIndex;
}

//---------------------------------------------------------------------------
bool ofQuickTimePlayer::setAudioDevice(int ID){
    if(qtAudioDeviceList.size() == 0){
        if(getAudioDevices().size() == 0){
            ofLogError() << "ofQuickTimePlayer::setAudioDevice: No quicktime audio devices found";
            return false;
        }
    }
    if(ID < qtAudioDeviceList.size()){
        return createAudioContext(qtAudioDeviceMap[qtAudioDeviceList[ID]]);
    }else{
        ofLogError() << "ofQuickTimePlayer::setAudioDevice: No quicktime device with this ID: " << ID;
        return false;
    }
}

//---------------------------------------------------------------------------
bool ofQuickTimePlayer::setAudioDevice(string deviceName){
    if(qtAudioDeviceList.size() == 0){
        if(getAudioDevices().size() == 0){
            ofLogError() << "ofQuickTimePlayer::setAudioDevice: No quicktime audio devices found";
            return false;
        }
    }
    if(qtAudioDeviceMap.find(deviceName) != qtAudioDeviceMap.end()){
        return createAudioContext(qtAudioDeviceMap[deviceName]);
    }else{
        ofLogError() << "ofQuickTimePlayer::setAudioDevice: No quicktime device with this name: " << deviceName;
        return false;
    }
}

//---------------------------------------------------------------------------
bool ofQuickTimePlayer::setAudioTrackToChannel(int trackIndex, int oldChannelLabel, int newChannelLabel){
    
    bool ok = false;
    UInt32 size;
    AudioChannelLayout* layout;
    
    Track track = GetMovieIndTrackType(moviePtr, trackIndex, SoundMediaType, movieTrackMediaType | movieTrackEnabledOnly);
    
    if(track == nil){
        ofLogError() << "Invalid track index: " << trackIndex;
        return false;
    }
    
    QTGetTrackPropertyInfo(track, kQTPropertyClass_Audio, kQTAudioPropertyID_ChannelLayout, nil, &size, nil);
    
    layout = (AudioChannelLayout*)calloc(1, size);
    
    QTGetTrackProperty(track, kQTPropertyClass_Audio, kQTAudioPropertyID_ChannelLayout, size, layout, nil );
    
    int kNumChannelDescr = layout->mNumberChannelDescriptions;
    
    layout->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;
    layout->mChannelBitmap =  0 ;
    
    
    for(int i = 0; i < layout->mNumberChannelDescriptions; i++){
        if(layout->mChannelDescriptions[i].mChannelLabel == oldChannelLabel){
            layout->mChannelDescriptions[i].mChannelLabel = newChannelLabel;
            layout->mChannelDescriptions[0].mChannelFlags = kAudioChannelFlags_AllOff;
            ok = true;
        }
    }
    
    if(!ok){
        ofLogError() << "No channel description matches: " << getAudioChannelAsString(oldChannelLabel) << " == " << oldChannelLabel;
        free(layout);
        return false;
    }

    if(QTSetTrackProperty(track, kQTPropertyClass_Audio, kQTAudioPropertyID_ChannelLayout, size, layout) != noErr){
        ok = false;
    }
    
    free(layout);
    
    if(ok){
        ofLogVerbose() << "Audio channel " << oldChannelLabel << " remapped to " << newChannelLabel;
        MoviesTask(moviePtr, 0);
        return true;
    }else{
        ofLogError() << "Could not assign the new channel layout";
        return false;
    }
    
}

//---------------------------------------------------------------------------
bool ofQuickTimePlayer::replaceAudioWithFile(string path, bool bMakeSelfContained, string copyToPath){
    
    OSStatus err = noErr;
    TimeScale duration;
    
    int trackIndex = getAudioTrackList();
    
    if(trackIndex != -1){
        ofLogWarning() << "Attempt to delete track: " << trackIndex << endl;
        
        Track track = GetMovieIndTrackType(moviePtr, trackIndex, SoundMediaType, movieTrackMediaType | movieTrackEnabledOnly);
        
        if(track == NULL){
            ofLogVerbose() << "No audio track to delete " << trackIndex;
            duration = GetMovieDuration(moviePtr);
        }else{
            ofLogVerbose() << "Deleting current audio track " << trackIndex;
            Media oldAudioMedia = GetTrackMedia(track);
            duration = GetMediaDuration(oldAudioMedia);
            DisposeMovieTrack(track);
        }
    }else{
        ofLogWarning() << "No audio track to delete!" << endl;
    }

    Movie audioMoviePtr;
    Track audioNewTrack;;
    FSSpec audioFileSpec;
    short audioMovieResFile, audioMovieResID;

    if(createMovieFromPath((char *)path.c_str(), audioMoviePtr, audioFileSpec, audioMovieResFile, audioMovieResID)){
        ofLogVerbose() << "Replacing audio with media from: " << path;
        
        // reset timescale - paranoia
        SetMovieTimeScale(audioMoviePtr, GetMovieTimeScale(moviePtr));
        
        // get track and media refs
        Track audioMovieTrack = GetMovieTrack(audioMoviePtr, 1);
        Media audioMovieMedia = GetTrackMedia(audioMovieTrack);
        
        // begin edits
        BeginMediaEdits(audioMovieMedia);
        
        // add an empty track
        err = AddEmptyTrackToMovie (audioMovieTrack, moviePtr, nil, nil, &audioNewTrack);

        // insert the media into the track
        err = InsertTrackSegment(audioMovieTrack, audioNewTrack, 0, GetMovieDuration(moviePtr), 0);
        
        // end edits
        EndMediaEdits(audioMovieMedia);
        
        if(!bMakeSelfContained){
            // save quicktime movie - thanks to: http://www.mactech.com/articles/mactech/Vol.20/20.05/ModernTimes/index.html
            Handle dataRef = NULL;
            unsigned long dataRefType = 0;
            DataHandler handler = NULL;
            err = QTNewDataReferenceFromFSSpec(&movieFSSpec, 0, &dataRef, &dataRefType);
            err = OpenMovieStorage(dataRef, dataRefType, kDataHCanRead + kDataHCanWrite, &handler);
            err = UpdateMovieInStorage(moviePtr, handler);
            CloseMovieFile(movieResFile);
            close();

            return true;
            
        }else{
            bool replace = (copyToPath == "");
            string previousPath = filePath;
            string tempFile;
            
            if(replace){
                tempFile = filePath + ".temp";
            }else{
                tempFile = copyToPath;
                
            }
            
            ofLogVerbose() << "Creating temp file: " << tempFile << endl;
            
            FILE * pFile;
            pFile = fopen (tempFile.c_str(),"w");
            fclose (pFile);
            
            FSSpec   tempFSSpec;
            createFSSpecFromPath((char*)tempFile.c_str(), tempFSSpec);
            
            Movie outputMovie;
            Handle outDataRef = NULL;
            unsigned long outDataRefType = 0;
            DataHandler outDataHandler = NULL;
            
            CreateMovieStorage(outDataRef, outDataRefType, FOUR_CHAR_CODE('TVOD'), smSystemScript, createMovieFileDeleteCurFile | createMovieFileDontCreateResFile, &outDataHandler, &outputMovie);
            SetMovieSelection(moviePtr, 0, duration);
            outputMovie = CopyMovieSelection(moviePtr);
            AddMovieToStorage(outputMovie, outDataHandler);
            
            CloseMovieFile(movieResFile);
            close();
            
            FlattenMovieData(outputMovie, flattenAddMovieToDataFork, &tempFSSpec, FOUR_CHAR_CODE('TVOD'), smSystemScript,  createMovieFileDeleteCurFile | createMovieFileDontCreateMovie);
            
            err = QTNewDataReferenceFromFSSpec(&tempFSSpec, 0, &outDataRef, &outDataRefType);
            err = OpenMovieStorage(outDataRef, outDataRefType, kDataHCanRead + kDataHCanWrite, &outDataHandler);
            err = UpdateMovieInStorage(outputMovie, outDataHandler);

            
            // Check for error.
            err = GetMoviesError ();
            if (err == noErr) {
                ofLogNotice() << "Created new file: " << tempFile;
                if(replace){
                    close();
                    ofFile f = ofFile(tempFile);
                    f.renameTo(previousPath, true, true);
                }
                return true;
            }else{
                ofLogError() << "Could NOT create new file: " << err << " " << tempFile;
                ofFile f = ofFile(tempFile);
                f.remove();
                return false;
            }
        }
        


        
        
    }else{
        
        ofLogError() << "Could NOT load audio to replace media from: " << path;
        
        return false;
        
    }
}

//---------------------------------------------------------------------------
//vector< vector<float> > ofQuickTimePlayer::extractAudio(int trackIndex){
//
//    vector< vector<float> > audio;
//
//    MovieAudioExtractionRef extractionSessionRef = nil;
//    AudioStreamBasicDescription asbd;
//    AudioBufferList * bufferList;
//    
//    err = MovieAudioExtractionBegin(moviePtr, 0, &extractionSessionRef);
//    
////    AudioStreamBasicDescription audioFormat;
////    memset(&audioFormat, 0, sizeof(audioFormat));
////    audioFormat.mSampleRate = 48000;
////    audioFormat.mFormatID = kAudioFormatLinearPCM;
////    audioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | 
////    kAudioFormatFlagIsPacked | 
////    kAppleLosslessFormatFlag_16BitSourceData | 
////    kAudioFormatFlagIsNonInterleaved;
////    audioFormat.mBytesPerPacket = 4;
////    audioFormat.mFramesPerPacket = 1;
////    audioFormat.mChannelsPerFrame = 1;
////    audioFormat.mBytesPerFrame = 4;
////    audioFormat.mBitsPerChannel = 16;
////    
////    err = MovieAudioExtractionSetProperty(extractionSessionRef,
////                                          kQTPropertyClass_MovieAudioExtraction_Audio,
////                                          kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
////                                          sizeof (audioFormat), &audioFormat);
//    
//    err = MovieAudioExtractionGetProperty(extractionSessionRef,
//                                          kQTPropertyClass_MovieAudioExtraction_Audio,
//                                          kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
//                                          sizeof (asbd), &asbd, nil);
//    
//    if (err){
//        ofLogError() << "MovieAudioExtractionGetProperty Error: " << err;
//    }
//    
//    bufferList = (AudioBufferList *)calloc(1, sizeof(AudioBufferList) + asbd.mChannelsPerFrame*sizeof(AudioBuffer));
//    bufferList->mNumberBuffers = asbd.mChannelsPerFrame;
//    
//    ofLogNotice()   << " format: " << asbd.mFormatID 
//                    << " samplerate: " << asbd.mSampleRate
//                    << " flags: " << asbd.mFormatFlags
//                    << " channels: " << asbd.mChannelsPerFrame
//                    << " frames: " << asbd.mFramesPerPacket
//                    << " bits: " << asbd.mBitsPerChannel
//                    << " bytes/frame: " << asbd.mBytesPerFrame
//                    << " bytes/pack: " << asbd.mBytesPerPacket;
//    
//    
//    if((asbd.mFormatFlags & kAudioFormatFlagIsFloat) == kAudioFormatFlagIsFloat){
//        cout << "kAudioFormatFlagIsFloat" << endl;
//    }
//    if((asbd.mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagIsBigEndian){
//        cout << "kAudioFormatFlagIsBigEndian" << endl;
//    }
//    if((asbd.mFormatFlags & kAudioFormatFlagIsSignedInteger) == kAudioFormatFlagIsSignedInteger){
//        cout << "kAudioFormatFlagIsSignedInteger" << endl;
//    }
//    if((asbd.mFormatFlags & kAudioFormatFlagIsPacked) == kAudioFormatFlagIsPacked){
//        cout << "kAudioFormatFlagIsPacked" << endl;
//    }
//    if((asbd.mFormatFlags & kAudioFormatFlagIsAlignedHigh) == kAudioFormatFlagIsAlignedHigh){
//        cout << "kAudioFormatFlagIsAlignedHigh" << endl;
//    }
//    if((asbd.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == kAudioFormatFlagIsNonInterleaved){
//        cout << "kAudioFormatFlagIsNonInterleaved" << endl;
//    }
//    if((asbd.mFormatFlags & kAudioFormatFlagIsNonMixable) == kAudioFormatFlagIsNonMixable){
//        cout << "kAudioFormatFlagIsNonMixable" << endl;
//    }
//    if((asbd.mFormatFlags & kAudioFormatFlagsAreAllClear) == kAudioFormatFlagsAreAllClear){
//        cout << "kAudioFormatFlagsAreAllClear" << endl;
//    }
//    if((asbd.mFormatFlags & kAppleLosslessFormatFlag_16BitSourceData) == kAppleLosslessFormatFlag_16BitSourceData){
//        cout << "kAppleLosslessFormatFlag_16BitSourceData" << endl;
//    }
//    if((asbd.mFormatFlags & kAppleLosslessFormatFlag_20BitSourceData) == kAppleLosslessFormatFlag_20BitSourceData){
//        cout << "kAppleLosslessFormatFlag_20BitSourceData" << endl;
//    }
//    if((asbd.mFormatFlags & kAppleLosslessFormatFlag_24BitSourceData) == kAppleLosslessFormatFlag_24BitSourceData){
//        cout << "kAppleLosslessFormatFlag_24BitSourceData" << endl;
//    }
//    if((asbd.mFormatFlags & kAppleLosslessFormatFlag_32BitSourceData) == kAppleLosslessFormatFlag_32BitSourceData){
//        cout << "kAppleLosslessFormatFlag_32BitSourceData" << endl;
//    }
//    
//    int estimatedFrameCount = getTotalNumFrames();
//    
//    for (int i = 0; i < asbd.mChannelsPerFrame; i ++){
//        AudioBuffer audioBuffer = bufferList->mBuffers[i];
//        audioBuffer.mNumberChannels = asbd.mChannelsPerFrame;
//        audioBuffer.mDataByteSize = estimatedFrameCount*asbd.mBytesPerFrame;
//        audioBuffer.mData = calloc(1, estimatedFrameCount*asbd.mBytesPerFrame);
//        bufferList->mBuffers[i] = audioBuffer;
//    }
//    
//    UInt32 numFrames = 512;
//    UInt32 flags;
//    UInt32 actualFrameCount = 0;
//    
//    while (true){
//        err = MovieAudioExtractionFillBuffer(extractionSessionRef, &numFrames, bufferList, &flags);
//        if (err){
//            ofLogError() << "MovieAudioExtractionFillBuffer Error: " << err;
//        }
//        actualFrameCount += 1;
//        
//        if (flags & kQTMovieAudioExtractionComplete){
//            ofLogNotice() << "end of movie";
//            break;
//        }
//    }
//    
//    ofLogNotice() << "Extracted: " << actualFrameCount << " actual frames vs " << estimatedFrameCount*asbd.mBytesPerFrame << " estimated frames";
//    
//    audio.resize(asbd.mChannelsPerFrame);
//    
//    for (int i = 0; i < asbd.mChannelsPerFrame; i ++){
//        vector<float> channel;
//        channel.resize(actualFrameCount);
//        ofLogNotice() << "channel: " << i;
//        Float32 *frames = (Float32 *)bufferList->mBuffers[i].mData;
//        for (int j = 0; j < actualFrameCount; j ++){
//            channel[j] = frames[j];
//        }
//        audio[i] = channel;
//    }
//    
//    err = MovieAudioExtractionEnd(extractionSessionRef);
//    if (err){
//        ofLogError() << "MovieAudioExtractionEnd Error: " << err;
//        
//    }
//    
//    free(bufferList);
//    
//    return audio;
//}

//---------------------------------------------------------------------------
bool ofQuickTimePlayer::createAudioContext(qtAudioDevice qtDevice){
    
    bool ok = false;
    QTAudioContextRef audioContext = NULL;
    
    if(QTAudioContextCreateForAudioDevice(kCFAllocatorDefault, qtDevice.deviceUID, /*options*/ NULL, &audioContext) == noErr){
        ofLogVerbose() << "ofQuickTimePlayer::createAudioContext: Audio context CRE ok: " << filePath << " " << bLoaded;
        if(SetMovieAudioContext(moviePtr, audioContext) == noErr){
            ofLogVerbose() << "ofQuickTimePlayer::createAudioContext: Audio context SET ok: " << filePath << " " << bLoaded;
            ok = true;
        }else{
            ofLogError() << "ofQuickTimePlayer::createAudioContext: Audio context SET FAIL: " << filePath << " " << bLoaded;
        }
    }else{
        ofLogError() << "ofQuickTimePlayer::createAudioContext: could not create audio context for: " << qtDevice.deviceName;
    }
    
    QTAudioContextRelease(audioContext);    // this is super IMPORTANT!!
                                            // thank you: https://code.google.com/p/simpledj/source/browse/trunk/QTMovie%2BSimpleDJ.m?spec=svn2&r=2
    return ok;
    
}

//---------------------------------------------------------------------------
string ofQuickTimePlayer::getAudioChannelAsString(AudioChannelLabel label){
    
    string labelString = "";
    
    switch (label) {
        case kAudioChannelLabel_Left:
            labelString = "Left";
            break;
        case kAudioChannelLabel_Right:
            labelString = "Right";
            break;
        case kAudioChannelLabel_Center:
            labelString = "Center";
            break;
        case kAudioChannelLabel_LFEScreen:
            labelString = "LFE Screen";
            break;
        case kAudioChannelLabel_LeftSurround:
            labelString = "Left Surround";
            break;
        case kAudioChannelLabel_RightSurround:
            labelString = "Right Surround";
            break;
        case kAudioChannelLabel_CenterSurround:
            labelString = "Center Surround";
            break;
        case kAudioChannelLabel_Mono:
            labelString = "Mono";
            break;
        case kAudioChannelLabel_Unused:
            labelString = "Unused";
            break;
        case kAudioChannelLabel_Discrete_0:
            labelString = "Discrete 0";
            break;
        case kAudioChannelLabel_Discrete_1:
            labelString = "Discrete 1";
            break;
        case kAudioChannelLabel_Discrete_2:
            labelString = "Discrete 2";
            break;
        case kAudioChannelLabel_Discrete_3:
            labelString = "Discrete 3";
            break;
        case kAudioChannelLabel_Discrete_4:
            labelString = "Discrete 4";
            break;
        case kAudioChannelLabel_Discrete_5:
            labelString = "Discrete 5";
            break;
        case kAudioChannelLabel_Discrete_6:
            labelString = "Discrete 6";
            break;
        case kAudioChannelLabel_Discrete_7:
            labelString = "Discrete 7";
            break;
        case kAudioChannelLabel_Discrete_8:
            labelString = "Discrete 8";
            break;
        case kAudioChannelLabel_Discrete_9:
            labelString = "Discrete 9";
            break;
        case kAudioChannelLabel_Discrete_10:
            labelString = "Discrete 10";
            break;
        case kAudioChannelLabel_Discrete_11:
            labelString = "Discrete 11";
            break;
        case kAudioChannelLabel_Discrete_12:
            labelString = "Discrete 12";
            break;
        case kAudioChannelLabel_Discrete_13:
            labelString = "Discrete 13";
            break;
        case kAudioChannelLabel_Discrete_14:
            labelString = "Discrete 14";
            break;
        case kAudioChannelLabel_Discrete_15:
            labelString = "Discrete 15";
            break;
        case kAudioChannelLabel_Discrete_0 | 16:
            labelString = "Discrete 16";
            break;
        case kAudioChannelLabel_Discrete_0 | 17:
            labelString = "Discrete 17";
            break;
        case kAudioChannelLabel_Discrete_0 | 18:
            labelString = "Discrete 18";
            break;
        case kAudioChannelLabel_Discrete_0 | 19:
            labelString = "Discrete 19";
            break;
        case kAudioChannelLabel_Discrete_0 | 20:
            labelString = "Discrete 20";
            break;
        case kAudioChannelLabel_Discrete_0 | 21:
            labelString = "Discrete 21";
            break;
        case kAudioChannelLabel_Discrete_0 | 22:
            labelString = "Discrete 22";
            break;
        case kAudioChannelLabel_Discrete_0 | 23:
            labelString = "Discrete 23";
            break;
        case kAudioChannelLabel_Discrete_0 | 24:
            labelString = "Discrete 24";
            break;
        case kAudioChannelLabel_Discrete_0 | 25:
            labelString = "Discrete 25";
            break;
        case kAudioChannelLabel_Discrete_0 | 26:
            labelString = "Discrete 26";
            break;
        case kAudioChannelLabel_Discrete_0 | 27:
            labelString = "Discrete 27";
            break;
        case kAudioChannelLabel_Discrete_0 | 28:
            labelString = "Discrete 28";
            break;
        case kAudioChannelLabel_Discrete_0 | 29:
            labelString = "Discrete 29";
            break;
        case kAudioChannelLabel_Discrete_0 | 30:
            labelString = "Discrete 30";
            break;
        case kAudioChannelLabel_Discrete_0 | 31:
            labelString = "Discrete 31";
            break;
        case kAudioChannelLabel_Discrete_0 | 32:
            labelString = "Discrete 32";
            break;
    }
    
    return labelString;
}
#endif
//---------------------------------------------------------------------------
bool ofQuickTimePlayer::getIsMovieDone(){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "getIsMovieDone(): movie not loaded";
		return false;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

		bool bIsMovieDone = (bool)IsMovieDone(moviePtr);
		return bIsMovieDone;

	//--------------------------------------
	#endif
	//--------------------------------------
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::firstFrame(){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "firstFrame(): movie not loaded";
		return;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	setFrame(0);

	//--------------------------------------
	#endif
	//--------------------------------------
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::nextFrame(){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "nextFrame(): movie not loaded";
		return;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	setFrame(getCurrentFrame() + 1);

	//--------------------------------------
	#endif
	//--------------------------------------
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::previousFrame(){
	if( !isLoaded() ){
		ofLogError("ofQuickTimePlayer") << "previousFrame(): movie not loaded";
		return;
	}

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

	setFrame(getCurrentFrame() - 1);

	//--------------------------------------
	#endif
	//--------------------------------------
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::setSpeed(float _speed){

	speed 				= _speed;

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

		if (bPlaying == true){
			//setMovieRate actually plays, so let's call it only when we are playing
			SetMovieRate(moviePtr, X2Fix(speed));
		}

	//--------------------------------------
	#endif
	//--------------------------------------
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::setPaused(bool _bPause){

	bPaused = _bPause;

	//--------------------------------------
	#ifdef OF_VIDEO_PLAYER_QUICKTIME
	//--------------------------------------

		// there might be a more "quicktime-ish" way (or smarter way)
		// to do this for now, to pause, just set the movie's speed to zero,
		// on un-pause, set the movie's speed to "speed"
		// (and hope that speed != 0...)
		if (bPlaying == true){
			if (bPaused == true) 	SetMovieRate(moviePtr, X2Fix(0));
			else 					SetMovieRate(moviePtr, X2Fix(speed));
		}

	//--------------------------------------
	#endif
	//--------------------------------------
}

//---------------------------------------------------------------------------
void ofQuickTimePlayer::clearMemory(){
	pixels.clear();
}

//---------------------------------------------------------------------------
float ofQuickTimePlayer::getSpeed(){
	return speed;
}

//------------------------------------
int ofQuickTimePlayer::getTotalNumFrames(){
	return nFrames;
}

//----------------------------------------------------------
float ofQuickTimePlayer::getWidth(){
	return (float)width;
}

//----------------------------------------------------------
float ofQuickTimePlayer::getHeight(){
	return (float)height;
}

//----------------------------------------------------------
bool ofQuickTimePlayer::isPaused(){
	return bPaused;
}

//----------------------------------------------------------
bool ofQuickTimePlayer::isLoaded(){
	return bLoaded;
}

//----------------------------------------------------------
bool ofQuickTimePlayer::isPlaying(){
	return bPlaying;
}

#endif

#endif
