package handlers

import (
	"database/sql"
	"fmt"
	"github.com/comebacknader/vidpipe/config"
	"github.com/comebacknader/vidpipe/models"
	"github.com/julienschmidt/httprouter"
	"github.com/satori/go.uuid"
	"golang.org/x/crypto/bcrypt"
	"html/template"
	"net/http"
	_ "os"
	"time"
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

// PostSignup signs up a user.
func PostSignup(w http.ResponseWriter, r *http.Request, ps httprouter.Params) {
	if r.Method != http.MethodPost {
		http.Error(w, http.StatusText(405), http.StatusMethodNotAllowed)
		return
	}

	usr := models.User{}
	usr.Username = r.FormValue("username")
	usr.Firstname = r.FormValue("firstname")
	usr.Lastname = r.FormValue("lastname")
	usr.Hash = r.FormValue("password")
	usr.LastLogin = time.Now().UTC()
	usr.Ip = r.RemoteAddr
	fmt.Println("usr.IP: " + usr.Ip)

	seshData := SessionData{}

	valErr := ValidateUserFields(w, usr, seshData)
	if valErr == 0 {
		return
	}

	doesNameExist := models.CheckUserName(usr.Username)
	if doesNameExist == true {
		seshData.Error = append(seshData.Error, "Username already taken.")
		w.WriteHeader(400)
		err := tpl.ExecuteTemplate(w, "signup.gohtml", seshData)
		config.HandleError(w, err)
		return
	}

	// Encrypt the password using Bcrypt
	hashPass, err := bcrypt.GenerateFromPassword([]byte(usr.Hash), bcrypt.MinCost)

	if err != nil {
		http.Error(w, http.StatusText(500), http.StatusInternalServerError)
		return
	}

	usr.Hash = string(hashPass[:])

	err = models.PostUser(usr)

	switch {
	case err == sql.ErrNoRows:
		http.NotFound(w, r)
		return
	case err != nil:
		http.Error(w, http.StatusText(500), http.StatusInternalServerError)
		return
	}

	user, _ := models.GetUserByName(usr.Username)

	sID, _ := uuid.NewV4()
	cookie := &http.Cookie{
		Name:     "session",
		Value:    sID.String(),
		Path:     "/",
		Expires:  time.Now().UTC().Add(time.Hour * 24),
		MaxAge:   86400,
		Secure:   false,
		HttpOnly: true,
	}
	http.SetCookie(w, cookie)

	activeTime := time.Now().UTC()

	models.CreateSession(user.ID, sID.String(), activeTime, usr.Ip)

	http.Redirect(w, r, "/", http.StatusSeeOther)
	return
}

// PostLogin logs a user in.
func PostLogin(w http.ResponseWriter, r *http.Request, ps httprouter.Params) {
	if r.Method != http.MethodPost {
		http.Error(w, http.StatusText(405), http.StatusMethodNotAllowed)
		return
	}

	cred := r.FormValue("username")
	password := r.FormValue("password")

	// Check if User submitted Username or Email
	var user models.User
	var exist bool
	seshData := SessionData{}

	user, exist = models.GetUserByName(cred)
	if exist == false {
		seshData.Error = append(seshData.Error, "Sorry, that username doesn't exist.")
		w.WriteHeader(400)
		err := tpl.ExecuteTemplate(w, "login.gohtml", seshData)
		config.HandleError(w, err)
		return
	}

	// Compare user submitted password to password in DB
	err := bcrypt.CompareHashAndPassword([]byte(user.Hash), []byte(password))
	if err != nil {
		seshData.Error = append(seshData.Error, "Username and/or password do not match.")
		w.WriteHeader(400)
		err := tpl.ExecuteTemplate(w, "login.gohtml", seshData)
		config.HandleError(w, err)
		return
	}

	// Create Session and store it in Cookie and DB
	sID, _ := uuid.NewV4()
	activeTime := time.Now().UTC()

	models.CreateSession(user.ID, sID.String(), activeTime, user.Ip)

	cookie := &http.Cookie{
		Name:     "session",
		Value:    sID.String(),
		Expires:  time.Now().UTC().Add(time.Hour * 24),
		MaxAge:   86400,
		Secure:   false,
		HttpOnly: true,
	}
	http.SetCookie(w, cookie)
	http.Redirect(w, r, "/", http.StatusSeeOther)
	return

}

// User Logout
func PostLogout(w http.ResponseWriter, r *http.Request, ps httprouter.Params) {
	if r.Method != http.MethodPost {
		http.Error(w, http.StatusText(405), http.StatusMethodNotAllowed)
		return
	}

	cookie, err := r.Cookie("session")
	if err != nil {
		http.Redirect(w, r, "/", http.StatusSeeOther)
		return
	}
	expCook := &http.Cookie{
		Name:     "session",
		Value:    "",
		Path:     "/",
		Expires:  time.Now().UTC(),
		MaxAge:   -1,
		Secure:   false,
		HttpOnly: true,
	}
	http.SetCookie(w, expCook)
	models.DelSessionByUUID(cookie.Value)
	http.Redirect(w, r, "/", http.StatusSeeOther)
}

// ValidateUserFields is a helper function to validate User fields.
// return 1 is error
// return 0 is non-error
func ValidateUserFields(w http.ResponseWriter, usr models.User, errors SessionData) int {
	// errors := CredErrors{}
	// errors.Error = "Username cannot be blank"
	if usr.Username == "" {
		errors.Error = append(errors.Error, "Username cannot be blank")
		w.WriteHeader(400)
		err := tpl.ExecuteTemplate(w, "signup.gohtml", errors)
		config.HandleError(w, err)
		return 0
	}

	if len(usr.Username) < 6 || len(usr.Username) > 30 {
		errors.Error = append(errors.Error, "Username must be between 6 and 30 characters.")
		w.WriteHeader(400)
		err := tpl.ExecuteTemplate(w, "signup.gohtml", errors)
		config.HandleError(w, err)
		return 0
	}

	if usr.Hash == "" {
		errors.Error = append(errors.Error, "Password cannot be blank.")
		w.WriteHeader(400)
		err := tpl.ExecuteTemplate(w, "signup.gohtml", errors)
		config.HandleError(w, err)
		return 0
	}

	if len(usr.Hash) < 6 || len(usr.Hash) > 50 {
		errors.Error = append(errors.Error, "Password must be between 6 and 50 characters.")
		w.WriteHeader(400)
		err := tpl.ExecuteTemplate(w, "signup.gohtml", errors)
		config.HandleError(w, err)
		return 0
	}

	return 1
}
