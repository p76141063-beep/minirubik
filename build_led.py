#!/usr/bin/env python3
from pathlib import Path
import argparse
parser=argparse.ArgumentParser()
parser.add_argument("--render", type=int, choices=(0,1), required=True)
args=parser.parse_args()
source=Path(__file__).with_name("solver_led.S")
lines=[];active=True;stack=[]
for line in source.read_text().splitlines():
    if line.startswith("#ifndef"):
        stack.append(active);active=False
    elif line.startswith("#define"):
        continue
    elif line=="#if RENDER":
        stack.append(active);active=active and bool(args.render)
    elif line=="#endif":
        active=stack.pop()
    elif active:
        lines.append(line)
if stack:raise RuntimeError("Unbalanced conditional")
out=source.with_name("solver_led_gui.s" if args.render else "solver_led_cli.s")
out.write_text("\n".join(lines)+"\n",encoding="ascii")
print(out.name)
