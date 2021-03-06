# ciFaceTracker 

### Compiling
This is an update for Cinder 0.9.0 of ciFaceTracker from [Hebali](https://github.com/Hebali/ciFaceTracker).  There are a couple of tricks to getting it to compile.  

1. The first is that FaceTracker wants to use opencv from /usr/local/ and _not_ Cinder-OpenCV.  I've decided to leave it like this in order to keep the original source of FaceTracker intact.

    The easiest way to install opencv is to use [Homebrew](http://brew.sh) on OSX.  At time of writing (7/28/2016) you can install opencv via the command line like so. 
    ```
    brew tap homebrew/science
    brew intall opencv
    ```

    In your "Header Search Path" you'll need to put the location of your opencv install.  If you installed via Homebrew, it will link it to your `/usr/local/include` folder.  If that doesn't work, try the full path which for me it was `/usr/local/Cellar/opencv/2.4.13/include`

2. If setting up from scratch, note that FaceTracker uses angled brackets , and needs to be included as a system path.  TinderBox should set this up correctly for you.  It should be placed in your "Header Search Path" instead of "User Header Search Path" as angled brackets will only work if XCode is treating it like a framework, and not an included library.


### Credits
FaceTracker is provided free for non-commercial use. For commercial use of FaceTracker, please [request a quote](http://facetracker.net/quote/).

Original Port - [Hebali](https://github.com/Hebali/ciFaceTracker)

Original ofxFaceTracker - [Kyle McDonald](https://github.com/kylemcdonald/ofxFaceTracker)

FaceTracker Lib by [Jason Saragih](http://jsaragih.org/) and [Kyle McDonald](http://kylemcdonald.net/) -  [FaceTracker](https://github.com/kylemcdonald/FaceTracker)
