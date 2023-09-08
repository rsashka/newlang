#!/bin/bash

root="."
if [ ! -z $1 ]; then
    root="$1"
fi

#echo `pwd` > ssssssssssssssssssssss

#sed -e 's/\/ru\//\//g'  ~/SOURCE/NewLang/newlang/site/content/en/_index.md  >  ~/SOURCE/NewLang/newlang/site/content/en/_index.en.md
# rm ~/SOURCE/NewLang/newlang/site/content/en/_index.md && mv ~/SOURCE/NewLang/newlang/site/content/en/_index.en.md ~/SOURCE/NewLang/newlang/site/content/en/_index.md

for i in $(find $root -name \*.md); do # Not recommended, will break on whitespace
  echo "$i"

  sed -e 's/\/ru\//\//g' "$i"  >  "$i.temp"
  rm "$i"
  mv "$i.temp" "$i"

  sed -e 's/Документация/Documentation/g' "$i"  >  "$i.temp"
  rm "$i"
  mv "$i.temp" "$i"

  sed -e 's/Статьи/Articles/g' "$i"  >  "$i.temp"
  rm "$i"
  mv "$i.temp" "$i"
done


