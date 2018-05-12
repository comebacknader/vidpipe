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


// This returns the two pupil points I need, now I need the (X,Y) coordinates. Store it in the database.
// @regions: the left eye region and the right eye region, each one being a rectangle (a bounding box).
TwoPupils findEyes(cv::Mat frame, std::pair<cv::Rect, cv::Rect> &regions) {
    cv::Rect leftRegion = regions.first;
    cv::Rect rightRegion = regions.second;

    cv::Point leftPupil = findEyeCenter(frame, leftRegion);
    cv::Point rightPupil = findEyeCenter(frame, rightRegion);

    rightPupil.x += rightRegion.x;
    rightPupil.y += rightRegion.y;
    leftPupil.x += leftRegion.x;
    leftPupil.y += leftRegion.y;

    TwoPupils pupils;
    pupils.leftPupil = leftPupil;
    pupils.rightPupil = rightPupil;
    return pupils;
}

// Gets the Fabian Timm coordinates for left and right pupil.
std::vector<TwoPupils> getEyePupils(cv::Mat &image, 
																		std::vector<cv::Rect> &openCVFaces, 
																		std::vector<full_object_detection> &allShapes) {
	
	std::vector<pair<Rect, Rect>> allRegions;
	for (const auto& s : allShapes) {
		pair<cv::Rect, cv::Rect> region;
		// Left eye 
		int llx = s.part(36).x();
		int lly = s.part(37).y();
		int lrx = s.part(39).x();
		int lry = s.part(41).y();

		cv::Rect lEye(cv::Point2i(llx, lly), cv::Point2i(lrx, lry)); 

		// Right Eye 
		int rlx = s.part(42).x();
		int rly = s.part(43).y();
		int rrx = s.part(45).x();
		int rry = s.part(47).y();

		cv::Rect rEye(cv::Point2i(rlx, rly), cv::Point2i(rrx, rry));

		region = make_pair(lEye, rEye);
		allRegions.push_back(region);
	}

  std::vector<cv::Mat> rgbChannels(3);
  cv::split(image, rgbChannels);
  cv::Mat frame = rgbChannels[2];

  std::vector<TwoPupils> points;

  for (int j = 0; j < openCVFaces.size(); j++) {
  	points.push_back(findEyes(frame, allRegions[j]));
  }

  return points;
}

// Draws the eyes.
void drawEyes(cv::Mat &image, std::vector<TwoPupils> &allPupils) {
	for (int k = 0; k < allPupils.size(); k++) {
		cv::circle(image, allPupils[k].leftPupil, 3, 1234);
		cv::circle(image, allPupils[k].rightPupil, 3, 1234);
	}
}

// Get the head pose estimations. 
std::vector<TwoPupils>  getPoseEstimations(cv::Mat &frame,
																				 std::vector<dlib::full_object_detection> &allShapes) {
    
    std::vector<std::vector<cv::Point2d>> allFaces;
    // 3D Model 
    std::vector<cv::Point3d> model;
    model.push_back(cv::Point3d(0.0f, 0.0f, 0.0f));
    model.push_back(cv::Point3d(0.0f, -330.0f, -65.0f));
    model.push_back(cv::Point3d(-225.0f, 170.0f, -135.0f));
    model.push_back(cv::Point3d(225.0f, 170.0f, -135.0f));
    model.push_back(cv::Point3d(-150.0f, -150.0f, -125.0f));
    model.push_back(cv::Point3d(150.0f, -150.0f, -125.0f));

    double focalLength = frame.cols; 
    Point2d center = cv::Point2d(frame.cols / 2, frame.rows / 2);
    cv::Mat camMatrix;
    camMatrix = (cv::Mat_<double>(3, 3) << focalLength, 0, center.x, 0, focalLength, center.y, 0, 0, 1);
    cv::Mat dist_coeffs = cv::Mat::zeros(4, 1, cv::DataType<double>::type); 

    std::vector<TwoPupils> posePoints;

    for (int i = 0; i < allShapes.size(); i++) {
        allFaces.push_back(std::vector<cv::Point2d>(6));

        allFaces[i][0] = cv::Point2d(allShapes[i].part(30).x(), allShapes[i].part(30).y()); //Nose tip, 31
        allFaces[i][1] = cv::Point2d(allShapes[i].part(8).x(), allShapes[i].part(8).y());   //Chin, 9
        allFaces[i][2] = cv::Point2d(allShapes[i].part(36).x(), allShapes[i].part(36).y()); //Left eye left corner, 37
        allFaces[i][3] = cv::Point2d(allShapes[i].part(45).x(), allShapes[i].part(45).y()); //Right eye right corner, 46
        allFaces[i][4] = cv::Point2d(allShapes[i].part(48).x(), allShapes[i].part(48).y()); //Left mouth corner, 49
        allFaces[i][5] = cv::Point2d(allShapes[i].part(54).x(), allShapes[i].part(54).y()); //Right mouth corner, 55

        cv::Mat rotation_vector;
        cv::Mat translation_vector;

        cv::solvePnP(model, allFaces[i], camMatrix, dist_coeffs, rotation_vector, translation_vector, CV_ITERATIVE);

        std::vector<Point3d> nose_end_point3D;
        std::vector<Point2d> nose_end_point2D;
        nose_end_point3D.push_back(Point3d(0, 0, 500.0));

        cv::projectPoints(nose_end_point3D, rotation_vector, translation_vector, camMatrix, dist_coeffs, nose_end_point2D);

        TwoPupils pupils;
        pupils.leftPupil = allFaces[i][0];
        pupils.rightPupil = nose_end_point2D[0];
        posePoints.push_back(pupils);
    }
    return posePoints;
}


