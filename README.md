# spotify-adblock-macos
Spotify adblocker for macos that works by wrapping `getaddrinfo` and *libcurl*'s `curl_easy_setopt` and blocking all domains that aren't whitelisted, as well as blacklisted URLs.

### Notes
Based on [`abba23/spotify-adblock-linux`](https://github.com/abba23/spotify-adblock-linux.git)  
tweaked to inject on macos with `DYLD_INSERT_LIBRARIES`  
updated and maintained by me  
This program requires gcc and will not compile with clang or the default macos gcc.  

## Usage
    $ DYLD_INSERT_LIBRARIES=spotify-adblock.dylib /Applications/Spotify.app/Contents/MacOS/Spotify
