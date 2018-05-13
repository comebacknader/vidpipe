package handlers

import (
	"database/sql"
	"encoding/json"
	"fmt"
	"github.com/comebacknader/vidpipe/config"
	"github.com/comebacknader/vidpipe/models"
	"github.com/julienschmidt/httprouter"
	"github.com/satori/go.uuid"
	"golang.org/x/crypto/bcrypt"
	"html/template"
	"net/http"
	"time"
)

var tpl *template.Template

func init() {
	tpl = config.Tpl
}

type LoginInfo struct {
	Username string `json:"username,omitempty"`
	Password string `json:"password,omitempty"`
}

// IsLoggedIn checks if User is signed in.
func IsLoggedIn(w http.ResponseWriter, r *http.Request, ps httprouter.Params) {
	if r.Method != http.MethodGet {
		http.Error(w, http.StatusText(405), http.StatusMethodNotAllowed)
		return
	}

	usrname, loggedIn := AlreadyLoggedIn(r)

	if loggedIn == false {
		http.Error(w, http.StatusText(405), http.StatusMethodNotAllowed)
		return
	}
	// Get the username from session value
	// Write the username back to the client in a json object
	lgInfo := LoginInfo{}
	lgInfo.Username = usrname.(string)
	fmt.Println("username: " + lgInfo.Username)
	json.NewEncoder(w).Encode(&lgInfo)
	return

}

// PostSignup signs up a user.
func PostSignup(w http.ResponseWriter, r *http.Request, ps httprouter.Params) {
	if r.Method != http.MethodPost {
		http.Error(w, http.StatusText(405), http.StatusMethodNotAllowed)
		return
	}

	// Extract from JSON request
	usr := models.User{}
	jerr := json.NewDecoder(r.Body).Decode(&usr)
	if jerr != nil {
		http.Error(w, http.StatusText(500), http.StatusInternalServerError)
		return
	}
	usr.LastLogin = time.Now().UTC()
	usr.Ip = r.RemoteAddr
	fmt.Println("usr.Username: " + usr.Username)
	fmt.Println("usr.IP: " + usr.Ip)

	valErr := ValidateUserFields(w, usr)
	if valErr == 0 {
		return
	}

	doesNameExist := models.CheckUserName(usr.Username)
	if doesNameExist == true {
		w.WriteHeader(400)
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

	// Create Session
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

	return
}

// PostLogin logs a user in.
func PostLogin(w http.ResponseWriter, r *http.Request, ps httprouter.Params) {
	if r.Method != http.MethodPost {
		http.Error(w, http.StatusText(405), http.StatusMethodNotAllowed)
		return
	}

	lgnInfo := LoginInfo{}
	jerr := json.NewDecoder(r.Body).Decode(&lgnInfo)
	if jerr != nil {
		http.Error(w, http.StatusText(500), http.StatusInternalServerError)
		return
	}

	// Check if User submitted Username or Email
	var user models.User
	var exist bool

	user, exist = models.GetUserByName(lgnInfo.Username)
	if exist == false {
		w.WriteHeader(400)
		return
	}

	// Compare user submitted password to password in DB
	err := bcrypt.CompareHashAndPassword([]byte(user.Hash),
		[]byte(lgnInfo.Password))
	if err != nil {
		w.WriteHeader(400)
		return
	}

	// Create Session and store it in Cookie and DB
	sID, _ := uuid.NewV4()
	activeTime := time.Now().UTC()

	models.CreateSession(user.ID, sID.String(), activeTime, user.Ip)

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
		w.WriteHeader(400)
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
	fmt.Println("Cookie: " + cookie.Value)
	models.DelSessionByUUID(cookie.Value)
	return
}

// ValidateUserFields is a helper function to validate User fields.
// return 1 is error
// return 0 is non-error
func ValidateUserFields(w http.ResponseWriter, usr models.User) int {

	if usr.Firstname == "" {
		w.WriteHeader(400)
		return 0
	}

	if usr.Lastname == "" {
		w.WriteHeader(400)
		return 0
	}

	if usr.Username == "" {
		w.WriteHeader(400)
		return 0
	}

	if len(usr.Username) < 6 || len(usr.Username) > 30 {
		w.WriteHeader(400)
		return 0
	}

	if usr.Hash == "" {
		w.WriteHeader(400)
		return 0
	}

	if len(usr.Hash) < 6 || len(usr.Hash) > 50 {
		w.WriteHeader(400)
		return 0
	}

	return 1
}
