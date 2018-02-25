#
#  Makefile
#  Tuner
#
#  Created by Bill Farmer on 10/10/2017.
#  Copyright © 2017 Bill Farmer. All rights reserved.
#

RM = rm
MD = mkdir
CP = cp
LN = ln
CC = gcc

HDIU = hdiutil
SWIFTC = swiftc
PLB = /usr/libexec/PlistBuddy

APPNAME = Tuner
APPVERSION = $(shell $(PLB) -c 'print :CFBundleShortVersionString' Info.plist)

APPBUNDLE = $(APPNAME).app
APPBUNDLECONTENTS = $(APPBUNDLE)/Contents
APPBUNDLEMACOS = $(APPBUNDLECONTENTS)/MacOS
APPBUNDLERESOURCES = $(APPBUNDLECONTENTS)/Resources
APPBUNDLEICON = $(APPBUNDLECONTENTS)/Resources

APPDMG = $(APPNAME).dmg
APPVDMG = $(APPNAME)-$(APPVERSION).dmg

HEADER = Tuner-Bridging-Header.h
SWIFTH = Tuner-Swift.h

SOURCES = AppDelegate.swift Scope.swift Spectrum.swift Strobe.swift \
		Display.swift Meter.swift Status.swift TunerView.swift Audio.o

SFLAGS = -g -target x86_64-apple-macosx10.9 \
	-import-objc-header $(HEADER) -emit-objc-header-path $(SWIFTH)

CFLAGS = -g -target x86_64-apple-macosx10.9


$(APPBUNDLE): $(APPNAME) $(APPNAME).icns
	$(RM) -rf $(APPBUNDLE)
	$(MD) $(APPBUNDLE)
	$(MD) $(APPBUNDLECONTENTS)
	$(MD) $(APPBUNDLEMACOS)
	$(MD) $(APPBUNDLERESOURCES)
	$(CP) Info.plist $(APPBUNDLECONTENTS)/
	$(CP) PkgInfo $(APPBUNDLECONTENTS)/
	$(CP) $(APPNAME).icns $(APPBUNDLERESOURCES)/
	$(CP) MainMenu.nib $(APPBUNDLERESOURCES)/
	$(CP) $(APPNAME) $(APPBUNDLEMACOS)/

$(APPNAME): $(SOURCES)
	$(SWIFTC) $(SOURCES) -o $(APPNAME) $(SFLAGS)


$(APPNAME).icns: $(APPNAME).png
	$(RM) -rf $(APPNAME).iconset
	$(MD) $(APPNAME).iconset
	sips -z 16 16   $(APPNAME).png --out $(APPNAME).iconset/icon_16x16.png
	sips -z 32 32   $(APPNAME).png --out $(APPNAME).iconset/icon_16x16@2x.png
	sips -z 32 32   $(APPNAME).png --out $(APPNAME).iconset/icon_32x32.png
	sips -z 64 64   $(APPNAME).png --out $(APPNAME).iconset/icon_32x32@2x.png
	sips -z 128 128 $(APPNAME).png --out $(APPNAME).iconset/icon_128x128.png
	sips -z 256 256 $(APPNAME).png --out $(APPNAME).iconset/icon_128x128@2x.png
	sips -z 256 256 $(APPNAME).png --out $(APPNAME).iconset/icon_256x256.png
	sips -z 512 512 $(APPNAME).png --out $(APPNAME).iconset/icon_256x256@2x.png
	sips -z 512 512 $(APPNAME).png --out $(APPNAME).iconset/icon_512x512.png
	$(CP) $(APPNAME).png $(APPNAME).iconset/icon_512x512@2x.png
	iconutil -c icns -o $(APPNAME).icns $(APPNAME).iconset
	$(RM) -r $(APPNAME).iconset

clean:
	$(RM) -rf $(APPBUNDLE)
	$(RM) $(APPNAME).icns
	$(RM) $(APPNAME)

$(APPVDMG): $(APPBUNDLE)
	$(HDIU) create -size 5m -fs HFS+ -volname $(APPNAME) -ov $(APPNAME) -quiet
	$(HDIU) attach $(APPDMG) -quiet
	$(CP) -r $(APPBUNDLE) /Volumes/$(APPNAME)
	$(LN) -s /Applications /Volumes/$(APPNAME)/Applications
	$(CP) Readme.rtf /Volumes/$(APPNAME)
	$(MD) /Volumes/$(APPNAME)/Sources
	$(CP) CHANGES COPYING Makefile $(APPNAME).c $(APPNAME).h Info.plist \
	PkgInfo $(APPNAME).icns $(APPNAME).png /Volumes/$(APPNAME)/Sources
	$(CP) -r Fonts /Volumes/$(APPNAME)/Sources
	$(HDIU) detach /Volumes/$(APPNAME) -quiet
	$(HDIU) convert -format UDZO -o $(APPVDMG) -ov $(APPDMG) -quiet

%.o: %.m
	$(CC) -c $(CFLAGS) $< -o $@
