import { Component, OnInit } from '@angular/core';
import { AuthService } from '../auth.service';
import { Router } from '@angular/router';

@Component({
  selector: 'app-watchvid',
  templateUrl: './watchvid.component.html',
  styleUrls: ['./watchvid.component.css']
})
export class WatchVidComponent implements OnInit {

  constructor(private auth: AuthService, private router: Router) { }

  ngOnInit() {
  	//this.auth.isAuthenticated();
  	setTimeout(()=>{ this.auth.isAuthenticated() }, 1000);
  }

}
