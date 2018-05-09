// EYELIKE EYE DETECTION FUNCTIONS

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>

#include <stdio.h>

using namespace std;
using namespace cv;
using namespace dlib;

// Every Pupil is a cv::point 
struct TwoPupils {
	cv::Point leftPupil;
	cv::Point rightPupil;
};

// findEyeCenter - returns a Pupil (cv::Point) object
cv::Point findEyeCenter(cv::Mat face, cv::Rect eye);

// findEyes - returns one TwoPupils object
TwoPupils findEyes(cv::Mat frame_gray, std::pair<cv::Rect, cv::Rect> regions);

// detectEyes - detects the eyes and returns vector<TwoPupils> 
std::vector<TwoPupils> detectEyes(cv::Mat &frame,
 																	const std::vector<cv::Rect> &allFaces, 
 																	std::vector<std::pair<cv::Rect, cv::Rect>> &regions);

// getEyePupils -- returns a vector<TwoPupils> returns detectEyes() 
std::vector<TwoPupils> getEyePupils(cv::Mat &frame,
																		const std::vector<cv::Rect> &allFaces,
																		const std::vector<full_object_detection> &allShapes);

// drawEye --> @input : vector<TwoPupils>
void drawEye(cv::Mat &image, const std::vector<TwoPupils> &allTwoPupils);

