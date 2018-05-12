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


int main(int argc, char **argv) {
  TwoPupils two;
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

  // 1. Get the metadata -- need to get the # of frames it is 

  // Username passed in as first argument to application
  string username = argv[1];

  cout << username << endl;
  string selStr = "select * from metadata where vidid='";
  string endSel = "';";
  string queryStr =  selStr + username + endSel;
  // cout << queryStr.c_str() << endl;  

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

  // 2. NOW I need to record data points to each still 
  // image in /[username]_stills by creating post
  // https://www.learnopencv.com/speeding-up-dlib-facial-landmark-detector/
  cv::Mat im;
  cv::Mat im_small, im_display, im_copy;

  string imgDir = "/home/nadercarun/go/src/github.com/comebacknader/vidpipe/assets/vid/" + username + "_stills/";
  string imgDir2 = "./" + username + "_stills";
  frontal_face_detector detector = get_frontal_face_detector();
  shape_predictor sp;
  deserialize("shape_predictor_68_face_landmarks.dat") >> sp;  

  std::vector<rectangle> faces;
  std::vector<full_object_detection> shapes;  
  // Find out number of still images that get created. 
  allFiles = get_files_in_directory_tree(imgDir, match_ending(".jpg"), 0);
  int numStillImgs = allFiles.size();
  cout << "numStillImgs: " << numStillImgs << endl;

  string fullImgPath;
  string fileNumStr;

  landmark_data *ildmrks;
  ildmrks = new landmark_data[numStillImgs];  

  // Iterate over each filename and grab each file by it's number. 
  for (i = 1; i <= numStillImgs; i++) {
    cout << "Processing Image " << i << endl;
   // clock_t startTime = clock();
    array2d<rgb_pixel> img;
    fileNumStr = std::to_string(i);
    fullImgPath = imgDir + username + "." + fileNumStr + ".jpg";    // to compute its execution duration in runtime
    //cout << "img instantiate:" << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;

    //startTime = clock();
    // im --> is the Image 
    im = imread(fullImgPath);

    //cout << "Load Image:" << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;

    cv::resize(im, im_small, cv::Size(), 1.0/4, 1.0/4); 

    cv_image<bgr_pixel> cimg_small(im_small);
    cv_image<bgr_pixel> cimg(im);

    if ( i % 3 == 0 ) {
      faces = detector(cimg_small);      
    }
    //cout << "Improved Detector Time:" << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;
   
    //startTime = clock();    
  

    std::vector<cv::Rect> openCVFaces;

    for (unsigned long k = 0; k < faces.size(); k++) {
      dlib::rectangle r(
        (long)(faces[k].left() * 4),
        (long)(faces[k].top() * 4),
        (long)(faces[k].right() * 4),
        (long)(faces[k].bottom() * 4)        
        );

      full_object_detection shape = sp(cimg, r);

      // 70 == NUMBER_OF_LANDMARKS
      for (int k = 0; k < 70 - 2; k++)
      {
          ildmrks[i].points[k] = shape.part(k);
      }

      shapes.push_back(shape);
      cv::Rect cvFace = cv::Rect(cv::Point2i(r.left(), r.top()), cv::Point2i(r.right() + 1, r.bottom() + 1));
      openCVFaces.push_back(cvFace); 

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

      // faceROI is the image for pupil dectection
      face = cv::Rect(faces[j].left(), faces[j].top(), faces[j].width(), faces[j].height());
      faceROI = frame_gray(face);

      //-- Find eye regions and draw them
      int leftLeft = landmarkData[i].points[36].x() - face.x;
      int leftTop = landmarkData[i].points[38].y() - face.y;
      int leftWidth = landmarkData[i].points[39].x() - landmarkData[i].points[36].x();
      if (leftWidth < 1) { leftWidth = 1; }
      int leftHeight = landmarkData[i].points[41].y() - landmarkData[i].points[38].y();
      if (leftHeight < 1) {
        leftHeight = 1;
        cv::Rect leftEyeRegion(leftLeft, leftTop, leftWidth, leftHeight);
      }
      int rightWidth = landmarkData[i].points[45].x() - landmarkData[i].points[42].x();
      if (rightWidth < 1) {
          rightWidth = 1;
      }
      int rightHeight = landmarkData[i].points[46].y() - landmarkData[i].points[43].y();
      if (rightHeight < 1)
          rightHeight = 1;
      cv::Rect rightEyeRegion(landmarkData[i].points[42].x() - face.x,
                              landmarkData[i].points[43].y() - face.y,
                              rightWidth,
                              rightHeight);

      //-- Find Eye Centers
      cv::Point leftPupil = findEyeCenter(faceROI, leftEyeRegion, "Left Eye");
      leftPupil.x += leftEyeRegion.x + face.x;
      leftPupil.y += leftEyeRegion.y + face.y;

      cv::Point rightPupil = findEyeCenter(faceROI, rightEyeRegion, "Right Eye");
      rightPupil.x += rightEyeRegion.x + face.x;
      rightPupil.y += rightEyeRegion.y + face.y;

      //-- Store Eye Centers in 68 and 69
      // Draw these at the end of drawDelTri() when iterating through landmark_data  
      ildmrks[i].points[70 - 2] = dlib::point(leftPupil.x, leftPupil.y);
      ildmrks[i].points[70 - 1] = dlib::point(rightPupil.x, rightPupil.y);      
    }
    //cout << "Shape Predicting:" << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;

    // EYE DETECTION 
    // allEyes should contain all the (X,Y) coordinates for each pupil out of the faces detected for this image. 
    // std::vector<TwoPupils> allEyes = getEyePupils(im, openCVFaces, shapes);  
    // drawEyes(im, allEyes);
    
    // GET THE YAW, PITCH, & ROLL 
    // std::vector<TwoPupils> allHeads = getPoseEstimations(im, shapes);
    // drawPose(im, allHeads); 

    // DRAW DELAUNAY TRIANGLES 
    drawDelTri(ldmarks, shapes, im);


    // Write out the image in /vids/comebacknader_drawn/comebacknader_[frame #].jpg
    // imwrite(output_dir + "/" + video_id + "." + to_string(i) + IMAGE_TYPE, images[i]);
  }

  // Close connection to DB and cleanup
  PQfinish(conn);

  return 0;      
}

