import time
from modes import PAL
from pimoroni import Button

from PCGameRunner.PCGame import backend_ActionScreen, backend_GameState, backend_ScreenActionOutcome


class InputButton():
    A = 0
    X = 1
    Y = 2


display = PAL()
BLACK = display.create_pen(0, 0, 0)
WHITE = display.create_pen(255, 255, 255)

button_a = lambda: display.is_button_a_pressed() and time.ticks_ms() - last_button_time >= last_required_diff
button_x = lambda: display.is_button_x_pressed() and time.ticks_ms() - last_button_time >= last_required_diff
button_y = lambda: Button(9, invert=True).read() and time.ticks_ms() - last_button_time >= last_required_diff
last_button_time = 0
last_required_diff = 250

padding: int = 5
offset: int = padding
background_colour = BLACK
foreground_colour = WHITE

def scr_setup() -> None:
    pass

def scr_reset() -> None:
    global offset
    global background_colour
    global foreground_colour

    offset = padding
    background_colour = BLACK
    foreground_colour = WHITE

    display.set_font("bitmap8")

    display.set_pen(background_colour)
    display.clear()
    display.set_pen(foreground_colour)

def scr_get_width(text: str, end="\n") -> int:
    global offset

    width: int = 0

    lines = (text + end).split("\n")
    for idx, line in enumerate(lines):
        new_width = display.measure_text(line, scale = 2, fixed_width = True)
        if new_width > width:
            width = new_width

    return width

def scr_get_height(text: str, end="\n") -> int:
    height: int = 0
    lines = (text + end).split("\n")

    del lines[-1]
    for line in lines:
        if line == "":
            height += 35
        else:
            height += 25

    return height

def scr_print(text: str, end="\n", inverted=False) -> None:
    global offset

    lines = (text + end).split("\n")

    if inverted:
        height = scr_get_height(text)
        width = scr_get_width(text)
        display.rectangle(0, offset - padding, width + padding, height)
        display.set_pen(background_colour)

    for idx, line in enumerate(lines):
        display.text(line, padding, offset, scale = 2, fixed_width = True)
        if idx == len(lines) - 1:
            continue
        elif line == "":
            offset += 35
        else:
            offset += 25

    if inverted:
        display.set_pen(foreground_colour)

def scr_cleanup(show_error: bool) -> None:
    print("Program has finished")


selected_option: int = 0
last_button_time: int

def update_screen(game: PCGame) -> None:
    global selected_option

    scr_reset()

    if isinstance(game.currentScreen, backend_ActionScreen):
        scr_print(game.currentScreen.GetBody(game), end="\n\n")
        scr_print("Cycle with X and Y and make a selection with A.")

        actions = game.currentScreen.GetActions(game)
        for idx, action in enumerate(actions):
            scr_print(f"{idx + 1}. {action.title}", inverted=(selected_option == idx))
    else:
        scr_print('Unable to get current screen')

    display.update()

def get_input() -> InputButton:
    while True:
        if button_x():
            return InputButton.X
        elif button_y():
            return InputButton.Y
        elif button_a():
            return InputButton.A

def handle_input(game: PCGame) -> ScreenOptionOutcome | str:
    global selected_option
    global last_button_time

    user_input = get_input()
    last_button_time = time.ticks_ms()
    outcome: ScreenOptionOutcome | str

    if not isinstance(game.currentScreen, backend_ActionScreen):
        outcome = 'Unable to get current screen'
        return

    actions = game.currentScreen.GetActions(game)

    if user_input == InputButton.X:
        outcome = backend_ScreenActionOutcome.GetNextOutput
        selected_option = (selected_option - 1) % len(actions)
    elif user_input == InputButton.Y:
        outcome = backend_ScreenActionOutcome.GetNextOutput
        selected_option = (selected_option + 1) % len(actions)
    elif user_input == InputButton.A:
        if selected_option >= len(actions):
            return

        outcome = game.HandleGameInput(actions[selected_option].type)
        selected_option = 0
    else:
        outcome = f"Unexpected button pressed: {user_input}"

    return outcome


game = backend_GameState()
show_error = False

scr_setup()

while True:
    update_screen(game)
    outcome = handle_input(game)

    if outcome == backend_ScreenActionOutcome.GetNextOutput:
        continue
    elif outcome == backend_ScreenActionOutcome.QuitGame:
        scr_reset()
        scr_print('Game is over')
        display.update()
        break
    elif isinstance(outcome, str):
        print(outcome)
        show_error = True
        break
    else:
        print(f"Unexpected outcome: {outcome}")
        show_error = True
        break

scr_cleanup(show_error)
