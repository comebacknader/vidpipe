package handlers

import (
	"encoding/json"
	"fmt"
	_ "github.com/comebacknader/vidpipe/models"
	"github.com/julienschmidt/httprouter"
	"net/http"
	"os/exec"
)

type Video struct {
	VideoEnc string `json:"video,omitempty"`
	Username string `json:"username,omitempty"`
}

// PostSignup signs up a user.
func UploadVid(w http.ResponseWriter, r *http.Request, ps httprouter.Params) {
	vidUrl := "/home/nadercarun/go/src/github.com/comebacknader/vidpipe/assets/vid/"

	vid := Video{}
	jsonErr := json.NewDecoder(r.Body).Decode(&vid)
	if jsonErr != nil {
		http.Error(w, http.StatusText(500), http.StatusInternalServerError)
		return
	}
	// Get the username
	usrnm := vid.Username
	// Store the video in the file system

	// Extract # of frames in video
	cmd := exec.Command("ffprobe", "-v", "error", "-count_frames", "-select_streams", "v:0", "-show_entries", "stream=nb_read_frames", "-of", "default=nokey=1:noprint_wrappers=1", vidUrl+"movie.mp4")
	//cmd := exec.Command("ffmpeg", "-h")
	cmdOut, err := cmd.Output()
	if err != nil {
		fmt.Println("Error: ", err)
	}
	fmt.Println(string(cmdOut))
	// Extract frame rate of video
	// Extract horizontal resolution
	// Extract vertical resolution
	// Get username
	// Get Video ID (?)
	// Store metadata in DB
	// Extract still images to a directory
	fmt.Println("Upload Video Handler")
	return
}
