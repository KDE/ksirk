#! /usr/bin/env bash

# process the .ui and .rc files
$EXTRACTRC `find . -name \*.ui -o -name \*.rc` >> rc.cpp
$EXTRACTRC --tag=desc `find skins -iname onu.xml` >> rc.cpp
./extract-onu-names.pl  `find skins -iname onu.xml` >> rc.cpp

$XGETTEXT `find $SRCDIR -name "*.cpp"` rc.cpp -o $podir/ksirk.pot
rm -f rc.cpp

