#/bin/sh
# Helper for Qt5 libs to generate all OctoXBPS translations

TRANSLATIONS=./resources/translations/*

for f in $TRANSLATIONS
do
  lrelease-qt5 $f
done
