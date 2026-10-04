#!/usr/bin/env python3
"""Compare the frames captured by two test runs and write an HTML report.

usage: compare.py <reference capture dir> <candidate capture dir> <report dir> [--no-known]

For every test the last captured frame (<test>_<n>.bmp with the highest n) is
compared pixel by pixel. A pixel differs when one of its channels differs by
more than TOLERANCE; a test passes when at most MAX_DIFF_RATIO of its pixels
differ. Tests listed in tests/known-differences.txt are reported as KNOWN
instead of FAIL when they differ, unless --no-known is given (for comparisons
with a snapshot of an earlier build). Exit code 0 when no test fails, 1 otherwise.
"""
import html
import os
import re
import shutil
import struct
import sys

TOLERANCE = 8
MAX_DIFF_RATIO = 0.001


def read_bmp(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[:2] != b"BM":
        raise ValueError("not a BMP file")
    offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    bits = struct.unpack_from("<H", data, 28)[0]
    if bits != 24:
        raise ValueError("only 24-bit BMP files are supported")
    row = (width * 3 + 3) & ~3
    # Rows bottom-up; keep them as stored, both images use the same order.
    pixels = [data[offset + y * row: offset + y * row + width * 3] for y in range(abs(height))]
    return width, abs(height), pixels


def write_bmp(path, width, height, rows):
    row = (width * 3 + 3) & ~3
    pad = b"\0" * (row - width * 3)
    body = b"".join(bytes(r) + pad for r in rows)
    header = struct.pack("<2sIHHI", b"BM", 54 + len(body), 0, 0, 54)
    info = struct.pack("<IiiHHIIiiII", 40, width, height, 1, 24, 0, len(body), 2835, 2835, 0, 0)
    with open(path, "wb") as f:
        f.write(header + info + body)


def last_frames(directory):
    frames = {}
    if not os.path.isdir(directory):
        return frames
    for name in os.listdir(directory):
        match = re.match(r"^(.+)_(\d{3})\.bmp$", name)
        if match:
            test, number = match.group(1), int(match.group(2))
            if test not in frames or number > frames[test][0]:
                frames[test] = (number, os.path.join(directory, name))
    return {test: path for test, (_, path) in frames.items()}


def compare(ref_path, new_path, diff_path):
    w1, h1, ref = read_bmp(ref_path)
    w2, h2, new = read_bmp(new_path)
    if (w1, h1) != (w2, h2):
        return None, "size %dx%d vs %dx%d" % (w1, h1, w2, h2)
    differing = 0
    diff_rows = []
    for a, b in zip(ref, new):
        out = bytearray(len(a))
        for i in range(0, len(a), 3):
            if max(abs(a[i] - b[i]), abs(a[i + 1] - b[i + 1]), abs(a[i + 2] - b[i + 2])) > TOLERANCE:
                differing += 1
                out[i:i + 3] = b"\x00\x00\xff"  # red (BGR)
            else:
                grey = (a[i] + a[i + 1] + a[i + 2]) // 12
                out[i:i + 3] = bytes((grey, grey, grey))
        diff_rows.append(out)
    write_bmp(diff_path, w1, h1, diff_rows)
    return differing / float(w1 * h1), "%dx%d" % (w1, h1)


def read_log(directory):
    path = os.path.join(directory, "amiga.log")
    if not os.path.exists(path):
        return "(no amiga.log - the tests did not run)"
    with open(path, "rb") as f:
        return f.read().decode("latin-1")


def read_known_differences():
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "known-differences.txt")
    known = {}
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith("#"):
                    test, _, reason = line.partition(" ")
                    known[test] = reason.strip()
    return known


def main():
    args = [a for a in sys.argv[1:] if a != "--no-known"]
    if len(args) != 3:
        sys.exit(__doc__)
    ref_dir, new_dir, report_dir = args
    known = {} if "--no-known" in sys.argv else read_known_differences()
    if os.path.isdir(report_dir):
        shutil.rmtree(report_dir)
    os.makedirs(report_dir)

    ref_frames = last_frames(ref_dir)
    new_frames = last_frames(new_dir)
    rows = []
    failures = 0
    for test in sorted(set(ref_frames) | set(new_frames)):
        ref, new = ref_frames.get(test), new_frames.get(test)
        images = {}
        for kind, path in (("ref", ref), ("new", new)):
            if path:
                images[kind] = "%s_%s.bmp" % (test, kind)
                shutil.copy(path, os.path.join(report_dir, images[kind]))
        if ref and new:
            images["diff"] = "%s_diff.bmp" % test
            ratio, detail = compare(ref, new, os.path.join(report_dir, images["diff"]))
            if ratio is None:
                status, detail = "FAIL", detail
                del images["diff"]
            else:
                status = "PASS" if ratio <= MAX_DIFF_RATIO else "FAIL"
                detail = "%.3f%% of pixels differ (%s)" % (ratio * 100, detail)
                if status == "FAIL" and test in known:
                    status = "KNOWN"
                    detail += " - known difference: " + known[test]
                elif status == "PASS" and test in known:
                    detail += " - listed as a known difference, but no longer differs"
        else:
            status, detail = "FAIL", "no frame from " + ("the new build" if ref else "the reference")
        failures += status == "FAIL"
        rows.append((test, status, detail, images))
        print("%-20s %s  %s" % (test, status, detail))

    if not rows:
        print("no captured frames found")
        failures = 1

    cells = []
    for test, status, detail, images in rows:
        pics = "".join(
            '<figure><img src="%s"><figcaption>%s</figcaption></figure>' % (images[k], label)
            if k in images else "<figure><figcaption>%s: missing</figcaption></figure>" % label
            for k, label in (("ref", "reference"), ("new", "new build"), ("diff", "difference")))
        cells.append('<section class="%s"><h2>%s <span>%s</span></h2><p>%s</p><div>%s</div></section>'
                     % (status.lower(), html.escape(test), status, html.escape(detail), pics))
    page = """<!doctype html><meta charset="utf-8"><title>QuarkTex reference tests</title>
<style>
body{font:14px system-ui,sans-serif;margin:16px;background:#fafafa;color:#222}
section{background:#fff;border:1px solid #ddd;border-radius:6px;padding:8px 12px;margin:12px 0}
h2{font-size:16px;margin:4px 0} h2 span{font-size:12px;padding:2px 6px;border-radius:4px;color:#fff}
.pass h2 span{background:#2a7d2e} .fail h2 span{background:#b3261e} .known h2 span{background:#a15c00}
div{display:flex;flex-wrap:wrap;gap:12px} figure{margin:0} img{image-rendering:pixelated;border:1px solid #ccc}
pre{background:#fff;border:1px solid #ddd;padding:8px;overflow:auto}
</style>
<h1>QuarkTex reference tests</h1>
<p>%d tests, %d failed. Tolerance %d per channel, at most %.1f%% differing pixels.</p>
%s
<h2>Amiga output (reference)</h2><pre>%s</pre>
<h2>Amiga output (new build)</h2><pre>%s</pre>
""" % (len(rows), failures, TOLERANCE, MAX_DIFF_RATIO * 100, "\n".join(cells),
       html.escape(read_log(ref_dir)), html.escape(read_log(new_dir)))
    with open(os.path.join(report_dir, "index.html"), "w", encoding="utf-8") as f:
        f.write(page)
    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()
