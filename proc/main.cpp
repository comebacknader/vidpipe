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
#include "delaunay.h"
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
  //char *numFramesStr = PQgetvalue(res,0,1);
  //int numFramesInt = std::stoi(numFramesStr);
  //cout << numFramesStr << endl;
  
  // Called when response (res) no longer needed
  PQclear(res);

  // https://www.learnopencv.com/speeding-up-dlib-facial-landmark-detector/

  string imgDir = "/home/nadercarun/go/src/github.com/comebacknader/vidpipe/assets/vid/" + username + "_stills/";
  string outputDir = "/home/nadercarun/go/src/github.com/comebacknader/vidpipe/assets/vid/" + username + "_output/";
  string imgDir2 = "./" + username + "_stills";
  frontal_face_detector detector = get_frontal_face_detector();
  shape_predictor sp;
  deserialize("/home/nadercarun/go/src/github.com/comebacknader/vidpipe/proc/shape_predictor_68_face_landmarks.dat") >> sp;  

  //std::vector<full_object_detection> shapes;

  // Find out number of still images that get created. 
  allFiles = get_files_in_directory_tree(imgDir, match_ending(".jpg"), 0);
  int numStillImgs = allFiles.size();
  cout << "numStillImgs: " << numStillImgs << endl;

  string fullImgPath, fileNumStr;

  lmarkData *ildmrks = new lmarkData[numStillImgs + 1];  

  cv::Rect face;
  cv::Mat faceROI;

  std::vector<cv::Mat> rgbChannels(3);
  std::vector<dlib::rectangle> faces;

  // I need an array of 68 points 
  // Every point x,y will be converted into a string and stored in here
  // Then all the strings will be appended to a INSERT...ARRAY + ..here.. + ]);
  std::vector<string> allPoints(70);
  int skull_id;
  int pupilandof_id; 
  clock_t startTime = clock();
  // Iterate over each filename and grab each file by it's number. 
  for (i = 1; i <= numStillImgs; i++) {
    
    cout << "Processing Image " << i << endl;
    clock_t sTime;
    //sTime = clock();  
    fileNumStr = std::to_string(i);
    fullImgPath = imgDir + username + "." + fileNumStr + ".jpg";    // to compute its execution duration in runtime

    cv::Mat im, frame_gray, frame_gray_small;   
    
    im = imread(fullImgPath);
    cv::split(im, rgbChannels);
    frame_gray = rgbChannels[2];

    cv::resize(frame_gray, frame_gray_small, cv::Size(), 1.0/NUMBER_SCALE_IMAGE, 1.0/NUMBER_SCALE_IMAGE); 
  
    // Converts the cv image into a dlib image  
    dlib::cv_image<unsigned char> cimg_small(frame_gray_small);
    dlib::cv_image<unsigned char> cimg(frame_gray);

    bool skipFlag = true;

    if ( i == 1 || (i % SKIPPED_IMAGES == 0)) {
      skipFlag = false;
      // sTime = clock();
      faces = detector(cimg_small);
      // cout << "detector() took: " << double( clock() - sTime ) / (double)CLOCKS_PER_SEC << " seconds." << endl;
    }
    
    for (unsigned long k = 0; k < faces.size(); k++) {

      if (!skipFlag) {
        long recLeft = (faces[k].left() * NUMBER_SCALE_IMAGE);
        long recRight = (faces[k].right() * NUMBER_SCALE_IMAGE);
        long recTop = (faces[k].top() * NUMBER_SCALE_IMAGE);
        long recBottom = (faces[k].bottom() * NUMBER_SCALE_IMAGE);
        faces[k].set_left(recLeft);
        faces[k].set_right(recRight);     
        faces[k].set_top(recTop);
        faces[k].set_bottom(recBottom);

        // Verify that the face rectangles aren't too small or too large. 
        if (faces[k].left() < 1) { faces[k].set_left(1); }
        if (faces[k].width() < 1 || ((faces[k].left() + faces[k].width()) >= im.size().width)) 
        {
            faces[k].set_right(im.size().width - 2);
        }
        if (faces[k].top() < 1) { faces[k].set_top(1); }
        if (faces[k].height() < 1 || ((faces[k].top() + faces[k].height()) >= im.size().height)) 
        {
            faces[k].set_bottom(im.size().height - 2);
        }
      } 

      // Creating dlib rectangle scaled back up 4x. 
      dlib::rectangle r(
        faces[k].left(),
        faces[k].top(),
        faces[k].right(),
        faces[k].bottom()        
        );

      full_object_detection shape = sp(cimg, r);
      
      // 70 == NUMBER_OF_LANDMARKS + 2 Eye Points
      // 70 - 2 == 68 data points on face
      for (int p = 0; p < 68; p++) {
          ildmrks[i].points[p] = shape.part(p);
          //Each point p has an x() and a y() that I need to 
          // store in a vector of strings and store in the DB
          string x = std::to_string(ildmrks[i].points[p].x());
          string y = std::to_string(ildmrks[i].points[p].y());
          string point =  string("point(") + x + string(",") + y + string(")");
          allPoints[p] = point;
      }

      // 2D points.
      std::vector<cv::Point2d> img_pts;
      img_pts.push_back(toCVpoint(shape.part(33))); // Nose tip
      img_pts.push_back(toCVpoint(shape.part(8)));  // Chin
      img_pts.push_back(toCVpoint(shape.part(45))); // Left eye left corner
      img_pts.push_back(toCVpoint(shape.part(36))); // Right eye right corner
      img_pts.push_back(toCVpoint(shape.part(54))); // Left Mouth corner
      img_pts.push_back(toCVpoint(shape.part(48))); // Right mouth corner

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
      cv::Mat rotVec; // Rotation in axis-angle form
      cv::Mat translation_vector;

      // Solve for pose
      // solvePnP will fill rotVec
      cv::solvePnP(mod_pts, img_pts, camera_matrix, dist_coeffs, rotVec, translation_vector,CV_ITERATIVE);

      // Calculate Yaw, Pitch, and Roll 
      double pi = 3.14159265;
      double yaw, roll, pit;
      bool rotVecNotNeg1 = rotVec.at<double>(2, 0) != -1;
      bool rotVecNotPos1 = rotVec.at<double>(2, 0) != 1;
      bool rotVecNeg1 = rotVec.at<double>(2, 0) == -1; 
      if ( rotVecNotNeg1 && rotVecNotNeg1) {
          pit = -1 * sin(rotVec.at<double>(2, 0));
          roll = atan2(rotVec.at<double>(2, 1) / cos(pit), rotVec.at<double>(2, 2) / cos(pit));
          yaw = atan2(rotVec.at<double>(1, 0) / cos(pit), rotVec.at<double>(0, 0) / cos(pit));
      } else {
        yaw = 0;
        if(rotVecNeg1) {
            roll = yaw + atan2(rotVec.at<double>(0, 1), rotVec.at<double>(0, 2));
            pit = pi/2;
        } else {
            roll = (-1 * yaw) + atan2((rotVec.at<double>(0, 1) * -1), (rotVec.at<double>(0, 2) * -1));
            pit = pi/2;
        }
      }

      // Store yaw, pitch, and roll in DB associated with a framenum..or not. 
      const char *paramValues[3];
      char y[10];
      char p[10];
      char ro[10];
      sprintf(y, "%lf", yaw);
      sprintf(p, "%lf", pit);
      sprintf(ro, "%lf", roll);
      remove_newline_from_string(&y[0]);
      remove_newline_from_string(&p[0]);
      remove_newline_from_string(&ro[0]);
      paramValues[0] = &y[0];
      paramValues[1] = &p[0];
      paramValues[2] = &ro[0];
      res = PQexecParams (conn, "INSERT INTO skull (yaw, pitch, roll) VALUES ($1::real, $2::real, $3::real) RETURNING id", 
        3, NULL, paramValues, NULL, NULL, 0);
      if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cout << "INSERT INTO skull not successful." << endl;
        cout << "Error Msg: " << PQresultErrorMessage(res) << endl;
      } else {
        skull_id = std::stoi(PQgetvalue(res, 0, 0));
      }
      PQclear(res);

      // EYE DETECTION

      face = cv::Rect(faces[k].left(), faces[k].top(), faces[k].width(), faces[k].height());
      faceROI = frame_gray(face);

      int leftLeft = ildmrks[i].points[36].x() - face.x;
      int leftTop = ildmrks[i].points[38].y() - face.y;
      
      int leftWidth = ildmrks[i].points[39].x() - ildmrks[i].points[36].x();
      if (leftWidth < 1) { leftWidth = 1; }

      int leftHeight = ildmrks[i].points[41].y() - ildmrks[i].points[38].y();
      if (leftHeight < 1) { leftHeight = 1; }

      cv::Rect leftEyeRegion(leftLeft, leftTop, leftWidth, leftHeight);
      
      int rightWidth = ildmrks[i].points[45].x() - ildmrks[i].points[42].x();
      if (rightWidth < 1) { rightWidth = 1; }

      int rightHeight = ildmrks[i].points[46].y() - ildmrks[i].points[43].y();
      if (rightHeight < 1) { rightHeight = 1; }


      cv::Rect rightEyeRegion(ildmrks[i].points[42].x() - face.x,
                              ildmrks[i].points[43].y() - face.y,
                              rightWidth,
                              rightHeight);


      cv::Point lPupil = findEyeCenter(faceROI, leftEyeRegion);
      lPupil.x += leftEyeRegion.x + face.x;
      lPupil.y += leftEyeRegion.y + face.y;

      cv::Point rPupil = findEyeCenter(faceROI, rightEyeRegion);
      rPupil.x += rightEyeRegion.x + face.x;
      rPupil.y += rightEyeRegion.y + face.y;

      // The Left Pupil goes in ildmarks[68]
      ildmrks[i].points[70 - 2] = dlib::point(lPupil.x, lPupil.y);

      // The Right Pupil goes in ildmarks[69]
      ildmrks[i].points[70 - 1] = dlib::point(rPupil.x, rPupil.y);       
    }

      // Store the all points in pupilandof 
    string pointArrayVals = allPoints[0];
    // I need to iterate over allPoints 
    for (int q = 1; q < 68; q++) {
      pointArrayVals += string(",") + allPoints[q];
    };

    // Getting the pupil points
    string lpoopstr = string("point(") + std::to_string(ildmrks[i].points[68].x()) + string(",") + std::to_string(ildmrks[i].points[68].y()) + ")";
    string rpoopstr = string("point(") + std::to_string(ildmrks[i].points[69].x()) + string(",") + std::to_string(ildmrks[i].points[69].y()) + ")";
    string insertPupilStr = "INSERT INTO pupilandof (leftploc, rightploc, ofd) VALUES (" + lpoopstr + "," + rpoopstr + "," + "ARRAY[" + pointArrayVals +"]) RETURNING id";
    res = PQexecParams (conn, insertPupilStr.c_str(), 0, NULL, NULL, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
      cout << "INSERT INTO pupilandof not successful." << endl;
      cout << "Error Msg: " << PQresultErrorMessage(res) << endl;
    } else {
      pupilandof_id = std::stoi(PQgetvalue(res, 0, 0));
    }
    PQclear(res);

    string insertStillImg = "INSERT INTO stillimg (vidid, framenum, skull_id, pupilandof_id) VALUES (\'"+username+"\'::varchar," + std::to_string(i) + "," + std::to_string(skull_id) + "," + std::to_string(pupilandof_id)+")";
    res = PQexecParams (conn, insertStillImg.c_str(), 0, NULL, NULL, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      cout << "INSERT INTO stillimg not successful." << endl;
      cout << "Error Msg: " << PQresultErrorMessage(res) << endl;
    } 
    PQclear(res);

    // Draw Delaunay Triangles 

    face_landmark_node *face_landmark_list_head;
    face_landmark_list_head = NULL;
    face_landmark_list_head = load_face_landmark_data(ildmrks[i], face_landmark_list_head, i);
    
    face_landmark_node *face_landmark_element;
    
    cv::Scalar delaunayClr(255,255,255,255), ptsClr(0,0,0,255), eyeClr(255,0,0,255);
    cv::Mat srcImg = rgbChannels[2]; 
    cv::Size srcImgRes;    

    if (!srcImg.empty()) {
      srcImgRes = srcImg.size();
      cv::Rect rect(0, 0, srcImgRes.width, srcImgRes.height);
      cv::Subdiv2D subdiv(rect);

      face_landmark_element = face_landmark_list_head;
      
      int cnt = 0;
      float xCoord, yCoord;
      while (cnt < 68 && face_landmark_element != NULL) {
        xCoord = face_landmark_element->x;
        yCoord = face_landmark_element->y;

        if (xCoord >= srcImgRes.width) { xCoord = srcImgRes.width - 1; }
        if (yCoord >= srcImgRes.height) { yCoord = srcImgRes.height - 1; }

        // Save x,y coordinates to an array that's gonna be inserted into db

        subdiv.insert(cv::Point2f(xCoord, yCoord));
        face_landmark_element = face_landmark_element->next;
        cnt++;
      }
      
      draw_delaunay(srcImg, subdiv, delaunayClr);

      face_landmark_element = face_landmark_list_head;
      
      cnt = 0;
      while (cnt < 68 && face_landmark_element != NULL) {
        draw_point(srcImg, cv::Point2f(face_landmark_element->x, face_landmark_element->y), ptsClr);
        face_landmark_element = face_landmark_element->next;
        cnt++;
      }

      // Drawing the pupils 
      circle(srcImg, cv::Point2f(face_landmark_element->x, face_landmark_element->y), 5, eyeClr, 3);
      face_landmark_element = face_landmark_element->next;
      circle(srcImg, cv::Point2f(face_landmark_element->x, face_landmark_element->y), 5, eyeClr, 3);
      
      // Clean up the pointers
      while (face_landmark_list_head != NULL) {
        face_landmark_element = face_landmark_list_head;
        face_landmark_list_head = face_landmark_list_head->next;
        free (face_landmark_element);
        face_landmark_element = NULL;
      }
      cv::merge(rgbChannels, srcImg);
      // Write out the image in /vids/comebacknader_drawn/[username].[still #].jpg
      imwrite(outputDir + "/" + username + "." + to_string(i) + ".jpg", srcImg);
    }
    // cout << "processing one image took: " << double( clock() - sTime ) / (double)CLOCKS_PER_SEC << " seconds." << endl;
    // END ANALYZING/DRAWING ONE IMAGE 
  }
  cout << "Pocessing Time: " << double( clock() - startTime ) / (double)CLOCKS_PER_SEC << " seconds." << endl;
  cout << "End Analyzing Frames." << endl;
  cout << "Closing Database." << endl;  
  // Close connection to DB and cleanup
  PQfinish(conn);
  delete [] ildmrks;
  cout << "Exiting App." << endl;
  return 0;      
}

