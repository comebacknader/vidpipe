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

#include "eyelike.h"
#include "constants.h"
#include "findEyeCenter.h"

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
  cout << queryStr.c_str() << endl;  

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
  cv::Mat im_small, im_display;

  string imgDir = "/home/nadercarun/go/src/github.com/comebacknader/vidpipe/assets/vid/" + username + "_stills/";
  string imgDir2 = "./" + username + "_stills";
  frontal_face_detector detector = get_frontal_face_detector();
  shape_predictor sp;
  deserialize("shape_predictor_68_face_landmarks.dat") >> sp;  

  std::vector<dlib::rectangle> faces;
  std::vector<full_object_detection> shapes;  
  // Find out number of still images that get created. 
  allFiles = get_files_in_directory_tree(imgDir, match_ending(".jpg"), 0);
  int numStillImgs = allFiles.size();
  cout << "numStillImgs: " << numStillImgs << endl;

  string fullImgPath;
  string fileNumStr;
  // Iterate over each filename and grab each file by it's number. 
  for (i = 1; i <= numStillImgs; i++) {
    cout << "Processing Image " << i << endl;
   // clock_t startTime = clock();
    array2d<rgb_pixel> img;
    fileNumStr = std::to_string(i);
    fullImgPath = imgDir + username + "." + fileNumStr + ".jpg";    // to compute its execution duration in runtime
    //cout << "img instantiate:" << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;

    //startTime = clock();
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
      shapes.push_back(shape);
      cv::Rect cvFace = cv::Rect(cv::Point2i(r.left(), r.top()), cv::Point2i(r.right() + 1, r.bottom() + 1));
      openCVFaces.push_back(cvFace);     
    }
    //cout << "Shape Predicting:" << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;

    // Why will it be important to store information in the database. I need to associate a 68 datapoints to an image later when I want to draw it. 
    
    // EYE DETECTION 

    std::vector<TwoPupils> allEyes = getEyePupils(im, openCVFaces, shapes);

  }

  // Close connection to DB and cleanup
  PQfinish(conn);

  return 0;      
}

