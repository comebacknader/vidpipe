package models

import (
	"database/sql"
	"github.com/comebacknader/vidpipe/config"
	"time"
)

type User struct {
	ID        int    `json:"id,omitempty"`
	Username  string `json:"username,omitempty"`
	Firstname string `json:"fname,omitempty"`
	Lastname  string `json:"lname,omitempty"`
	Hash      string `json:"password,omitempty"`
	LastLogin time.Time
	Ip        string
}

// UserExistById returns whether user exists by supplied user ID.
func UserExistById(uid int) bool {
	usr := User{}
	err := config.DB.
		QueryRow("SELECT ID FROM users WHERE ID = $1", uid).
		Scan(&usr.ID)
	if err != nil {
		if err == sql.ErrNoRows {
			return false
		}
		panic(err)
	}
	return true
}

// Check if a User exists with supplied Username
// Return true if exists, false otherwise
func CheckUserName(usr string) bool {
	user := User{}
	err := config.DB.QueryRow("SELECT id FROM users WHERE username = $1", usr).Scan(&user.Username)
	if err != nil {
		if err == sql.ErrNoRows {
			return false
		}
		panic(err)
	}
	return true
}

// Posts a User to the Database
func PostUser(usr User) error {
	_, err := config.DB.
		Exec("INSERT INTO users (username, firstname, lastname, password, lastlogin, ip) VALUES ($1, $2, $3, $4, $5, $6)",
			usr.Username, usr.Firstname, usr.Lastname, usr.Hash, usr.LastLogin, usr.Ip)
	if err != nil {
		panic(err)
	}
	return nil
}

// GetUserByName gets the user by their username. Return bool if exists.
func GetUserByName(username string) (User, bool) {
	usr := User{}
	err := config.DB.
		QueryRow("SELECT ID, username, firstname, lastname, password, lastlogin, ip FROM users WHERE username = $1", username).
		Scan(&usr.ID, &usr.Username, &usr.Firstname, &usr.Lastname, &usr.Hash, &usr.LastLogin, &usr.Ip)
	if err != nil {
		if err == sql.ErrNoRows {
			return usr, false
		}
		panic(err)
	}
	return usr, true
}

// GetUsernameById gets the user by their user ID. Return bool if exists.
func GetUsernameById(uid int) (string, bool) {
	usr := User{}
	err := config.DB.
		QueryRow("SELECT username FROM users WHERE ID = $1", uid).
		Scan(&usr.Username)
	if err != nil {
		if err == sql.ErrNoRows {
			return "", false
		}
		panic(err)
	}
	return (usr.Username), true
}
