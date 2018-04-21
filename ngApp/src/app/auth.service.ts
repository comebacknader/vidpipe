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
	username = '';

	constructor(private http: HttpClient, private router: Router) {}

	signupUser(data: object) {
		return this.http.post(this.signupURL, data);
	}

	loginUser(data: object) {
		return this.http.post(this.loginURL, data);
	}

	logoutUser() {
		this.username = '';
		return this.http.post(this.logoutURL, null);
	}

	checkAuth() {
		this.http.get(this.isAuthURL).subscribe(data => {
			this.isAuth = true;
			this.username = data['username'];
		}, error => {
			this.isAuth = false;
		});		
	}

	isAuthenticated() {
		// Check the cookie, for a session
		//var cookie = this.getCookie("session");
		//if (cookie != "") return true; 
		if (this.isAuth) return true;
		this.router.navigate(['/']);
		return false;		
	}

	 getCookie(cname) {
	    var name = cname + "=";
	    var ca = document.cookie.split(';');
	    for(var i = 0; i < ca.length; i++) {
	        var c = ca[i];
	        while (c.charAt(0) == ' ') {
	            c = c.substring(1);
	        }
	        if (c.indexOf(name) == 0) {
	            return c.substring(name.length, c.length);
	        }
	    }
	    return "";
		}

	getUsername() {
		return this.username;
	}	

}