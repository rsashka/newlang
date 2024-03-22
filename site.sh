#!/bin/bash

root="."
if [ ! -z $1 ]; then
    root="$1"
fi

rm -rf $root/docs/*
rm $root/site/content/ru/playground/*.src
cp $root/examples/*.src  $root/site/content/ru/playground/
cp -f $root/site/content/ru/playground/* $root/site/content/en/playground/

echo Rename link in dir $root/content/en:

for i in $(find $root/site/content/en/ -name \*.md -or -name \*.html); do # Not recommended, will break on whitespace
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


cd  $root/site

hugo server --cleanDestinationDir -d ../docs

