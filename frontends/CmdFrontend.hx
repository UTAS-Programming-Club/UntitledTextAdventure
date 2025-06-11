package frontends;

import backend.Action;
import backend.Game;
import backend.coregame.Outcomes;
// TODO: Recreate some merged type that works for switch exhaustion?
import backend.GameInfo;
import backend.Screen;
import haxe.io.Bytes;

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
  static function GetTextInput(): Null<UnicodeString> {
    var bufSize: Int = 16;
    var bufOffset: Int = 0;
    var buf: Bytes = Bytes.alloc(bufSize);

    while (true) {
      if (bufOffset == bufSize) {
        final newBufSize: Int = bufSize * bufSize;
        final newBuf: Bytes = haxe.io.Bytes.alloc(newBufSize);
        newBuf.blit(0, buf, 0, bufSize);
        bufSize = newBufSize;
        buf = newBuf;
      }

      final inputChar: Int = Sys.getChar(false);
      buf.fill(bufOffset, 1, inputChar);
      // This also detects any key that sends an escape sequence e.g. arrow keys
      if (inputChar == 0x1B) {
        return null;
      }

      if (inputChar == 0x7F && bufOffset >= 1) {
        Sys.print("\010 \010");
        --bufOffset;
      } else if (inputChar == 9 || (inputChar >= 32 && inputChar <= 126)) {
        Sys.print(String.fromCharCode(inputChar));
        ++bufOffset;
      } else if (inputChar == "\r".code) {
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

  static function HandleActionInput(state: Game, screen: ActionScreen): GameOutcome {
    final actions: Array<Action> = screen.GetActions();

    final inputIndex: Int = GetButtonInput();
    final actionindex: Int = MapInputIndex(state, actions, inputIndex);
    if (actionindex >= actions.length) {
      // This is a recoverable error so just ignore it
      return GetNextOutput;
    }

    return actions[actionindex].handleAction(state);
  }

  static function HandleTextInput(state: Game, screen: TextScreen): GameOutcome {
    final input: Null<UnicodeString> = GetTextInput();
    Sys.print("\n\n\n\n\n"); // In case of exceptions
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
        return true;
      case QuitGame:
        return false;
      default:
       throw ': Unknown screen action outcome $outcome received';
    }
  }


  static function main(): Void {
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
