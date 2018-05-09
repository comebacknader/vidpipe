#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>

#include <iostream>
#include <queue>
#include <stdio.h>
#include <math.h>

#include "eyelike.h"
#include "constants.h"
#include "findEyeCenter.h"

using namespace std;
using namespace cv;
using namespace dlib;


TwoPupils findEyes(cv::Mat frame, std::pair<cv::Rect, cv::Rect> &regions) {
    cv::Rect leftRegion = regions.first;
    cv::Rect rightRegion = regions.second;

    cv::Point leftPupil = findEyeCenter(frame, leftRegion);
    cv::Point rightPupil = findEyeCenter(frame, rightRegion);

    rightPupil.x += rightEyeRegion.x;
    rightPupil.y += rightEyeRegion.y;
    leftPupil.x += leftRegion.x;
    leftPupil.y += leftRegion.y;

    TwoPupils pupils;
    pupils.point1 = leftPupil;
    pupils.point2 = rightPupil;
    return p;
}

std::vector<TwoPupils> getEyePupils(cv::Mat &image, 
																		std::vector<cv::Rect> &openCVFaces, 
																		std::vector<full_object_detection &shapes) {
	
}
