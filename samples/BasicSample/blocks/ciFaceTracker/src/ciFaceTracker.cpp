#include "ciFaceTracker.h"

// can be compiled with OpenMP for even faster threaded execution

#define it at<int>
#define db at<double>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace FACETRACKER;

vector<int> consecutive(int start, int end) {
	int n = end - start;
	vector<int> result(n);
	for(int i = 0; i < n; i++) {
		result[i] = start + i;
	}
	return result;
}

vector<int> FACETRACKER::ciFaceTracker::getFeatureIndices(Feature feature) {
	switch(feature) {
		case LEFT_JAW: return consecutive(0, 9);
		case RIGHT_JAW: return consecutive(8, 17);
		case JAW: return consecutive(0, 17);
		case LEFT_EYEBROW: return consecutive(17, 22);
		case RIGHT_EYEBROW: return consecutive(22, 27);
		case LEFT_EYE: return consecutive(36, 42);
		case RIGHT_EYE: return consecutive(42, 48);
		case OUTER_MOUTH: return consecutive(48, 60);
		case INNER_MOUTH: {
			static int innerMouth[] = {48,60,61,62,54,63,64,65};
			return vector<int>(innerMouth, innerMouth + 8);
		}
		case NOSE_BRIDGE: return consecutive(27, 31);
		case NOSE_BASE: return consecutive(31, 36);
		case FACE_OUTLINE: {
			static int faceOutline[] = {17,18,19,20,21,22,23,24,25,26, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
			return vector<int>(faceOutline, faceOutline + 27);
		}
		case ALL_FEATURES: return consecutive(0, 66);
	}
}
		
FACETRACKER::ciFaceTracker::ciFaceTracker()
:rescale(1)
,iterations(15) // [1-25] 1 is fast and inaccurate, 25 is slow and accurate
,clamp(3) // [0-4] 1 gives a very loose fit, 4 gives a very tight fit
,tolerance(.01) // [.01-1] match tolerance
,attempts(1) // [1-4] 1 is fast and may not find faces, 4 is slow but will find faces
,failed(true)
,fcheck(true) // check for whether the tracking failed
,frameSkip(-1) // how often to skip frames
,useInvisible(true)
,age(-1)
{
}

void FACETRACKER::ciFaceTracker::setup() {
	wSize1.resize(1);
	wSize1[0] = 7;
	
	wSize2.resize(3);
	wSize2[0] = 11;
	wSize2[1] = 9;
	wSize2[2] = 7;
    
    ci::fs::path tBasePath = ci::fs::path( ci::app::getResourceDirectory() );
    
    ci::fs::path tFtPath  = tBasePath / FACE_TRACKER_FACE_TRACKER;
    ci::fs::path tFt2Path  = tBasePath / FACE_TRACKER_FACE2_TRACKER;
    ci::fs::path tTriPath = tBasePath / FACE_TRACKER_FACE_TRI;
    ci::fs::path tConPath = tBasePath / FACE_TRACKER_FACE_CON;
    
	tracker.Load( tFt2Path.string().c_str() );
	tri = IO::LoadTri( tTriPath.string().c_str() );
	con = IO::LoadCon( tConPath.string().c_str() );  // not being used right now
}

bool FACETRACKER::ciFaceTracker::update(cv::Mat image) {
	if(rescale == 1) {
		im = image; 
	} else {
		resize(image, im, cv::Size(rescale * image.cols, rescale * image.rows));
	}
	
	cvtColor(im, gray, CV_RGB2GRAY);
	
	bool tryAgain = true;
	for(int i = 0; tryAgain && i < attempts; i++) {
		vector<int> wSize;
		if(failed) {
			wSize = wSize2;
		} else {
			wSize = wSize1;
		}
		
		if(tracker.Track(gray, wSize, frameSkip, iterations, clamp, tolerance, fcheck) == 0) {
			currentView = tracker._clm.GetViewIdx();
			failed = false;
			age++;
			tryAgain = false;
			updateObjectPoints();
		} else {
			tracker.FrameReset();
			failed = true;
			age = -1;
		}
	}
	return !failed;
}

void FACETRACKER::ciFaceTracker::draw(bool drawLabels) const{
	if(failed) {
		return;
	}
    
    gl::ScopedColor scCol( Color::white() );
	
    gl::draw( getImageFeature(LEFT_EYE) );
	gl::draw( getImageFeature(RIGHT_EYE) );
	gl::draw( getImageFeature(LEFT_EYEBROW) );
	gl::draw( getImageFeature(RIGHT_EYEBROW) );
	gl::draw( getImageFeature(NOSE_BRIDGE) );
	gl::draw( getImageFeature(NOSE_BASE) );
	gl::draw( getImageFeature(INNER_MOUTH) );
	gl::draw( getImageFeature(OUTER_MOUTH) );
	gl::draw( getImageFeature(JAW) );
	
	if(drawLabels) {
		int n = size();
		for(int i = 0; i < n; i++){
			if(getVisibility(i)) {
                stringstream ss;
                ss << i;
                gl::drawString( ss.str(), vec2(getImagePoint(i)) );
			}
		}
	}
}

void FACETRACKER::ciFaceTracker::reset() {
	tracker.FrameReset();
}

int FACETRACKER::ciFaceTracker::size() const {
	return tracker._shape.rows / 2;
}

bool FACETRACKER::ciFaceTracker::getFound() const {
	return !failed;
}

int FACETRACKER::ciFaceTracker::getAge() const {
	return age;
}

bool FACETRACKER::ciFaceTracker::getVisibility(int i) const {
	if(failed) {
		return false;
	}
	const cv::Mat& visi = tracker._clm._visi[currentView];
	return (visi.it(i, 0) != 0);
}

vector<vec3> FACETRACKER::ciFaceTracker::getImagePoints() const {
	int n = size();
	vector<vec3> imagePoints(n);
	for(int i = 0; i < n; i++) {
		imagePoints[i] = getImagePoint(i);
	}
	return imagePoints;
}

vector<vec3> FACETRACKER::ciFaceTracker::getObjectPoints() const {
	int n = size();
	vector<vec3> objectPoints(n);
	for(int i = 0; i < n; i++) {
		objectPoints[i] = getObjectPoint(i);
	}
	return objectPoints;
}

vector<vec3> FACETRACKER::ciFaceTracker::getMeanObjectPoints() const {
	int n = size();
	vector<vec3> meanObjectPoints(n);
	for(int i = 0; i < n; i++) {
		meanObjectPoints[i] = getMeanObjectPoint(i);
	}
	return meanObjectPoints;
}

ci::vec3 FACETRACKER::ciFaceTracker::getImagePoint(int i) const {
	if(failed) {
		return ci::vec3(0);
	}
	const cv::Mat& shape = tracker._shape;
	int n = shape.rows / 2;
	return ci::vec3( shape.db(i, 0),
                     shape.db(i + n, 0),
                    0.0) / vec3(rescale);
}

vec3 FACETRACKER::ciFaceTracker::getObjectPoint(int i) const {
	if(failed) {
		return vec3();
	}	
	int n = objectPoints.rows / 3;
	return vec3(objectPoints.db(i,0), objectPoints.db(i+n,0), objectPoints.db(i+n+n,0));
}

vec3 FACETRACKER::ciFaceTracker::getMeanObjectPoint(int i) const {
	const cv::Mat& mean = tracker._clm._pdm._M;
	int n = mean.rows / 3;
	return vec3(mean.db(i,0), mean.db(i+n,0), mean.db(i+n+n,0));
}

TriMesh FACETRACKER::ciFaceTracker::getImageMesh() const {
	return getMesh(getImagePoints());
}

TriMesh FACETRACKER::ciFaceTracker::getObjectMesh() const {
	return getMesh(getObjectPoints());
}

TriMesh FACETRACKER::ciFaceTracker::getMeanObjectMesh() const {
	return getMesh(getMeanObjectPoints());
}

TriMesh FACETRACKER::ciFaceTracker::getMesh(vector<vec3> points) const {
	TriMesh mesh;
	if(!failed) {
		int n = size();
		for(int i = 0; i < n; i++) {
            vec3 tPt = points[i];
			mesh.appendPosition( points[i] );
			mesh.appendTexCoord( vec2(getImagePoint(i)) );
		}
		addTriangleIndices(mesh);
	}
	return mesh;
}

const cv::Mat& FACETRACKER::ciFaceTracker::getObjectPointsMat() const {
	return objectPoints;
}

vec2 FACETRACKER::ciFaceTracker::getPosition() const {
	const cv::Mat& pose = tracker._clm._pglobl;
	return vec2(pose.db(4,0), pose.db(5,0)) / vec2(rescale);
}

 // multiply by ~20-23 to get pixel units (+/-20 units in the x axis, +23/-18 on the y axis)
float FACETRACKER::ciFaceTracker::getScale() const {
	const cv::Mat& pose = tracker._clm._pglobl;
	return pose.db(0,0) / rescale;
}

vec3 FACETRACKER::ciFaceTracker::getOrientation() const {
	const cv::Mat& pose = tracker._clm._pglobl;
	vec3 euler(pose.db(1, 0), pose.db(2, 0), pose.db(3, 0));
	return euler;
}

mat4 FACETRACKER::ciFaceTracker::getRotationMatrix() const {
	vec3 euler = getOrientation();
    
	mat4 matrix = glm::orientate4(euler); //mat4::createRotation(euler);
    return matrix;
}

FACETRACKER::ciFaceTracker::Direction FACETRACKER::ciFaceTracker::getDirection() const {
	if(failed) {
		return FACING_UNKNOWN;
	}
	switch(currentView) {
		case 0: return FACING_FORWARD;
		case 1: return FACING_LEFT;
		case 2: return FACING_RIGHT;
	}
    return FACING_UNKNOWN;
}

Path2d FACETRACKER::ciFaceTracker::getImageFeature(Feature feature) const {
	return getFeature(feature, getImagePoints());
}

Path2d FACETRACKER::ciFaceTracker::getObjectFeature(Feature feature) const {
	return getFeature(feature, getObjectPoints());
}

Path2d FACETRACKER::ciFaceTracker::getMeanObjectFeature(Feature feature) const {
	return getFeature(feature, getMeanObjectPoints());
}

Path2d FACETRACKER::ciFaceTracker::getFeature(Feature feature, vector<vec3> points) const {
	Path2d polyline;
	if(!failed) {
		vector<int> indices = getFeatureIndices(feature);
		for(int i = 0; i < indices.size(); i++) {
			int cur = indices[i];
			if(useInvisible || getVisibility(cur)) {
                if(polyline.empty()) { polyline.moveTo( vec2(points[cur]) ); }
                else                 { polyline.lineTo( vec2(points[cur]) ); }
			}
		}
		switch(feature) {
			case LEFT_EYE:
			case RIGHT_EYE:
			case OUTER_MOUTH:
			case INNER_MOUTH:
			case FACE_OUTLINE:
				polyline.close();
                break;
            default:
                break;
		}
	}
	return polyline;
}

float FACETRACKER::ciFaceTracker::getGesture(Gesture gesture) const {
	if(failed) {
		return 0;
	}
	int start = 0, end = 0;
	switch(gesture) {
		// left to right of mouth
		case MOUTH_WIDTH: start = 48; end = 54; break;
		 // top to bottom of inner mouth
		case MOUTH_HEIGHT: start = 61; end = 64; break;
		// center of the eye to middle of eyebrow
		case LEFT_EYEBROW_HEIGHT: start = 38; end = 20; break;
		// center of the eye to middle of eyebrow
		case RIGHT_EYEBROW_HEIGHT: start = 43; end = 23; break;
		// upper inner eye to lower outer eye
		case LEFT_EYE_OPENNESS: start = 38; end = 41; break;
		// upper inner eye to lower outer eye
		case RIGHT_EYE_OPENNESS: start = 43; end = 46; break;
		// nose center to chin center
		case JAW_OPENNESS: start = 33; end = 8; break;
		// left side of nose to right side of nose
		case NOSTRIL_FLARE: start = 31; end = 35; break;
	}
	
    return length( getObjectPoint(start) - getObjectPoint(end) );
}

void FACETRACKER::ciFaceTracker::setRescale(float rescale) {
	this->rescale = rescale;
}

void FACETRACKER::ciFaceTracker::setIterations(int iterations) {
	this->iterations = iterations;
}

void FACETRACKER::ciFaceTracker::setClamp(float clamp) {
	this->clamp = clamp;
}

void FACETRACKER::ciFaceTracker::setTolerance(float tolerance) {
	this->tolerance = tolerance;
}

void FACETRACKER::ciFaceTracker::setAttempts(int attempts) {
	this->attempts = attempts;
}

void FACETRACKER::ciFaceTracker::setUseInvisible(bool useInvisible) {
	this->useInvisible = useInvisible;
}

void FACETRACKER::ciFaceTracker::updateObjectPoints() {
	const cv::Mat& mean = tracker._clm._pdm._M;
	const cv::Mat& variation = tracker._clm._pdm._V;
	const cv::Mat& weights = tracker._clm._plocal;
	objectPoints = mean + variation * weights;
}

void FACETRACKER::ciFaceTracker::addTriangleIndices(TriMesh& mesh) const {
	for(int i = 0; i < tri.rows; i++) {
		int i0 = tri.it(i, 0), i1 = tri.it(i, 1), i2 = tri.it(i, 2);
		bool visible = getVisibility(i0) && getVisibility(i1) && getVisibility(i2);
		if(useInvisible || visible) {
            mesh.appendTriangle( i0, i1, i2 );
		}
	}
}
