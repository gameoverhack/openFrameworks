#pragma once

#include "ofConstants.h"
#include "ofBaseTypes.h"
#include "ofPixels.h"
#include <map.h>

#ifdef OF_VIDEO_PLAYER_QUICKTIME
	#include "ofQtUtils.h"

struct qtAudioDevice{
    string deviceName;
    string deviceManufacturer;
    int deviceID;
    int internalDeviceID;
    CFStringRef deviceUID;
    int inputStreamCount;
    int outputStreamCount;
};

static vector<string> qtAudioDeviceList;
static map<string, qtAudioDevice> qtAudioDeviceMap;

class ofQuickTimePlayer : public ofBaseVideoPlayer{

	public:

		ofQuickTimePlayer();
		~ofQuickTimePlayer();

        bool			loadMovie(string name);
        void			closeMovie();	
        void			close();
        void			update();

        void			play();
        void			stop();
		 
        void			clearMemory();
		 
		bool            setPixelFormat(ofPixelFormat pixelFormat);
		ofPixelFormat   getPixelFormat();		 

        bool 			isFrameNew();
        unsigned char * getPixels();
        ofPixelsRef		getPixelsRef();
        const ofPixels&	getPixelsRef() const;
		
        float 			getWidth();
        float 			getHeight();

        bool			isPaused();
        bool			isLoaded();
        bool			isPlaying();

        float 			getPosition();
        float 			getDuration();
        int             getTotalNumFrames();
        float			getSpeed();
        bool			getIsMovieDone();
        ofLoopType      getLoopState();
    
        vector<string> getAudioDevices();
        int            getAudioTrackList();

        void 			setPosition(float pct);
        void 			setVolume(float volume);
        void            setPan(float pan); // L -1.0...1.0 R
        void 			setLoopState(ofLoopType state);
        void            setSpeed(float speed);
        void			setFrame(int frame);  // frame 0 = first frame...
        void 			setPaused(bool bPause);

        bool            setAudioDevice(int ID);
        bool            setAudioDevice(string deviceName);
    
        bool            setAudioTrackToChannel(int trackIndex, int oldChannelLabel, int newChannelLabel);

        vector< vector<float> > extractAudio(int trackIndex);


        float 		    getVolume();
        float 		    getPan();
        int             getCurrentFrame();

        void			firstFrame();
        void			nextFrame();
        void			previousFrame();

		bool 				bHavePixelsChanged;

	protected:
		
        void createImgMemAndGWorld();
    
        bool createAudioContext(qtAudioDevice qtDevice);
        string getAudioChannelAsString(AudioChannelLabel label);

		void start();

		ofPixels		 	pixels;
		int					width, height;
		bool				bLoaded;

		//these are public because the ofQuickTimePlayer implementation has some callback functions that need access
		//todo - fix this

		int					nFrames;				// number of frames
		bool				allocated;				// so we know to free pixels or not

		ofLoopType			currentLoopState;
		bool 				bStarted;
		bool 				bPlaying;
		bool 				bPaused;
		bool 				bIsFrameNew;			// if we are new
		float				speed;

		MovieDrawingCompleteUPP myDrawCompleteProc;
		MovieController  	thePlayer;
		GWorldPtr 			offscreenGWorld;
		Movie 			 	moviePtr;
		unsigned char * 	offscreenGWorldPixels;	// 32 bit: argb (qt k32ARGBPixelFormat)
		void				qtGetFrameCount(Movie & movForcount);

        ofPixelFormat       internalPixelFormat;
};

#endif





