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

FOR = for
IN = in
DO = do
DONE = done

HDIU = hdiutil
SWIFTC = swiftc
PLB = /usr/libexec/PlistBuddy
LIPO = lipo
SIPS = sips
IBT = ibtool

AUDIO = Audio
AUDIOX86OBJ = $(AUDIO).x86.o
AUDIOARMOBJ = $(AUDIO).arm.o

APPNAME = Tuner
APPNAMEX86 = $(APPNAME)_x86
APPNAMEARM = $(APPNAME)_arm

APPVERSION = $(shell $(PLB) -c 'print :CFBundleShortVersionString' Info.plist)

APPBUNDLE = $(APPNAME).app
APPBUNDLECONTENTS = $(APPBUNDLE)/Contents
APPBUNDLEMACOS = $(APPBUNDLECONTENTS)/MacOS
APPBUNDLERESOURCES = $(APPBUNDLECONTENTS)/Resources
APPBUNDLEBASE = $(APPBUNDLERESOURCES)/Base.lproj
APPBUNDLEICON = $(APPBUNDLECONTENTS)/Resources

APPDMG = $(APPNAME).dmg
APPVDMG = $(APPNAME)-$(APPVERSION).dmg

HEADER = Tuner-Bridging-Header.h
SWIFTH = Tuner-Swift.h

SOURCES = AppDelegate.swift ScopeView.swift SpectrumView.swift \
	StrobeView.swift DisplayView.swift MeterView.swift \
	StaffView.swift TunerView.swift main.swift

LIBRARIES = libswiftAccelerate.dylib libswiftCoreGraphics.dylib \
	libswiftMetal.dylib libswiftAppKit.dylib libswiftCoreImage.dylib \
	libswiftObjectiveC.dylib libswiftCore.dylib libswiftDarwin.dylib \
	libswiftos.dylib libswiftCoreAudio.dylib libswiftDispatch.dylib \
	libswiftQuartzCore.dylib libswiftCoreData.dylib \
	libswiftFoundation.dylib libswiftSwiftOnoneSupport.dylib \
	libswiftCoreFoundation.dylib libswiftIOKit.dylib libswiftXPC.dylib

XCODELIB = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/swift-5.0/macosx/

FRAMEWORKS = Frameworks

SFLAGSX86 = -target x86_64-apple-macos10.12 \
	-import-objc-header $(HEADER) -emit-objc-header-path $(SWIFTH) \
	-Xlinker -rpath -Xlinker @loader_path/../Frameworks

SFLAGSARM = -target arm64-apple-macos11 \
	-import-objc-header $(HEADER) -emit-objc-header-path $(SWIFTH) \
	-Xlinker -rpath -Xlinker @loader_path/../Frameworks

CFLAGSX86 = -target x86_64-apple-macos10.12

CFLAGSARM = -target arm64-apple-macos11

$(APPBUNDLE): $(APPNAME) $(APPNAME).icns Info.plist $(FRAMEWORKS)
	$(RM) -rf $(APPBUNDLE)
	$(MD) -p $(APPBUNDLEMACOS)
	$(MD) -p $(APPBUNDLERESOURCES)
	$(CP) Info.plist $(APPBUNDLECONTENTS)/
	$(CP) PkgInfo $(APPBUNDLECONTENTS)/
	$(CP) -r $(FRAMEWORKS) $(APPBUNDLECONTENTS)/
	$(CP) $(APPNAME).icns $(APPBUNDLERESOURCES)/
	$(CP) $(APPNAME).entitlements $(APPBUNDLERESOURCES)/
	$(CP) $(APPNAME) $(APPBUNDLEMACOS)/

$(FRAMEWORKS): $(XCODELIB)
	$(FOR) file $(IN) $(LIBRARIES); \
	$(DO) $(CP) $(XCODELIB)$$file $(FRAMEWORKS); \
	$(DONE);

$(APPNAME): $(APPNAMEX86) $(APPNAMEARM)
	$(LIPO) -create -output $(APPNAME) $(APPNAMEX86) $(APPNAMEARM)

$(APPNAMEX86): $(SOURCES) $(AUDIOX86OBJ)
	$(SWIFTC) $(SOURCES) $(AUDIOX86OBJ) -o $(APPNAMEX86) $(SFLAGSX86)

$(APPNAMEARM): $(SOURCES) $(AUDIOARMOBJ)
	$(SWIFTC) $(SOURCES) $(AUDIOARMOBJ) -o $(APPNAMEARM) $(SFLAGSARM)

$(AUDIOX86OBJ):	$(AUDIO).m $(AUDIO).h Temperaments.h Makefile
	$(CC) -c $(CFLAGSX86) $(AUDIO).m -o $(AUDIOX86OBJ)

$(AUDIOARMOBJ):	$(AUDIO).m $(AUDIO).h Temperaments.h Makefile
	$(CC) -c $(CFLAGSARM) $(AUDIO).m -o $(AUDIOARMOBJ)

$(APPNAME).icns: $(APPNAME).png
	$(RM) -rf $(APPNAME).iconset
	$(MD) $(APPNAME).iconset
	$(SIPS) -z 16 16   $(APPNAME).png --out $(APPNAME).iconset/icon_16x16.png
	$(SIPS) -z 32 32   $(APPNAME).png --out $(APPNAME).iconset/icon_16x16@2x.png
	$(SIPS) -z 32 32   $(APPNAME).png --out $(APPNAME).iconset/icon_32x32.png
	$(SIPS) -z 64 64   $(APPNAME).png --out $(APPNAME).iconset/icon_32x32@2x.png
	$(SIPS) -z 128 128 $(APPNAME).png --out $(APPNAME).iconset/icon_128x128.png
	$(SIPS) -z 256 256 $(APPNAME).png --out $(APPNAME).iconset/icon_128x128@2x.png
	$(SIPS) -z 256 256 $(APPNAME).png --out $(APPNAME).iconset/icon_256x256.png
	$(SIPS) -z 512 512 $(APPNAME).png --out $(APPNAME).iconset/icon_256x256@2x.png
	$(SIPS) -z 512 512 $(APPNAME).png --out $(APPNAME).iconset/icon_512x512.png
	$(SIPS) -z 1024 1024 $(APPNAME).png --out $(APPNAME).iconset/icon_512x512@2x.png
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

.PHONY:	$(FRAMEWORKS)
