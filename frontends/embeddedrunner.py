from embeddedgame import (
  backend_ActionScreen,
  backend_Game,
  backend_GameOutcome,
  backend_Screen,
  backend_ScreenAction,
  campaigns__UntitledTextAdventure_UntitledTextAdventure_Fields_
)
import sys
import uselect

inputPoll: Poll

def SetupConsole() -> None:
  global inputPoll

  inputPoll = uselect.poll()
  inputPoll.register(sys.stdin, uselect.POLLIN)

def ResetConsole() -> None:
  global inputPoll

  inputPoll.unregister(sys.stdin)

# Based on https://forum.micropython.org/viewtopic.php?p=42665&sid=422c614f932259441421141222552e56#p42665
def GetButtonInput() -> int:
  global inputPoll

  while True:
    inputChar: str = sys.stdin.read(1) if inputPoll.poll(0) else None
    if inputChar is None or not inputChar.isdigit():
      continue

    inputVal: int = int(inputChar)
    if 1 <= inputVal and inputVal <= 9:
        return inputVal - 1

def PrintOutputBody(body: str) -> None:
  print(body)

def PrintButtonInputs(state: backend_Game, screen: backend_ActionScreen) -> None:
  print("\n\nUse the numbers below to make a selection.")

  actions: list[backend_ScreenAction] = screen.GetActions(state)
  inputNumber: int = 0
  for action in actions:
    if action.isVisible(state, screen):
      inputNumber += 1
      print(f"{inputNumber}. {action.title}")


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
  screen: backend_Screen = state.getScreen()
  if not isinstance(screen, backend_ActionScreen):
    return False

  actionScreen: backend_ScreenAction = screen
  actions: list[backend_ScreenAction] = actionScreen.GetActions(state)

  inputIndex: int = GetButtonInput()
  index: int = MapInputIndex(state, actionScreen, actions, inputIndex)
  if index >= len(actions):
    return True

  outcome: backend_GameOutcome = actions[index].handleAction(state)
  if outcome is backend_GameOutcome.GetNextOutput:
    return True
  elif outcome is backend_GameOutcome.QuitGame:
    return False
  else:
    raise Exception(f"Unknown screen action outcome {outcome} received.")


SetupConsole()

state = backend_Game(campaigns__UntitledTextAdventure_UntitledTextAdventure_Fields_.UntitledTextAdventure)

while True:
  if not HandleOutput(state):
    break
  elif not HandleInput(state):
    break

ResetConsole()

print('Game is over')
