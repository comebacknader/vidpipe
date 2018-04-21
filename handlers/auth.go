package handlers

import (
	"fmt"
	"github.com/comebacknader/vidpipe/models"
	"net/http"
)

// AlreadyLoggedIn determines whether the User is already logged in.
func AlreadyLoggedIn(req *http.Request) (interface{}, bool) {
	c, err := req.Cookie("session")
	if err != nil {
		fmt.Println("Cookie not set.!.")
		return nil, false
	}
	// Get a User ID from the cookie's value
	// Checks session database and returns User ID
	uid := models.GetUserIDByCookie(c.Value)
	if uid == 0 {
		return nil, false
	}
	// Make sure User exists with User ID
	exist := models.UserExistById(uid)

	// Update session's activity
	models.UpdateSessionActivity(c.Value)

	usrname, _ := models.GetUsernameById(uid)
	return usrname, exist
}
