import { Component, OnInit } from '@angular/core';
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

  constructor(private auth: AuthService, private http: HttpClient,
   private router: Router) { }

  ngOnInit() {
  	//this.auth.isAuthenticated();
  	// setTimeout(()=>{ this.auth.isAuthenticated() }, 1000);
  	this.uploadVidForm = new FormGroup({
  		'video': new FormControl(null),
      'username': new FormControl(this.auth.username)
  	});  	
  }

  onSubmit() {
    this.uploadVidForm.get('username').setValue(this.auth.username); 
    this.http.post('http://localhost:8080/api/upload', this.uploadVidForm.value).subscribe(
      data => console.log('Successful.'), error => console.log('Error')
      );
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
