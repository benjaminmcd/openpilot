import os
from typing import cast

from selfdrive.hardware.base import HardwareBase
from selfdrive.hardware.eon.hardware import Android
from selfdrive.hardware.tici.hardware import Tici
from selfdrive.hardware.pc.hardware import Pc
from selfdrive.hardware.nx.hardware import Nx

EON = os.path.isfile('/EON')
TICI = os.path.isfile('/TICI')
NX = os.path.isfile('/NX')
PC = not (EON or TICI or NX)


if EON:
  HARDWARE = cast(HardwareBase, Android())
elif TICI:
  HARDWARE = cast(HardwareBase, Tici())
elif NX:
  #HARDWARE = cast(HardwareBase, Nx())
  HARDWARE = cast(HardwareBase, Nx())
else:
  HARDWARE = cast(HardwareBase, Pc())
