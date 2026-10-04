#!/usr/bin/env python3
"""Turn a .fd file into functable.h: one QT_FUNC(name) per entry, in LVO order."""
import re
import sys

ENTRY = re.compile(r"^([A-Za-z_]\w*)\s*\(")


def main():
    if len(sys.argv) != 3:
        sys.exit("usage: fd2functable.py <file.fd> <functable.h>")
    entries = []  # (name or None for an unused slot, LVO)
    offset = None
    with open(sys.argv[1], encoding="latin-1") as fd:
        for line in fd:
            line = line.strip()
            if line.startswith("##bias"):
                bias = int(line.split()[1])
                if offset is None:
                    if bias != 30:
                        sys.exit("%s: first ##bias must be 30" % sys.argv[1])
                    offset = bias
                if bias < offset or (bias - offset) % 6:
                    sys.exit("%s: ##bias %d does not follow -%d" % (sys.argv[1], bias, offset))
                while offset < bias:
                    entries.append((None, offset))
                    offset += 6
            elif line and not line.startswith("*") and not line.startswith("##"):
                match = ENTRY.match(line)
                if not match:
                    sys.exit("%s: cannot parse '%s'" % (sys.argv[1], line))
                if offset is None:
                    offset = 30
                entries.append((match.group(1), offset))
                offset += 6
    with open(sys.argv[2], "w") as out:
        out.write("/* Generated from %s - do not edit */\n" % sys.argv[1])
        for name, lvo in entries:
            if name:
                out.write("QT_FUNC(%s) /* -%d */\n" % (name, lvo))
            else:
                out.write("QT_PAD /* -%d */\n" % lvo)


if __name__ == "__main__":
    main()
