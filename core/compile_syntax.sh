#!/bin/bash

bison --output-file=parser.yy.cpp --defines=parser.yy.h --warnings=all parser.y
flex  --outfile=lexer.yy.cpp --header-file=lexer.yy.h --noline  lexer.l