import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Router } from '@angular/router';

@Injectable()
export class AuthService {
	signupURL = 'http://localhost:8080/api/signup/new';
	loginURL = 'http://localhost:8080/api/login';
	logoutURL = 'http://localhost:8080/api/logout';
	isAuthURL = 'http://localhost:8080/api/isLoggedIn';
	isAuth = false;

	constructor(private http: HttpClient, private router: Router) {}

	signupUser(data: object) {
		return this.http.post(this.signupURL, data);
	}

	loginUser(data: object) {
		return this.http.post(this.loginURL, data);
	}

	logoutUser() {
		return this.http.post(this.logoutURL, null);
	}

	checkAuth() {
		this.http.get(this.isAuthURL).subscribe(data => {
			this.isAuth = true;
		}, error => {
			this.isAuth = false;
		});		
	}

	isAuthenticated() {
		// This is an async call
		if (this.isAuth) { return true; }
		this.router.navigate(['/']);
		return false;		
	}

}