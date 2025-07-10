package backend.coregame;

import backend.Game;
import backend.Room;

class UnusedRoom extends Room {
  function getRoomBody(Game): UnicodeString return '';
  function getMapSymbol(): UnicodeString return 'NO';
}

class EmptyRoom extends Room {
  function getRoomBody(Game): UnicodeString return '';
  function getMapSymbol(): UnicodeString return '';
}
