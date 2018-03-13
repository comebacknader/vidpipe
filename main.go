package main

import (
	"fmt"
	"github.com/julienschmidt/httprouter"
	"html/template"
	"log"
	"net/http"
)

var tpl *template.Template

func init() {
	tpl = template.Must(template.ParseGlob("templates/*"))
}

func main() {

	mux := httprouter.New()

	mux.GET("/", index)

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
