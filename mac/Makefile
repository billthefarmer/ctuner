################################################################################
#
#  Makefile for Tuner
#
################################################################################

RM = rm
MD = mkdir
CP = cp
LN = ln
PLB = /usr/libexec/PlistBuddy
HDIU = hdiutil

GCC = gcc
CFLAGS = -framework Carbon -framework Audiounit -framework CoreAudio \
		-framework Accelerate -arch i386 \
		-mmacosx-version-min=10.4

APPNAME = Tuner
APPVERSION = $(shell $(PLB) -c 'print :CFBundleShortVersionString' Info.plist)

APPBUNDLE = $(APPNAME).app
APPBUNDLECONTENTS = $(APPBUNDLE)/Contents
APPBUNDLEMACOS = $(APPBUNDLECONTENTS)/MacOS
APPBUNDLERESOURCES = $(APPBUNDLECONTENTS)/Resources
APPBUNDLEICON = $(APPBUNDLECONTENTS)/Resources

APPDMG = $(APPNAME).dmg
APPVDMG = $(APPNAME)-$(APPVERSION).dmg

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

$(APPBUNDLE): $(APPNAME) $(APPNAME).icns
	$(RM) -rf $(APPBUNDLE)
	$(MD) $(APPBUNDLE)
	$(MD) $(APPBUNDLECONTENTS)
	$(MD) $(APPBUNDLEMACOS)
	$(MD) $(APPBUNDLERESOURCES)
	$(CP) Info.plist $(APPBUNDLECONTENTS)/
	$(CP) PkgInfo $(APPBUNDLECONTENTS)/
	$(CP) $(APPNAME).icns $(APPBUNDLERESOURCES)/
	$(CP) -r Fonts $(APPBUNDLERESOURCES)/
	$(CP) $(APPNAME) $(APPBUNDLEMACOS)/

$(APPNAME): $(APPNAME).c
	$(GCC) $(APPNAME).c -o $(APPNAME) $(CFLAGS)

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
