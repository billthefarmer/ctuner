//
//  Audio.h
//  Tuner
//
//  Created by Bill Farmer on 18/02/2018.
//  Copyright © 2018 Bill Farmer. All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef Audio_h
#define Audio_h

#include <pthread.h>
#include <Cocoa/Cocoa.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudio.h>
#include <Accelerate/Accelerate.h>

#include "Tuner-Swift.h"

// Macros
#define Length(a) (sizeof(a) / sizeof(a[0]))

// Constants
#define kMin        0.5
#define kScale   2048.0
#define kTimerDelay 0.1

// Audio in values
enum
    {kSampleRate1      = 11025,
     kSampleRate2      = 12000,
     kBytesPerPacket   = 4,
     kBytesPerFrame    = 4,
     kChannelsPerFrame = 1};

// Audio processing values
enum
    {kOversample = 16,
     kSamples = 16384,
     kLog2Samples = 14,
     kSamples2 = kSamples / 2,
     kMaxima = 8,
     kFrames = 512,
     kRange = kSamples * 7 / 16,
     kStep = kSamples / kOversample};

// Tuner reference values
enum
    {kA5Reference = 440,
     kC5Offset    = 57,
     kAOffset     = 9,
     kOctave      = 12,
     kEqual       = 8};

// Checkbox tags
enum
    {kZoom   = 'Zoom',
     kFilt   = 'Filt',
     kMult   = 'Mult',
     kFund   = 'Fund',
     kStrobe = 'Strb',
     kDown   = 'Down',
     kLock   = 'Lock',
     kNote   = 'Note'};

// Expand and colour tags
enum
    {kExpand = 'Expd',
     kColour = 'Colr'};

// Reference tags
enum
    {kRefText = 'RefT',
     kRefStep = 'RefS'};

// Transpose and temperament tags
enum
    {kTrans = 'Tran',
     kTemp  = 'Temp',
     kKey   = 'Key '};

// Maximum
typedef struct
{
    double f;
    double fr;
    int n;
} maximum;

ScopeView *scopeView;
SpectrumView *spectrumView;
DisplayView *displayView;
StrobeView *strobeView;
StaffView *staffView;
MeterView *meterView;

// Scope data
typedef struct
{
    double *data;
    int length;
} ScopeData;
ScopeData scopeData;

// Spectrum data
typedef struct
{
    int length;
    int expand;
    int count;
    bool zoom;
    double f;
    double r;
    double l;
    double h;
    double *data;
    double *values;
} SpectrumData;
SpectrumData spectrumData;

// Display data
typedef struct
{
    maximum *maxima;
    double f;
    double fr;
    double c;
    bool lock;
    bool zoom;
    bool mult;
    int count;
    int n;
} DisplayData;
DisplayData displayData;

// Strobe data
typedef struct
{
    bool changed;
    bool enable;
    int colours;
    double c;
} StrobeData;
StrobeData strobeData;

// Staff data
typedef struct
{
    bool enable;
    int note;
} StaffData;
StaffData staffData;

// Meter data
typedef struct
{
    double c;
} MeterData;
MeterData meterData;

