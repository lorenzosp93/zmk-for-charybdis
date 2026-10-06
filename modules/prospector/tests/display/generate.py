"""Compile real screen/widget bodies against a small simulated ZMK environment."""
import re
import sys
from pathlib import Path
root = Path(__file__).parent
parts = [(root / "prefix.c").read_text()]
for source in sys.argv[1:3]:
    parts.append(re.sub(r"^#include[^\n]*\n", "", Path(source).read_text(), flags=re.MULTILINE))
parts.append((root / "main.c").read_text())
Path(sys.argv[3]).write_text("\n".join(parts))
