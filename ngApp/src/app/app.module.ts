import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';


import { AppComponent } from './app.component';
import { HeaderComponent } from './header/header.component';
import { SplashComponent } from './splash/splash.component';
import { AppRouterModule } from './modules/app-router.module';
import { AuthComponent } from './auth/auth.component';
import { WatchVidComponent } from './watchvid/watchvid.component';
import { ListvidsComponent } from './listvids/listvids.component';


@NgModule({
  declarations: [
    AppComponent,
    HeaderComponent,
    SplashComponent,
    AuthComponent,
    WatchVidComponent,
    ListvidsComponent
  ],
  imports: [
    BrowserModule,
    AppRouterModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
