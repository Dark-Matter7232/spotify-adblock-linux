# spotify-adblock-macos
Spotify adblocker for macos that works by wrapping `getaddrinfo` and *libcurl*'s `curl_easy_setopt` and blocking all domains that aren't whitelisted, as well as blacklisted URLs.
### Build Process 
```
$ git clone https://github.com/Dark-Matter7232/spotify-adblock-mac.git
$ cd spotify-adblock-linux
$ wget -O cef.tar.bz2 http://opensource.spotify.com/cefbuilds/cef_binary_80.0.8%2Bgf96cd1d%2Bchromium-80.0.3987.132_macosx64_minimal.tar.bz2
$ tar -xf cef.tar.bz2 --wildcards '*/include' --strip-components=1
$ make
```
### Notes
Based on [`abba23/spotify-adblock-linux`](https://github.com/abba23/spotify-adblock-linux.git)  
tweaked to inject on macos with `DYLD_INSERT_LIBRARIES`  
updated and maintained by me  
This program requires gcc and will not compile with clang or the default macos gcc.  

## Usage
    $ DYLD_INSERT_LIBRARIES=spotify-adblock.dylib /Applications/Spotify.app/Contents/MacOS/Spotify
