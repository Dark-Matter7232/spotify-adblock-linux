# spotify-adblock-macos
Spotify adblocker for macos that works by wrapping `getaddrinfo` and *libcurl*'s `curl_easy_setopt` and blocking all domains that aren't whitelisted, as well as blacklisted URLs.
# Build
Prerequisites:
* Git
* Wget
* tar
* Make
* GCC

### Build Process 
```
$ brew install gcc make gnutar  
$ git clone https://github.com/Dark-Matter7232/spotify-adblock-mac.git
$ cd spotify-adblock-mac
$ wget -O cef.tar.bz2 https://cef-builds.spotifycdn.com/cef_binary_91.1.21%2Bg9dd45fe%2Bchromium-91.0.4472.114_macosx64_minimal.tar.bz2
$ gtar -xf cef.tar.bz2 --wildcards '*/include' --strip-components=1
$ make
```
# Notes
* Maintained By ['Dark-Matter7232'](https://github.com/dark-matter7232)  
* Based on [`abba23/spotify-adblock-linux`](https://github.com/abba23/spotify-adblock-linux.git)  
* tweaked to inject on macos with `DYLD_INSERT_LIBRARIES`  
* This program requires gcc and will not compile with clang or the default macos gcc.  

# Usage
    $ DYLD_INSERT_LIBRARIES=spotify-adblock.dylib /Applications/Spotify.app/Contents/MacOS/Spotify
