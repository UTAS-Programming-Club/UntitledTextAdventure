from embeddedgame import (
  backend_ActionScreen,
  backend_Game,
  backend_GameOutcome,
  backend_Screen,
  backend_ScreenAction,
  campaigns__UntitledTextAdventure_UntitledTextAdventure_Fields_
)
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

def ResetOutput() -> None:
  squixl.lcd.deinit()

def ClearOutput() -> None:
  global current_y
  current_y = padding

  fb.fill(background_colour)

def GetButtonInput(input_count: int) -> int:
  global last_touch_time

  # print(buttons_start_y)
  # row_count: int = -(20 // -button_row_count) # Ceiling
  # full_button_width: int = button_width + inner_button_padding
  # full_button_height: int = button_height + inner_button_padding
  # for y in range(row_count):
  #   line_y: int = buttons_start_y + y * full_button_height
  #   fb.hline(0, line_y, screen_size, text_colour)
  #   for x in range(button_row_count):
  #     fb.vline(x * full_button_width, line_y, full_button_height, text_colour)
  #   fb.vline(4 * full_button_width - 1, line_y, full_button_height, text_colour) # Cheat as 4 * full_button_width == screen_size
  # fb.hline(0, buttons_start_y + row_count * full_button_height, screen_size, text_colour)

  while True:
    point_count: int
    points: list[array[int]]
    point_count, points = squixl.touch.read_points()
    if time.ticks_ms() >= last_touch_time + required_touch_diff:
      if point_count > 0:
        last_touch_time = time.ticks_ms()
        point = points[point_count - 1]
        # print(f"{point}, {last_touch_time}", end="")
        button_x: int = point[1] // (button_width + inner_button_padding)
        button_y: int = (point[0] - buttons_start_y) // (button_height + inner_button_padding)
        button_input: int = button_row_count * button_y + button_x
        # print(f", ({button_x}, {button_y}), {button_input}")
        # fb.pixel(point[1], point[0], 0xffc0)
        return button_input

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

def PrintOutputBody(body: str) -> None:
  ClearOutput()
  PrintString(body)

def PrintButtonInputs(state: backend_Game, screen: backend_ActionScreen) -> None:
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
      # print(f"fb.rect({button_x}, {current_y}, button_width, button_height, button_colour, True)")
      fb.rect(button_x, current_y, button_width, button_height, button_colour, True)
      # print(f"fb.text(action.title, {text_x}, {text_y}, text_colour)")
      fb.text(action.title, text_x, text_y, text_colour)

      column += 1
      if column == button_row_count:
        column = 0
        current_y += 50

def HandleOutput(state: backend_Game) -> bool:
  screen: backend_Screen = state.getScreen()
  PrintOutputBody(screen.GetBody(state))

  if isinstance(screen, backend_ActionScreen):
    PrintButtonInputs(state, screen)
  else:
    return False

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

  actions: list[backend_ScreenAction] = screen.GetActions(state)
  visible_inputs: int = len([action for action in actions if action.isVisible(state, screen)])

  inputIndex: int = GetButtonInput(visible_inputs)
  index: int = MapInputIndex(state, screen, actions, inputIndex)
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

PrintOutputBody('Game is over')

time.sleep(5)

ResetOutput()
