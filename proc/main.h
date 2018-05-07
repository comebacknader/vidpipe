//
//  main.h
//  cs160
//
//  Created by Vishan Menon on 5/3/18.
//  Copyright © 2018 Vishan Menon. All rights reserved.
//

#ifndef PROCESS_OPENCVDRAWING_H
#define PROCESS_OPENCVDRAWING_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/image_io.h>

#define NUM_THREADS (std::thread::hardware_concurrency())
#define NUM_LANDMARKS 68

using namespace dlib;
using namespace std;

typedef struct landmark_data {
   dlib::point points[NUM_LANDMARKS];
} landmark_data;

typedef struct face_landmark_node { // Used for drawing in openCV
   int frame;
   int indice;
   float x;
   float y;
   struct face_landmark_node *next;
} face_landmark_node;

typedef struct thread_data {

   int thread_id;
   int start_idx;
   int end_idx;
   char** images;
   int numImages;
   landmark_data *landmarkData;
} thread_data;

face_landmark_node * add_face_landmark_element(face_landmark_node *face_landmark_list_head, int frame, int indice, float pixel_location_x, float pixel_location_y);
face_landmark_node * buildLandmarkListThread(landmark_data landmarks, face_landmark_node* face_landmark_list_head, int frame);
void drawDelauneyTrianglesThread(landmark_data* landmarks, char** fileNames, int size);
static void run (face_landmark_node *face_landmark_list_head, char* fileName);
static void draw_delaunay (cv::Mat &img, cv::Subdiv2D &subdiv, cv::Scalar delaunay_color);
static void draw_point (cv::Mat &img, cv::Point2f fp, cv::Scalar color);
void *getLandmarksThread(void *threadArg);

#endif //PROCESS_OPENCVDRAWING_H1