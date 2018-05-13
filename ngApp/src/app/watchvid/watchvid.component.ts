import { Component, OnInit, ViewChild, ElementRef } from '@angular/core';
import { AuthService } from '../auth.service';
import { Router } from '@angular/router';
import { FormGroup, FormControl } from '@angular/forms';
import { HttpClient } from '@angular/common/http'; 

@Component({
  selector: 'app-watchvid',
  templateUrl: './watchvid.component.html',
  styleUrls: ['./watchvid.component.css']
})
export class WatchVidComponent implements OnInit {
	uploadVidForm: FormGroup;
  vidSrc = "assets/vid/movie.mp4";
  errExist = false;
  vidPending = false;
  vidFinished = false;
  @ViewChild("videoPlayer") video: ElementRef;  
  
  constructor(private auth: AuthService, private http: HttpClient,
   private router: Router) { }

  ngOnInit() {
  	//this.auth.isAuthenticated();
  	setTimeout(()=>{ this.auth.isAuthenticated() }, 1000);
  	this.uploadVidForm = new FormGroup({
  		'video': new FormControl(null),
      'username': new FormControl(this.auth.username)
  	});  	
  }

  onSubmit() {
    this.uploadVidForm.get('username').setValue(this.auth.username);
    this.vidPending = true; 
    this.http.post('http://localhost:8080/api/upload', this.uploadVidForm.value).subscribe(
      data => {
        // change the vidSrc
        // load() the video
        this.vidPending = false;
        this.vidFinished = true; 
        this.vidSrc = "assets/vid/" + this.auth.username + ".mp4";
        this.video.nativeElement.load();
        this.video.nativeElement.play();
        setTimeout(() => { this.vidFinished = false; }, 3000) 
        console.log('Successful.');
      }, error => {
        this.vidPending = false;
        this.errExist = true;
        setTimeout(() => { this.errExist = false; }, 4000);        
        console.log('Error');
      });
  }

  onFileChange(event) {
    let reader = new FileReader();
    if (event.target.files && event.target.files.length > 0) {
      let f = event.target.files[0];
      reader.readAsDataURL(f);
      reader.onload = () => {
        this.uploadVidForm.get('video').setValue(reader.result.split(',')[1]);
      }
    }
  }

}
