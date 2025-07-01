package frontends;

import backend.Action;
import backend.Game;
import backend.coregame.Outcomes;
import backend.GameInfo;
import backend.Screen;

@:pythonImport("nativehelpers")
extern class Native {
  static var current_input: Int;

  static function SetupOutput(): Void;
  static function ClearOutput(): Void;
  static function UpdateOutput(): Void;
  static function ResetOutput(): Void;

  static function PrintString(str: UnicodeString, end: UnicodeString = "\n", highlightAction: Bool = false): Void;
  static function PrintActionInputs(state: Game, screen: ActionScreen): Void;

  static function GetActionInput(inputCount: Int): Int;
}

class EmbeddedFrontend {
  static function PrintOutputBody(body: UnicodeString): Void {
    Native.ClearOutput();
    Native.PrintString(body);
  }

  static function HandleOutput(state: Game): Bool {
    final screen: Screen = state.getScreen();
    PrintOutputBody(screen.getBody(state));

    if (screen is ActionScreen) {
      Native.PrintActionInputs(state, cast(screen, ActionScreen));
    } else {
      return false;
    }

    Native.UpdateOutput();

    return true;
  }


  static function MapInputIndex(state: Game, screen: ActionScreen, actions: Array<Action>, inputIndex: Int): Int {
    var index: Int;
    for (index in 0...actions.length) {
      if (!actions[index].isVisible(state, screen)) {
        continue;
      }

      if (inputIndex == 0) {
        return index;
      }

      inputIndex--;
    }

    return actions.length;
  }

  static function HandleInput(state: Game): Bool {
    final screen: Screen = state.getScreen();
    if (!(screen is ActionScreen)) {
      return false;
    }

    final actionScreen: ActionScreen = cast screen;
    final actions: Array<Action> = actionScreen.GetActions();
    final visibleInputs: Array<Action> = [
      for (action in actions) {
        if (action.isVisible(state, actionScreen)) {
          action;
        }
      }
    ];

    final inputIndex: Int = Native.GetActionInput(visibleInputs.length);
    final actionindex: Int = MapInputIndex(state, actionScreen, actions, inputIndex);
    if (actionindex >= actions.length) {
      return true;
    }

    final outcome: GameOutcome = actions[actionindex].handleAction(state);
    switch (outcome) {
      case GetNextOutput:
        Native.current_input = 0;
        return true;
      case QuitGame:
        return false;
      default:
      throw ': Unknown screen action outcome $outcome received';
    }
  }


  static function main(): Void {
    Native.SetupOutput();

    final state = new Game();
    do {
      if (!HandleOutput(state)) {
        break;
      }
    } while(HandleInput(state));

    PrintOutputBody('Game is over');
    Native.UpdateOutput();

    Native.ResetOutput();
  }
}
