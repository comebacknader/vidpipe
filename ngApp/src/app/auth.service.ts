import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Router } from '@angular/router';
import {Subject} from 'rxjs/Subject';
import {BehaviorSubject} from 'rxjs/BehaviorSubject';


@Injectable()
export class AuthService {
	signupURL = 'http://localhost:8080/api/signup/new';
	loginURL = 'http://localhost:8080/api/login';
	logoutURL = 'http://localhost:8080/api/logout';
	isAuthURL = 'http://localhost:8080/api/isLoggedIn';
	username = '';
	isAuth = new BehaviorSubject<boolean>(false);
	isAuth$ = this.isAuth.asObservable();

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
			this.isAuth.next(true);
			this.username = data['username'];
		}, error => {
			this.isAuth.next(false);
		});		
	}

	isAuthenticated() {
		// Check the cookie, for a session
		//var cookie = this.getCookie("session");
		//if (cookie != "") return true; 
		if (this.isAuth.getValue()) return true;
		this.router.navigate(['/']);
		return false;		
	}

	isLoggedIn() {
		if (this.isAuth) return true;
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