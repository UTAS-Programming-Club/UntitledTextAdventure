# Uncomment for type checking
# from embeddedgame import (
#   backend_ActionScreen,
#   backend_Game,
#   backend_ScreenAction
# )
from modes import PAL
from pimoroni import Button
import time

display: PAL = PAL()

background_colour: int = display.create_pen(0, 0, 0)
text_colour      : int = display.create_pen(255, 255, 255)

padding: int = 5

current_y: int = padding
current_input: int = 0

button_a = lambda: display.is_button_a_pressed() and time.ticks_ms() - last_button_time >= last_required_diff
button_x = lambda: display.is_button_x_pressed() and time.ticks_ms() - last_button_time >= last_required_diff
button_y = lambda: Button(9, invert=True).read() and time.ticks_ms() - last_button_time >= last_required_diff
last_button_time: int = 0
last_required_diff: int = 250

def SetupOutput() -> None:
  display.set_font("bitmap8")
  display.set_pen(text_colour)

def ClearOutput() -> None:
  global current_y
  current_y = padding

  display.set_pen(background_colour)
  display.clear()
  display.set_pen(text_colour)

def UpdateOutput() -> None:
  display.update()

def ResetOutput() -> None:
  pass


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
  global current_y

  lines: list[str] = (text + end).split("\n")

  if highlight:
    height: int = GetTextHeight(text)
    width: int = GetTextWidth(text)
    display.rectangle(0, current_y - padding, width + padding, height)
    display.set_pen(background_colour)

  for idx, line in enumerate(lines):
    display.text(line, padding, current_y, scale = 2, fixed_width = True)
    if idx == len(lines) - 1:
      continue
    elif line == "":
      current_y += 35
    else:
      current_y += 25

    if highlight:
      display.set_pen(text_colour)

def PrintActionInputs(state: backend_Game, screen: backend_ActionScreen) -> None:
  global current_input

  PrintString("\n\nCycle with X and Y and make a selection with A.")

  actions: list[backend_ScreenAction] = screen.GetActions()

  input_number: int = 0
  for action in actions:
    if action.isVisible(state):
      highlight: bool = input_number == current_input
      input_number += 1
      PrintString(f"{input_number}. {action.title}", highlight=highlight)


def GetActionInput(input_count: int) -> int:
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
