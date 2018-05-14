import { Component, OnInit } from '@angular/core';
import { AuthService } from '../auth.service';
import { Router } from '@angular/router';

@Component({
  selector: 'app-header',
  templateUrl: './header.component.html',
  styleUrls: ['./header.component.css']
})
export class HeaderComponent implements OnInit {
  loggedIn = false;
  constructor(private auth: AuthService, private router: Router) { }

  ngOnInit() {
    //setTimeout(()=>{ this.loggedIn = this.auth.isAuth; }, 1000);
    this.auth.isAuth$.subscribe((data) => {
      this.loggedIn = data;
    });
  }

  logout() {
  	this.auth.logoutUser().subscribe(
        data => {
          this.loggedIn = false; 
          this.auth.isAuth.next(false);
          this.router.navigate(['']);
        },
        error => console.log("Error: " + error)
        );
  }

}
