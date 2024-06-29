#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from datetime import datetime
from re import search, sub, findall
from os import path

import argparse
parser = argparse.ArgumentParser()
parser.add_argument('set',
                    nargs    = '*',
                    type     = str,
                    metavar  = 'SET',
                    default  = "main",
                    help     = "Sets of files to update")
parser.add_argument('--major',
                    dest     = 'major',
                    action   = 'store_true',
                    help     = "Major version",
                    required = False)
parser.add_argument('--minor',
                    dest     = 'minor',
                    action   = 'store_true',
                    help     = "Minor version",
                    required = False)
parser.add_argument('--patch',
                    dest     = 'patch',
                    action   = 'store_true',
                    help     = "Patch version",
                    required = False)
parser.add_argument('--dry-run',
                    dest     = 'dry_run',
                    action   = 'store_true',
                    help     = "Dry run",
                    required = False)
args = vars(parser.parse_args())

if not args['major'] and not args['minor'] and not args['patch']:
    print("Nothing to do.")
    exit(0)
else:
    major = int(args['major'])
    minor = int(args['minor'])
    patch = int(args['patch'])

base = [
    path.join("src", "gtools.pkg"),
    path.join("src", "stata.toc"),
    path.join("src", "ado", "_gtools_internal.ado"),
    path.join("src", "ado", "gtools.ado"),
    path.join("docs", "stata", "gtools.sthlp"),
    path.join(".appveyor.yml"),
    path.join("README.md"),
    path.join("docs", "index.md")]
main = [
    path.join("src", "ado", "gcollapse.ado"),
    path.join("src", "ado", "gcontract.ado"),
    path.join("src", "ado", "gegen.ado"),
    path.join("src", "ado", "gunique.ado"),
    path.join("src", "ado", "gdistinct.ado"),
    path.join("src", "ado", "glevelsof.ado"),
    path.join("src", "ado", "gtop.ado"),
    path.join("src", "ado", "gtoplevelsof.ado"),
    path.join("src", "ado", "gisid.ado"),
    path.join("src", "ado", "gstats.ado"),
    path.join("src", "ado", "gduplicates.ado"),
    path.join("src", "ado", "gquantiles.ado"),
    path.join("src", "ado", "fasterxtile.ado"),
    path.join("src", "ado", "hashsort.ado"),
    path.join("docs", "stata", "gcollapse.sthlp"),
    path.join("docs", "stata", "gcontract.sthlp"),
    path.join("docs", "stata", "gegen.sthlp"),
    path.join("docs", "stata", "gunique.sthlp"),
    path.join("docs", "stata", "gdistinct.sthlp"),
    path.join("docs", "stata", "glevelsof.sthlp"),
    path.join("docs", "stata", "gtoplevelsof.sthlp"),
    path.join("docs", "stata", "gisid.sthlp"),
    path.join("docs", "stata", "gstats.sthlp"),
    path.join("docs", "stata", "gduplicates.sthlp"),
    path.join("docs", "stata", "gquantiles.sthlp"),
    path.join("docs", "stata", "hashsort.sthlp")]
test = [path.join("src", "test", "gtools_tests.do")]
plug = [path.join("src", "plugin", "gtools.c")]

callok = False
todo   = base
if "base" in args['set']:
    callok = True

if "main" in args['set']:
    todo  += main
    callok = True

if "test" in args['set']:
    todo  += test
    callok = True

if "plug" in args['set']:
    todo  += plug
    callok = True

if "all" in args['set']:
    todo   = base + main + test + plug
    callok = True

if not callok:
    msg = "Don't know '{0}'".format(', '.join(args['set']))
    print(msg + "; specify any of 'main, test, plug, all'.")
    print("Will ignore; updating main files only.")
else:
    print("Will update version in files:")

months = ["Jan",
          "Feb",
          "Mar",
          "Apr",
          "May",
          "Jun",
          "Jul",
          "Aug",
          "Sep",
          "Oct",
          "Nov",
          "Dec"]
remonths = "(" + '|'.join(months) + ")"

for fname in todo:
    print("\t" + fname)
    with open(fname, 'r') as fhandle:
        flines = fhandle.readlines()

    with open(fname, 'w') as fhandle:
        for line in flines:
            if search('^d.+Distribution.+(\d{8,8})', line):
                today = datetime.strftime(datetime.now(), "%Y%m%d")
                oline = sub("\d{8,8}", today, line)
                print("\t\t" + line)
                print("\t\t" + oline)
                if args['dry_run']:
                    fhandle.write(line)
                else:
                    fhandle.write(oline)

                continue

            v = search(r'((^|\b)v|[Vv]ersion).*?(?P<version>(\d+\.?){3,3})', line)
            s = search('Stata version', line)
            if v and not s:
                try:
                    rep = v.groupdict()['version']
                    res = findall('(\d+)([^\d]|$)', rep)
                    new_major = int(res[0][0]) + major
                    new_minor = 0 if major else int(res[1][0]) + minor
                    new_patch = 0 if major or minor else int(res[2][0]) + patch
                    new = "{0}.{1}.{2}".format(new_major, new_minor, new_patch)
                    oline = line.replace(rep, new)
                    if search("\d+" + remonths + "\d\d+", line):
                        today_day   = datetime.strftime(datetime.now(), "%d")
                        today_month = datetime.strftime(datetime.now(), "%B")
                        today_year  = datetime.strftime(datetime.now(), "%Y")
                        today = today_day + today_month[:3] + today_year
                        oline = sub("\d+" + remonths + "\d\d+", today, oline)

                    print("\t\t" + line)
                    print("\t\t" + oline)
                    if args['dry_run']:
                        fhandle.write(line)
                    else:
                        fhandle.write(oline)
                except:
                    fhandle.write(line)
            else:
                fhandle.write(line)
