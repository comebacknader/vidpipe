import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';

@Injectable()
export class AuthService {
	signupURL = 'http://localhost:8080/api/signup/new';
	loginURL = 'http://localhost:8080/api/login';
	logoutURL = 'http://localhost:8080/api/logout';

	constructor(private http: HttpClient) {}

	signupUser(data: object) {
		return this.http.post(this.signupURL, data);
	}

	loginUser(data: object) {
		return this.http.post(this.loginURL, data);
	}

	logoutUser() {
		return this.http.post(this.logoutURL, null);
	}

}