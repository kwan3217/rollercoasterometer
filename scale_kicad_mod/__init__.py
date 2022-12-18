"""
Scale a kicad_mod PCB footprint

Note! This does not try to understand the whole file s-code (or whatever it is)
file format, but instead just looks for a particular pattern that Kicad always
uses -- a line (xy xxx.xxxxxx yyy.yyyyyy) is a coordinate to be scaled
"""
import re

scale_factor=0.6

#with open("kicad/kwan_kicad_lib/KwanSystems.pretty/OSHW-Symbol_6.7x6mm_SolderMask.kicad_mod","rt") as inf:
#    with open("kicad/kwan_kicad_lib/KwanSystems.pretty/OSHW-Symbol_4x3.6mm_SolderMask.kicad_mod","wt") as ouf:
with open("kicad/kwan_kicad_lib/KwanSystems.pretty/StKwansSoldermask.kicad_mod", "rt") as inf:
    with open("kicad/kwan_kicad_lib/KwanSystems.pretty/StKwansSoldermask_0.6.kicad_mod", "wt") as ouf:
        for line in inf:
            if match:=re.match("(?P<prefix>\s*\(xy\s+)(?P<x>-?\d+(\.\d+)?)(?P<infix>\s+)(?P<y>-?\d+(\.\d+)?)(?P<suffix>\)\s*)",line):
                line=(match.group("prefix")
                     +f'{float(match.group("x"))*scale_factor:.6f}'+match.group('infix')
                     +f'{float(match.group("y"))*scale_factor:.6f}'+match.group('suffix'))
            print(line,file=ouf,end='')

