package main

import (
	_ "database/sql"
	"errors"
	"fmt"
	"github.com/comebacknader/vidpipe/config"
	"github.com/comebacknader/vidpipe/handlers"
	_ "github.com/comebacknader/vidpipe/models"
	"github.com/julienschmidt/httprouter"
	_ "github.com/lib/pq"
	"html/template"
	"log"
	"net/http"
	"os"
)

var tpl *template.Template

type Server struct {
	r *httprouter.Router
}

func init() {
	tpl = config.Tpl
}

func main() {

	mux := httprouter.New()

	config.NewDB("postgres://" + os.Getenv("VIDPIPE_DB_U") + ":" + os.Getenv("VIDPIPE_DB_P") +
		"@" + os.Getenv("VIDPIPE_HOST") + "/" + os.Getenv("VIDPIPE_DB_NAME") + "?sslmode=disable")

	mux.GET("/", index)
	mux.GET("/signup", index)
	mux.GET("/login", index)
	mux.GET("/watch", index)

	mux.POST("/api/signup/new", handlers.PostSignup)
	mux.POST("/api/login", handlers.PostLogin)
	mux.POST("/api/logout", handlers.PostLogout)
	mux.GET("/api/isLoggedIn", handlers.IsLoggedIn)
	mux.POST("/api/upload", handlers.UploadVid)

	// Serves the css files called by HTML files
	mux.ServeFiles("/assets/css/*filepath", http.Dir("ngApp/dist"))

	// Serves the javascript files called by HTML files
	mux.ServeFiles("/assets/js/*filepath", http.Dir("ngApp/dist"))

	// Serves the images called by HTML files
	mux.ServeFiles("/assets/img/*filepath", http.Dir("assets/img/"))

	// Serve video files
	mux.ServeFiles("/assets/vid/*filepath", http.Dir("assets/vid/"))

	mux.NotFound = http.RedirectHandler("/", 301)

	fmt.Println("Listening on 8080")
	log.Fatal(http.ListenAndServe(":8080", &Server{mux}))
}

// Serves the home page
func index(w http.ResponseWriter, req *http.Request, _ httprouter.Params) {
	err := tpl.ExecuteTemplate(w, "index.html", nil)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Fatalln(err)
	}

	return
}

// Sets up CORS for all requests
func (s *Server) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if origin := r.Header.Get("Origin"); origin != "" {
		w.Header().Set("Access-Control-Allow-Origin", origin)
		w.Header().Set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE")
		w.Header().Set("Access-Control-Allow-Headers",
			"Accept, Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization")
		w.Header().Set("Access-Control-Allow-Credentials",
			"true")
	}
	// Stop here if its Preflighted OPTIONS request
	if r.Method == "OPTIONS" {
		errors.New("Request method is OPTIONS")
	}
	s.r.ServeHTTP(w, r)
}
