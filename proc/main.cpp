/*
 * 
 * Main application for recording recording data points,
 * tracking pupils, and drawing delauney triangles.
 *  Author: Nader Carun, Vishan Menon
 * 
 */
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/opencv/cv_image.h>
#include <dlib/image_io.h>
#include <dlib/dir_nav.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "libpq-fe.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <dlib/opencv.h>

#include "findEyeCenter.h"
#include "eyelike.h"
#include "constants.h"


using namespace dlib;
using namespace std;
using namespace cv;

std::vector<dlib::file> allFiles;

static void exit_nicely(PGconn *conn) {
   PQfinish(conn);
   exit(1);
}

// Prints out the psql rows/cols to help debug if necessary
static void printRes(PGresult * res) {
    int nFields, i, j;
    /* first, print out the attribute names */
    nFields = PQnfields(res);
    for (i = 0; i < nFields; i++)
        printf("%-15s", PQfname(res, i));
    printf("\n\n");

    /* next, print out the rows */
    for (i = 0; i < PQntuples(res); i++)
    {
        for (j = 0; j < nFields; j++)
            printf("%-15s", PQgetvalue(res, i, j));
        printf("\n");
    }
}


int main(int argc, char **argv) 
{
  // Postgres DB information
  const char *conninfo;
  PGconn     *conn;
  PGresult   *res;
  int nFields, i, j;

  conninfo = "dbname = vidpipe";
   /* Make a connection to the database */
  conn = PQconnectdb(conninfo);
  /* Check to see that the backend connection was successfully made */
  if (PQstatus(conn) != CONNECTION_OK)
  {
      fprintf(stderr, "Connection to database failed: %s",
              PQerrorMessage(conn));
      cout << "Ran failure...";
      exit_nicely(conn);
  }


  cout << "Connected to the Database." << endl;

  // argv[1] == [username] passed in as first argument 
  string username = argv[1];

  cout << username << endl;
  string selStr = "select * from metadata where vidid='";
  string endSel = "';";
  string queryStr =  selStr + username + endSel;

  res = PQexec(conn, queryStr.c_str());
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
      fprintf(stderr, "Querying metadata failed: %s", PQerrorMessage(conn));
      PQclear(res);
      exit_nicely(conn);
  }
  //printRes(res);
  char *numFramesStr = PQgetvalue(res,0,1);
  int numFramesInt = std::stoi(numFramesStr);
  cout << numFramesStr << endl;
  // Called when response (res) no longer needed
  PQclear(res);

  // https://www.learnopencv.com/speeding-up-dlib-facial-landmark-detector/

  string imgDir = "/home/nadercarun/go/src/github.com/comebacknader/vidpipe/assets/vid/" + username + "_stills/";
  string outputDir = "/home/nadercarun/go/src/github.com/comebacknader/vidpipe/assets/vid/" + username + "_output/";
  string imgDir2 = "./" + username + "_stills";
  frontal_face_detector detector = get_frontal_face_detector();
  shape_predictor sp;
  deserialize("/home/nadercarun/go/src/github.com/comebacknader/vidpipe/proc/shape_predictor_68_face_landmarks.dat") >> sp;  

  std::vector<full_object_detection> shapes;

  // Find out number of still images that get created. 
  allFiles = get_files_in_directory_tree(imgDir, match_ending(".jpg"), 0);
  int numStillImgs = allFiles.size();
  cout << "numStillImgs: " << numStillImgs << endl;

  string fullImgPath;
  string fileNumStr;

  landmark_data *ildmrks;
  ildmrks = new landmark_data[numStillImgs + 1];  

  cv::Rect face;
  cv::Mat faceROI;

  std::vector<cv::Mat> rgbChannels(3);
  std::vector<dlib::rectangle> faces;
  // Iterate over each filename and grab each file by it's number. 
  for (i = 1; i <= numStillImgs; i++) {
   
    //cout << "Processing Image " << i << endl;
   
    fileNumStr = std::to_string(i);
    fullImgPath = imgDir + username + "." + fileNumStr + ".jpg";    // to compute its execution duration in runtime

    cv::Mat frame_gray;
    cv::Mat im;    
    
    im = imread(fullImgPath);
    cv::split(im, rgbChannels);
    frame_gray = rgbChannels[2];  

    //cv::resize(im, im_small, cv::Size(), 1.0/4, 1.0/4); 
    //cv_image<bgr_pixel> cimg_small(im_small);
    //cv_image<bgr_pixel> cimg(im);

    // Converts the cv image into a dlib image  
    dlib::cv_image<unsigned char> img(frame_gray);
    
    if ( i == 1 || (i % 100 == 0)) {
      faces = detector(img);
    }
    // std::vector<dlib::rectangle> faces = detector(img);
    //cout << "Improved Detector Time:" << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;
   
    //startTime = clock();    

    for (unsigned long k = 0; k < faces.size(); k++) 
    {
      // dlib::rectangle r(
      //   (long)(faces[k].left() * 4),
      //   (long)(faces[k].top() * 4),
      //   (long)(faces[k].right() * 4),
      //   (long)(faces[k].bottom() * 4)        
      //   );

      // Make sure that the coordinates are not below 0. 
      if (faces[k].left() < 1) 
      {
          faces[k].set_left(1);
      }
      if (faces[k].width() < 1 || faces[k].left() + faces[k].width() >= im.size().width) 
      {
          faces[k].set_right(im.size().width - 2);
      }
      if (faces[k].top() < 1) 
      {
          faces[k].set_top(1);
      }
      if (faces[k].height() < 1 || faces[k].top() + faces[k].height() >= im.size().height) 
      {
          faces[k].set_bottom(im.size().height - 2);
      }

      full_object_detection shape = sp(img, faces[k]);

      // 70 == NUMBER_OF_LANDMARKS
      for (int p = 0; p < 70 - 2; p++)
      {
          ildmrks[i].points[p] = shape.part(p);
      }

      shapes.push_back(shape);

      // 2D points.
      std::vector<cv::Point2d> img_pts;
      img_pts.push_back(toCv(shape.part(33))); // Nose tip
      img_pts.push_back(toCv(shape.part(8)));  // Chin
      img_pts.push_back(toCv(shape.part(45))); // Left eye left corner
      img_pts.push_back(toCv(shape.part(36))); // Right eye right corner
      img_pts.push_back(toCv(shape.part(54))); // Left Mouth corner
      img_pts.push_back(toCv(shape.part(48))); // Right mouth corner

      // 3D points.
      std::vector<cv::Point3d> mod_pts;
      mod_pts.push_back(cv::Point3d(0.0f, 0.0f, 0.0f));          // Nose tip
      mod_pts.push_back(cv::Point3d(0.0f, -330.0f, -65.0f));     // Chin
      mod_pts.push_back(cv::Point3d(-225.0f, 170.0f, -135.0f));  // Left eye left corner
      mod_pts.push_back(cv::Point3d(225.0f, 170.0f, -135.0f));   // Right eye right corner
      mod_pts.push_back(cv::Point3d(-150.0f, -150.0f, -125.0f)); // Left Mouth corner
      mod_pts.push_back(cv::Point3d(150.0f, -150.0f, -125.0f));  // Right mouth corner

      // Camera internals
      double focal_length = im.cols; // Approximate focal length.
      cv::Point2d center = cv::Point2d(im.cols / 2, im.rows / 2);
      cv::Mat camera_matrix = (cv::Mat_<double>(3, 3) << focal_length, 0, center.x, 0, focal_length, center.y, 0, 0, 1);
      cv::Mat dist_coeffs = cv::Mat::zeros(4, 1, cv::DataType<double>::type); // Assuming no lens distortion

      // Output rotation and translation
      cv::Mat rotation_vector; // Rotation in axis-angle form
      cv::Mat translation_vector;

      // Solve for pose
      // solvePnP will fill rotation_vector
      cv::solvePnP(mod_pts, img_pts, camera_matrix, dist_coeffs, rotation_vector, translation_vector,CV_ITERATIVE);

      cv::Matx33d rotation;
      cv::Rodrigues(rotation_vector, rotation);
      // Project a 3D point (0, 0, 1000.0) onto the image plane.
      // We use this to draw a line sticking out of the nose
      std::vector<cv::Point3d> nose_end_point3D;
      std::vector<cv::Point2d> nose_end_point2D;
      nose_end_point3D.push_back(cv::Point3d(0, 0, 1000.0));
      cv::projectPoints(nose_end_point3D, rotation_vector, translation_vector, camera_matrix, dist_coeffs, nose_end_point2D);
      
      // Draws the pose points  
      for (int i = 0; i < img_pts.size(); i++)
      {
          cv::circle(im, img_pts[i], 3, cv::Scalar(0, 0, 255), -1);
      }
      cv::line(im, img_pts[0], nose_end_point2D[0], cv::Scalar(255, 0, 0), 2);

      // EYE DETECTION

      face = cv::Rect(faces[k].left(), faces[k].top(), faces[k].width(), faces[k].height());
      faceROI = frame_gray(face);

      int leftLeft = ildmrks[i].points[36].x() - face.x;
      int leftTop = ildmrks[i].points[38].y() - face.y;
      
      int leftWidth = ildmrks[i].points[39].x() - ildmrks[i].points[36].x();
      
      if (leftWidth < 1) 
      { 
        leftWidth = 1; 
      }
      int leftHeight = ildmrks[i].points[41].y() - ildmrks[i].points[38].y();
      if (leftHeight < 1) 
      {
        leftHeight = 1;
      }

      cv::Rect leftEyeRegion(leftLeft, leftTop, leftWidth, leftHeight);
      int rightWidth = ildmrks[i].points[45].x() - ildmrks[i].points[42].x();
      if (rightWidth < 1) 
      {
          rightWidth = 1;
      }
      int rightHeight = ildmrks[i].points[46].y() - ildmrks[i].points[43].y();
      if (rightHeight < 1) 
      {
          rightHeight = 1;
      }
      cv::Rect rightEyeRegion(ildmrks[i].points[42].x() - face.x,
                              ildmrks[i].points[43].y() - face.y,
                              rightWidth,
                              rightHeight);

      //-- Find Eye Centers
      cv::Point leftPupil = findEyeCenter(faceROI, leftEyeRegion);
      leftPupil.x += leftEyeRegion.x + face.x;
      leftPupil.y += leftEyeRegion.y + face.y;

      cv::Point rightPupil = findEyeCenter(faceROI, rightEyeRegion);
      rightPupil.x += rightEyeRegion.x + face.x;
      rightPupil.y += rightEyeRegion.y + face.y;

      //-- Store Eye Centers in 68 and 69
      // Draw these at the end of drawDelTri() when iterating through landmark_data  
      ildmrks[i].points[70 - 2] = dlib::point(leftPupil.x, leftPupil.y);
      ildmrks[i].points[70 - 1] = dlib::point(rightPupil.x, rightPupil.y);      
    }
    //cout << "Shape Predicting:" << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;

    // Draw Delaunay Triangles 

    face_landmark_node *face_landmark_list_head;
    face_landmark_list_head = NULL;
    face_landmark_list_head = load_face_landmark_data(ildmrks[i], face_landmark_list_head, i);
    

    face_landmark_node *face_landmark_element;
    cv::Scalar delaunay_color(255,0,0), points_color(0, 255, 0); // Note: delaunay_color and points_color are in BGR (BLUE, GREEN, RED) format
    cv::Mat srcImg;
    cv::Size source_image_resolution;    

    srcImg = rgbChannels[2];
    if (!srcImg.empty())
    {
        source_image_resolution = srcImg.size();
        cv::Rect rect(0, 0, source_image_resolution.width, source_image_resolution.height);
        cv::Subdiv2D subdiv(rect);


        face_landmark_element = face_landmark_list_head;
        
        int count = 0;
        float xCoord, yCoord;
        while (count < (70 - 2) && face_landmark_element != NULL)
        {
            xCoord = face_landmark_element->x;
            yCoord = face_landmark_element->y;
            

            if(xCoord >= source_image_resolution.width)
                xCoord = source_image_resolution.width - 1;
            if(yCoord >= source_image_resolution.height)
                yCoord = source_image_resolution.height - 1;

            ++count;
            subdiv.insert(cv::Point2f(xCoord, yCoord));
            face_landmark_element = face_landmark_element->next;
        }
        
        draw_delaunay(srcImg, subdiv, delaunay_color);
        face_landmark_element = face_landmark_list_head;
        
        count = 0;
        while (count < (70 - 2) && face_landmark_element != NULL)
        {
            ++count;
            draw_point(srcImg, cv::Point2f(face_landmark_element->x, face_landmark_element->y), points_color);
            face_landmark_element = face_landmark_element->next;
        }

        circle(srcImg, cv::Point2f(face_landmark_element->x, face_landmark_element->y), 5, 1234, 4);
        face_landmark_element = face_landmark_element->next;
        circle(srcImg, cv::Point2f(face_landmark_element->x, face_landmark_element->y), 5, 1234, 4);

        //scout << "writing to " << output_filename << endl;
        cv::merge(rgbChannels, srcImg);
        // Write out the image in /vids/comebacknader_drawn/comebacknader.[still #].jpg
        imwrite(outputDir + "/" + username + "." + to_string(i) + ".jpg", srcImg);
    }
    // END ANALYZING/DRAWING ONE IMAGE 
  }
  cout << "End Analyzing Frames." << endl;
  cout << "Closing Database." << endl;  
  // Close connection to DB and cleanup
  PQfinish(conn);
  delete [] ildmrks;
  cout << "Exiting App." << endl;
  return 0;      
}

