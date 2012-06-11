#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{

	img.loadImage("C:/Projects/openFrameworksWIN64/apps/devApps/EmptyExample/bin/data/test.jpg");
	ofBackground(0,0,0);

	mySound.loadSound("C:/Projects/openFrameworksWIN64/apps/devApps/EmptyExample/bin/data/sounds/beat.wav");
	mySound.play();
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw()
{

	ofSetColor(255,0,0);
	ofEnableAlphaBlending();
	ofSetColor(255,0,0,100);
	ofRect(100,100,200,100);

	ofSetColor(255,255,255);
	img.draw(300,100);

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}