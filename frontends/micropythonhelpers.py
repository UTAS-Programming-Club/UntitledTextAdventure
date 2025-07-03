# Uncomment for type checking
# from embeddedgame import (
#   backend_ActionScreen,
#   backend_Game,
#   backend_ScreenAction
# )
import machine
import sys
import time
import uselect

inputPoll: Poll

def SetupOutput() -> None:
  global inputPoll

  inputPoll = uselect.poll()
  inputPoll.register(sys.stdin, uselect.POLLIN)

def ClearOutput() -> None:
  print("\x1b[2J\x1b[H", end="")

def UpdateOutput() -> None:
  pass

def ResetOutput() -> None:
  global inputPoll

  inputPoll.unregister(sys.stdin)

  time.sleep(1)
  machine.reset()


def PrintString(text: str, end="\n", highlight: bool=False) -> None:
  print(text, end=end)

def PrintActionInputs(state: backend_Game, screen: backend_ActionScreen) -> None:
  print("\n\nUse the numbers below to make a selection.")

  actions: list[backend_ScreenAction] = screen.GetActions(state)
  inputNumber: int = 0
  for action in actions:
    if action.isVisible(state):
      inputNumber += 1
      print(f"{inputNumber}. {action.title}")


# Based on https://forum.micropython.org/viewtopic.php?p=42665&sid=422c614f932259441421141222552e56#p42665
def GetActionInput(input_count: int) -> int:
  global inputPoll

  while True:
    inputChar: str = sys.stdin.read(1) if inputPoll.poll(0) else None
    if inputChar is None or not inputChar.isdigit():
      continue

    inputVal: int = int(inputChar)
    if 1 <= inputVal and inputVal <= 9:
        return inputVal - 1
