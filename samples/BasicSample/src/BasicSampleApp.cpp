#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"

#include "ciFaceTracker.h"
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicSampleApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    ci::CaptureRef              mCapture;
    FACETRACKER::ciFaceTracker  mFaceTracker;
    gl::TextureRef              mCamTex;
};

void BasicSampleApp::setup()
{
    mCapture = Capture::create(768, 768);
    mCapture->start();
    mFaceTracker.setup();
}

void BasicSampleApp::mouseDown( MouseEvent event )
{
}

void BasicSampleApp::update()
{
    if( mCapture->checkNewFrame() ){
        cv::Mat input( toOcv( *mCapture->getSurface() ) );
        mFaceTracker.update(input);
    }
}

void BasicSampleApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    gl::color( Color::white() );
    
    if( mCapture->getSurface() ){
        if(!mCamTex){
            mCamTex = gl::Texture::create( *mCapture->getSurface() );
        }else{
            mCamTex->update( *mCapture->getSurface() );
        }
        gl::draw(mCamTex);
    }
    
    mFaceTracker.draw(true);
}

CINDER_APP( BasicSampleApp, RendererGl )
