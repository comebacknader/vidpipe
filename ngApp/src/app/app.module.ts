import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { ReactiveFormsModule } from '@angular/forms';
import { HttpClientModule } from '@angular/common/http';

import { AppComponent } from './app.component';
import { HeaderComponent } from './header/header.component';
import { SplashComponent } from './splash/splash.component';
import { AppRouterModule } from './modules/app-router.module';
import { WatchVidComponent } from './watchvid/watchvid.component';
import { ListvidsComponent } from './listvids/listvids.component';
import { SignupComponent } from './signup/signup.component';
import { LoginComponent } from './login/login.component';

import { AuthService } from './auth.service';


@NgModule({
  declarations: [
    AppComponent,
    HeaderComponent,
    SplashComponent,
    WatchVidComponent,
    ListvidsComponent,
    SignupComponent,
    LoginComponent
  ],
  imports: [
    BrowserModule,
    AppRouterModule,
    ReactiveFormsModule,
    HttpClientModule
  ],
  providers: [AuthService],
  bootstrap: [AppComponent]
})
export class AppModule { }
