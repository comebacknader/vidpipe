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
// struct TwoPupils {
// 	cv::Point leftPupil;
// 	cv::Point rightPupil;
// };

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


face_landmark_node * add_face_landmark_element (face_landmark_node *face_landmark_list_head, int frame, int indice, float pixel_location_x, float pixel_location_y);
face_landmark_node * load_face_landmark_data(landmark_data lmrks, face_landmark_node* face_landmark_list_head, int frame);

cv::Point2f toCv(const point& p);

void draw_point(cv::Mat &img, cv::Point2f fp, cv::Scalar color);
void draw_delaunay(cv::Mat &img, cv::Subdiv2D &subdiv, cv::Scalar delaunay_color);
