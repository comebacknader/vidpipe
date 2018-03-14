package handlers

import (
	"database/sql"
	"github.com/comebacknader/vidpipe/config"
	"github.com/comebacknader/vidpipe/models"
	"github.com/julienschmidt/httprouter"
	"github.com/satori/go.uuid"
	"golang.org/x/crypto/bcrypt"
	"net/http"
	"os"
	"time"
)

// Data passed to all pages dealing with Sessions
type SessionData struct {
	UserCred UserStatus
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
		seshData.UserCred.IsLogIn = false
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
		seshData.UserCred.IsLogIn = false
		err := tpl.ExecuteTemplate(w, "signup.gohtml", seshData)
		config.HandleError(w, err)
	}
}
