package handlers

import (
	_ "database/sql"
	"github.com/comebacknader/vidpipe/config"
	_ "github.com/comebacknader/vidpipe/models"
	"github.com/julienschmidt/httprouter"
	_ "github.com/satori/go.uuid"
	_ "golang.org/x/crypto/bcrypt"
	"html/template"
	"net/http"
	_ "os"
	_ "time"
)

var tpl *template.Template

func init() {
	tpl = config.Tpl
}

// Data passed to all pages dealing with Sessions
type SessionData struct {
	IsLogIn  bool
	CurrUser string
	Error    []string
	Success  string
	Token    string
}

// GetLogin gets the login page.
func GetLogin(w http.ResponseWriter, r *http.Request, ps httprouter.Params) {
	loggedIn := AlreadyLoggedIn(r)
	if loggedIn == true {
		http.Redirect(w, r, "/", http.StatusSeeOther)
	} else {
		seshData := SessionData{}
		seshData.IsLogIn = false
		err := tpl.ExecuteTemplate(w, "login.gohtml", seshData)
		config.HandleError(w, err)
	}
	return
}

// GetSignup gets the signup page.
func GetSignup(w http.ResponseWriter, r *http.Request, ps httprouter.Params) {
	loggedIn := AlreadyLoggedIn(r)
	if loggedIn == true {
		http.Redirect(w, r, "/", http.StatusSeeOther)
	} else {
		seshData := SessionData{}
		seshData.IsLogIn = false
		err := tpl.ExecuteTemplate(w, "signup.gohtml", seshData)
		config.HandleError(w, err)
	}
}
