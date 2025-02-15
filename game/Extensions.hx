package game;

class Extension {
  private final rooms: Enum<Any>;
  private final actions: Enum<Any>;

  public function new(rooms: Enum<Any>, actions: Enum<Any>) {
    this.rooms = rooms;
    this.actions = actions;
  }
}
