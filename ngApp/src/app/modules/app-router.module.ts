import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { SplashComponent } from '../splash/splash.component';
import { SignupComponent } from '../signup/signup.component';
import { LoginComponent } from '../login/login.component';
import { WatchVidComponent } from '../watchvid/watchvid.component'; 

const appRoutes: Routes = [
	{ path: '', component: SplashComponent },
	{ path: 'signup', component: SignupComponent },
	{ path: 'login', component: LoginComponent },
	{ path: 'watch', component: WatchVidComponent}	
];

@NgModule({
	imports: [RouterModule.forRoot(appRoutes)],
	exports: [RouterModule]
})

export class AppRouterModule {}