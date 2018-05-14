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
	"os"
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

	if vid.VideoEnc == "" {
		http.Error(w, http.StatusText(500), http.StatusInternalServerError)
		return
	}

	// Get the username
	usrnm := vid.Username
	// Store the video in the file system
	vidEnc, _ := b64.StdEncoding.DecodeString(vid.VideoEnc)

	pathVid := vidUrl + usrnm
	fileExt := ".mov"
	finalExt := ".mp4"
	bigFileName := pathVid + "_big" + fileExt
	err := ioutil.WriteFile(bigFileName, vidEnc, 0644)
	if err != nil {
		fmt.Println("Error writing to file : " + err.Error())
		return
	}

	// Transcode the video into a smaller resolution.
	// ffmpeg -y -i bigFileName -vf scale=480:-2,setsar=1:1 -c:v libx264 -crf 51 -preset ultrafast -b:v 512k -an pathVid+fileExt
	tran := exec.Command("ffmpeg", "-y", "-i", bigFileName, "-vf", "scale=480:-2,setsar=1:1", "-c:v", "libx264", "-crf", "40", "-preset", "ultrafast", "-b:v", "512k", "-an", pathVid+fileExt)
	_, err = tran.Output()
	if err != nil {
		fmt.Println("Error transcoding: " + err.Error())
		return
	}

	// Extract # of frames in video
	cmd := exec.Command("ffprobe", "-v", "error", "-count_frames", "-select_streams", "v:0", "-show_entries", "stream=nb_read_frames", "-of", "default=nokey=1:noprint_wrappers=1", pathVid+fileExt)
	//cmd := exec.Command("ffmpeg", "-h")
	cmdOut, err := cmd.Output()
	if err != nil {
		fmt.Println("Error: ", err)
		return
	}
	num, err := strconv.Atoi(strings.TrimSuffix(string(cmdOut), "\n"))
	if err != nil {
		fmt.Println("Error for strconv.Atoi(cmdOut): " + err.Error())
		return
	}
	// Extract frame rate of video
	// ffprobe -v 0 -of csv=p=0 -select_streams 0 -show_entries stream=r_frame_rate infile
	frmRteCmd := exec.Command("ffprobe", "-v", "0", "-of", "csv=p=0",
		"-select_streams", "V:0", "-show_entries", "stream=r_frame_rate", pathVid+fileExt)
	frmRteOutDrty, err := frmRteCmd.Output()
	if err != nil {
		fmt.Println("Error for frmRate: " + err.Error())
		return
	}
	frmRteOut := strings.Trim(string(frmRteOutDrty), " \n")
	arrFrmRate := strings.Split(frmRteOut, "/")
	numerator, _ := strconv.Atoi(arrFrmRate[0])
	denominator, _ := strconv.Atoi(arrFrmRate[1])
	var frameRate int
	if denominator <= 0 {
		fmt.Println("Error: frmRate denominator == 0")
		return
	} else {
		frameRate = numerator / denominator
	}

	// Extract horizontal & vertical resolution
	// ffprobe -v error -select_streams v:0 -show_entries stream=height,width -of csv=s=x:p=0
	resolution := exec.Command("ffprobe", "-v", "error", "-select_streams",
		"v:0", "-show_entries", "stream=height,width", "-of", "csv=s=x:p=0", pathVid+fileExt)
	resOutDirty, err := resolution.Output()
	if err != nil {
		fmt.Println("Error getting Resolution: " + err.Error())
		return
	}
	resOut := strings.Trim(string(resOutDirty), " \n")
	fmt.Println("resOut:", string(resOut))
	resArr := strings.Split(string(resOut), "x")
	hzn := resArr[0]
	hznRes, _ := strconv.Atoi(hzn)
	//vert := strings.Split(resArr[1], "\n")[0]
	vert := resArr[1]
	vertRes, err := strconv.Atoi(vert)
	if err != nil {
		fmt.Println("Error strconv.Atoi(vert): " + err.Error())
		return
	}

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
	stillPath := pathVid + "_stills/"
	outputDirPath := pathVid + "_output/"

	// Check if the folder exists

	// If it does, delete both still and output directories
	if _, err := os.Stat(stillPath); err == nil {
		// stillPath does exist
		// Deleting the folders
		delStillPath := exec.Command("rm", "-rf", stillPath)
		_, err = delStillPath.Output()
		if err != nil {
			fmt.Println("Error deleting still path: " + err.Error())
			return
		}
		delVidStillPath := exec.Command("rm", "-rf", outputDirPath)
		_, err = delVidStillPath.Output()
		if err != nil {
			fmt.Println("Error deleting drawn stills dir: " + err.Error())
			return
		}
	}
	// If it doesn't, mkdir
	if _, err := os.Stat(stillPath); os.IsNotExist(err) {
		// stillPath does not exist
		err = os.MkdirAll(stillPath, 0777)
		if err != nil {
			fmt.Println("Error making directory: " + err.Error())
			return
		}
		// make a directory for the drawn over stills
		err = os.MkdirAll(outputDirPath, 0777)
		if err != nil {
			fmt.Println("Error making output directory: " + err.Error())
			return
		}
	}

	outputPath := stillPath + usrnm + ".%d.jpg"
	fps := strconv.Itoa(metaData.Rate)
	//fpsInv := "1" + "\\" + strconv.Itoa(metaData.Rate)
	fmt.Println("fps: " + fps)
	extStill := exec.Command("ffmpeg", "-i", pathVid+fileExt,
		"-r", fps, outputPath)
	_, err = extStill.Output()
	if err != nil {
		fmt.Println("Error extracting to still images: " + err.Error())
		return
	}

	// Now I have to call the application to process the video

	execDraw := exec.Command("/home/nadercarun/go/src/github.com/comebacknader/vidpipe/proc/proc", usrnm)
	err = execDraw.Run()
	// If err == nil, then success.
	if err != nil {
		fmt.Println("Error calling processing application: " + err.Error())
		return
	}
	fmt.Println("Still images drawn in /comebacknader_output/")

	// Now I need to call ffMpeg to collate the still images together to make a movie.
	// Then I need to return a success status
	//ffmpeg -r 30 -start_number 1 -f image2 -i input_image_%d.png -c:v libx264 output.mp4
	//ffmpeg -r 29 -start_number 1 -f image2 -i /home/nadercarun/go/src/github.com/comebacknader/vidpipe/assets/vid/comebacknader_output/comebacknader.%d.jpg -c:v libx264 /home/nadercarun/go/src/github.com/comebacknader/vidpipe/assets/vid/comebacknader.mp4
	outVidPath := outputDirPath + usrnm + ".%d.jpg"
	execCollate := exec.Command("ffmpeg", "-y", "-r", fps,
		"-start_number", "1", "-f", "image2", "-i", outVidPath, "-c:v", "libx264", pathVid+finalExt)
	err = execCollate.Run()
	// If err == nil, then success.
	if err != nil {
		fmt.Println("Error collating the video from stills: " + err.Error())
		return
	}

	fmt.Println(usrnm + " uploaded a video!")
	w.WriteHeader(200)
	return
}

// AddMetaData adds a MetaData object to the DB.
func AddMetaData(metaData MetaData) error {
	// Delete the current metaData object
	_, err := config.DB.Exec(`DELETE from metadata where username = $1;`, metaData.Username)
	if err != nil {
		panic(err)
	}
	// Now, add the new metadata object
	_, err = config.DB.
		Exec("INSERT INTO metadata (username, frames, fps, width, height, vidid) VALUES ($1, $2, $3, $4, $5, $6)",
			metaData.Username, metaData.Frames, metaData.Rate, metaData.HznRes, metaData.VertRes, metaData.VidId)
	if err != nil {
		panic(err)
	}
	return nil
}
