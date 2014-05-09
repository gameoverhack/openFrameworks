#include "ofBaseTypes.h"
#include "ofUtils.h"


//---------------------------------------------------------------------------
ofBaseVideoGrabber::~ofBaseVideoGrabber(){

}

//---------------------------------------------------------------------------
void ofBaseVideoGrabber::setVerbose(bool bTalkToMe){
	ofLogWarning("ofBaseVideoGrabber") << "setVerbose() not implemented";
}

//---------------------------------------------------------------------------
void ofBaseVideoGrabber::setDeviceID(int _deviceID){
	ofLogWarning("ofBaseVideoGrabber") << "setDeviceID() not implemented";
}

//---------------------------------------------------------------------------
void ofBaseVideoGrabber::setDesiredFrameRate(int framerate){
	ofLogWarning("ofBaseVideoGrabber") << "setDesiredFrameRate() not implemented";
}

//---------------------------------------------------------------------------
void ofBaseVideoGrabber::videoSettings(){
	ofLogWarning("ofBaseVideoGrabber") << "videoSettings() not implemented";
}

//---------------------------------------------------------------------------
//void ofBaseVideoGrabber::setPixelFormat(ofPixelFormat pixelFormat){
//	ofLogWarning("ofBaseVideoGrabber") << "setPixelFormat() not implemented";
//}
//
//ofPixelFormat ofBaseVideoGrabber::getPixelFormat(){
//	ofLogWarning("ofBaseVideoGrabber") << "getPixelFormat() not implemented";
//	return OF_PIXELS_RGB;
//}

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
ofBaseVideoPlayer::~ofBaseVideoPlayer(){

}

//---------------------------------------------------------------------------
float ofBaseVideoPlayer::getPosition(){
	ofLogWarning("ofBaseVideoPlayer") << "getPosition() not implemented";
	return 0.0;
}

//---------------------------------------------------------------------------
float ofBaseVideoPlayer::getSpeed(){
	ofLogWarning("ofBaseVideoPlayer") << "getSpeed() not implemented";
	return 0.0;
}

//---------------------------------------------------------------------------
float ofBaseVideoPlayer::getDuration(){
	ofLogWarning("ofBaseVideoPlayer") << "getDuration() not implemented";
	return 0.0;
}

//---------------------------------------------------------------------------
bool ofBaseVideoPlayer::getIsMovieDone(){
	ofLogWarning("ofBaseVideoPlayer") << "getIsMovieDone() not implemented";
	return false;
}

//---------------------------------------------------------------------------
vector<string> ofBaseVideoPlayer::getAudioDevices(){
	ofLog(OF_LOG_WARNING, "ofBaseVideoPlayer::getAudioDevices not implemented");
    vector<string> emptyVec;
	return emptyVec;
}

//---------------------------------------------------------------------------
int ofBaseVideoPlayer::getAudioTrackList(){
    ofLog(OF_LOG_WARNING, "ofBaseVideoPlayer::getAudioTrackList not implemented");
    return 0;
}

//---------------------------------------------------------------------------
bool ofBaseVideoPlayer::setAudioDevice(int ID){
    ofLog(OF_LOG_WARNING, "ofBaseVideoPlayer::setAudioDevice not implemented");
    return false;
}

//---------------------------------------------------------------------------
bool ofBaseVideoPlayer::setAudioDevice(string deviceName){
    ofLog(OF_LOG_WARNING, "ofBaseVideoPlayer::setAudioDevice not implemented");
    return false;
}

//---------------------------------------------------------------------------
bool ofBaseVideoPlayer::setAudioTrackToChannel(int trackIndex, int oldChannelLabel, int newChannelLabel){
    ofLog(OF_LOG_WARNING, "ofBaseVideoPlayer::setAudioTrackToChannel not implemented");
    return false;
}

//---------------------------------------------------------------------------
bool  ofBaseVideoPlayer::replaceAudioWithFile(string path, bool bMakeSelfContained, string copyToPath){
    ofLog(OF_LOG_WARNING, "ofBaseVideoPlayer::replaceAudioWithFile not implemented");
    return false;
}

