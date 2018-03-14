package models

import (
	"database/sql"
	"github.com/comebacknader/vidpipe/config"
	"time"
)

type Session struct {
	ID     int
	UUID   string
	UserID int
}

// GetUserIDByCookie gets the user ID from a supplied Cookie.
func GetUserIDByCookie(cookie string) int {
	sesh := Session{}
	err := config.DB.QueryRow("SELECT user_id FROM sessions WHERE uuid = $1", cookie).
		Scan(&sesh.UserID)
	if err != nil {
		if err == sql.ErrNoRows {
			return 0
		}
		panic(err)
	}
	return sesh.UserID
}

// UpdateSessionActivity updates a session's activity.
func UpdateSessionActivity(uuid string) {
	newTime := time.Now().UTC()
	_, err := config.DB.
		Exec("UPDATE sessions SET logintime = $1 WHERE uuid = $2", newTime, uuid)
	if err != nil {
		panic(err)
	}
}
