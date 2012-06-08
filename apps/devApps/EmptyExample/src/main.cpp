#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main( ){




	/*for(int i=0; i<150; i++)
	{
		int *ptr;
	   try {
		 //ptr = new int[1200*1200*300];
		 ptr = new int[1200*1200*10];
		 printf("%u allocated!\n", _msize(ptr));
		 printf("%u bytes allocated and %d Gigabyte in total!\n", _msize(ptr)*i,float(_msize(ptr)*i)/1073741824.0);
	   }
	   catch(std::bad_alloc){ 
		 puts("new failed!\n");
		// return -1;
	   }
		 
	}*/ // got 4287432704 bytes; 3.99298 Gigabytes
   
	
    ofAppGlutWindow window;
	//ofSetupOpenGL(&window, 1024,768, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	//ofRunApp( new testApp());
	

}
