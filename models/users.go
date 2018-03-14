package models

import (
	_ "database/sql"
	_ "github.com/comebacknader/vidpipe/config"
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
