package backend;

import backend.GameInfo;
import backend.Screen;

function getBody(): UnicodeString {
  return GameInfo.ScreenInfo[GameRooms].body;
}

function getActions(): Array<ScreenAction> {
  return GameInfo.ScreenInfo[GameRooms].actions;
}

function printScreens(): Void {
  for (_ => info in GameInfo.ScreenInfo) {
    Sys.println(info.body);
    Sys.println("");
    for (action in info.actions) {
      Sys.println(action.action + ": " + action.title);
    }
    Sys.println("");
    Sys.println("");
  }
}
