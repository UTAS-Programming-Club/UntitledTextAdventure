package frontends;

import backend.Action;
import backend.Game;
import backend.coregame.Outcomes;
// TODO: Recreate some merged type that works for switch exhaustion?
import backend.GameInfo;
import backend.Screen;

class CmdFrontend {
  static final ESC = "\x1B";
  static final CSI = ESC + "[";

  static function SetupConsole(): Void {
    Sys.print(CSI + "?1049h"); // Switch to alternative buffer
  }

  static function ResetConsole(): Void {
    Sys.print(CSI + "?1049l"); // Restore original buffer
  }


  static function PrintOutputBody(body: UnicodeString): Void {
    Sys.print(CSI + "?25l"); // Hide cursor
    Sys.print(CSI + "0;0H"); // Move cursor to 0, 0
    Sys.print(CSI + "0J");   // Erase entire screen
    Sys.print(body);
    Sys.print(CSI + "?25h"); // Show cursor
  }


  static function GetButtonInput(): Int {
    while (true) {
      final input: Int = Sys.getChar(false);
      if ("1".code <= input && input <= "9".code) {
        return input - "1".code;
      }
    }
  }

  static function PrintButtonInputs(state: Game, screen: ActionScreen): Void {
    Sys.println("\n\nUse the numbers below to make a selection.");

    final actions: Array<Action> = screen.GetActions();
    var inputNumber: Int = 0;
    for (action in actions) {
      if (action.isVisible(state)) {
        inputNumber++;
        Sys.println('$inputNumber. ${action.title}');
      }
    }
  }


  // Returns input text as a utf-8 string if enter is pressed, "\x1B" on esc
  // being pressed or NULL if buffer allocation failed
  static function GetTextInput(): String {
    var bufSize = 16;
    var bufOffset = 0;
    var buf = haxe.io.Bytes.alloc(bufSize);

    while (true) {
      if (bufOffset == bufSize) {
        final newBufSize = bufSize * bufSize;
        var newBuf = haxe.io.Bytes.alloc(newBufSize);
        newBuf.blit(0, buf, 0, bufSize);
        bufSize = newBufSize;
        buf = newBuf;
      }

      final input = Sys.getChar(false);
      buf.fill(bufOffset, 1, input);
      // This also detects any key that sends an escape sequence e.g. arrow keys
      if (input == 0x1B) {
        return ESC;
      }

      if (input == 0x7F && bufOffset >= 1) {
        Sys.print("\010 \010");
        --bufOffset;
      } else if (input == 9 || (input >= 32 && input <= 126)) {
        Sys.print(String.fromCharCode(input));
        ++bufOffset;
      } else if (input == "\r".code) {
        return buf.getString(0, bufOffset);
      }

      // TODO: Check if this is needed
      Sys.stdout().flush();
    }
  }

  static function PrintTextInput(): Void {
    Sys.print(": ");
    Sys.println(ESC + "7"); // Backup cursor position
    Sys.println("\nPress Enter to confirm text entry.\nPress Esc to return to the previous screen.");
    Sys.print(ESC + "8"); // Restore cursor position
  }


  static function HandleOutput(state: Game): Bool {
    final screen: Screen = state.getScreen();
    PrintOutputBody(screen.getBody(state));

    if (screen is ActionScreen) {
      PrintButtonInputs(state, cast(screen, ActionScreen));
    } else if (screen is TextScreen) {
      PrintTextInput();
    } else {
      return false;
    }

    return true;
  }

  static function MapInputIndex(state: Game, actions: Array<Action>, inputIndex: Int): Int {
    var index: Int;
    for (index in 0...actions.length) {
      if (!actions[index].isVisible(state)) {
        continue;
      }

      if (inputIndex == 0) {
        return index;
      }

      inputIndex--;
    }

    return actions.length;
  }

  static function HandleActionInput(state: Game, screen: ActionScreen): Bool {
    final actions: Array<Action> = screen.GetActions();

    final inputIndex: Int = GetButtonInput();
    final index: Int = MapInputIndex(state, actions, inputIndex);
    if (index >= actions.length) {
      // This is a recoverable error so just ignore it
      return true;
    }

    final outcome: GameOutcome = actions[index].handleAction(state);
    switch (outcome) {
      case GetNextOutput:
        return true;
      case QuitGame:
        return false;
      default:
       throw ": Unknown screen action outcome $outcome received";
    }
  }

  // TODO: Handle ESC
  static function HandleTextInput(state: Game): Bool {
    final input: UnicodeString = GetTextInput();
    Sys.println("\n\n\n\n\nYour input: " + input);
    Sys.getChar(true);
    return false;
  }

  static function HandleInput(state: Game): Bool {
    final screen: Screen = state.getScreen();

    if (screen is ActionScreen) {
      return HandleActionInput(state, cast(screen, ActionScreen));
    } else if (screen is TextScreen) {
      return HandleTextInput(state);
    } else {
      return false;
    }
  }

  public static function main(): Void {
    SetupConsole();

    final state = new Game();
    do {
      if (!HandleOutput(state)) {
        break;
      }
    } while(HandleInput(state));

    ResetConsole();
  }
}
