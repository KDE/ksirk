#! /usr/bin/env bash
$EXTRACTRC *.rc *.ui *.kcfg >> rc.cpp
$XGETTEXT *.cpp -o $podir/ksirkskineditor.pot
