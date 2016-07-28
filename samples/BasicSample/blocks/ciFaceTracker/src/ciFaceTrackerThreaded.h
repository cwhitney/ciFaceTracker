#pragma once

#include "cinder/Utilities.h"

#include "ciThread.h"
#include "ciFaceTracker.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace FACETRACKER;

class ciFaceTrackerThreaded : public ciFaceTracker, public ciThread {
public:
	ciFaceTrackerThreaded()
	:needsUpdatingBack(false)
	,needsUpdatingFront(false)
	,meanObjectPointsReady(false) {
	}
	~ciFaceTrackerThreaded() {
		stopThread(false);
        ci::sleep(500);
	}
	void setup() {
		failed = true;
		ciFaceTracker::setup();
		startThread(true, false);
	}
	bool update(cv::Mat image) {
		lock();
		image.copyTo(imageMiddle);
		objectPointsFront = objectPointsMiddle;
		imagePointsFront = imagePointsMiddle;
		meanObjectPointsFront = meanObjectPointsMiddle;
		objectPointsMatFront = objectPointsMatMiddle;
		failed = failedMiddle;
		needsUpdatingFront = true;
		unlock();
		if(!failed) {
			meanObjectPointsReady = true;
		}
		return getFound();
	}
	const cv::Mat& getObjectPointsMat() const {
		return objectPointsMatFront;
	}
	ci::vec3 getImagePoint(int i) const {
		if(failed) {
			return ci::vec3::zero();
		}
		return imagePointsFront[i];
	}
	ci::vec3 getObjectPoint(int i) const {
		if(failed) {
			return ci::vec3::zero();
		}
		return objectPointsFront[i];
	}
	ci::vec3 getMeanObjectPoint(int i) const {
		if(meanObjectPointsReady) {
			return meanObjectPointsFront[i];
		} else {
			return ci::vec3::zero();
		}
	}
	bool getVisibility(int i) const {
		return failed;
	}
	ci::vec3 getOrientation() const {
		return orientation;
	}
	ci::vec2 getPosition() const {
		return position;
	}
	float getScale() const {
		return scale;
	}
	
protected:
	void threadedFunction() {
		ciFaceTracker* threadedTracker = new ciFaceTracker();
		threadedTracker->setup();
		while( threadRunning ) {
			lock();
			needsUpdatingBack = needsUpdatingFront;
			if(needsUpdatingBack) {
				imageMiddle.copyTo(imageBack);
			}
			unlock();
            			
			threadedTracker->setRescale(rescale);
			threadedTracker->setIterations(iterations);
			threadedTracker->setClamp(clamp);
			threadedTracker->setTolerance(tolerance);
			threadedTracker->setAttempts(attempts);
			threadedTracker->setUseInvisible(useInvisible);
			
			if(needsUpdatingBack) {
				threadedTracker->update(imageBack);
			} else {
                ci::sleep(4);
			}
			
			lock();
			objectPointsMiddle = threadedTracker->getObjectPoints();
			imagePointsMiddle = threadedTracker->getImagePoints();
			meanObjectPointsMiddle = threadedTracker->getMeanObjectPoints();
			failedMiddle = !threadedTracker->getFound();
			position = threadedTracker->getPosition();
			orientation = threadedTracker->getOrientation();
			scale = threadedTracker->getScale();
			objectPointsMatMiddle = threadedTracker->getObjectPointsMat();
			unlock();
		}
		delete threadedTracker;
	}
		
	bool needsUpdatingBack, needsUpdatingFront;
	cv::Mat imageMiddle, imageBack;
	vector<ci::vec3> objectPointsFront, objectPointsMiddle;
	vector<ci::vec3> imagePointsFront, imagePointsMiddle;
	vector<ci::vec3> meanObjectPointsFront, meanObjectPointsMiddle;
	bool failedMiddle;
	bool meanObjectPointsReady;
	
	ci::vec3 orientation;
	float scale;
	ci::vec2 position;
	cv::Mat objectPointsMatBack, objectPointsMatMiddle, objectPointsMatFront; 
};
