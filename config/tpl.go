package config

import (
	"html/template"
	"log"
	"net/http"
)

var Tpl *template.Template

func init() {
	Tpl = template.Must(template.ParseFiles("ngApp/dist/index.html"))
}

// HandleError :  A generic error handler.
func HandleError(res http.ResponseWriter, err error) {
	if err != nil {
		http.Error(res, err.Error(), http.StatusInternalServerError)
		log.Fatalln(err)
	}
}
