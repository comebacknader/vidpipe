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
  errExist = false;

  constructor(private auth: AuthService, private router: Router) { }

  ngOnInit() {
  	this.loginForm = new FormGroup({
  		'username': new FormControl(null),
  		'password': new FormControl(null)
  	});
  }

  onSubmit() {
    this.auth.username = this.loginForm.value.username;
    this.auth.loginUser(this.loginForm.value)
      .subscribe(
        data => { 
          this.auth.isAuth.next(true);
          this.router.navigate(['watch']);
        },
        error =>  {
          this.errExist = true;
          setTimeout(() => {this.errExist=false;}, 4000);
          console.log("Error: " + error);
        });
  }

}
