/*
 ciFaceTracker provides an interface to Jason Saragih's FaceTracker library.
 Ported from Kyle McDonald's ofxFaceTracker by Patrick Hebron.
 
 getImagePoint()/getImageMesh() are in image space. This means that all the
 points will line up with the pixel coordinates of the image you fed into
 ciFaceTracker.
 
 getObjectPoint()/getObjectMesh() are in 3d object space. This is a product of
 the mean mesh with only the expression applied. There is no rotation or
 translation applied to the object space.
 
 getMeanObjectPoint()/getMeanObjectMesh() are also in 3d object space. However,
 there is no expression applied to the mesh.
 */

#pragma once

#include <sstream>

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Vector.h"
#include "cinder/TriMesh.h"

#include "Resources.h"
#include "CinderOpenCv.h"

#include "FaceTracker/Tracker.h"
#include "ExpressionClassifier.h"

namespace FACETRACKER {

    class ciFaceTracker {
    public:
        ciFaceTracker();
        void setup();
        virtual bool update(cv::Mat image);
        void draw(bool drawLabels = false) const;
        void reset();
        
        int size() const;
        bool getFound() const;
        int getAge() const;
        virtual bool getVisibility(int i) const;
        
        std::vector<ci::vec3> getImagePoints() const;
        std::vector<ci::vec3> getObjectPoints() const;
        std::vector<ci::vec3> getMeanObjectPoints() const;
        
        virtual ci::vec3 getImagePoint(int i) const;
        virtual ci::vec3 getObjectPoint(int i) const;
        virtual ci::vec3 getMeanObjectPoint(int i) const;
        
        TriMesh getImageMesh() const;
        TriMesh getObjectMesh() const;
        TriMesh getMeanObjectMesh() const;
        TriMesh getMesh(std::vector<ci::vec3> points) const;
        
        virtual const cv::Mat& getObjectPointsMat() const;
        
        virtual ci::vec2 getPosition() const;
        virtual float getScale() const;
        virtual ci::vec3 getOrientation() const;
        glm::mat4 getRotationMatrix() const;
        
        enum Direction {
            FACING_FORWARD,
            FACING_LEFT, FACING_RIGHT,
            FACING_UNKNOWN
        };
        Direction getDirection() const;
        
        enum Feature {
            LEFT_EYEBROW, RIGHT_EYEBROW,
            LEFT_EYE, RIGHT_EYE,
            LEFT_JAW, RIGHT_JAW, JAW,
            OUTER_MOUTH, INNER_MOUTH,
            NOSE_BRIDGE, NOSE_BASE,
            FACE_OUTLINE, ALL_FEATURES
        };
        Path2d getImageFeature(Feature feature) const;
        Path2d getObjectFeature(Feature feature) const;
        Path2d getMeanObjectFeature(Feature feature) const;
        
        enum Gesture {
            MOUTH_WIDTH, MOUTH_HEIGHT,
            LEFT_EYEBROW_HEIGHT, RIGHT_EYEBROW_HEIGHT,
            LEFT_EYE_OPENNESS, RIGHT_EYE_OPENNESS,
            JAW_OPENNESS,
            NOSTRIL_FLARE
        };
        float getGesture(Gesture gesture) const;
        
        void setRescale(float rescale);
        void setIterations(int iterations);
        void setClamp(float clamp);
        void setTolerance(float tolerance);
        void setAttempts(int attempts);
        void setUseInvisible(bool useInvisible);
        
    protected:
        void updateObjectPoints();
        void addTriangleIndices(TriMesh& mesh) const;
        static vector<int> getFeatureIndices(Feature feature);
        Path2d getFeature(Feature feature, vector<ci::vec3> points) const;
        
        bool        failed;
        int         age;
        int         currentView;
        
        bool        fcheck;
        double      rescale;
        int         frameSkip;
        
        vector<int> wSize1, wSize2;
        int         iterations;
        int         attempts;
        double      clamp, tolerance;
        bool        useInvisible;
        
        FACETRACKER::Tracker tracker;
        cv::Mat     tri, con;
        
        cv::Mat     im, gray;
        cv::Mat     objectPoints;
    };
}
