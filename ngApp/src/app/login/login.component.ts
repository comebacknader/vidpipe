import { Component, OnInit } from '@angular/core';
import { FormGroup, FormControl } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { AuthService } from '../auth.service';
import { Router } from '@angular/router';

@Component({
  selector: 'app-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.css']
})
export class LoginComponent implements OnInit {
	loginForm: FormGroup;

  constructor(private auth: AuthService, private router: Router) { }

  ngOnInit() {
  	this.loginForm = new FormGroup({
  		'username': new FormControl(null),
  		'password': new FormControl(null)
  	})
  }

  onSubmit() {
    this.auth.loginUser(this.loginForm.value)
      .subscribe(
        data => this.router.navigate(['watch']),
        error => console.log("Error: " + error)
        );
  }

}
