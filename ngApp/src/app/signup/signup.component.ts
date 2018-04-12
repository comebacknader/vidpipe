import { Component, OnInit } from '@angular/core';
import { FormGroup, FormControl } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { AuthService } from '../auth.service';
import { Router } from '@angular/router';

@Component({
  selector: 'app-signup',
  templateUrl: './signup.component.html',
  styleUrls: ['./signup.component.css']
})
export class SignupComponent implements OnInit {
  signupForm: FormGroup;

  constructor(private auth: AuthService, private router: Router) { }

  ngOnInit() {
  	this.signupForm = new FormGroup({
  		'fname': new FormControl(null),
  		'lname': new FormControl(null),
  		'username': new FormControl(null),
  		'password': new FormControl(null)
  	});
  }

  onSubmit() {
  	this.auth.signupUser(this.signupForm.value)
  		.subscribe(
  			data => { 
          this.auth.isAuth = true;
          this.router.navigate(['watch']);
        },
  			error => console.log("Error: " + error)
  			);
  }
}
