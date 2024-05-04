---
title: Simple complex programming
slug: simple-complex-programming
date: 2024-05-04
---

I always pay attention to assessing the complexity of programming in a particular language. Programming is indeed not an easy task and this is perceived as a fact and usually does not require any confirmation.

But the concept of “complexity” is akin to the term “heap”. For some, five coconuts is not so much, but for someone who ate one and “didn’t want any more,” this means that even one coconut will be too much for him.

The same goes for the complexity of programs. It seems that the constant increase in the complexity of programs is obvious to everyone and is observed in all areas of application of IT technologies, and programming languages themselves become more and more complex as they develop, but assessing “complexity” using numerical metrics is a problem. obviously a thankless task, but also “You can’t manage what you can’t measure...”


Typically, talk of “complexity” only implies value judgments without any numerical evaluation. And since I am personally interested in the issue of the complexity of programming languages, I decided to calculate the complexity of implementing the gcc compiler on some conditional “parrots”. What if we could see some patterns of difficulty changing over time?


### Choosing "parrots" to measure
I didn’t come up with my own and calculate empirical program code metrics and, as a “parrot,” I decided to take the simplest metric [SLOC (Source Lines of Code)](https://en.wikipedia.org/wiki/Source_lines_of_code) - the number of lines of source code compiler, which is very easy to calculate.

But it will be possible to evaluate the complexity of a language with its help only under the following assumption: the complexity of the language should be directly dependent on the complexity of its implementation, if simple syntactic structures require less code than more complex ones.

Of course, using the “number of lines of source code” metric has its drawbacks, since it strongly depends on the programming language used, the style of source code and, in general, does not allow correct comparison of several different projects.

But for numerically assessing the complexity of code within **one project**, the SLOC metric is well suited.

### Methodology for calculating SLOC
Initially I tried to use a simple bash script with mask search and counting the number of lines in source files via `wc -l`. But after a while it became clear that I had to invent another bicycle.

So I decided to take a ready-made utility [SLOCCount](https://dwheeler.com/sloccount/), which can analyze almost three dozen types of sources .  
   
Moreover, it counts not just the number of lines of source code, but can ignore comments, exclude duplicate files from the calculation (compares their hash sums), and also displays the estimated labor intensity, an approximate estimate of the cost of developing the analyzed project file and other characteristics.

I was initially interested in the volume of source codes in C/C++ and, possibly, in assembler, if there were a lot of such files. But after I started working, I was very glad that I did not reinvent the wheel, but took a ready-made toolkit, because it separately calculates the statistics of the Yacc/Bison parser source files (.y), which determines the actual complexity of the parser (read the complexity of the programming language syntax).

I took the old gcc sources from https://gcc.gnu.org/mirrors.html, but before running the analyzer I deleted the directories of other compilers (ada, fortran, java, etc.) so that they would not be included in the final statistics.

### Results in "parrots"

![](https://habrastorage.org/webt/yb/cs/bo/ybcsbo5gssucq3ccllzub3c6mju.jpeg)

#### Yacc/Bison parser code size

![](https://habrastorage.org/webt/7l/b5/-n/7lb5-nwrjzxstyazy6msyr-jrng.png)


#### Size of entire GCC source code (C and C++ languages only)

![](https://habrastorage.org/webt/wn/ae/9h/wnae9haqifqpl3kib4miu1pun4i.jpeg)


Unfortunately, the Yacc/Bison parser was used only up to version 3, and after that its use was reduced to nothing. That's why we can estimate the complexity of C/C++ syntax with the help of the parser's code volume only till about 1996-98, after which it was gradually phased out, i.e. a little less than ten years. But even during this period the volume of the parser's code base grew two times, which approximately corresponds to the time of implementing the C99 standard.

But even if we don't take into account the code of the parser, the volume of the total code base also correlates with the introduction of new C++ standards: C99, C11 and C14. 

The graph doesn't show a pronounced peak for C+17 and the next versions, but I suppose that with the current size of the code base (more than 4 million lines of C and C++ code only), several thousand lines needed to support syntactic constructions of new standards are simply unnoticeable. 

### The first conclusion is obvious. Increasing complexity of development tools

In fact, on the example of the GCC project we can see the constant and inevitable growth of complexity of programmers' working tools.

And no matter how much they talk about degradation of development, about system crisis of software, which is generational in nature, but it seems to me that the matter is a bit different.

Personnel renewal and as a consequence - the necessity to train new employees in old developments, here it is not so much about knowledge transfer as about the possibility to **absorb** this knowledge.

And the ability to assimilate knowledge for different generations will be different, but not because the previous generation was smarter, and not because the new generation does not have enough sense to understand it. It's just that the environment itself is changing and the working tools are more complicated than those used by the previous generation.

###  The second conclusion is the entry threshold
Imagine that you need to “make your own website”. Naturally, you need to determine which CMS to use for this.

And if there are simple solutions for simple sites, then for those who are not looking for easy ways, there is the CMS Drupal, which is notable for the fact that it has a fantastically high entry threshold for starting to use it.

![](https://habrastorage.org/webt/ei/gl/z1/eiglz1cnctqm0y4dsjtfvedezuw.jpeg)

What I'm talking about? When using any tool, such as a programming language, there is a certain minimum level of comfort level.

Moreover, this level is directly proportional to the size of the target audience for which it is intended. More precisely, **the size of the possible audience** is determined, among other things, by the requirements for the level of starting knowledge and qualifications of the potential user.


### The final conclusion is disappointing
If we consider the increase in complexity of the software itself, then this is one thing. Here's an example:

- September 17, 1991: Linux version 0.01 (*10,239* lines of code).
- March 14, 1994: Linux version 1.0.0 (*176,250* lines of code).
- March 1995: Linux version 1.2.0 (*310,950* lines of code).
- June 9, 1996: Linux version 2.0.0 (*777,956* lines of code). 
- January 25, 1999: Linux version 2.2.0, initially rather unfinished (*1,800,847* lines of code). 
- January 4, 2001: Linux version 2.4.0 (*3,377,902* lines of code). 
- December 18, 2003: Linux version 2.6.0 (*5,929,913* lines of code). 
- March 23, 2009: Linux version 2.6.29, temporary Linux symbol - Tasmanian devil Tuz (*11,010,647* lines of code). 
- July 22, 2011: Linux 3.0 released (*14.6* million lines of code). 
- October 24, 2011: Linux 3.1 release. 
- January 15, 2012: Linux 3.3 release surpasses *15 million* lines of code. 
- February 23, 2015: First release candidate of Linux 4.0 (*more than 19 million* lines of code). 
- January 7, 2019: First release candidate of Linux 5.0 (*more than 26 million* lines of code) ...

And what if the complexity of software is superimposed on the tendency of constant complication of the working tools themselves? After all, the constant development of programming languages inevitably raises the entry threshold for all beginners and only exacerbates the problem of software development complexity.

In other words, no matter how well documented the code is and how completely it is covered with tests, after some time the tools used become obsolete, the life cycles of external dependencies are completed, and most importantly, new people come to replace those who have developed or managed to understand the system.

And new people have a need to understand the system from the beginning, but under **different initial conditions**. And because of this, the complexity of learning the system for all new people will be higher simply by the fact that the external conditions have changed and the working tools that new employees have to use have become more complex.

It is clear that the further you go, the easier it will not be. After all, the IT field is the most competitive environment. And how not to remember Lewis Carroll, that his winged expression 

> You need to run as fast just to stay in place, but to get somewhere, you must run at least twice as fast!
 
This applies not only to Alice in Wonderland, but to all information technology in general!