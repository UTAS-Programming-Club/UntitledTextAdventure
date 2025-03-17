package extensions.trap;

enum Room {
  Trap;
}

class TrapRoom extends backend.Room {
  public var activatedTrap: Bool = false;
}
