# Uncomment for type checking
# from embeddedgame import (
#   backend_ActionScreen,
#   backend_Game,
#   backend_ScreenAction
# )
import framebuf
import SQUiXL as squixl
import time

screen_size: int = 480 # Hardcoded for display
text_size: int = 8 # Hardcoded for FrameBuffer
display: bytearray = squixl.create_display()
fb: FrameBuffer = framebuf.FrameBuffer(display, screen_size, screen_size, framebuf.RGB565)

background_colour: int = 0x0000 # Black
text_colour      : int = 0xffff # White
button_colour    : int = 0xf800 # Red

# TODO: Simulate qwerty
text_buttons: list[str] = [
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
  'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
  'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', '!', '"', '#', '$', '%', '&', "'", '(', ')', '*',
  '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@',
 '\\', '[', ']', '^', '_', ' ', ' ', ' ', ' ', '<', ' ', '>'
]
back_index = len(text_buttons) - 3
enter_index = len(text_buttons) - 1

padding: int = 5
outer_button_padding: int = 2 * padding
inner_button_padding: int = 2 * outer_button_padding # GetButtonInput depends on 2x
action_button_width: int = 100
text_button_width: int = 20
action_button_height: int = 40
text_button_height: int = 32

# This gives 4 buttons per row with outer_button_padding to the edges and inner_button_padding between
# Result must be a whole number(even without //) therefore action_button_width and outer_button_padding must be even
action_button_row_count: int = 4 # (screen_size - 2 * outer_button_padding + inner_button_padding) / (action_button_width + inner_button_padding)
text_button_row_count: int = 12 # (screen_size - 2 * outer_button_padding + inner_button_padding) / (text_button_width + inner_button_padding)

current_y: int = padding
buttons_start_y: int

last_touch_time: int = 0
required_touch_diff: int = 500

def SetupOutput() -> None:
  squixl.screen_init_spi_bitbanged()

def ClearOutput() -> None:
  global current_y
  current_y = padding

  fb.fill(background_colour)

def UpdateOutput() -> None:
  pass

def ResetOutput() -> None:
  time.sleep(5)
  squixl.lcd.deinit()


def PrintString(text: str, end="\n", highlight: bool=False) -> None:
  global current_y

  lines: list[str] = (text + end).split("\n")

  for idx, line in enumerate(lines):
    fb.text(line, padding, current_y, text_colour)
    if idx == len(lines) - 1:
      continue
    elif line == "":
      current_y += 17
    else:
      current_y += 12

def PrintActionInputs(state: backend_Game, screen: backend_ActionScreen) -> None:
  global buttons_start_y
  global current_y

  PrintString("\n\nPress a button below to make a selection.")

  buttons_start_y = current_y + outer_button_padding
  current_y += 2 * outer_button_padding

  actions: list[backend_ScreenAction] = screen.GetActions(state)

  column: int = 0
  for action in actions:
    if action.isVisible(state, screen):
      button_x: int = outer_button_padding + (inner_button_padding + action_button_width) * column
      text_x: int = button_x + (action_button_width - text_size * len(action.title)) // 2
      text_y: int = current_y + (action_button_height - text_size) // 2
      fb.rect(button_x, current_y, action_button_width, action_button_height, button_colour, True)
      fb.text(action.title, text_x, text_y, text_colour)

      column += 1
      if column == action_button_row_count:
        column = 0
        current_y += action_button_height + 2 * outer_button_padding

# Designed for password entry
def PrintTextInput() -> None:
  global buttons_start_y
  global current_y

  PrintString(':\n\nPress a button below to select a character.')

  buttons_start_y = current_y + outer_button_padding
  current_y += 2 * outer_button_padding

  column: int = 0
  for char in text_buttons:
    if char != ' ':
      button_x: int = outer_button_padding + (inner_button_padding + text_button_width) * column
      text_x: int = button_x + (text_button_width - text_size) // 2
      text_y: int = current_y + (text_button_height - text_size) // 2
      fb.rect(button_x, current_y, text_button_width, text_button_height, button_colour, True)
      fb.text(char, text_x, text_y, text_colour)

    column += 1
    if column == text_button_row_count:
      column = 0
      current_y += text_button_height + 2 * outer_button_padding


def GetActionInput(input_count: int) -> int:
  global last_touch_time

  while True:
    point_count: int
    points: list[array[int]]
    point_count, points = squixl.touch.read_points()
    if time.ticks_ms() >= last_touch_time + required_touch_diff:
      if point_count > 0:
        last_touch_time = time.ticks_ms()
        point: array[int] = points[point_count - 1]
        button_x: int = point[1] // (action_button_width + inner_button_padding)
        button_y: int = (point[0] - buttons_start_y) // (action_button_height + inner_button_padding)
        button_input: int = action_button_row_count * button_y + button_x
        return button_input

# TODO: Print to screen rather than console
# Designed for password entry
def GetTextInput() -> str | None:
  global last_touch_time

  text: str = ''

  while True:
    point_count: int
    points: list[array[int]]
    point_count, points = squixl.touch.read_points()
    if time.ticks_ms() >= last_touch_time + required_touch_diff:
      if point_count > 0:
        last_touch_time = time.ticks_ms()
        point: array[int] = points[point_count - 1]
        button_x: int = point[1] // (text_button_width + inner_button_padding)
        button_y: int = (point[0] - buttons_start_y) // (text_button_height + inner_button_padding)
        button_input: int = text_button_row_count * button_y + button_x
        if button_input == enter_index: # Enter
          return text
        elif button_input == back_index and len(text) == 0: # ESC
          return None
        elif button_input == back_index: # Backspace
          print('\010 \010', end='')
          text = text[:-1]
        elif button_input < len(text_buttons):
          input_char: str = text_buttons[button_input]
          text += input_char
          print(input_char, end='')
