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
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "libpq-fe.h"

using namespace dlib;
using namespace std;

static void exit_nicely(PGconn *conn) {
   PQfinish(conn);
   exit(1);
}

int main(int argc, char **argv) {
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
      std::cout << "Ran failure...";
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
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
      fprintf(stderr, "Querying metadata failed: %s", PQerrorMessage(conn));
      PQclear(res);
      exit_nicely(conn);
  }

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

    char *numFrames = PQgetvalue(res, 0,1);
    cout << numFrames << endl;

  // Called when response (res) no longer needed
   PQclear(res);

   // Close connection to DB and cleanup
   PQfinish(conn);

   return 0;      
}