//---------------------------------------------------------------------------
//vector< vector<float> >  ofBaseVideoPlayer::extractAudio(int trackIndex){
//    ofLog(OF_LOG_WARNING, "ofBaseVideoPlayer::extractAudio not implemented");
//    vector< vector<float> > v;
//    return v;
//}

//---------------------------------------------------------------------------
void ofBaseVideoPlayer::setPaused(bool bPause){
	ofLogWarning("ofBaseVideoPlayer") << "setPaused() not implemented";
}

//---------------------------------------------------------------------------
void ofBaseVideoPlayer::setPosition(float pct){
	ofLogWarning("ofBaseVideoPlayer") << "setPosition() not implemented";
}

//---------------------------------------------------------------------------
void ofBaseVideoPlayer::setVolume(float volume){
	ofLogWarning("ofBaseVideoPlayer") << "setVolume() not implemented";
}

//---------------------------------------------------------------------------
float ofBaseVideoPlayer::getVolume(){
	ofLog(OF_LOG_WARNING, "ofBaseVideoPlayer::getVolume not implemented");
	return 0.0f;
}

//---------------------------------------------------------------------------
void ofBaseVideoPlayer::setPan(float pan){
	ofLog(OF_LOG_WARNING, "ofBaseVideoPlayer::setPan not implemented");
}

//---------------------------------------------------------------------------
float ofBaseVideoPlayer::getPan(){
	ofLog(OF_LOG_WARNING, "ofBaseVideoPlayer::getPan not implemented");
	return 0.0f;
}

//---------------------------------------------------------------------------
void ofBaseVideoPlayer::setLoopState(ofLoopType state){
	ofLogWarning("ofBaseVideoPlayer") << "setLoopState() not implemented";
}

//---------------------------------------------------------------------------
void ofBaseVideoPlayer::setSpeed(float speed){
	ofLogWarning("ofBaseVideoPlayer") << "setSpeed() not implemented";
}

//---------------------------------------------------------------------------
void ofBaseVideoPlayer::setFrame(int frame){
	ofLogWarning("ofBaseVideoPlayer") << "setFrame() not implemented";
}

//---------------------------------------------------------------------------
int	ofBaseVideoPlayer::getCurrentFrame(){
	ofLogWarning("ofBaseVideoPlayer") << "getCurrentFrame() not implemented";
	return 0;
}

//---------------------------------------------------------------------------
int	ofBaseVideoPlayer::getTotalNumFrames(){
	ofLogWarning("ofBaseVideoPlayer") << "getTotalNumFrames() not implemented";
	return 0;
}

//---------------------------------------------------------------------------
ofLoopType ofBaseVideoPlayer::getLoopState(){
	ofLogWarning("ofBaseVideoPlayer") << "getLoopState() not implemented";
	return OF_LOOP_NONE;
}

//---------------------------------------------------------------------------
void ofBaseVideoPlayer::firstFrame(){
	ofLogWarning("ofBaseVideoPlayer") << "firstFrame() not implemented";
}

//---------------------------------------------------------------------------
void ofBaseVideoPlayer::nextFrame(){
	ofLogWarning("ofBaseVideoPlayer") << "nextFrame() not implemented";
}

//---------------------------------------------------------------------------
void ofBaseVideoPlayer::previousFrame(){
	ofLogWarning("ofBaseVideoPlayer") << "previousFrame() not implemented";
}

//---------------------------------------------------------------------------
//void ofBaseVideoPlayer::setPixelFormat(ofPixelFormat pixelFormat){
//	ofLogWarning("ofBaseVideoPlayer") << "setPixelFormat() not implemented";
//}
//---------------------------------------------------------------------------
//ofPixelFormat ofBaseVideoPlayer::getPixelFormat(){
//	ofLogWarning("ofBaseVideoPlayer") << "getPixelFormat() not implemented";
//	return OF_PIXELS_RGB;
//}
