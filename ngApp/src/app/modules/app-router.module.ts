import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { SplashComponent } from '../splash/splash.component';
import { AuthComponent } from '../auth/auth.component';
import { WatchVidComponent } from '../watchvid/watchvid.component'; 

const appRoutes: Routes = [
	{ path: '', component: SplashComponent },
	{ path: 'signup', component: AuthComponent },
	{ path: 'login', component: AuthComponent },
	{ path: 'watch', component: WatchVidComponent}	
];

@NgModule({
	imports: [RouterModule.forRoot(appRoutes)],
	exports: [RouterModule]
})

export class AppRouterModule {}