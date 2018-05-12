// EYELIKE EYE DETECTION FUNCTIONS

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>

#include <stdio.h>

#define NUMBER_OF_LANDMARKS 70

using namespace std;
using namespace cv;
using namespace dlib;

// Every Pupil is a cv::point 
struct TwoPupils {
	cv::Point leftPupil;
	cv::Point rightPupil;
};

typedef struct landmark_data {
    dlib::point points[NUMBER_OF_LANDMARKS];
} landmark_data;

typedef struct face_landmark_node 
{
  int frame;
  int indice;
  float x;
  float y;
  struct face_landmark_node *next;
} face_landmark_node;



// findEyes - returns one TwoPupils object
TwoPupils findEyes(cv::Mat frame_gray, std::pair<cv::Rect, cv::Rect> &regions);

// getEyePupils -- returns a vector<TwoPupils> returns detectEyes() 
std::vector<TwoPupils> getEyePupils(cv::Mat &frame,
																		std::vector<cv::Rect> &allFaces,
																		std::vector<dlib::full_object_detection> &allShapes);

// drawEye --> @input : vector<TwoPupils>
void drawEyes(cv::Mat &image, std::vector<TwoPupils> &allTwoPupils);

std::vector<TwoPupils> getPoseEstimations(cv::Mat &frame,
																				 std::vector<dlib::full_object_detection> &allShapes);

void drawPose(cv::Mat &im, const std::vector<TwoPupils> &heads);

void drawDelTri();

face_landmark_node * add_face_landmark_element (face_landmark_node *face_landmark_list_head, int frame, int indice, float pixel_location_x, float pixel_location_y);

static void run (face_landmark_node *face_landmark_list_head);
static void draw_delaunay (cv::Mat &img, cv::Subdiv2D &subdiv, cv::Scalar delaunay_color);
static void draw_point (cv::Mat &img, cv::Point2f fp, cv::Scalar color);
