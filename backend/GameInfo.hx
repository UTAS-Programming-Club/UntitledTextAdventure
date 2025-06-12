package backend;

import backend.Action;
import backend.Equipment;
import backend.Outcome;
import backend.Room;
import backend.Screen;

// TODO: Just use Action?
typedef GameAction = Class<Action>;
// TODO: Split by equipment type
typedef GameEquipment = Equipment;
// TODO: Just use Outcome?
typedef GameOutcome = Class<Outcome>;
typedef GameRoom = Room;
typedef GameScreen = Screen;
