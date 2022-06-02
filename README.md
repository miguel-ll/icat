# imcat

Preview any image in a terminal window.
imcat is a 24-bit image viewer that uses ANSI terminal colours to display any image supported by STB.

## Usage

```
$ imcat file1 [file2 .. fileN]
```

If you want to blend the image with the terminal background, then you need to specify the background color of your terminal. For instance:

```
$ export IMCATBG="#dad9cc"
```

## Authors

[Miguel MKN (Maintainer)](https://bibliotheca.neocities.org/donate.html)

[Bram Stolk](http://stolk.org). A shout out to [Frogtoss](http://github.com/mlabbe) for the idea and help. Thanks!

[stb_image.h](http://nothings.org/stb_image.h) is by Sean Barrett et al.
