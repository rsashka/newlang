#!/bin/bash

rm -rf public/*
rm content/ru/playground/*.src
cp ../examples/*.src  content/ru/playground/
cp -f content/ru/playground/* content/en/playground/

hugo server 