// Temperaments data
double temperamentValues[32][12] =
  {
        // Kirnberger II
        {1.000000000, 1.053497163, 1.125000000, 1.185185185,
         1.250000000, 1.333333333, 1.406250000, 1.500000000,
         1.580245745, 1.677050983, 1.777777778, 1.875000000},

        // Kirnberger III
        {1.000000000, 1.053497163, 1.118033989, 1.185185185,
         1.250000000, 1.333333333, 1.406250000, 1.495348781,
         1.580245745, 1.671850762, 1.777777778, 1.875000000},

        // Werckmeister III
        {1.000000000, 1.053497942, 1.117403309, 1.185185185,
         1.252827249, 1.333333333, 1.404663923, 1.494926960,
         1.580246914, 1.670436332, 1.777777778, 1.879240873},

        // Werckmeister IV
        {1.000000000, 1.048750012, 1.119929822, 1.185185185,
         1.254242806, 1.333333333, 1.404663923, 1.493239763,
         1.573125018, 1.672323742, 1.785826183, 1.872885231},

        // Werckmeister V
        {1.000000000, 1.057072991, 1.125000000, 1.189207115,
         1.257078722, 1.337858004, 1.414213562, 1.500000000,
         1.580246914, 1.681792831, 1.783810673, 1.885618083},

        // Werckmeister VI
        {1.000000000, 1.053497942, 1.114163307, 1.187481762,
         1.255862545, 1.333333333, 1.410112936, 1.497099016,
         1.580246914, 1.674483394, 1.781222643, 1.883793818},

        // Bach (Klais)
        {262.76, 276.87, 294.30, 311.46, 328.70, 350.37,
         369.18, 393.70, 415.30, 440.00, 467.18, 492.26},

        // Just (Barbour)
        {264.00, 275.00, 297.00, 316.80, 330.00, 352.00,
         371.25, 396.00, 412.50, 440.00, 475.20, 495.00},

        // Equal
        {1.000000000, 1.059463094, 1.122462048, 1.189207115,
         1.259921050, 1.334839854, 1.414213562, 1.498307077,
         1.587401052, 1.681792831, 1.781797436, 1.887748625},

        // Pythagorean
        {1.000000000, 1.067871094, 1.125000000, 1.185185185,
         1.265625000, 1.333333333, 1.423828125, 1.500000000,
         1.601806641, 1.687500000, 1.777777778, 1.898437500},

        // Van Zwolle
        {1.000000000, 1.053497942, 1.125000000, 1.185185185,
         1.265625000, 1.333333333, 1.404663923, 1.500000000,
         1.580246914, 1.687500000, 1.777777778, 1.898437500},

        // Meantone (-1/4)
        {1.000000000, 1.044906727, 1.118033989, 1.196279025,
         1.250000000, 1.337480610, 1.397542486, 1.495348781,
         1.562500000, 1.671850762, 1.788854382, 1.869185977},

        // Silbermann (-1/6)
        {1.000000000, 1.052506113, 1.120351187, 1.192569588,
         1.255186781, 1.336096753, 1.406250000, 1.496897583,
         1.575493856, 1.677050983, 1.785154534, 1.878886059},

        // Salinas (-1/3)
        {1.000000000, 1.037362210, 1.115721583, 1.200000000,
         1.244834652, 1.338865900, 1.388888889, 1.493801582,
         1.549613310, 1.666666667, 1.792561899, 1.859535972},

        // Zarlino (-2/7)
        {1.000000000, 1.041666667, 1.117042372, 1.197872314,
         1.247783660, 1.338074130, 1.393827219, 1.494685500,
         1.556964062, 1.669627036, 1.790442378, 1.865044144},

        // Rossi (-1/5)
        {1.000000000, 1.049459749, 1.119423732, 1.194051981,
         1.253109491, 1.336650124, 1.402760503, 1.496277870,
         1.570283397, 1.674968957, 1.786633554, 1.875000000},

        // Rossi (-2/9)
        {1.000000000, 1.047433739, 1.118805855, 1.195041266,
         1.251726541, 1.337019165, 1.400438983, 1.495864870,
         1.566819334, 1.673582375, 1.787620248, 1.872413760},

        // Rameau (-1/4)
        {1.000000000, 1.051417112, 1.118033989, 1.179066456,
         1.250000000, 1.337480610, 1.401889482, 1.495348781,
         1.577125668, 1.671850762, 1.775938357, 1.869185977},

        // Kellner
        {1.000000000, 1.053497942, 1.118918532, 1.185185185,
         1.251978681, 1.333333333, 1.404663923, 1.495940194,
         1.580246914, 1.673835206, 1.777777778, 1.877968022},

        // Vallotti
        {1.000000000, 1.055879962, 1.119929822, 1.187864958,
         1.254242806, 1.336348077, 1.407839950, 1.496616064,
         1.583819943, 1.676104963, 1.781797436, 1.877119933},

        // Young II
        {1.000000000, 1.053497942, 1.119929822, 1.185185185,
         1.254242806, 1.333333333, 1.404663923, 1.496616064,
         1.580246914, 1.676104963, 1.777777778, 1.877119933},

        // Bendeler III
        {1.000000000, 1.057072991, 1.117403309, 1.185185185,
         1.257078722, 1.333333333, 1.409430655, 1.494926960,
         1.585609487, 1.676104963, 1.777777778, 1.879240873},

        // Neidhardt I
        {1.000000000, 1.055879962, 1.119929822, 1.186524315,
         1.254242806, 1.333333333, 1.407839950, 1.496616064,
         1.583819943, 1.676104963, 1.777777778, 1.879240873},

        // Neidhardt II
        {1.000000000, 1.057072991, 1.119929822, 1.187864958,
         1.255659964, 1.334839854, 1.411023157, 1.496616064,
         1.583819943, 1.676104963, 1.781797436, 1.883489946},

        // Neidhardt III
        {1.000000000, 1.057072991, 1.119929822, 1.187864958,
         1.255659964, 1.333333333, 1.411023157, 1.496616064,
         1.583819943, 1.676104963, 1.779786472, 1.883489946},

        // Bruder 1829
        {1.000000000, 1.056476308, 1.124364975, 1.187194447,
         1.253534828, 1.334086381, 1.409032810, 1.499576590,
         1.583819943, 1.678946488, 1.779786472, 1.879240873},

        // Barnes 1977
        {1.000000000, 1.055879962, 1.119929822, 1.187864958,
         1.254242806, 1.336348077, 1.407839950, 1.496616064,
         1.583819943, 1.676104963, 1.781797436, 1.881364210},

        // Lambert 1774
        {1.000000000, 1.055539344, 1.120652732, 1.187481762,
         1.255862545, 1.335916983, 1.407385792, 1.497099016,
         1.583309016, 1.677728102, 1.781222643, 1.880150581},

        // Schlick (H. Vogel)
        {1.000000000, 1.050646611, 1.118918532, 1.185185185,
         1.251978681, 1.336951843, 1.400862148, 1.495940194,
         1.575969916, 1.673835206, 1.782602458, 1.872885231},

        // Meantone # (-1/4)
        {1.000000000, 1.044906727, 1.118033989, 1.168241235,
         1.250000000, 1.337480610, 1.397542486, 1.495348781,
         1.562500000, 1.671850762, 1.746928107, 1.869185977},

        // Meantone b (-1/4)
        {1.000000000, 1.069984488, 1.118033989, 1.196279025,
         1.250000000, 1.337480610, 1.431083506, 1.495348781,
         1.600000000, 1.671850762, 1.788854382, 1.869185977},

        // Lehman-Bach
        {1.000000000, 1.058267368, 1.119929822, 1.187864958,
         1.254242806, 1.336348077, 1.411023157, 1.496616064,
         1.585609487, 1.676104963, 1.779786472, 1.881364210},
    };

// Audio data
typedef struct
{
    AudioUnit output;
    AudioDeviceID id;
    int divisor;
    int frames;
    int temper;
    int trans;
    int key;
    double *buffer;
    double sample;
    double reference;
    bool fund;
    bool filt;
    bool note;
    bool down;
} AudioData;
AudioData audioData;

// Audio filter
typedef struct
{
    bool note[12];
    bool octave[9];
} FilterData;
FilterData filterData;

// Functions
OSStatus SetupAudio(void);
OSStatus ShutdownAudio(void);
OSStatus InputProc(void *, AudioUnitRenderActionFlags *,
		   const AudioTimeStamp *, UInt32, UInt32,
		   AudioBufferList *);
char *AudioUnitErrString(OSStatus);
void (^ProcessAudio)();

// Boolean array access functions
bool getNote(int);
void setNote(bool, int);
bool getOctave(int);
void setOctave(bool, int);

#endif /* Audio_h */
