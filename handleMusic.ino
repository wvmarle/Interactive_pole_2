void initMusic() {
  mySoftwareSerial.begin(9600);
  Serial.println();
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  else {
    Serial.println(F("DFPlayer Mini online."));
  }
  delay (3000);                                             // power-up safety delay
  myDFPlayer.setTimeOut(500);                               // Set serial communication time out 500ms
  myDFPlayer.volume(musicVolume);                           // Set volume value (0~30).
//  myDFPlayer.loop(1);                                       // Loop the first mp3.
//  myDFPlayer.pause();                                       // But don't start playing just yet!
}

void handleMusic() {
  static bool handPresented;
//  if (bitRead(motionState, HAND_PRESENTED) && handPresented == false) {
  if (bitRead(motionState, SWIPE) && handPresented == false) {
    if (musicPlaying) {
      myDFPlayer.pause();
      musicPlaying = false;
      Serial.println("Hand presented: stop playing.");
    }
    else {
//      myDFPlayer.start();
      myDFPlayer.loop(1);
      musicPlaying = true;
      Serial.println("Hand presented: start playing.");
    }
  }
//  handPresented = bitRead(motionState, HAND_PRESENTED); // Remember the state.
  handPresented = bitRead(motionState, SWIPE); // Remember the state.
//  if (proximityDetected == false &&
//      musicPlaying == true) {
//    myDFPlayer.pause();
//    musicPlaying = false;
//    Serial.println("No proximity - pausing the music.");
//  }

}
