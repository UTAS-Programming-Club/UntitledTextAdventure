package campaigns;

import backend.Campaign;
import backend.GameInfo;
import backend.coregame.CoreGame;
import extensions.rooms.Rooms;

final UntitledTextAdventure: Campaign = {
  mainMenu:   "Untitled text adventure game\n"
            + "----------------------------\n"
            + "By the UTAS Programming Club\n"
            + "\n"
            + "Currently unimplemented :(",

  extensions: [CoreGameExt, RoomsExt],

  initialScreen: MainMenu,
  gameScreen: GameRooms
};
