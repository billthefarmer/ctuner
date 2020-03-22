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

AUDIO = Audio
AUDIOOBJ = $(AUDIO).o

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

SOURCES = AppDelegate.swift ScopeView.swift SpectrumView.swift \
		StrobeView.swift DisplayView.swift MeterView.swift \
		StaffView.swift TunerView.swift $(AUDIOOBJ)

SFLAGS = -target x86_64-apple-macosx10.10 \
	-import-objc-header $(HEADER) -emit-objc-header-path $(SWIFTH) \
	-Xlinker -rpath -Xlinker @loader_path/../Frameworks

CFLAGS = -target x86_64-apple-macosx10.10

$(APPBUNDLE): $(APPNAME) $(APPNAME).icns Info.plist Frameworks
	$(RM) -rf $(APPBUNDLE)
	$(MD) $(APPBUNDLE)
	$(MD) $(APPBUNDLECONTENTS)
	$(MD) $(APPBUNDLEMACOS)
	$(MD) $(APPBUNDLERESOURCES)
	$(CP) Info.plist $(APPBUNDLECONTENTS)/
	$(CP) PkgInfo $(APPBUNDLECONTENTS)/
	$(CP) -r Frameworks $(APPBUNDLECONTENTS)/
	$(CP) $(APPNAME).icns $(APPBUNDLERESOURCES)/
	$(CP) MainMenu.nib $(APPBUNDLERESOURCES)/
	$(CP) $(APPNAME).entitlements $(APPBUNDLERESOURCES)/
	$(CP) $(APPNAME) $(APPBUNDLEMACOS)/

$(APPNAME): $(SOURCES)
	$(SWIFTC) $(SOURCES) -o $(APPNAME) $(SFLAGS)

$(AUDIOOBJ):	$(AUDIO).m Audio.h Temperaments.h Makefile

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
	$(RM) Audio.o

$(APPVDMG): $(APPBUNDLE)
	$(HDIU) create -size 16m -fs HFS+ -volname $(APPNAME) -ov $(APPNAME) -quiet
	$(HDIU) attach $(APPDMG) -quiet
	$(CP) -r $(APPBUNDLE) /Volumes/$(APPNAME)
	$(LN) -s /Applications /Volumes/$(APPNAME)/Applications
	$(CP) README.md /Volumes/$(APPNAME)
	$(HDIU) detach /Volumes/$(APPNAME) -quiet
	$(HDIU) convert -format UDZO -o $(APPVDMG) -ov $(APPDMG) -quiet

dmg:	$(APPVDMG)

%.o: %.m
	$(CC) -c $(CFLAGS) $< -o $@
