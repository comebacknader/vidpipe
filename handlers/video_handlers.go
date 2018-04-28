package handlers

import (
	b64 "encoding/base64"
	"encoding/json"
	"fmt"
	"github.com/comebacknader/vidpipe/config"
	_ "github.com/comebacknader/vidpipe/models"
	"github.com/julienschmidt/httprouter"
	"io/ioutil"
	"net/http"
	"os/exec"
	"strconv"
	"strings"
)

type Video struct {
	VideoEnc string `json:"video,omitempty"`
	Username string `json:"username,omitempty"`
}

type MetaData struct {
	Username string
	Frames   int
	Rate     int
	HznRes   int
	VertRes  int
	VidId    string
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
	vidEnc, _ := b64.StdEncoding.DecodeString(vid.VideoEnc)

	pathVid := vidUrl + usrnm
	err := ioutil.WriteFile(pathVid, vidEnc, 0644)
	if err != nil {
		fmt.Println("Error writing to file : " + err.Error())
		return
	}
	// Extract # of frames in video
	cmd := exec.Command("ffprobe", "-v", "error", "-count_frames", "-select_streams", "v:0", "-show_entries", "stream=nb_read_frames", "-of", "default=nokey=1:noprint_wrappers=1", pathVid)
	//cmd := exec.Command("ffmpeg", "-h")
	cmdOut, err := cmd.Output()
	if err != nil {
		fmt.Println("Error: ", err)
		return
	}
	fmt.Println(string(cmdOut))
	num, err := strconv.Atoi(strings.TrimSuffix(string(cmdOut), "\n"))
	if err != nil {
		fmt.Println("Error for strconv.Atoi(cmdOut): " + err.Error())
		return
	}
	fmt.Println(num)
	// Extract frame rate of video
	// ffprobe -v 0 -of csv=p=0 -select_streams 0 -show_entries stream=r_frame_rate infile
	frmRteCmd := exec.Command("ffprobe", "-v", "0", "-of", "csv=p=0", "-select_streams", "0", "-show_entries", "stream=r_frame_rate", pathVid)
	frmRteOut, err := frmRteCmd.Output()
	if err != nil {
		fmt.Println("Error for frmRate: " + err.Error())
		return
	}
	fmt.Println(string(frmRteOut))
	arrFrmRate := strings.Split(string(frmRteOut), "/")
	numerator, _ := strconv.Atoi(arrFrmRate[0])
	//denominator, _ := strconv.Atoi(arrFrmRate[1])
	var frameRate int
	if numerator < 0 {
		frameRate = numerator
	} else {
		frameRate = numerator
	}

	fmt.Println(frameRate)
	// Extract horizontal & vertical resolution
	// ffprobe -v error -select_streams v:0 -show_entries stream=height,width -of csv=s=x:p=0
	resolution := exec.Command("ffprobe", "-v", "error", "-select_streams", "v:0", "-show_entries", "stream=height,width", "-of", "csv=s=x:p=0", pathVid)
	resOut, err := resolution.Output()
	if err != nil {
		fmt.Println("Error getting Resolution: " + err.Error())
		return
	}
	fmt.Println("resOut: " + string(resOut))
	resArr := strings.Split(string(resOut), "x")
	hzn := resArr[0]
	hznRes, _ := strconv.Atoi(hzn)
	vert := strings.TrimSuffix(strings.Trim(resArr[1], " "), "\n")
	vertRes, err := strconv.Atoi(vert)
	if err != nil {
		fmt.Println("Error strconv.Atoi(vert): " + err.Error())
		return
	}
	fmt.Println(hznRes)
	fmt.Println(vertRes)
	fmt.Println("vert: " + vert)
	// Get Video ID (?) -- I'll just use username
	// Store metadata in DB
	metaData := MetaData{}
	metaData.Username = usrnm
	metaData.Frames = num
	metaData.Rate = frameRate
	metaData.HznRes = hznRes
	metaData.VertRes = vertRes
	metaData.VidId = metaData.Username
	AddMetaData(metaData)
	// Extract still images to a directory
	fmt.Println("End Upload Video Handler")
	return
}

func AddMetaData(metaData MetaData) error {
	_, err := config.DB.
		Exec("INSERT INTO metadata (username, frames, fps, width, height, vidid) VALUES ($1, $2, $3, $4, $5, $6)",
			metaData.Username, metaData.Frames, metaData.Rate, metaData.HznRes, metaData.VertRes, metaData.VidId)
	if err != nil {
		panic(err)
	}
	return nil
}
