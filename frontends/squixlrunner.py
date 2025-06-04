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

screen_size: int = 480
display: bytearray = squixl.create_display()
fb: FrameBuffer = framebuf.FrameBuffer(display, screen_size, screen_size, framebuf.RGB565)
background_colour: int = 0x0000 # Black
foreground_colour: int = 0xffff # White
button_colour    : int = 0xf800 # Red

padding: int = 5
inner_button_padding: int = 4 * padding
offset: int = padding

buttons_start_offset: int
last_button_time: int = 0
last_required_diff: int = 500

def SetupOutput() -> None:
  squixl.screen_init_spi_bitbanged()

def ResetOutput() -> None:
  squixl.lcd.deinit()

def ClearOutput() -> None:
  global offset
  offset = padding

  fb.fill(background_colour)

def GetButtonInput(input_count: int) -> int:
  global last_button_time

  button_input: int = -1

  while True:
    n, points = squixl.touch.read_points()
    if time.ticks_ms() >= last_button_time + last_required_diff:
      if n > 0:
        print(f"{points[n - 1]}, {last_button_time}")
        last_button_time = time.ticks_ms()

  return button_input

def PrintString(text: str, end="\n", highlight: bool=False) -> None:
  global offset

  lines: list[str] = (text + end).split("\n")

  for idx, line in enumerate(lines):
    fb.text(line, padding, offset, foreground_colour)
    if idx == len(lines) - 1:
      continue
    elif line == "":
      offset += 17
    else:
      offset += 12

def PrintOutputBody(body: str) -> None:
  ClearOutput()
  PrintString(body)

def PrintButtonInputs(state: backend_Game, screen: backend_ActionScreen) -> None:
  global current_input
  global offset

  PrintString("\n\nPress a button below to make a selection.")

  button_width: int = 100
  button_height: int = 40
  text_size: int = 8 # Hardcoded for FrameBuffer

  outer_padding: int = 2 * padding
  # This gives 4 buttons per row with outer_padding to the edges and inner_button_padding between
  max_button_count: int = (screen_size - 2 * outer_padding + inner_button_padding) / (button_width + inner_button_padding)
  actions: list[backend_ScreenAction] = screen.GetActions(state)

  offset += 2 * outer_padding
  buttons_start_offset = offset

  button_column: int = 0
  for action in actions:
    if action.isVisible(state, screen):
      button_x: int = outer_padding + (inner_button_padding + button_width) * button_column
      text_x: int = button_x + (button_width - text_size * len(action.title)) // 2
      text_y: int = offset + (button_height - text_size) // 2
      # print(f"fb.rect({button_x}, {offset}, button_width, button_height, button_colour, True)")
      fb.rect(button_x, offset, button_width, button_height, button_colour, True)
      # print(f"fb.text(action.title, {text_x}, {text_y}, foreground_colour)")
      fb.text(action.title, text_x, text_y, foreground_colour)

      button_column += 1
      if button_column == max_button_count:
        button_column = 0
        offset += 50

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

PrintOutputBody('Game is over')

# ResetOutput()
