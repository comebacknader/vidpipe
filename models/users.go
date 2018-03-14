package models

import (
	"database/sql"
	"github.com/comebacknader/vidpipe/config"
	_ "time"
)

type User struct {
	ID        int    `json:"id,omitempty"`
	Username  string `json:"username,omitempty"`
	Email     string `json:"email,omitempty"`
	Hash      string `json:"password,omitempty"`
	LastLogin string
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