// Draw the pose. 
void drawPose(Mat &im, const std::vector<TwoPupils> &allPupils) {
    for (int k = 0; k < allPupils.size(); k++) {
        cv::line(im, allPupils[k].leftPupil, allPupils[k].rightPupil, cv::Scalar(0, 255, 0), 2);
    }	
}

void drawDelTri(std::vector<full_object_detection> &shapes, cv::Mat &im) {
  face_landmark_node *face_landmark_list_head, *face_landmark_element;

  face_landmark_list_head = NULL;
  face_landmark_list_head = load_face_landmark_data (face_landmark_list_head);
  run (face_landmark_list_head);
  while (face_landmark_list_head != NULL)
  {
    face_landmark_element = face_landmark_list_head;
    face_landmark_list_head = face_landmark_list_head->next;
    free (face_landmark_element);
    face_landmark_element = NULL;
  }


};

static void draw_point (Mat &img, Point2f fp, Scalar color)
{
  circle (img, fp, 3, color, CV_FILLED, CV_AA, 0);
};

 
static void draw_delaunay (Mat &img, Subdiv2D &subdiv, Scalar delaunay_color)
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
};

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


static void run (face_landmark_node *face_landmark_list_head)
{
  face_landmark_node *face_landmark_element;
  Scalar delaunay_color(255,0,0), points_color(0, 0, 255); // Note: delaunay_color and points_color are in BGR (BLUE, GREEN, RED) format
  Mat source_image;
  Size source_image_resolution;
  char input_filename[1280], output_filename[1280]; // 1024 bytes for path + 256 bytes for filename  = 1280 bytes.

  memset (&input_filename, 0, sizeof(input_filename) - 1);
  memset (&output_filename, 0, sizeof(input_filename) - 1);
  strncpy (&input_filename[0], "rob.png", sizeof(input_filename) - 1);
  snprintf (&output_filename[0], sizeof(output_filename) - 1,  "OUTPUT-%s", &input_filename[0]);
  if (input_filename[0] != '\0')
  {
    source_image = imread (&input_filename[0]);
    if (!source_image.empty())
    {
      source_image_resolution = source_image.size();
      Rect rect(0, 0, source_image_resolution.width, source_image_resolution.height);
      Subdiv2D subdiv(rect);

      face_landmark_element = face_landmark_list_head;
      while (face_landmark_element != NULL)
      {
        subdiv.insert(Point2f(face_landmark_element->x, face_landmark_element->y));
        face_landmark_element = face_landmark_element->next;
      }
      draw_delaunay (source_image, subdiv, delaunay_color);
      face_landmark_element = face_landmark_list_head;
      while (face_landmark_element != NULL)
      {
        draw_point (source_image, Point2f(face_landmark_element->x, face_landmark_element->y), points_color);
        face_landmark_element = face_landmark_element->next;
      }
      imwrite (&output_filename[0], source_image);
    }
  }
}
