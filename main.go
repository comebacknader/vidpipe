package main

import (
	"database/sql"
	"fmt"
	"github.com/comebacknader/vidpipe/config"
	"github.com/comebacknader/vidpipe/handlers"
	"github.com/comebacknader/vidpipe/models"
	"github.com/julienschmidt/httprouter"
	_ "github.com/lib/pq"
	"html/template"
	"log"
	"net/http"
)

var tpl *template.Template

func init() {
	tpl = config.Tpl
}

func main() {

	mux := httprouter.New()

	config.NewDB("postgres://" + os.Getenv("VIDPIPE_DB_U") +
		"@" + os.Getenv("VIDPIPE_HOST") + "/" + os.Getenv("VIDPIPE_DB_NAME") + "?sslmode=disable")

	mux.GET("/", index)

	// Handlers for Authentication
	mux.GET("/signup", handlers.GetSignup)
	mux.POST("/signup", handlers.PostSignup)
	mux.GET("/login", handlers.GetLogin)
	mux.POST("/login", handlers.PostLogin)
	mux.POST("/logout", handlers.PostLogout)

	// Serves the css files called by HTML files
	mux.ServeFiles("/assets/css/*filepath", http.Dir("assets/css/"))

	// Serves the javascript files called by HTML files
	mux.ServeFiles("/assets/js/*filepath", http.Dir("assets/js/"))

	// Serves the images called by HTML files
	mux.ServeFiles("/assets/img/*filepath", http.Dir("assets/img/"))

	fmt.Println("Listening on 8080")
	log.Fatal(http.ListenAndServe(":8080", mux))
}

// Serves the home page
func index(w http.ResponseWriter, req *http.Request, _ httprouter.Params) {
	err := tpl.ExecuteTemplate(w, "index.gohtml", nil)

	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		log.Fatalln(err)
	}

	return
}
