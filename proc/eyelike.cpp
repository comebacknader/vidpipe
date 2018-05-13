#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>

#include <iostream>
#include <queue>
#include <stdio.h>
#include <math.h>

#include "findEyeCenter.h"
#include "eyelike.h"
#include "constants.h"

using namespace std;
using namespace cv;
using namespace dlib;

cv::Point2f toCv(const point& p)
{
    return cv::Point2f(p.x(), p.y());
}

void draw_point(cv::Mat &img, cv::Point2f fp, cv::Scalar color)
{
  circle (img, fp, 3, color, CV_FILLED, CV_AA, 0);
}

void draw_delaunay(cv::Mat &img, cv::Subdiv2D &subdiv, cv::Scalar delaunay_color)
{
  std::vector<cv::Vec6f> triangleList;
  subdiv.getTriangleList(triangleList);
  std::vector<cv::Point> pt(3);
  Size size = img.size();
  cv::Rect rect(0,0, size.width, size.height);
 
  for (size_t i = 0; i < triangleList.size(); i++)
  {
    Vec6f t = triangleList[i];
    pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
    pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
    pt[2] = Point(cvRound(t[4]), cvRound(t[5]));
         
    // Draw rectangles completely inside the image.
    if (rect.contains(pt[0]) && rect.contains(pt[1]) && rect.contains(pt[2]))
    {
      cv::line (img, pt[0], pt[1], delaunay_color, 1, CV_AA, 0);
      cv::line (img, pt[1], pt[2], delaunay_color, 1, CV_AA, 0);
      cv::line (img, pt[2], pt[0], delaunay_color, 1, CV_AA, 0);
    }
  }
}

face_landmark_node * add_face_landmark_element (face_landmark_node *face_landmark_list_head, int frame, int indice, float pixel_location_x, float pixel_location_y)
{
  face_landmark_node *new_face_landmark_element, *face_landmark_element, *previous_face_landmark_element;

  new_face_landmark_element = (face_landmark_node *) malloc (sizeof (face_landmark_node));
  if (new_face_landmark_element != NULL)
  {
    new_face_landmark_element->frame = frame;
    new_face_landmark_element->indice = indice;
    new_face_landmark_element->x = pixel_location_x;
    new_face_landmark_element->y = pixel_location_y;
    new_face_landmark_element->next = NULL;
    if (face_landmark_list_head != NULL)
    {
      face_landmark_element = face_landmark_list_head;
      while (face_landmark_element->next != NULL) 
      {
        face_landmark_element = face_landmark_element->next;
      }
      face_landmark_element->next = new_face_landmark_element;
    }
    else
    {
      face_landmark_list_head = new_face_landmark_element;
    }
  }
  return face_landmark_list_head;
}

face_landmark_node * load_face_landmark_data(landmark_data lmrks, face_landmark_node* face_landmark_list_head, int frame){
    for (int i = 0; i < NUMBER_OF_LANDMARKS; i++){
        face_landmark_list_head = add_face_landmark_element (face_landmark_list_head,
                                                             frame,
                                                             i,
                                                             lmrks.points[i].x(), lmrks.points[i].y());
    }
    return face_landmark_list_head;
}