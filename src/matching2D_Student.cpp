#include <numeric>
#include "matching2D.hpp"

using namespace std;

// Find best matches for keypoints in two camera images based on several matching methods
void matchDescriptors(std::vector<cv::KeyPoint> &kPtsSource, std::vector<cv::KeyPoint> &kPtsRef, cv::Mat &descSource, cv::Mat &descRef,
                      std::vector<cv::DMatch> &matches, std::string descriptorType, std::string matcherType, std::string selectorType)
{
    // configure matcher
    bool crossCheck = false;
    cv::Ptr<cv::DescriptorMatcher> matcher;

    if (matcherType.compare("MAT_BF") == 0)
    {
        int normType = cv::NORM_HAMMING;
        if (descriptorType.compare("DES_HOG") == 0)
        {
            normType = cv::NORM_L2;

        }
        matcher = cv::BFMatcher::create(normType, crossCheck);
    }
    else if (matcherType.compare("MAT_FLANN") == 0)
    {
        if (descSource.type()!=CV_32F)
        {
            descSource.convertTo(descSource,CV_32F);
        }
         if (descRef.type()!=CV_32F)
        {
            descRef.convertTo(descRef,CV_32F);
        }
       //cv::FlannBasedMatcher matchers =cv::FlannBasedMatcher(cv::makePtr< cv::flann::LshIndexParams>(20,10,2));
        matcher=cv::FlannBasedMatcher::create();
        // ...
    }

    // perform matching task
    if (selectorType.compare("SEL_NN") == 0)
    { // nearest neighbor (best match)

        matcher->match(descSource, descRef, matches); // Finds the best match for each descriptor in desc1
    }
    else if (selectorType.compare("SEL_KNN") == 0)
    { // k nearest neighbors (k=2)
        std::vector< std::vector<cv::DMatch> > m;
        matcher->knnMatch(descSource, descRef, m,2);
        const float ratio=0.8;
        for (int i=0;i<static_cast<int>(m.size());i++)
        {
            if (!m[i].size())
            {
                continue;
            }
        if (m[i][0].distance < (ratio*m[i][1].distance) )
        {
            matches.push_back(m[i][0]);
        }
            //matches.push_back(m[i][0]);
        }

         
    }

}

