package frontends;

import backend.GameState;
import backend.GlobalData;
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


  static function PrintOutputBody(body: String): Void {
    Sys.print(CSI + "?25l"); // Hide cursor
    Sys.print(CSI + "0;0H"); // Move cursor to 0, 0
    Sys.print(CSI + "0J");   // Erase entire screen
    Sys.print(body);
    Sys.print(CSI + "?25h"); // Show cursor
  }


  static function GetButtonInput(): UInt {
    while (true) {
      final input: Int = Sys.getChar(false);
      if ("1".code <= input && input <= "9".code) {
        return input - "1".code;
      }
    }
  }

  static function PrintButtonInputs(state: GameState, screen: ActionScreen): Void {
    Sys.println("\n\nUse the numbers below to make a selection.");

    final actions: Array<ScreenAction> = screen.GetActions(state);
    for (i in 0...actions.length) {
      Sys.println((i + 1) + ". " + actions[i].title);
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
    Sys.println(ESC + "7"); // Backup cursor position
    // TODO: Move to backend
    Sys.println("\nPress Enter to confirm password entry.\nPress Esc to return to the previous screen.");
    Sys.print(ESC + "8"); // Restore cursor position
  }


  static function HandleOutput(state: GameState): Bool {
    PrintOutputBody(state.currentScreen.body);

    if (state.currentScreen is ActionScreen) {
      PrintButtonInputs(state, cast(state.currentScreen, ActionScreen));
    } else {
      return false;
    }

    return true;
  }

  static function HandleInput(state: GameState): Bool {
    if (!(state.currentScreen is ActionScreen)) {
      return false;
    }

    final screen = cast(state.currentScreen, ActionScreen);
    final index: UInt = GetButtonInput();
    final actions: Array<ScreenAction> = screen.GetActions(state);
    if (index >= actions.length) {
      // This is a recoverable error so just ignore it
      return true;
    }

    final outcome: ScreenActionOutcome = state.HandleGameInput(actions[index].type);
    switch (outcome) {
      case GetNextOutput:
        return true;
      case QuitGame:
        return false;
      default:
        throw new haxe.Exception("Unknown screen action outcome " + outcome + " received");
    }
  }


  public static function main(): Void {
    SetupConsole();
    GlobalData.Init();

    var state = new GameState();
    do {
      if (!HandleOutput(state)) {
        break;
      }
    } while(HandleInput(state));

    ResetConsole();
  }
}
