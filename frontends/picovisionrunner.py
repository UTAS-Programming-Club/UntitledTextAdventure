from embeddedgame import (
  backend_ActionScreen,
  backend_Game,
  backend_GameOutcome,
  backend_Screen,
  backend_ScreenAction,
  campaigns__UntitledTextAdventure_UntitledTextAdventure_Fields_
)
from modes import PAL
from pimoroni import Button
import time

display: PAL = PAL()
background_colour: int = display.create_pen(0, 0, 0)
foreground_colour: int = display.create_pen(255, 255, 255)

button_a = lambda: display.is_button_a_pressed() and time.ticks_ms() - last_button_time >= last_required_diff
button_x = lambda: display.is_button_x_pressed() and time.ticks_ms() - last_button_time >= last_required_diff
button_y = lambda: Button(9, invert=True).read() and time.ticks_ms() - last_button_time >= last_required_diff
last_button_time: int = 0
last_required_diff: int = 250

padding: int = 5
offset: int = padding
current_input: int = 0

def SetupOutput() -> None:
  display.set_font("bitmap8")
  display.set_pen(foreground_colour)

def ResetOutput() -> None:
  pass

def ClearOutput() -> None:
  global offset
  offset = padding

  display.set_pen(background_colour)
  display.clear()
  display.set_pen(foreground_colour)

def GetButtonInput(input_count: int) -> int:
  global current_input
  global last_button_time

  button_input: int = -1

  if button_x() and current_input > 0:
    current_input -= 1
    button_input = input_count
  elif button_y() and current_input < input_count - 1:
    current_input += 1
    button_input = input_count
  elif button_a():
    button_input = current_input

  if button_input != -1:
    last_button_time = time.ticks_ms()
  return button_input

def GetTextWidth(text: str) -> int:
    width: int = 0
    lines: list[str] = text.split("\n")

    for idx, line in enumerate(lines):
        new_width = display.measure_text(line, scale = 2, fixed_width = True)
        if new_width > width:
            width = new_width

    return width

def GetTextHeight(text: str) -> int:
    height: int = 0
    lines: list[str] = text.split("\n")

    for line in lines:
        if line == "":
            height += 35
        else:
            height += 25

    return height

def PrintString(text: str, end="\n", highlight: bool=False) -> None:
  global offset

  lines: list[str] = (text + end).split("\n")

  if highlight:
    height: int = GetTextHeight(text)
    width: int = GetTextWidth(text)
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

    if highlight:
      display.set_pen(foreground_colour)

def PrintOutputBody(body: str) -> None:
  ClearOutput()
  PrintString(body)

def PrintButtonInputs(state: backend_Game, screen: backend_ActionScreen) -> None:
  global current_input

  PrintString("\n\nCycle with X and Y and make a selection with A.")

  actions: list[backend_ScreenAction] = screen.GetActions(state)
  input_number: int = 0
  for action in actions:
    if action.isVisible(state, screen):
      highlight: bool = input_number == current_input
      input_number += 1
      PrintString(f"{input_number}. {action.title}", highlight=highlight)

def HandleOutput(state: backend_Game) -> bool:
  screen: backend_Screen = state.getScreen()
  PrintOutputBody(screen.GetBody(state))

  if isinstance(screen, backend_ActionScreen):
    PrintButtonInputs(state, screen)
  else:
    return False

  display.update()

  return True

def MapInputIndex(state: backend_Game, screen: backend_ActionScreen, actions: list[backend_ScreenAction], inputIndex: int) -> int:
  for index in range(len(actions)):
    if not actions[index].isVisible(state, screen):
      continue

    if inputIndex == 0:
      return index

    inputIndex -= 1

  return len(actions)

def HandleInput(state: backend_Game) -> bool:
  global current_input

  screen: backend_Screen = state.getScreen()
  if not isinstance(screen, backend_ActionScreen):
    return False

  actionScreen: backend_ActionScreen = screen
  actions: list[backend_ScreenAction] = actionScreen.GetActions(state)

  visible_inputs: int = len([action for action in actions if action.isVisible(state, screen)])

  inputIndex: int = GetButtonInput(visible_inputs)
  index: int = MapInputIndex(state, actionScreen, actions, inputIndex)
  if index >= len(actions):
    return True

  outcome: backend_GameOutcome = actions[index].handleAction(state)
  if outcome is backend_GameOutcome.GetNextOutput:
    current_input = 0
    return True
  elif outcome is backend_GameOutcome.QuitGame:
    return False
  else:
    raise Exception(f"Unknown screen action outcome {outcome} received.")


SetupOutput()

state = backend_Game(campaigns__UntitledTextAdventure_UntitledTextAdventure_Fields_.UntitledTextAdventure)

while True:
  if not HandleOutput(state):
    break
  elif not HandleInput(state):
   break

ResetOutput()

PrintOutputBody('Game is over')
