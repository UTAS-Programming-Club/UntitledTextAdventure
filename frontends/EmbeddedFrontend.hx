package frontends;

import backend.Game;
import backend.GameInfo;
import backend.Screen;
import campaigns.UntitledTextAdventure;

@:pythonImport("nativehelpers")
extern class Native {
  static var current_input: Int;

  static function SetupOutput(): Void;
  static function ClearOutput(): Void;
  static function UpdateOutput(): Void;
  static function ResetOutput(): Void;

  static function PrintString(str: UnicodeString, end: UnicodeString = "\n", highlightAction: Bool = false): Void;
  static function PrintActionInputs(state: Game, screen: ActionScreen): Void;
  static function PrintTextInput(): Void;

  static function GetActionInput(inputCount: Int): Int;
  static function GetTextInput(): Null<UnicodeString>;
}

class EmbeddedFrontend {
  static function PrintOutputBody(body: UnicodeString): Void {
    Native.ClearOutput();
    Native.PrintString(body, "");
  }

  static function HandleOutput(state: Game): Bool {
    final screen: Screen = state.getScreen();
    PrintOutputBody(screen.GetBody(state));

    if (screen is ActionScreen) {
      Native.PrintActionInputs(state, cast(screen, ActionScreen));
    } else if (screen is TextScreen) {
      Native.PrintTextInput();
    } else {
      return false;
    }

    Native.UpdateOutput();

    return true;
  }


  static function MapInputIndex(state: Game, screen: ActionScreen, actions: Array<ScreenAction>, inputIndex: Int): Int {
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

  static function HandleActionInput(state: Game, screen: ActionScreen): GameOutcome {
    final actions: Array<ScreenAction> = screen.GetActions(state);
    final visibleInputs: Array<ScreenAction> = [
      for (action in actions) {
        if (action.isVisible(state, screen)) {
          action;
        }
      }
    ];

    final inputIndex: Int = Native.GetActionInput(visibleInputs.length);
    final actionindex: Int = MapInputIndex(state, screen, actions, inputIndex);
    if (actionindex >= actions.length) {
      // TODO: Is this still the case?
      // This is a recoverable error so just ignore it
      return GetNextOutput;
    }

    return actions[actionindex].handleAction(state);
  }

  static function HandleTextInput(state: Game, screen: TextScreen): GameOutcome {
    // // TODO: Get Input
    final input: Null<UnicodeString> = Native.GetTextInput();
    return screen.handleInput(state, input);
  }

  static function HandleInput(state: Game): Bool {
    final screen: Screen = state.getScreen();

    var outcome: GameOutcome;
    if (screen is ActionScreen) {
      outcome = HandleActionInput(state, cast(screen, ActionScreen));
    } else if (screen is TextScreen) {
      outcome = HandleTextInput(state, cast(screen, TextScreen));
    } else {
      return false;
    }

    switch (outcome) {
      case GetNextOutput:
        Native.current_input = 0;
        return true;
      case QuitGame:
        return false;
      default:
       throw 'Unknown screen action outcome $outcome received.';
    }
  }


  static function main(): Void {
    Native.SetupOutput();

    final state = new Game(UntitledTextAdventure);
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