// Use one of several types of state-of-art descriptors to uniquely identify keypoints
void descKeypoints(vector<cv::KeyPoint> &keypoints, cv::Mat &img, cv::Mat &descriptors, string descriptorType)
{
    // select appropriate descriptor
    cv::Ptr<cv::DescriptorExtractor> extractor;
    if (descriptorType.compare("BRISK") == 0)
    {

        int threshold = 30;        // FAST/AGAST detection threshold score.
        int octaves = 3;           // detection octaves (use 0 to do single scale)
        float patternScale = 1.0f; // apply this scale to the pattern used for sampling the neighbourhood of a keypoint.

        extractor = cv::BRISK::create(threshold, octaves, patternScale);
    }
    else if (descriptorType.compare("ORB") == 0)
    {
               int feature=500;
        float scale=1.2f;
        int nlevel=8;
        int edgeThreshold=31;
        int firstLevel=0;
        int WTA_K=2;
        cv::ORB::ScoreType scoreType=cv::ORB::HARRIS_SCORE;
        int patchSize=31;
        int fastThreshold=20;
        extractor = cv::ORB::create(feature,scale,nlevel,edgeThreshold,firstLevel,WTA_K,
        scoreType,patchSize,fastThreshold);
    }
    else if (descriptorType.compare("AKAZE") == 0)
    {
        cv::AKAZE::DescriptorType desc=cv::AKAZE::DESCRIPTOR_MLDB;
        int size=0;
        int channel=1;
        float threshold=0.001f;
        int octave=4;
        int octaveLayer=4;
        //cv::KAZE::DiffusivityType diff=cv::KAZE::DIFF_PM_G2;
        //extractor = cv::AKAZE::create(desc,size,channel,threshold,octave,octaveLayer,diff);
        extractor = cv::AKAZE::create();
    }  
    else if (descriptorType.compare("SIFT") == 0)
    {
        int feature=0;
        int octaveLayer=3;
        double constThresdhold=0.04;
        double edgeThreshold=10;
        double sigma=1.6;
        extractor = cv::xfeatures2d::SIFT::create(feature,octaveLayer,constThresdhold,edgeThreshold,sigma);
    } 
    else if (descriptorType.compare("BRIEF") == 0)
    {
        extractor = cv::xfeatures2d::BriefDescriptorExtractor::create(32,false);
    } 
    else if (descriptorType.compare("FREAK") == 0)
    {
        extractor = cv::xfeatures2d::FREAK::create(true,true,22.0,4);
    }
    else
    {
         //extractor== cv::SIFT::create();

        //...
    }

    // perform feature description
    double t = (double)cv::getTickCount();
    extractor->compute(img, keypoints, descriptors);
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << descriptorType << " descriptor extraction in " << 1000 * t / 1.0 << " ms" << endl;
}
void detKeypointsModern(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, std::string detectorType, bool bVis)
{
    double t = (double)cv::getTickCount();
    if (detectorType.compare("FAST") == 0)
    {
        cv::Ptr<cv::FastFeatureDetector> detector = cv::FastFeatureDetector::create();
        detector->detect(img,keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << "FAST detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;
    }
    else if (detectorType.compare("BRISK") == 0)
    {
        int threshold=30;
        int octave=3;
        float patternScale=1.0f;
        cv::Ptr<cv::BRISK> detector = cv::BRISK::create(threshold,octave,patternScale);
        detector->detect(img,keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << "BRISK detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;  
    }
      else if (detectorType.compare("ORB") == 0)
    {
        int feature=500;
        float scale=1.2f;
        int nlevel=8;
        int edgeThreshold=31;
        int firstLevel=0;
        int WTA_K=2;
        cv::ORB::ScoreType scoreType=cv::ORB::HARRIS_SCORE;
        int patchSize=31;
        int fastThreshold=20;
        cv::Ptr<cv::ORB> detector = cv::ORB::create(feature,scale,nlevel,edgeThreshold,firstLevel,
                   WTA_K, scoreType,patchSize,fastThreshold);
        detector->detect(img,keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << "ORB detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;  
    }
    else if (detectorType.compare("AKAZE") == 0)
    {

        cv::AKAZE::DescriptorType desc=cv::AKAZE::DESCRIPTOR_MLDB;
        int size=0;
        int channel=1;
        float threshold=0.001f;
        int octave=4;
        int octaveLayer=4;
        cv::KAZE::DiffusivityType diff=cv::KAZE::DIFF_PM_G2;
        //::Ptr<cv::AKAZE> detector = cv::AKAZE::create(desc,size,channel,threshold,octave,octaveLayer,diff);
        cv::Ptr<cv::AKAZE> detector = cv::AKAZE::create();
        detector->detect(img,keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << "AKAZE detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;  
    }
    else if (detectorType.compare("SIFT") == 0)
    {
        int feature=0;
        int octaveLayer=3;
        double constThresdhold=0.04;
        double edgeThreshold=10;
        double sigma=1.6;
        cv::Ptr<cv::xfeatures2d::SIFT> detector = cv::xfeatures2d::SIFT::create(feature,octaveLayer,constThresdhold,edgeThreshold,sigma);
        detector->detect(img,keypoints);
        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        cout << "SIFT detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;  
    }
    if(bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img,keypoints,visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "Shi-Tomasi Corner Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        cv::waitKey(0);
    }
}
// Detect keypoints in image using the traditional Shi-Thomasi detector
void detKeypointsShiTomasi(vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    // compute detector parameters based on image size
    int blockSize = 4;       //  size of an average block for computing a derivative covariation matrix over each pixel neighborhood
    double maxOverlap = 0.0; // max. permissible overlap between two features in %
    double minDistance = (1.0 - maxOverlap) * blockSize;
    int maxCorners = img.rows * img.cols / max(1.0, minDistance); // max. num. of keypoints

    double qualityLevel = 0.01; // minimal accepted quality of image corners
    double k = 0.04;

    // Apply corner detection
    double t = (double)cv::getTickCount();
    vector<cv::Point2f> corners;
    cv::goodFeaturesToTrack(img, corners, maxCorners, qualityLevel, minDistance, cv::Mat(), blockSize, false, k);

    // add corners to result vector
    for (auto it = corners.begin(); it != corners.end(); ++it)
    {

        cv::KeyPoint newKeyPoint;
        newKeyPoint.pt = cv::Point2f((*it).x, (*it).y);
        newKeyPoint.size = blockSize;
        keypoints.push_back(newKeyPoint);
    }
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "Shi-Tomasi detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "Shi-Tomasi Corner Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        cv::waitKey(0);
    }
}
void detKeypointsHarris(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{

// compute detector parameters based on image size
    int blockSize = 4;       //  size of an average block for computing a derivative covariation matrix over each pixel neighborhood
    double maxOverlap = 0.0; // max. permissible overlap between two features in %
    double minDistance = (1.0 - maxOverlap) * blockSize;
    int maxCorners = img.rows * img.cols / max(1.0, minDistance); // max. num. of keypoints

    double qualityLevel = 0.01; // minimal accepted quality of image corners
    double k = 0.04;

    // Apply corner detection
    double t = (double)cv::getTickCount();
    vector<cv::Point2f> corners;
    cv::goodFeaturesToTrack(img, corners, maxCorners, qualityLevel, minDistance, cv::Mat(), blockSize, true, k);

    // add corners to result vector
    for (auto it = corners.begin(); it != corners.end(); ++it)
    {

        cv::KeyPoint newKeyPoint;
        newKeyPoint.pt = cv::Point2f((*it).x, (*it).y);
        newKeyPoint.size = blockSize;
        keypoints.push_back(newKeyPoint);
    }
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "Harris detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "Harris Corner Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        cv::waitKey(0);
    }

}
