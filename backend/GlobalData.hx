package backend;

import backend.Entity;
import backend.Screen;

class GlobalData {
  // public static final rooms: Array<Array<Room>> = [
  //   [
  //   ]
  // ];

  public static final mainMenu = new Screen("This is a test");

  public static final enemyStats: Map<UnicodeString, EntityStats> = [
    "Demon" => new EntityStats(100, 100, 10, 10, 10, 10, 10)
  ];
}
