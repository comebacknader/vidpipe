/*
 * 
 *	Main application for recording recording data points,
 *	tracking pupils, and drawing delauney triangles.
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
  const char *conninfo;
  PGconn     *conn;
  PGresult   *res;

  char *username = argv[1];

  cout << username << endl ;

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

  res = PQexec(conn, "select * from users;");


  // Called when response (res) no longer needed
 	PQclear(res);

 	// Close connection to DB and cleanup
	PQfinish(conn);

  cout << "Ran success!" << endl;

	return 0;     	
}