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
display: bytearray = squixl.create_display()
fb: FrameBuffer = framebuf.FrameBuffer(display, screen_size, screen_size, framebuf.RGB565)

background_colour: int = 0x0000 # Black
text_colour      : int = 0xffff # White
button_colour    : int = 0xf800 # Red

padding: int = 5
outer_button_padding: int = 2 * padding
inner_button_padding: int = 2 * outer_button_padding # GetButtonInput depends on 2x
button_width: int = 100
button_height: int = 40

# This gives 4 buttons per row with outer_button_padding to the edges and inner_button_padding between
# Result must be a whole number(even without //) therefore button_width and outer_button_padding must be even
button_row_count: int = 4 # (screen_size - 2 * outer_button_padding + inner_button_padding) / (button_width + inner_button_padding)

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

  text_size: int = 8 # Hardcoded for FrameBuffer

  buttons_start_y = current_y + outer_button_padding
  current_y += 2 * outer_button_padding

  actions: list[backend_ScreenAction] = screen.GetActions(state)

  column: int = 0
  for action in actions:
    if action.isVisible(state, screen):
      button_x: int = outer_button_padding + (inner_button_padding + button_width) * column
      text_x: int = button_x + (button_width - text_size * len(action.title)) // 2
      text_y: int = current_y + (button_height - text_size) // 2
      fb.rect(button_x, current_y, button_width, button_height, button_colour, True)
      fb.text(action.title, text_x, text_y, text_colour)

      column += 1
      if column == button_row_count:
        column = 0
        current_y += 50


def GetActionInput(input_count: int) -> int:
  global last_touch_time

  while True:
    point_count: int
    points: list[array[int]]
    point_count, points = squixl.touch.read_points()
    if time.ticks_ms() >= last_touch_time + required_touch_diff:
      if point_count > 0:
        last_touch_time = time.ticks_ms()
        point = points[point_count - 1]
        button_x: int = point[1] // (button_width + inner_button_padding)
        button_y: int = (point[0] - buttons_start_y) // (button_height + inner_button_padding)
        button_input: int = button_row_count * button_y + button_x
        return button_input
