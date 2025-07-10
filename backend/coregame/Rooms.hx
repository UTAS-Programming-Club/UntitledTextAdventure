package backend.coregame;

import backend.Game;
import backend.Room;

class UnusedRoom extends Room {
  function getName(): UnicodeString return 'Unused';
  function getRoomBody(Game): UnicodeString return '';
  function getMapSymbol(): UnicodeString return '';
}

class EmptyRoom extends Room {
  function getName(): UnicodeString return 'Empty';
  function getRoomBody(Game): UnicodeString return '';
  function getMapSymbol(): UnicodeString return '';
}
