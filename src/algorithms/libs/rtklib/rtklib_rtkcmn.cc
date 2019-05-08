/*!
 * \file rtklib_rtkcmn.cc
 * \brief rtklib common functions
 * \authors <ul>
 *          <li> 2007-2013, T. Takasu
 *          <li> 2017, Javier Arribas
 *          <li> 2017, Carles Fernandez
 *          </ul>
 *
 * This is a derived work from RTKLIB http://www.rtklib.com/
 * The original source code at https://github.com/tomojitakasu/RTKLIB is
 * released under the BSD 2-clause license with an additional exclusive clause
 * that does not apply here. This additional clause is reproduced below:
 *
 * " The software package includes some companion executive binaries or shared
 * libraries necessary to execute APs on Windows. These licenses succeed to the
 * original ones of these software. "
 *
 * Neither the executive binaries nor the shared libraries are required by, used
 * or included in GNSS-SDR.
 *
 * -------------------------------------------------------------------------
 * Copyright (C) 2007-2013, T. Takasu
 * Copyright (C) 2017, Javier Arribas
 * Copyright (C) 2017, Carles Fernandez
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *----------------------------------------------------------------------------*/

#include "rtklib_rtkcmn.h"
#include <glog/logging.h>
#include <dirent.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>


const double GPST0[] = {1980, 1, 6, 0, 0, 0}; /* gps time reference */
const double GST0[] = {1999, 8, 22, 0, 0, 0}; /* galileo system time reference */
const double BDT0[] = {2006, 1, 1, 0, 0, 0};  /* beidou time reference */

static double timeoffset_ = 0.0;

double leaps[MAXLEAPS + 1][7] = {/* leap seconds (y,m,d,h,m,s,utc-gpst) */
    {2017, 1, 1, 0, 0, 0, -18},
    {2015, 7, 1, 0, 0, 0, -17},
    {2012, 7, 1, 0, 0, 0, -16},
    {2009, 1, 1, 0, 0, 0, -15},
    {2006, 1, 1, 0, 0, 0, -14},
    {1999, 1, 1, 0, 0, 0, -13},
    {1997, 7, 1, 0, 0, 0, -12},
    {1996, 1, 1, 0, 0, 0, -11},
    {1994, 7, 1, 0, 0, 0, -10},
    {1993, 7, 1, 0, 0, 0, -9},
    {1992, 7, 1, 0, 0, 0, -8},
    {1991, 1, 1, 0, 0, 0, -7},
    {1990, 1, 1, 0, 0, 0, -6},
    {1988, 1, 1, 0, 0, 0, -5},
    {1985, 7, 1, 0, 0, 0, -4},
    {1983, 7, 1, 0, 0, 0, -3},
    {1982, 7, 1, 0, 0, 0, -2},
    {1981, 7, 1, 0, 0, 0, -1},
    {}};


const char *formatstrs[32] = {/* stream format strings */
    "RTCM 2",                 /*  0 */
    "RTCM 3",                 /*  1 */
    "NovAtel OEM6",           /*  2 */
    "NovAtel OEM3",           /*  3 */
    "u-blox",                 /*  4 */
    "Superstar II",           /*  5 */
    "Hemisphere",             /*  6 */
    "SkyTraq",                /*  7 */
    "GW10",                   /*  8 */
    "Javad",                  /*  9 */
    "NVS BINR",               /* 10 */
    "BINEX",                  /* 11 */
    "Trimble RT17",           /* 12 */
    "Septentrio",             /* 13 */
    "CMR/CMR+",               /* 14 */
    "LEX Receiver",           /* 15 */
    "RINEX",                  /* 16 */
    "SP3",                    /* 17 */
    "RINEX CLK",              /* 18 */
    "SBAS",                   /* 19 */
    "NMEA 0183",              /* 20 */
    nullptr};


char obscodes[][3] = {
    /* observation code strings */
    "", "1C", "1P", "1W", "1Y", "1M", "1N", "1S", "1L", "1E",   /*  0- 9 */
    "1A", "1B", "1X", "1Z", "2C", "2D", "2S", "2L", "2X", "2P", /* 10-19 */
    "2W", "2Y", "2M", "2N", "5I", "5Q", "5X", "7I", "7Q", "7X", /* 20-29 */
    "6A", "6B", "6C", "6X", "6Z", "6S", "6L", "8L", "8Q", "8X", /* 30-39 */
    "2I", "2Q", "6I", "6Q", "3I", "3Q", "3X", "1I", "1Q", "5A", /* 40-49 */
    "5B", "5C", "9A", "9B", "9C", "9X", "", "", "", ""          /* 50-59 */
};


unsigned char obsfreqs[] = {
    /* 1:L1/E1, 2:L2/B1, 3:L5/E5a/L3, 4:L6/LEX/B3, 5:E5b/B2, 6:E5(a+b), 7:S */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*  0- 9 */
    1, 1, 1, 1, 2, 2, 2, 2, 2, 2, /* 10-19 */
    2, 2, 2, 2, 3, 3, 3, 5, 5, 5, /* 20-29 */
    4, 4, 4, 4, 4, 4, 4, 6, 6, 6, /* 30-39 */
    2, 2, 4, 4, 3, 3, 3, 1, 1, 3, /* 40-49 */
    3, 3, 7, 7, 7, 7, 0, 0, 0, 0  /* 50-59 */
};


char codepris[7][MAXFREQ][16] = {
    /* code priority table */

    /* L1/E1      L2/B1        L5/E5a/L3 L6/LEX/B3 E5b/B2    E5(a+b)  S */
    {"CPYWMNSL", "PYWCMNDSLX", "IQX", "", "", "", ""}, /* GPS */
    {"PC", "PC", "IQX", "", "", "", ""},               /* GLO */
    {"CABXZ", "", "IQX", "ABCXZ", "IQX", "IQX", ""},   /* GAL */
    {"CSLXZ", "SLX", "IQX", "SLX", "", "", ""},        /* QZS */
    {"C", "", "IQX", "", "", "", ""},                  /* SBS */
    {"IQX", "IQX", "IQX", "IQX", "IQX", "", ""},       /* BDS */
    {"", "", "ABCX", "", "", "", "ABCX"}               /* IRN */
};


fatalfunc_t *fatalfunc = nullptr; /* fatal callback function */

/* crc tables generated by util/gencrc ---------------------------------------*/
const uint16_t TBL_CR_C16[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};


const unsigned int TBL_CR_C24_Q[] = {
    0x000000, 0x864CFB, 0x8AD50D, 0x0C99F6, 0x93E6E1, 0x15AA1A, 0x1933EC, 0x9F7F17,
    0xA18139, 0x27CDC2, 0x2B5434, 0xAD18CF, 0x3267D8, 0xB42B23, 0xB8B2D5, 0x3EFE2E,
    0xC54E89, 0x430272, 0x4F9B84, 0xC9D77F, 0x56A868, 0xD0E493, 0xDC7D65, 0x5A319E,
    0x64CFB0, 0xE2834B, 0xEE1ABD, 0x685646, 0xF72951, 0x7165AA, 0x7DFC5C, 0xFBB0A7,
    0x0CD1E9, 0x8A9D12, 0x8604E4, 0x00481F, 0x9F3708, 0x197BF3, 0x15E205, 0x93AEFE,
    0xAD50D0, 0x2B1C2B, 0x2785DD, 0xA1C926, 0x3EB631, 0xB8FACA, 0xB4633C, 0x322FC7,
    0xC99F60, 0x4FD39B, 0x434A6D, 0xC50696, 0x5A7981, 0xDC357A, 0xD0AC8C, 0x56E077,
    0x681E59, 0xEE52A2, 0xE2CB54, 0x6487AF, 0xFBF8B8, 0x7DB443, 0x712DB5, 0xF7614E,
    0x19A3D2, 0x9FEF29, 0x9376DF, 0x153A24, 0x8A4533, 0x0C09C8, 0x00903E, 0x86DCC5,
    0xB822EB, 0x3E6E10, 0x32F7E6, 0xB4BB1D, 0x2BC40A, 0xAD88F1, 0xA11107, 0x275DFC,
    0xDCED5B, 0x5AA1A0, 0x563856, 0xD074AD, 0x4F0BBA, 0xC94741, 0xC5DEB7, 0x43924C,
    0x7D6C62, 0xFB2099, 0xF7B96F, 0x71F594, 0xEE8A83, 0x68C678, 0x645F8E, 0xE21375,
    0x15723B, 0x933EC0, 0x9FA736, 0x19EBCD, 0x8694DA, 0x00D821, 0x0C41D7, 0x8A0D2C,
    0xB4F302, 0x32BFF9, 0x3E260F, 0xB86AF4, 0x2715E3, 0xA15918, 0xADC0EE, 0x2B8C15,
    0xD03CB2, 0x567049, 0x5AE9BF, 0xDCA544, 0x43DA53, 0xC596A8, 0xC90F5E, 0x4F43A5,
    0x71BD8B, 0xF7F170, 0xFB6886, 0x7D247D, 0xE25B6A, 0x641791, 0x688E67, 0xEEC29C,
    0x3347A4, 0xB50B5F, 0xB992A9, 0x3FDE52, 0xA0A145, 0x26EDBE, 0x2A7448, 0xAC38B3,
    0x92C69D, 0x148A66, 0x181390, 0x9E5F6B, 0x01207C, 0x876C87, 0x8BF571, 0x0DB98A,
    0xF6092D, 0x7045D6, 0x7CDC20, 0xFA90DB, 0x65EFCC, 0xE3A337, 0xEF3AC1, 0x69763A,
    0x578814, 0xD1C4EF, 0xDD5D19, 0x5B11E2, 0xC46EF5, 0x42220E, 0x4EBBF8, 0xC8F703,
    0x3F964D, 0xB9DAB6, 0xB54340, 0x330FBB, 0xAC70AC, 0x2A3C57, 0x26A5A1, 0xA0E95A,
    0x9E1774, 0x185B8F, 0x14C279, 0x928E82, 0x0DF195, 0x8BBD6E, 0x872498, 0x016863,
    0xFAD8C4, 0x7C943F, 0x700DC9, 0xF64132, 0x693E25, 0xEF72DE, 0xE3EB28, 0x65A7D3,
    0x5B59FD, 0xDD1506, 0xD18CF0, 0x57C00B, 0xC8BF1C, 0x4EF3E7, 0x426A11, 0xC426EA,
    0x2AE476, 0xACA88D, 0xA0317B, 0x267D80, 0xB90297, 0x3F4E6C, 0x33D79A, 0xB59B61,
    0x8B654F, 0x0D29B4, 0x01B042, 0x87FCB9, 0x1883AE, 0x9ECF55, 0x9256A3, 0x141A58,
    0xEFAAFF, 0x69E604, 0x657FF2, 0xE33309, 0x7C4C1E, 0xFA00E5, 0xF69913, 0x70D5E8,
    0x4E2BC6, 0xC8673D, 0xC4FECB, 0x42B230, 0xDDCD27, 0x5B81DC, 0x57182A, 0xD154D1,
    0x26359F, 0xA07964, 0xACE092, 0x2AAC69, 0xB5D37E, 0x339F85, 0x3F0673, 0xB94A88,
    0x87B4A6, 0x01F85D, 0x0D61AB, 0x8B2D50, 0x145247, 0x921EBC, 0x9E874A, 0x18CBB1,
    0xE37B16, 0x6537ED, 0x69AE1B, 0xEFE2E0, 0x709DF7, 0xF6D10C, 0xFA48FA, 0x7C0401,
    0x42FA2F, 0xC4B6D4, 0xC82F22, 0x4E63D9, 0xD11CCE, 0x575035, 0x5BC9C3, 0xDD8538};


extern "C" {
void dgemm_(char *, char *, int *, int *, int *, double *, double *, int *, double *, int *, double *, double *, int *);
extern void dgetrf_(int *, int *, double *, int *, int *, int *);
extern void dgetri_(int *, double *, int *, int *, double *, int *, int *);
extern void dgetrs_(char *, int *, int *, double *, int *, int *, double *, int *, int *);
}


/* function prototypes -------------------------------------------------------*/


#ifdef IERS_MODEL
extern int gmf_(double *mjd, double *lat, double *lon, double *hgt, double *zd,
    double *gmfh, double *gmfw);
#endif


/* fatal error ---------------------------------------------------------------*/
void fatalerr(const char *format, ...)
{
    char msg[1024];
    va_list ap;
    va_start(ap, format);
    vsprintf(msg, format, ap);
    va_end(ap);
    fprintf(stderr, "%s", msg);
    exit(-9);
}


/* satellite system+prn/slot number to satellite number ------------------------
 * convert satellite system+prn/slot number to satellite number
 * args   : int    sys       I   satellite system (SYS_GPS,SYS_GLO,...)
 *          int    prn       I   satellite prn/slot number
 * return : satellite number (0:error)
 *-----------------------------------------------------------------------------*/
int satno(int sys, int prn)
{
    if (prn <= 0)
        {
            return 0;
        }
    switch (sys)
        {
        case SYS_GPS:
            if (prn < MINPRNGPS || MAXPRNGPS < prn)
                {
                    return 0;
                }
            return prn - MINPRNGPS + 1;
        case SYS_GLO:
            if (prn < MINPRNGLO || MAXPRNGLO < prn)
                {
                    return 0;
                }
            return NSATGPS + prn - MINPRNGLO + 1;
        case SYS_GAL:
            if (prn < MINPRNGAL || MAXPRNGAL < prn)
                {
                    return 0;
                }
            return NSATGPS + NSATGLO + prn - MINPRNGAL + 1;
        case SYS_QZS:
            if (prn < MINPRNQZS || MAXPRNQZS < prn)
                {
                    return 0;
                }
            return NSATGPS + NSATGLO + NSATGAL + prn - MINPRNQZS + 1;
        case SYS_BDS:
            if (prn < MINPRNBDS || MAXPRNBDS < prn)
                {
                    return 0;
                }
            return NSATGPS + NSATGLO + NSATGAL + NSATQZS + prn - MINPRNBDS + 1;
        case SYS_IRN:
            if (prn < MINPRNIRN || MAXPRNIRN < prn)
                {
                    return 0;
                }
            return NSATGPS + NSATGLO + NSATGAL + NSATQZS + NSATBDS + prn - MINPRNIRN + 1;
        case SYS_LEO:
            if (prn < MINPRNLEO || MAXPRNLEO < prn)
                {
                    return 0;
                }
            return NSATGPS + NSATGLO + NSATGAL + NSATQZS + NSATBDS + NSATIRN +
                   prn - MINPRNLEO + 1;
        case SYS_SBS:
            if (prn < MINPRNSBS || MAXPRNSBS < prn)
                {
                    return 0;
                }
            return NSATGPS + NSATGLO + NSATGAL + NSATQZS + NSATBDS + NSATIRN + NSATLEO +
                   prn - MINPRNSBS + 1;
        }
    return 0;
}


/* satellite number to satellite system ----------------------------------------
 * convert satellite number to satellite system
 * args   : int    sat       I   satellite number (1-MAXSAT)
 *          int    *prn      IO  satellite prn/slot number (NULL: no output)
 * return : satellite system (SYS_GPS,SYS_GLO,...)
 *-----------------------------------------------------------------------------*/
int satsys(int sat, int *prn)
{
    int sys = SYS_NONE;
    if (sat <= 0 || MAXSAT < sat)
        {
            sat = 0;
        }
    else if (sat <= NSATGPS)
        {
            sys = SYS_GPS;
            sat += MINPRNGPS - 1;
        }
    else if ((sat -= NSATGPS) <= NSATGLO)
        {
            sys = SYS_GLO;
            sat += MINPRNGLO - 1;
        }
    else if ((sat -= NSATGLO) <= NSATGAL)
        {
            sys = SYS_GAL;
            sat += MINPRNGAL - 1;
        }
    else if ((sat -= NSATGAL) <= NSATQZS)
        {
            sys = SYS_QZS;
            sat += MINPRNQZS - 1;
        }
    else if ((sat -= NSATQZS) <= NSATBDS)
        {
            sys = SYS_BDS;
            sat += MINPRNBDS - 1;
        }
    else if ((sat -= NSATBDS) <= NSATIRN)
        {
            sys = SYS_IRN;
            sat += MINPRNIRN - 1;
        }
    else if ((sat -= NSATIRN) <= NSATLEO)
        {
            sys = SYS_LEO;
            sat += MINPRNLEO - 1;
        }
    else if ((sat -= NSATLEO) <= NSATSBS)
        {
            sys = SYS_SBS;
            sat += MINPRNSBS - 1;
        }
    else
        {
            sat = 0;
        }
    if (prn)
        {
            *prn = sat;
        }
    return sys;
}


/* satellite id to satellite number --------------------------------------------
 * convert satellite id to satellite number
 * args   : char   *id       I   satellite id (nn,Gnn,Rnn,Enn,Jnn,Cnn,Inn or Snn)
 * return : satellite number (0: error)
 * notes  : 120-142 and 193-199 are also recognized as sbas and qzss
 *-----------------------------------------------------------------------------*/
int satid2no(const char *id)
{
    int sys, prn;
    char code;

    if (sscanf(id, "%d", &prn) == 1)
        {
            if (MINPRNGPS <= prn && prn <= MAXPRNGPS)
                {
                    sys = SYS_GPS;
                }
            else if (MINPRNSBS <= prn && prn <= MAXPRNSBS)
                {
                    sys = SYS_SBS;
                }
            else if (MINPRNQZS <= prn && prn <= MAXPRNQZS)
                {
                    sys = SYS_QZS;
                }
            else
                {
                    return 0;
                }
            return satno(sys, prn);
        }
    if (sscanf(id, "%c%d", &code, &prn) < 2)
        {
            return 0;
        }

    switch (code)
        {
        case 'G':
            sys = SYS_GPS;
            prn += MINPRNGPS - 1;
            break;
        case 'R':
            sys = SYS_GLO;
            prn += MINPRNGLO - 1;
            break;
        case 'E':
            sys = SYS_GAL;
            prn += MINPRNGAL - 1;
            break;
        case 'J':
            sys = SYS_QZS;
            prn += MINPRNQZS - 1;
            break;
        case 'C':
            sys = SYS_BDS;
            prn += MINPRNBDS - 1;
            break;
        case 'I':
            sys = SYS_IRN;
            prn += MINPRNIRN - 1;
            break;
        case 'L':
            sys = SYS_LEO;
            prn += MINPRNLEO - 1;
            break;
        case 'S':
            sys = SYS_SBS;
            prn += 100;
            break;
        default:
            return 0;
        }
    return satno(sys, prn);
}


/* satellite number to satellite id --------------------------------------------
 * convert satellite number to satellite id
 * args   : int    sat       I   satellite number
 *          char   *id       O   satellite id (Gnn,Rnn,Enn,Jnn,Cnn,Inn or nnn)
 * return : none
 *-----------------------------------------------------------------------------*/
void satno2id(int sat, char *id)
{
    int prn;
    switch (satsys(sat, &prn))
        {
        case SYS_GPS:
            sprintf(id, "G%02d", prn - MINPRNGPS + 1);
            return;
        case SYS_GLO:
            sprintf(id, "R%02d", prn - MINPRNGLO + 1);
            return;
        case SYS_GAL:
            sprintf(id, "E%02d", prn - MINPRNGAL + 1);
            return;
        case SYS_QZS:
            sprintf(id, "J%02d", prn - MINPRNQZS + 1);
            return;
        case SYS_BDS:
            sprintf(id, "C%02d", prn - MINPRNBDS + 1);
            return;
        case SYS_IRN:
            sprintf(id, "I%02d", prn - MINPRNIRN + 1);
            return;
        case SYS_LEO:
            sprintf(id, "L%02d", prn - MINPRNLEO + 1);
            return;
        case SYS_SBS:
            sprintf(id, "%03d", prn);
            return;
        }
    strcpy(id, "");
}


/* test excluded satellite -----------------------------------------------------
 * test excluded satellite
 * args   : int    sat       I   satellite number
 *          int    svh       I   sv health flag
 *          prcopt_t *opt    I   processing options (NULL: not used)
 * return : status (1:excluded,0:not excluded)
 *-----------------------------------------------------------------------------*/
int satexclude(int sat, int svh, const prcopt_t *opt)
{
    int sys = satsys(sat, nullptr);

    if (svh < 0)
        {
            trace(3, "ephemeris unavailable: sat=%3d svh=%02X\n", sat, svh);
            return 1; /* ephemeris unavailable */
        }

    if (opt)
        {
            if (opt->exsats[sat - 1] == 1)
                {
                    trace(3, "excluded satellite: sat=%3d svh=%02X\n", sat, svh);
                    return 1; /* excluded satellite */
                }
            if (opt->exsats[sat - 1] == 2)
                {
                    return 0; /* included satellite */
                }
            if (!(sys & opt->navsys))
                {
                    trace(3, "unselected sat sys: sat=%3d svh=%02X\n", sat, svh);
                    return 1; /* unselected sat sys */
                }
        }
    if (sys == SYS_QZS)
        {
            svh &= 0xFE; /* mask QZSS LEX health */
        }
    if (svh)
        {
            trace(3, "unhealthy satellite: sat=%3d svh=%02X\n", sat, svh);
            return 1;
        }
    return 0;
}


/* test SNR mask ---------------------------------------------------------------
 * test SNR mask
 * args   : int    base      I   rover or base-station (0:rover,1:base station)
 *          int    freq      I   frequency (0:L1,1:L2,2:L3,...)
 *          double el        I   elevation angle (rad)
 *          double snr       I   C/N0 (dBHz)
 *          snrmask_t *mask  I   SNR mask
 * return : status (1:masked,0:unmasked)
 *-----------------------------------------------------------------------------*/
int testsnr(int base, int freq, double el, double snr,
    const snrmask_t *mask)
{
    double minsnr, a;
    int i;

    if (!mask->ena[base] || freq < 0 || freq >= NFREQ)
        {
            return 0;
        }

    a = (el * R2D + 5.0) / 10.0;
    i = static_cast<int>(floor(a));
    a -= i;
    if (i < 1)
        {
            minsnr = mask->mask[freq][0];
        }
    else if (i > 8)
        {
            minsnr = mask->mask[freq][8];
        }
    else
        {
            minsnr = (1.0 - a) * mask->mask[freq][i - 1] + a * mask->mask[freq][i];
        }

    return snr < minsnr;
}


/* obs type string to obs code -------------------------------------------------
 * convert obs code type string to obs code
 * args   : char   *str   I      obs code string ("1C","1P","1Y",...)
 *          int    *freq  IO     frequency (1:L1,2:L2,3:L5,4:L6,5:L7,6:L8,0:err)
 *                               (NULL: no output)
 * return : obs code (CODE_???)
 * notes  : obs codes are based on reference [6] and qzss extension
 *-----------------------------------------------------------------------------*/
unsigned char obs2code(const char *obs, int *freq)
{
    int i;
    if (freq)
        {
            *freq = 0;
        }
    for (i = 1; *obscodes[i]; i++)
        {
            if (strcmp(obscodes[i], obs) != 0)
                {
                    continue;
                }
            if (freq)
                {
                    *freq = obsfreqs[i];
                }
            return static_cast<unsigned char>(i);
        }
    return CODE_NONE;
}


/* obs code to obs code string -------------------------------------------------
 * convert obs code to obs code string
 * args   : unsigned char code I obs code (CODE_???)
 *          int    *freq  IO     frequency (NULL: no output)
 *                               (1:L1/E1, 2:L2/B1, 3:L5/E5a/L3, 4:L6/LEX/B3,
                                 5:E5b/B2, 6:E5(a+b), 7:S)
 * return : obs code string ("1C","1P","1P",...)
 * notes  : obs codes are based on reference [6] and qzss extension
 *-----------------------------------------------------------------------------*/
char *code2obs(unsigned char code, int *freq)
{
    if (freq)
        {
            *freq = 0;
        }
    if (code <= CODE_NONE || MAXCODE < code)
        {
            return (char *)"";
        }
    if (freq)
        {
            *freq = obsfreqs[code];
        }
    return obscodes[code];
}


/* set code priority -----------------------------------------------------------
 * set code priority for multiple codes in a frequency
 * args   : int    sys     I     system (or of SYS_???)
 *          int    freq    I     frequency (1:L1,2:L2,3:L5,4:L6,5:L7,6:L8,7:L9)
 *          char   *pri    I     priority of codes (series of code characters)
 *                               (higher priority precedes lower)
 * return : none
 *-----------------------------------------------------------------------------*/
void setcodepri(int sys, int freq, const char *pri)
{
    trace(3, "setcodepri : sys=%d freq=%d pri=%s\n", sys, freq, pri);

    if (freq <= 0 || MAXFREQ < freq)
        {
            return;
        }
    if (strlen(pri) < 17)
        {
            if (sys & SYS_GPS)
                {
                    strcpy(codepris[0][freq - 1], pri);
                }
            if (sys & SYS_GLO)
                {
                    strcpy(codepris[1][freq - 1], pri);
                }
            if (sys & SYS_GAL)
                {
                    strcpy(codepris[2][freq - 1], pri);
                }
            if (sys & SYS_QZS)
                {
                    strcpy(codepris[3][freq - 1], pri);
                }
            if (sys & SYS_SBS)
                {
                    strcpy(codepris[4][freq - 1], pri);
                }
            if (sys & SYS_BDS)
                {
                    strcpy(codepris[5][freq - 1], pri);
                }
            if (sys & SYS_IRN)
                {
                    strcpy(codepris[6][freq - 1], pri);
                }
        }
    else
        {
            trace(1, "pri array is too long");
        }
}


/* get code priority -----------------------------------------------------------
 * get code priority for multiple codes in a frequency
 * args   : int    sys     I     system (SYS_???)
 *          unsigned char code I obs code (CODE_???)
 *          char   *opt    I     code options (NULL:no option)
 * return : priority (15:highest-1:lowest,0:error)
 *-----------------------------------------------------------------------------*/
int getcodepri(int sys, unsigned char code, const char *opt)
{
    const char *p, *optstr;
    char *obs, str[8] = "";
    int i, j;

    switch (sys)
        {
        case SYS_GPS:
            i = 0;
            optstr = "-GL%2s";
            break;
        case SYS_GLO:
            i = 1;
            optstr = "-RL%2s";
            break;
        case SYS_GAL:
            i = 2;
            optstr = "-EL%2s";
            break;
        case SYS_QZS:
            i = 3;
            optstr = "-JL%2s";
            break;
        case SYS_SBS:
            i = 4;
            optstr = "-SL%2s";
            break;
        case SYS_BDS:
            i = 5;
            optstr = "-CL%2s";
            break;
        case SYS_IRN:
            i = 6;
            optstr = "-IL%2s";
            break;
        default:
            return 0;
        }
    obs = code2obs(code, &j);

    /* parse code options */
    for (p = opt; p && (p = strchr(p, '-')); p++)
        {
            if (sscanf(p, optstr, str) < 1 || str[0] != obs[0])
                {
                    continue;
                }
            return str[1] == obs[1] ? 15 : 0;
        }
    /* search code priority */
    return (p = strchr(codepris[i][j - 1], obs[1])) ? 14 - static_cast<int>(p - codepris[i][j - 1]) : 0;
}


/* extract unsigned/signed bits ------------------------------------------------
 * extract unsigned/signed bits from byte data
 * args   : unsigned char *buff I byte data
 *          int    pos    I      bit position from start of data (bits)
 *          int    len    I      bit length (bits) (len <= 32)
 * return : extracted unsigned/signed bits
 *-----------------------------------------------------------------------------*/
unsigned int getbitu(const unsigned char *buff, int pos, int len)
{
    unsigned int bits = 0;
    int i;
    for (i = pos; i < pos + len; i++)
        {
            bits = (bits << 1) + ((buff[i / 8] >> (7 - i % 8)) & 1u);
        }
    return bits;
}


int getbits(const unsigned char *buff, int pos, int len)
{
    unsigned int bits = getbitu(buff, pos, len);
    if (len <= 0 || 32 <= len || !(bits & (1u << (len - 1))))
        {
            return static_cast<int>(bits);
        }
    return static_cast<int>(bits | (~0u << len)); /* extend sign */
}


/* set unsigned/signed bits ----------------------------------------------------
 * set unsigned/signed bits to byte data
 * args   : unsigned char *buff IO byte data
 *          int    pos    I      bit position from start of data (bits)
 *          int    len    I      bit length (bits) (len <= 32)
 *         (unsigned) int I      unsigned/signed data
 * return : none
 *-----------------------------------------------------------------------------*/
void setbitu(unsigned char *buff, int pos, int len, unsigned int data)
{
    unsigned int mask = 1u << (len - 1);
    int i;
    if (len <= 0 || 32 < len)
        {
            return;
        }
    for (i = pos; i < pos + len; i++, mask >>= 1)
        {
            if (data & mask)
                {
                    buff[i / 8] |= 1u << (7 - i % 8);
                }
            else
                {
                    buff[i / 8] &= ~(1u << (7 - i % 8));
                }
        }
}


void setbits(unsigned char *buff, int pos, int len, int data)
{
    if (data < 0)
        {
            data |= 1 << (len - 1);
        }
    else
        {
            data &= ~(1 << (len - 1)); /* set sign bit */
        }
    setbitu(buff, pos, len, static_cast<unsigned int>(data));
}


/* crc-32 parity ---------------------------------------------------------------
 * compute crc-32 parity for novatel raw
 * args   : unsigned char *buff I data
 *          int    len    I      data length (bytes)
 * return : crc-32 parity
 * notes  : see NovAtel OEMV firmware manual 1.7 32-bit CRC
 *-----------------------------------------------------------------------------*/
unsigned int rtk_crc32(const unsigned char *buff, int len)
{
    unsigned int crc = 0;
    int i, j;

    trace(4, "rtk_crc32: len=%d\n", len);

    for (i = 0; i < len; i++)
        {
            crc ^= buff[i];
            for (j = 0; j < 8; j++)
                {
                    if (crc & 1)
                        {
                            crc = (crc >> 1) ^ POLYCRC32;
                        }
                    else
                        {
                            crc >>= 1;
                        }
                }
        }
    return crc;
}


/* crc-24q parity --------------------------------------------------------------
 * compute crc-24q parity for sbas, rtcm3
 * args   : unsigned char *buff I data
 *          int    len    I      data length (bytes)
 * return : crc-24Q parity
 * notes  : see reference [2] A.4.3.3 Parity
 *-----------------------------------------------------------------------------*/
unsigned int rtk_crc24q(const unsigned char *buff, int len)
{
    unsigned int crc = 0;
    int i;

    trace(4, "rtk_crc24q: len=%d\n", len);

    for (i = 0; i < len; i++)
        {
            crc = ((crc << 8) & 0xFFFFFF) ^ TBL_CR_C24_Q[(crc >> 16) ^ buff[i]];
        }
    return crc;
}


/* crc-16 parity ---------------------------------------------------------------
 * compute crc-16 parity for binex, nvs
 * args   : unsigned char *buff I data
 *          int    len    I      data length (bytes)
 * return : crc-16 parity
 * notes  : see reference [10] A.3.
 *-----------------------------------------------------------------------------*/
uint16_t rtk_crc16(const unsigned char *buff, int len)
{
    uint16_t crc = 0;
    int i;

    trace(4, "rtk_crc16: len=%d\n", len);

    for (i = 0; i < len; i++)
        {
            crc = (crc << 8) ^ TBL_CR_C16[((crc >> 8) ^ buff[i]) & 0xFF];
        }
    return crc;
}


/* decode navigation data word -------------------------------------------------
 * check party and decode navigation data word
 * args   : unsigned int word I navigation data word (2+30bit)
 *                              (previous word D29*-30* + current word D1-30)
 *          unsigned char *data O decoded navigation data without parity
 *                              (8bitx3)
 * return : status (1:ok,0:parity error)
 * notes  : see reference [1] 20.3.5.2 user parity algorithm
 *-----------------------------------------------------------------------------*/
int decode_word(unsigned int word, unsigned char *data)
{
    const unsigned int hamming[] = {
        0xBB1F3480, 0x5D8F9A40, 0xAEC7CD00, 0x5763E680, 0x6BB1F340, 0x8B7A89C0};
    unsigned int parity = 0, w;
    int i;

    trace(5, "decodeword: word=%08x\n", word);

    if (word & 0x40000000)
        {
            word ^= 0x3FFFFFC0;
        }

    for (i = 0; i < 6; i++)
        {
            parity <<= 1;
            for (w = (word & hamming[i]) >> 6; w; w >>= 1)
                {
                    parity ^= w & 1;
                }
        }
    if (parity != (word & 0x3F))
        {
            return 0;
        }

    for (i = 0; i < 3; i++)
        {
            data[i] = static_cast<unsigned char>(word >> (22 - i * 8));
        }
    return 1;
}


/* new matrix ------------------------------------------------------------------
 * allocate memory of matrix
 * args   : int    n,m       I   number of rows and columns of matrix
 * return : matrix pointer (if n<=0 or m<=0, return NULL)
 *-----------------------------------------------------------------------------*/
double *mat(int n, int m)
{
    double *p;

    if (n <= 0 || m <= 0)
        {
            return nullptr;
        }
    if (!(p = static_cast<double *>(malloc(sizeof(double) * n * m))))
        {
            fatalerr("matrix memory allocation error: n=%d,m=%d\n", n, m);
        }
    return p;
}


/* new integer matrix ----------------------------------------------------------
 * allocate memory of integer matrix
 * args   : int    n,m       I   number of rows and columns of matrix
 * return : matrix pointer (if n <= 0 or m <= 0, return NULL)
 *-----------------------------------------------------------------------------*/
int *imat(int n, int m)
{
    int *p;

    if (n <= 0 || m <= 0)
        {
            return nullptr;
        }
    if (!(p = static_cast<int *>(malloc(sizeof(int) * n * m))))
        {
            fatalerr("integer matrix memory allocation error: n=%d,m=%d\n", n, m);
        }
    return p;
}


/* zero matrix -----------------------------------------------------------------
 * generate new zero matrix
 * args   : int    n,m       I   number of rows and columns of matrix
 * return : matrix pointer (if n <= 0 or m <= 0, return NULL)
 *-----------------------------------------------------------------------------*/
double *zeros(int n, int m)
{
    double *p;

#if NOCALLOC
    if ((p = mat(n, m)))
        for (n = n * m - 1; n >= 0; n--) p[n] = 0.0;
#else
    if (n <= 0 || m <= 0)
        {
            return nullptr;
        }
    if (!(p = static_cast<double *>(calloc(sizeof(double), n * m))))
        {
            fatalerr("matrix memory allocation error: n=%d,m=%d\n", n, m);
        }
#endif
    return p;
}


/* identity matrix -------------------------------------------------------------
 * generate new identity matrix
 * args   : int    n         I   number of rows and columns of matrix
 * return : matrix pointer (if n <= 0, return NULL)
 *-----------------------------------------------------------------------------*/
double *eye(int n)
{
    double *p;
    int i;

    if ((p = zeros(n, n)))
        {
            for (i = 0; i < n; i++)
                {
                    p[i + i * n] = 1.0;
                }
        }
    return p;
}


/* inner product ---------------------------------------------------------------
 * inner product of vectors
 * args   : double *a,*b     I   vector a,b (n x 1)
 *          int    n         I   size of vector a,b
 * return : a'*b
 *-----------------------------------------------------------------------------*/
double dot(const double *a, const double *b, int n)
{
    double c = 0.0;

    while (--n >= 0)
        {
            c += a[n] * b[n];
        }
    return c;
}


/* euclid norm -----------------------------------------------------------------
 * euclid norm of vector
 * args   : double *a        I   vector a (n x 1)
 *          int    n         I   size of vector a
 * return : || a ||
 *-----------------------------------------------------------------------------*/
double norm_rtk(const double *a, int n)
{
    return std::sqrt(dot(a, a, n));
}


/* outer product of 3d vectors -------------------------------------------------
 * outer product of 3d vectors
 * args   : double *a,*b     I   vector a,b (3 x 1)
 *          double *c        O   outer product (a x b) (3 x 1)
 * return : none
 *-----------------------------------------------------------------------------*/
void cross3(const double *a, const double *b, double *c)
{
    c[0] = a[1] * b[2] - a[2] * b[1];
    c[1] = a[2] * b[0] - a[0] * b[2];
    c[2] = a[0] * b[1] - a[1] * b[0];
}


/* normalize 3d vector ---------------------------------------------------------
 * normalize 3d vector
 * args   : double *a        I   vector a (3 x 1)
 *          double *b        O   normlized vector (3 x 1) || b || = 1
 * return : status (1:ok,0:error)
 *-----------------------------------------------------------------------------*/
int normv3(const double *a, double *b)
{
    double r;
    if ((r = norm_rtk(a, 3)) <= 0.0)
        {
            return 0;
        }
    b[0] = a[0] / r;
    b[1] = a[1] / r;
    b[2] = a[2] / r;
    return 1;
}


/* copy matrix -----------------------------------------------------------------
 * copy matrix
 * args   : double *A        O   destination matrix A (n x m)
 *          double *B        I   source matrix B (n x m)
 *          int    n,m       I   number of rows and columns of matrix
 * return : none
 *-----------------------------------------------------------------------------*/
void matcpy(double *A, const double *B, int n, int m)
{
    memcpy(A, B, sizeof(double) * n * m);
}

/* matrix routines -----------------------------------------------------------*/


/* multiply matrix (wrapper of blas dgemm) -------------------------------------
 * multiply matrix by matrix (C=alpha*A*B+beta*C)
 * args   : char   *tr       I  transpose flags ("N":normal,"T":transpose)
 *          int    n,k,m     I  size of (transposed) matrix A,B
 *          double alpha     I  alpha
 *          double *A,*B     I  (transposed) matrix A (n x m), B (m x k)
 *          double beta      I  beta
 *          double *C        IO matrix C (n x k)
 * return : none
 *-----------------------------------------------------------------------------*/
void matmul(const char *tr, int n, int k, int m, double alpha,
    const double *A, const double *B, double beta, double *C)
{
    int lda = tr[0] == 'T' ? m : n, ldb = tr[1] == 'T' ? k : m;

    dgemm_(const_cast<char *>(tr), const_cast<char *>(tr) + 1, &n, &k, &m, &alpha, const_cast<double *>(A), &lda, const_cast<double *>(B),
        &ldb, &beta, C, &n);
}


/* inverse of matrix -----------------------------------------------------------
 * inverse of matrix (A=A^-1)
 * args   : double *A        IO  matrix (n x n)
 *          int    n         I   size of matrix A
 * return : status (0:ok,0>:error)
 *-----------------------------------------------------------------------------*/
int matinv(double *A, int n)
{
    double *work;
    int info, lwork = n * 16, *ipiv = imat(n, 1);

    work = mat(lwork, 1);
    dgetrf_(&n, &n, A, &n, ipiv, &info);
    if (!info)
        {
            dgetri_(&n, A, &n, ipiv, work, &lwork, &info);
        }
    free(ipiv);
    free(work);
    return info;
}


/* solve linear equation -------------------------------------------------------
 * solve linear equation (X=A\Y or X=A'\Y)
 * args   : char   *tr       I   transpose flag ("N":normal,"T":transpose)
 *          double *A        I   input matrix A (n x n)
 *          double *Y        I   input matrix Y (n x m)
 *          int    n,m       I   size of matrix A,Y
 *          double *X        O   X=A\Y or X=A'\Y (n x m)
 * return : status (0:ok,0>:error)
 * notes  : matirix stored by column-major order (fortran convention)
 *          X can be same as Y
 *-----------------------------------------------------------------------------*/
int solve(const char *tr, const double *A, const double *Y, int n,
    int m, double *X)
{
    double *B = mat(n, n);
    int info, *ipiv = imat(n, 1);

    matcpy(B, A, n, n);
    matcpy(X, Y, n, m);
    dgetrf_(&n, &n, B, &n, ipiv, &info);
    if (!info)
        {
            dgetrs_(const_cast<char *>(tr), &n, &m, B, &n, ipiv, X, &n, &info);
        }
    free(ipiv);
    free(B);
    return info;
}


/* end of matrix routines ----------------------------------------------------*/

/* least square estimation -----------------------------------------------------
 * least square estimation by solving normal equation (x=(A*A')^-1*A*y)
 * args   : double *A        I   transpose of (weighted) design matrix (n x m)
 *          double *y        I   (weighted) measurements (m x 1)
 *          int    n,m       I   number of parameters and measurements (n <= m)
 *          double *x        O   estmated parameters (n x 1)
 *          double *Q        O   esimated parameters covariance matrix (n x n)
 * return : status (0:ok,0>:error)
 * notes  : for weighted least square, replace A and y by A*w and w*y (w=W^(1/2))
 *          matirix stored by column-major order (fortran convention)
 *-----------------------------------------------------------------------------*/
int lsq(const double *A, const double *y, int n, int m, double *x,
    double *Q)
{
    double *Ay;
    int info;

    if (m < n)
        {
            return -1;
        }
    Ay = mat(n, 1);
    matmul("NN", n, 1, m, 1.0, A, y, 0.0, Ay); /* Ay=A*y */
    matmul("NT", n, n, m, 1.0, A, A, 0.0, Q);  /* Q=A*A' */
    if (!(info = matinv(Q, n)))
        {
            matmul("NN", n, 1, n, 1.0, Q, Ay, 0.0, x); /* x=Q^-1*Ay */
        }
    free(Ay);
    return info;
}


/* kalman filter ---------------------------------------------------------------
 * kalman filter state update as follows:
 *
 *   K=P*H*(H'*P*H+R)^-1, xp=x+K*v, Pp=(I-K*H')*P
 *
 * args   : double *x        I   states vector (n x 1)
 *          double *P        I   covariance matrix of states (n x n)
 *          double *H        I   transpose of design matrix (n x m)
 *          double *v        I   innovation (measurement - model) (m x 1)
 *          double *R        I   covariance matrix of measurement error (m x m)
 *          int    n,m       I   number of states and measurements
 *          double *xp       O   states vector after update (n x 1)
 *          double *Pp       O   covariance matrix of states after update (n x n)
 * return : status (0:ok,<0:error)
 * notes  : matirix stored by column-major order (fortran convention)
 *          if state x[i]==0.0, not updates state x[i]/P[i+i*n]
 *-----------------------------------------------------------------------------*/
int filter_(const double *x, const double *P, const double *H,
    const double *v, const double *R, int n, int m,
    double *xp, double *Pp)
{
    double *F = mat(n, m), *Q = mat(m, m), *K = mat(n, m), *I = eye(n);
    int info;

    matcpy(Q, R, m, m);
    matcpy(xp, x, n, 1);
    matmul("NN", n, m, n, 1.0, P, H, 0.0, F); /* Q=H'*P*H+R */
    matmul("TN", m, m, n, 1.0, H, F, 1.0, Q);
    if (!(info = matinv(Q, m)))
        {
            matmul("NN", n, m, m, 1.0, F, Q, 0.0, K);  /* K=P*H*Q^-1 */
            matmul("NN", n, 1, m, 1.0, K, v, 1.0, xp); /* xp=x+K*v */
            matmul("NT", n, n, m, -1.0, K, H, 1.0, I); /* Pp=(I-K*H')*P */
            matmul("NN", n, n, n, 1.0, I, P, 0.0, Pp);
        }
    free(F);
    free(Q);
    free(K);
    free(I);
    return info;
}


int filter(double *x, double *P, const double *H, const double *v,
    const double *R, int n, int m)
{
    double *x_, *xp_, *P_, *Pp_, *H_;
    int i, j, k, info, *ix;

    ix = imat(n, 1);
    for (i = k = 0; i < n; i++)
        {
            if (x[i] != 0.0 && P[i + i * n] > 0.0)
                {
                    ix[k++] = i;
                }
        }
    x_ = mat(k, 1);
    xp_ = mat(k, 1);
    P_ = mat(k, k);
    Pp_ = mat(k, k);
    H_ = mat(k, m);
    for (i = 0; i < k; i++)
        {
            x_[i] = x[ix[i]];
            for (j = 0; j < k; j++)
                {
                    P_[i + j * k] = P[ix[i] + ix[j] * n];
                }
            for (j = 0; j < m; j++)
                {
                    H_[i + j * k] = H[ix[i] + j * n];
                }
        }
    info = filter_(x_, P_, H_, v, R, k, m, xp_, Pp_);
    for (i = 0; i < k; i++)
        {
            x[ix[i]] = xp_[i];
            for (j = 0; j < k; j++)
                {
                    P[ix[i] + ix[j] * n] = Pp_[i + j * k];
                }
        }
    free(ix);
    free(x_);
    free(xp_);
    free(P_);
    free(Pp_);
    free(H_);
    return info;
}


/* smoother --------------------------------------------------------------------
 * combine forward and backward filters by fixed-interval smoother as follows:
 *
 *   xs=Qs*(Qf^-1*xf+Qb^-1*xb), Qs=(Qf^-1+Qb^-1)^-1)
 *
 * args   : double *xf       I   forward solutions (n x 1)
 * args   : double *Qf       I   forward solutions covariance matrix (n x n)
 *          double *xb       I   backward solutions (n x 1)
 *          double *Qb       I   backward solutions covariance matrix (n x n)
 *          int    n         I   number of solutions
 *          double *xs       O   smoothed solutions (n x 1)
 *          double *Qs       O   smoothed solutions covariance matrix (n x n)
 * return : status (0:ok,0>:error)
 * notes  : see reference [4] 5.2
 *          matirix stored by column-major order (fortran convention)
 *-----------------------------------------------------------------------------*/
int smoother(const double *xf, const double *Qf, const double *xb,
    const double *Qb, int n, double *xs, double *Qs)
{
    double *invQf = mat(n, n), *invQb = mat(n, n), *xx = mat(n, 1);
    int i, info = -1;

    matcpy(invQf, Qf, n, n);
    matcpy(invQb, Qb, n, n);
    if (!matinv(invQf, n) && !matinv(invQb, n))
        {
            for (i = 0; i < n * n; i++)
                {
                    Qs[i] = invQf[i] + invQb[i];
                }
            if (!(info = matinv(Qs, n)))
                {
                    matmul("NN", n, 1, n, 1.0, invQf, xf, 0.0, xx);
                    matmul("NN", n, 1, n, 1.0, invQb, xb, 1.0, xx);
                    matmul("NN", n, 1, n, 1.0, Qs, xx, 0.0, xs);
                }
        }
    free(invQf);
    free(invQb);
    free(xx);
    return info;
}


/* print matrix ----------------------------------------------------------------
 * print matrix to stdout
 * args   : double *A        I   matrix A (n x m)
 *          int    n,m       I   number of rows and columns of A
 *          int    p,q       I   total columns, columns under decimal point
 *         (FILE  *fp        I   output file pointer)
 * return : none
 * notes  : matirix stored by column-major order (fortran convention)
 *-----------------------------------------------------------------------------*/
void matfprint(const double A[], int n, int m, int p, int q, FILE *fp)
{
    int i, j;

    for (i = 0; i < n; i++)
        {
            for (j = 0; j < m; j++)
                {
                    fprintf(fp, " %*.*f", p, q, A[i + j * n]);
                }
            fprintf(fp, "\n");
        }
}

void matsprint(const double A[], int n, int m, int p, int q, std::string &buffer)
{
    int i, j;
    buffer += '\n';
    for (i = 0; i < n; i++)
        {
            for (j = 0; j < m; j++)
                {
                    char buf_[256];
                    sprintf(buf_, " %*.*f", p, q, A[i + j * n]);
                    std::string s(buf_);
                    buffer = buffer + s;
                }
            buffer += '\n';
        }
}


void matprint(const double A[], int n, int m, int p, int q)
{
    matfprint(A, n, m, p, q, stdout);
}


/* string to number ------------------------------------------------------------
 * convert substring in string to number
 * args   : char   *s        I   string ("... nnn.nnn ...")
 *          int    i,n       I   substring position and width
 * return : converted number (0.0:error)
 *-----------------------------------------------------------------------------*/
double str2num(const char *s, int i, int n)
{
    double value;
    char str[256], *p = str;

    if (i < 0 || static_cast<int>(strlen(s)) < i || static_cast<int>(sizeof(str)) - 1 < n)
        {
            return 0.0;
        }
    for (s += i; *s && --n >= 0; s++)
        {
            *p++ = *s == 'd' || *s == 'D' ? 'E' : *s;
        }
    *p = '\0';
    return sscanf(str, "%lf", &value) == 1 ? value : 0.0;
}


/* string to time --------------------------------------------------------------
 * convert substring in string to gtime_t struct
 * args   : char   *s        I   string ("... yyyy mm dd hh mm ss ...")
 *          int    i,n       I   substring position and width
 *          gtime_t *t       O   gtime_t struct
 * return : status (0:ok,0>:error)
 *-----------------------------------------------------------------------------*/
int str2time(const char *s, int i, int n, gtime_t *t)
{
    double ep[6];
    char str[256], *p = str;

    if (i < 0 || static_cast<int>(strlen(s)) < i || static_cast<int>(sizeof(str)) - 1 < i)
        {
            return -1;
        }
    for (s += i; *s && --n >= 0;)
        {
            *p++ = *s++;
        }
    *p = '\0';
    if (sscanf(str, "%lf %lf %lf %lf %lf %lf", ep, ep + 1, ep + 2, ep + 3, ep + 4, ep + 5) < 6)
        {
            return -1;
        }
    if (ep[0] < 100.0)
        {
            ep[0] += ep[0] < 80.0 ? 2000.0 : 1900.0;
        }
    *t = epoch2time(ep);
    return 0;
}


/* convert calendar day/time to time -------------------------------------------
 * convert calendar day/time to gtime_t struct
 * args   : double *ep       I   day/time {year,month,day,hour,min,sec}
 * return : gtime_t struct
 * notes  : proper in 1970-2037 or 1970-2099 (64bit time_t)
 *-----------------------------------------------------------------------------*/
gtime_t epoch2time(const double *ep)
{
    const int doy[] = {1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
    gtime_t time = {0, 0};
    int days, sec, year = static_cast<int>(ep[0]), mon = static_cast<int>(ep[1]), day = static_cast<int>(ep[2]);

    if (year < 1970 || 2099 < year || mon < 1 || 12 < mon)
        {
            return time;
        }

    /* leap year if year%4==0 in 1901-2099 */
    days = (year - 1970) * 365 + (year - 1969) / 4 + doy[mon - 1] + day - 2 + (year % 4 == 0 && mon >= 3 ? 1 : 0);
    sec = static_cast<int>(floor(ep[5]));
    time.time = static_cast<time_t>(days) * 86400 + static_cast<int>(ep[3]) * 3600 + static_cast<int>(ep[4]) * 60 + sec;
    time.sec = ep[5] - sec;
    return time;
}


/* time to calendar day/time ---------------------------------------------------
 * convert gtime_t struct to calendar day/time
 * args   : gtime_t t        I   gtime_t struct
 *          double *ep       O   day/time {year,month,day,hour,min,sec}
 * return : none
 * notes  : proper in 1970-2037 or 1970-2099 (64bit time_t)
 *-----------------------------------------------------------------------------*/
void time2epoch(gtime_t t, double *ep)
{
    const int mday[] = {/* # of days in a month */
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
        31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int days, sec, mon, day;

    /* leap year if year%4==0 in 1901-2099 */
    days = static_cast<int>(t.time / 86400);
    sec = static_cast<int>(t.time - static_cast<time_t>(days) * 86400);
    for (day = days % 1461, mon = 0; mon < 48; mon++)
        {
            if (day >= mday[mon])
                {
                    day -= mday[mon];
                }
            else
                {
                    break;
                }
        }
    ep[0] = 1970 + days / 1461 * 4 + mon / 12;
    ep[1] = mon % 12 + 1;
    ep[2] = day + 1;
    ep[3] = sec / 3600;
    ep[4] = sec % 3600 / 60;
    ep[5] = sec % 60 + t.sec;
}


/* gps time to time ------------------------------------------------------------
 * convert week and tow in gps time to gtime_t struct
 * args   : int    week      I   week number in gps time
 *          double sec       I   time of week in gps time (s)
 * return : gtime_t struct
 *-----------------------------------------------------------------------------*/
gtime_t gpst2time(int week, double sec)
{
    gtime_t t = epoch2time(GPST0);

    if (sec < -1e9 || 1e9 < sec)
        {
            sec = 0.0;
        }
    t.time += static_cast<time_t>(86400) * 7 * week + static_cast<int>(sec);
    t.sec = sec - static_cast<int>(sec);
    return t;
}


/* time to gps time ------------------------------------------------------------
 * convert gtime_t struct to week and tow in gps time
 * args   : gtime_t t        I   gtime_t struct
 *          int    *week     IO  week number in gps time (NULL: no output)
 * return : time of week in gps time (s)
 *-----------------------------------------------------------------------------*/
double time2gpst(gtime_t t, int *week)
{
    gtime_t t0 = epoch2time(GPST0);
    time_t sec = t.time - t0.time;
    int w = static_cast<int>(sec / 604800);

    if (week)
        {
            *week = w;
        }
    return (static_cast<double>(sec - static_cast<time_t>(w * 604800)) + t.sec);
}


/* galileo system time to time -------------------------------------------------
 * convert week and tow in galileo system time (gst) to gtime_t struct
 * args   : int    week      I   week number in gst
 *          double sec       I   time of week in gst (s)
 * return : gtime_t struct
 *-----------------------------------------------------------------------------*/
gtime_t gst2time(int week, double sec)
{
    gtime_t t = epoch2time(GST0);

    if (sec < -1e9 || 1e9 < sec)
        {
            sec = 0.0;
        }
    t.time += static_cast<time_t>(86400) * 7 * week + static_cast<int>(sec);
    t.sec = sec - static_cast<int>(sec);
    return t;
}


/* time to galileo system time -------------------------------------------------
 * convert gtime_t struct to week and tow in galileo system time (gst)
 * args   : gtime_t t        I   gtime_t struct
 *          int    *week     IO  week number in gst (NULL: no output)
 * return : time of week in gst (s)
 *-----------------------------------------------------------------------------*/
double time2gst(gtime_t t, int *week)
{
    gtime_t t0 = epoch2time(GST0);
    time_t sec = t.time - t0.time;
    int w = static_cast<int>(sec / (86400 * 7));

    if (week)
        {
            *week = w;
        }
    return (sec - static_cast<double>(w) * 86400 * 7) + t.sec;
}


/* beidou time (bdt) to time ---------------------------------------------------
 * convert week and tow in beidou time (bdt) to gtime_t struct
 * args   : int    week      I   week number in bdt
 *          double sec       I   time of week in bdt (s)
 * return : gtime_t struct
 *-----------------------------------------------------------------------------*/
gtime_t bdt2time(int week, double sec)
{
    gtime_t t = epoch2time(BDT0);

    if (sec < -1e9 || 1e9 < sec)
        {
            sec = 0.0;
        }
    t.time += static_cast<time_t>(86400) * 7 * week + static_cast<int>(sec);
    t.sec = sec - static_cast<int>(sec);
    return t;
}


/* time to beidouo time (bdt) --------------------------------------------------
 * convert gtime_t struct to week and tow in beidou time (bdt)
 * args   : gtime_t t        I   gtime_t struct
 *          int    *week     IO  week number in bdt (NULL: no output)
 * return : time of week in bdt (s)
 *-----------------------------------------------------------------------------*/
double time2bdt(gtime_t t, int *week)
{
    gtime_t t0 = epoch2time(BDT0);
    time_t sec = t.time - t0.time;
    int w = static_cast<int>(sec / (86400 * 7));

    if (week)
        {
            *week = w;
        }
    return (sec - static_cast<double>(w) * 86400 * 7) + t.sec;
}


/* add time --------------------------------------------------------------------
 * add time to gtime_t struct
 * args   : gtime_t t        I   gtime_t struct
 *          double sec       I   time to add (s)
 * return : gtime_t struct (t+sec)
 *-----------------------------------------------------------------------------*/
gtime_t timeadd(gtime_t t, double sec)
{
    double tt;

    t.sec += sec;
    tt = floor(t.sec);
    t.time += static_cast<int>(tt);
    t.sec -= tt;
    return t;
}


/* time difference -------------------------------------------------------------
 * difference between gtime_t structs
 * args   : gtime_t t1,t2    I   gtime_t structs
 * return : time difference (t1-t2) (s)
 *-----------------------------------------------------------------------------*/
double timediff(gtime_t t1, gtime_t t2)
{
    return difftime(t1.time, t2.time) + t1.sec - t2.sec;
}

/* time difference accounting with week crossovers -------------------------------------------------------------
 * difference between gtime_t structs
 * args   : gtime_t t1,t2    I   gtime_t structs
 * return : time difference (t1-t2) (s)
 *-----------------------------------------------------------------------------*/
double timediffweekcrossover(gtime_t t1, gtime_t t2)
{
    //as stated in IS-GPS-200J table 20-IV footnote among other parts of the ICD,

    //if tk=(t - toe) > 302400s then tk = tk - s
    //if tk=(t - toe) < -302400s then tk = tk + 604800s
    double tk = difftime(t1.time, t2.time) + t1.sec - t2.sec;
    if (tk > 302400.0)
        {
            tk -= 604800.0;
        }
    else if (tk < -302400.0)
        {
            tk += 604800.0;
        }
    return tk;
}
/* get current time in utc -----------------------------------------------------
 * get current time in utc
 * args   : none
 * return : current time in utc
 *-----------------------------------------------------------------------------*/
gtime_t timeget(void)
{
    gtime_t time;
    double ep[6] = {};
    struct timeval tv
    {
    };
    struct tm *tt;

    if (!gettimeofday(&tv, nullptr) && (tt = gmtime(&tv.tv_sec)))
        {
            ep[0] = tt->tm_year + 1900;
            ep[1] = tt->tm_mon + 1;
            ep[2] = tt->tm_mday;
            ep[3] = tt->tm_hour;
            ep[4] = tt->tm_min;
            ep[5] = tt->tm_sec + tv.tv_usec * 1e-6;
        }
    time = epoch2time(ep);

#ifdef CPUTIME_IN_GPST /* cputime operated in gpst */
    time = gpst2utc(time);
#endif
    return time;
}


/* set current time in utc -----------------------------------------------------
 * set current time in utc
 * args   : gtime_t          I   current time in utc
 * return : none
 * notes  : just set time offset between cpu time and current time
 *          the time offset is reflected to only timeget()
 *          not reentrant
 *-----------------------------------------------------------------------------*/
void timeset(gtime_t t)
{
    timeoffset_ += timediff(t, timeget());
}

/* read leap seconds table by text -------------------------------------------*/
int read_leaps_text(FILE *fp)
{
    char buff[256], *p;
    int i, n = 0, ep[6], ls;

    rewind(fp);

    while (fgets(buff, sizeof(buff), fp) && n < MAXLEAPS)
        {
            if ((p = strchr(buff, '#')))
                {
                    *p = '\0';
                }
            if (sscanf(buff, "%d %d %d %d %d %d %d", ep, ep + 1, ep + 2, ep + 3, ep + 4, ep + 5,
                    &ls) < 7)
                {
                    continue;
                }
            for (i = 0; i < 6; i++)
                {
                    leaps[n][i] = ep[i];
                }
            leaps[n++][6] = ls;
        }
    return n;
}


/* read leap seconds table by usno -------------------------------------------*/
int read_leaps_usno(FILE *fp)
{
    static const char *months[] = {
        "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    double jd, tai_utc;
    char buff[256], month[32], ls[MAXLEAPS][7] = {};
    int i, j, y, m, d, n = 0;

    rewind(fp);

    while (fgets(buff, sizeof(buff), fp) && n < MAXLEAPS)
        {
            if (sscanf(buff, "%d %s %d =JD %lf TAI-UTC= %lf", &y, month, &d, &jd,
                    &tai_utc) < 5)
                {
                    continue;
                }
            if (y < 1980)
                {
                    continue;
                }
            for (m = 1; m <= 12; m++)
                {
                    if (!strcmp(months[m - 1], month))
                        {
                            break;
                        }
                }
            if (m >= 13)
                {
                    continue;
                }
            ls[n][0] = y;
            ls[n][1] = m;
            ls[n][2] = d;
            ls[n++][6] = static_cast<char>(19.0 - tai_utc);
        }
    for (i = 0; i < n; i++)
        {
            for (j = 0; j < 7; j++)
                {
                    leaps[i][j] = ls[n - i - 1][j];
                }
        }
    return n;
}


/* read leap seconds table -----------------------------------------------------
 * read leap seconds table
 * args   : char    *file    I   leap seconds table file
 * return : status (1:ok,0:error)
 * notes  : The leap second table should be as follows or leapsec.dat provided
 *          by USNO.
 *          (1) The records in the table file cosist of the following fields:
 *              year month day hour min sec UTC-GPST(s)
 *          (2) The date and time indicate the start UTC time for the UTC-GPST
 *          (3) The date and time should be descending order.
 *-----------------------------------------------------------------------------*/
int read_leaps(const char *file)
{
    FILE *fp;
    int i, n;

    if (!(fp = fopen(file, "re")))
        {
            return 0;
        }

    /* read leap seconds table by text or usno */
    if (!(n = read_leaps_text(fp)) && !(n = read_leaps_usno(fp)))
        {
            fclose(fp);
            return 0;
        }
    for (i = 0; i < 7; i++)
        {
            leaps[n][i] = 0.0;
        }
    fclose(fp);
    return 1;
}


/* gpstime to utc --------------------------------------------------------------
 * convert gpstime to utc considering leap seconds
 * args   : gtime_t t        I   time expressed in gpstime
 * return : time expressed in utc
 * notes  : ignore slight time offset under 100 ns
 *-----------------------------------------------------------------------------*/
gtime_t gpst2utc(gtime_t t)
{
    gtime_t tu;
    int i;

    for (i = 0; leaps[i][0] > 0; i++)
        {
            tu = timeadd(t, leaps[i][6]);
            if (timediff(tu, epoch2time(leaps[i])) >= 0.0)
                {
                    return tu;
                }
        }
    return t;
}


/* utc to gpstime --------------------------------------------------------------
 * convert utc to gpstime considering leap seconds
 * args   : gtime_t t        I   time expressed in utc
 * return : time expressed in gpstime
 * notes  : ignore slight time offset under 100 ns
 *-----------------------------------------------------------------------------*/
gtime_t utc2gpst(gtime_t t)
{
    int i;

    for (i = 0; leaps[i][0] > 0; i++)
        {
            if (timediff(t, epoch2time(leaps[i])) >= 0.0)
                {
                    return timeadd(t, -leaps[i][6]);
                }
        }
    return t;
}


/* gpstime to bdt --------------------------------------------------------------
 * convert gpstime to bdt (beidou navigation satellite system time)
 * args   : gtime_t t        I   time expressed in gpstime
 * return : time expressed in bdt
 * notes  : ref [8] 3.3, 2006/1/1 00:00 BDT = 2006/1/1 00:00 UTC
 *          no leap seconds in BDT
 *          ignore slight time offset under 100 ns
 *-----------------------------------------------------------------------------*/
gtime_t gpst2bdt(gtime_t t)
{
    return timeadd(t, -14.0);
}


/* bdt to gpstime --------------------------------------------------------------
 * convert bdt (beidou navigation satellite system time) to gpstime
 * args   : gtime_t t        I   time expressed in bdt
 * return : time expressed in gpstime
 * notes  : see gpst2bdt()
 *-----------------------------------------------------------------------------*/
gtime_t bdt2gpst(gtime_t t)
{
    return timeadd(t, 14.0);
}


/* time to day and sec -------------------------------------------------------*/
double time2sec(gtime_t time, gtime_t *day)
{
    double ep[6], sec;
    time2epoch(time, ep);
    sec = ep[3] * 3600.0 + ep[4] * 60.0 + ep[5];
    ep[3] = ep[4] = ep[5] = 0.0;
    *day = epoch2time(ep);
    return sec;
}


/* utc to gmst -----------------------------------------------------------------
 * convert utc to gmst (Greenwich mean sidereal time)
 * args   : gtime_t t        I   time expressed in utc
 *          double ut1_utc   I   UT1-UTC (s)
 * return : gmst (rad)
 *-----------------------------------------------------------------------------*/
double utc2gmst(gtime_t t, double ut1_utc)
{
    const double ep2000[] = {2000, 1, 1, 12, 0, 0};
    gtime_t tut, tut0;
    double ut, t1, t2, t3, gmst0, gmst;

    tut = timeadd(t, ut1_utc);
    ut = time2sec(tut, &tut0);
    t1 = timediff(tut0, epoch2time(ep2000)) / 86400.0 / 36525.0;
    t2 = t1 * t1;
    t3 = t2 * t1;
    gmst0 = 24110.54841 + 8640184.812866 * t1 + 0.093104 * t2 - 6.2E-6 * t3;
    gmst = gmst0 + 1.002737909350795 * ut;

    return fmod(gmst, 86400.0) * PI / 43200.0; /* 0 <= gmst <= 2*PI */
}


/* time to string --------------------------------------------------------------
 * convert gtime_t struct to string
 * args   : gtime_t t        I   gtime_t struct
 *          char   *s        O   string ("yyyy/mm/dd hh:mm:ss.ssss")
 *          int    n         I   number of decimals
 * return : none
 *-----------------------------------------------------------------------------*/
void time2str(gtime_t t, char *s, int n)
{
    double ep[6];

    if (n < 0)
        {
            n = 0;
        }
    else if (n > 12)
        {
            n = 12;
        }
    if (1.0 - t.sec < 0.5 / pow(10.0, n))
        {
            t.time++;
            t.sec = 0.0;
        };
    time2epoch(t, ep);
    sprintf(s, "%04.0f/%02.0f/%02.0f %02.0f:%02.0f:%0*.*f", ep[0], ep[1], ep[2],
        ep[3], ep[4], n <= 0 ? 2 : n + 3, n <= 0 ? 0 : n, ep[5]);
}


/* get time string -------------------------------------------------------------
 * get time string
 * args   : gtime_t t        I   gtime_t struct
 *          int    n         I   number of decimals
 * return : time string
 * notes  : not reentrant, do not use multiple in a function
 *-----------------------------------------------------------------------------*/
char *time_str(gtime_t t, int n)
{
    static char buff[64];
    time2str(t, buff, n);
    return buff;
}


/* time to day of year ---------------------------------------------------------
 * convert time to day of year
 * args   : gtime_t t        I   gtime_t struct
 * return : day of year (days)
 *-----------------------------------------------------------------------------*/
double time2doy(gtime_t t)
{
    double ep[6];

    time2epoch(t, ep);
    ep[1] = ep[2] = 1.0;
    ep[3] = ep[4] = ep[5] = 0.0;
    return timediff(t, epoch2time(ep)) / 86400.0 + 1.0;
}


/* adjust gps week number ------------------------------------------------------
 * adjust gps week number using cpu time
 * args   : int   week       I   not-adjusted gps week number
 * return : adjusted gps week number
 *-----------------------------------------------------------------------------*/
int adjgpsweek(int week)
{
    int w;
    (void)time2gpst(utc2gpst(timeget()), &w);
    if (w < 1560)
        {
            w = 1560; /* use 2009/12/1 if time is earlier than 2009/12/1 */
        }
    return week + (w - week + 512) / 1024 * 1024;
}


/* get tick time ---------------------------------------------------------------
 * get current tick in ms
 * args   : none
 * return : current tick in ms
 *-----------------------------------------------------------------------------*/
unsigned int tickget(void)
{
    struct timespec tp = {0, 0};
    struct timeval tv = {0, 0};

#ifdef CLOCK_MONOTONIC_RAW
    /* linux kernel > 2.6.28 */
    if (!clock_gettime(CLOCK_MONOTONIC_RAW, &tp))
        {
            return tp.tv_sec * 1000u + tp.tv_nsec / 1000000u;
        }
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000u + tv.tv_usec / 1000u;

#else
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000u + tv.tv_usec / 1000u;
#endif
}


/* sleep ms --------------------------------------------------------------------
 * sleep ms
 * args   : int   ms         I   milliseconds to sleep (<0:no sleep)
 * return : none
 *-----------------------------------------------------------------------------*/
void sleepms(int ms)
{
    struct timespec ts = {0, 0};
    if (ms <= 0)
        {
            return;
        }
    ts.tv_sec = static_cast<time_t>(ms / 1000);
    ts.tv_nsec = static_cast<int64_t>(ms % 1000 * 1000000);
    nanosleep(&ts, nullptr);
}


/* convert degree to deg-min-sec -----------------------------------------------
 * convert degree to degree-minute-second
 * args   : double deg       I   degree
 *          double *dms      O   degree-minute-second {deg, min, sec}
 *          int    ndec      I   number of decimals of second
 * return : none
 *-----------------------------------------------------------------------------*/
void deg2dms(double deg, double *dms, int ndec)
{
    double sign = deg < 0.0 ? -1.0 : 1.0, a = fabs(deg);
    double unit = pow(0.1, ndec);
    dms[0] = floor(a);
    a = (a - dms[0]) * 60.0;
    dms[1] = floor(a);
    a = (a - dms[1]) * 60.0;
    dms[2] = floor(a / unit + 0.5) * unit;
    if (dms[2] >= 60.0)
        {
            dms[2] = 0.0;
            dms[1] += 1.0;
            if (dms[1] >= 60.0)
                {
                    dms[1] = 0.0;
                    dms[0] += 1.0;
                }
        }
    dms[0] *= sign;
}

void deg2dms(double deg, double *dms)
{
    double sign = deg < 0.0 ? -1.0 : 1.0, a = fabs(deg);
    dms[0] = floor(a);
    a = (a - dms[0]) * 60.0;
    dms[1] = floor(a);
    a = (a - dms[1]) * 60.0;
    dms[2] = a;
    dms[0] *= sign;
}

/* convert deg-min-sec to degree -----------------------------------------------
 * convert degree-minute-second to degree
 * args   : double *dms      I   degree-minute-second {deg,min,sec}
 * return : degree
 *-----------------------------------------------------------------------------*/
double dms2deg(const double *dms)
{
    double sign = dms[0] < 0.0 ? -1.0 : 1.0;
    return sign * (fabs(dms[0]) + dms[1] / 60.0 + dms[2] / 3600.0);
}


/* transform ecef to geodetic position ------------------------------------------
 * transform ecef position to geodetic position
 * args   : double *r        I   ecef position {x,y,z} (m)
 *          double *pos      O   geodetic position {lat,lon,h} (rad,m)
 * return : none
 * notes  : WGS84, ellipsoidal height
 *-----------------------------------------------------------------------------*/
void ecef2pos(const double *r, double *pos)
{
    double e2 = FE_WGS84 * (2.0 - FE_WGS84), r2 = dot(r, r, 2), z, zk, v = RE_WGS84, sinp;

    for (z = r[2], zk = 0.0; fabs(z - zk) >= 1e-4;)
        {
            zk = z;
            sinp = z / sqrt(r2 + z * z);
            v = RE_WGS84 / sqrt(1.0 - e2 * sinp * sinp);
            z = r[2] + v * e2 * sinp;
        }
    pos[0] = r2 > 1e-12 ? atan(z / sqrt(r2)) : (r[2] > 0.0 ? PI / 2.0 : -PI / 2.0);
    pos[1] = r2 > 1e-12 ? atan2(r[1], r[0]) : 0.0;
    pos[2] = sqrt(r2 + z * z) - v;
}


/* transform geodetic to ecef position -----------------------------------------
 * transform geodetic position to ecef position
 * args   : double *pos      I   geodetic position {lat, lon,h} (rad,m)
 *          double *r        O   ecef position {x,y,z} (m)
 * return : none
 * notes  : WGS84, ellipsoidal height
 *-----------------------------------------------------------------------------*/
void pos2ecef(const double *pos, double *r)
{
    double sinp = sin(pos[0]), cosp = cos(pos[0]), sinl = sin(pos[1]), cosl = cos(pos[1]);
    double e2 = FE_WGS84 * (2.0 - FE_WGS84), v = RE_WGS84 / sqrt(1.0 - e2 * sinp * sinp);

    r[0] = (v + pos[2]) * cosp * cosl;
    r[1] = (v + pos[2]) * cosp * sinl;
    r[2] = (v * (1.0 - e2) + pos[2]) * sinp;
}


/* ecef to local coordinate transformation matrix ------------------------------
 * compute ecef to local coordinate transformation matrix
 * args   : double *pos      I   geodetic position {lat,lon} (rad)
 *          double *E        O   ecef to local coord transformation matrix (3x3)
 * return : none
 * notes  : matirix stored by column-major order (fortran convention)
 *-----------------------------------------------------------------------------*/
void xyz2enu(const double *pos, double *E)
{
    double sinp = sin(pos[0]), cosp = cos(pos[0]), sinl = sin(pos[1]), cosl = cos(pos[1]);

    E[0] = -sinl;
    E[3] = cosl;
    E[6] = 0.0;
    E[1] = -sinp * cosl;
    E[4] = -sinp * sinl;
    E[7] = cosp;
    E[2] = cosp * cosl;
    E[5] = cosp * sinl;
    E[8] = sinp;
}


/* transform ecef vector to local tangental coordinate -------------------------
 * transform ecef vector to local tangental coordinate
 * args   : double *pos      I   geodetic position {lat,lon} (rad)
 *          double *r        I   vector in ecef coordinate {x,y,z}
 *          double *e        O   vector in local tangental coordinate {e,n,u}
 * return : none
 *-----------------------------------------------------------------------------*/
void ecef2enu(const double *pos, const double *r, double *e)
{
    double E[9];

    xyz2enu(pos, E);
    matmul("NN", 3, 1, 3, 1.0, E, r, 0.0, e);
}


/* transform local vector to ecef coordinate -----------------------------------
 * transform local tangental coordinate vector to ecef
 * args   : double *pos      I   geodetic position {lat,lon} (rad)
 *          double *e        I   vector in local tangental coordinate {e,n,u}
 *          double *r        O   vector in ecef coordinate {x,y,z}
 * return : none
 *-----------------------------------------------------------------------------*/
void enu2ecef(const double *pos, const double *e, double *r)
{
    double E[9];

    xyz2enu(pos, E);
    matmul("TN", 3, 1, 3, 1.0, E, e, 0.0, r);
}


/* transform covariance to local tangental coordinate --------------------------
 * transform ecef covariance to local tangental coordinate
 * args   : double *pos      I   geodetic position {lat, lon} (rad)
 *          double *P        I   covariance in ecef coordinate
 *          double *Q        O   covariance in local tangental coordinate
 * return : none
 *-----------------------------------------------------------------------------*/
void covenu(const double *pos, const double *P, double *Q)
{
    double E[9], EP[9];

    xyz2enu(pos, E);
    matmul("NN", 3, 3, 3, 1.0, E, P, 0.0, EP);
    matmul("NT", 3, 3, 3, 1.0, EP, E, 0.0, Q);
}


/* transform local enu coordinate covariance to xyz-ecef -----------------------
 * transform local enu covariance to xyz-ecef coordinate
 * args   : double *pos      I   geodetic position {lat,lon} (rad)
 *          double *Q        I   covariance in local enu coordinate
 *          double *P        O   covariance in xyz-ecef coordinate
 * return : none
 *-----------------------------------------------------------------------------*/
void covecef(const double *pos, const double *Q, double *P)
{
    double E[9], EQ[9];

    xyz2enu(pos, E);
    matmul("TN", 3, 3, 3, 1.0, E, Q, 0.0, EQ);
    matmul("NN", 3, 3, 3, 1.0, EQ, E, 0.0, P);
}


/* astronomical arguments: f={l,l',F,D,OMG} (rad) ----------------------------*/
void ast_args(double t, double *f)
{
    static const double fc[][5] = {/* coefficients for iau 1980 nutation */
        {134.96340251, 1717915923.2178, 31.8792, 0.051635, -0.00024470},
        {357.52910918, 129596581.0481, -0.5532, 0.000136, -0.00001149},
        {93.27209062, 1739527262.8478, -12.7512, -0.001037, 0.00000417},
        {297.85019547, 1602961601.2090, -6.3706, 0.006593, -0.00003169},
        {125.04455501, -6962890.2665, 7.4722, 0.007702, -0.00005939}};
    double tt[4];
    int i, j;

    for (tt[0] = t, i = 1; i < 4; i++)
        {
            tt[i] = tt[i - 1] * t;
        }
    for (i = 0; i < 5; i++)
        {
            f[i] = fc[i][0] * 3600.0;
            for (j = 0; j < 4; j++)
                {
                    f[i] += fc[i][j + 1] * tt[j];
                }
            f[i] = fmod(f[i] * AS2R, 2.0 * PI);
        }
}


/* iau 1980 nutation ---------------------------------------------------------*/
void nut_iau1980(double t, const double *f, double *dpsi, double *deps)
{
    static const double nut[106][10] = {
        {0, 0, 0, 0, 1, -6798.4, -171996, -174.2, 92025, 8.9},
        {0, 0, 2, -2, 2, 182.6, -13187, -1.6, 5736, -3.1},
        {0, 0, 2, 0, 2, 13.7, -2274, -0.2, 977, -0.5},
        {0, 0, 0, 0, 2, -3399.2, 2062, 0.2, -895, 0.5},
        {0, -1, 0, 0, 0, -365.3, -1426, 3.4, 54, -0.1},
        {1, 0, 0, 0, 0, 27.6, 712, 0.1, -7, 0.0},
        {0, 1, 2, -2, 2, 121.7, -517, 1.2, 224, -0.6},
        {0, 0, 2, 0, 1, 13.6, -386, -0.4, 200, 0.0},
        {1, 0, 2, 0, 2, 9.1, -301, 0.0, 129, -0.1},
        {0, -1, 2, -2, 2, 365.2, 217, -0.5, -95, 0.3},
        {-1, 0, 0, 2, 0, 31.8, 158, 0.0, -1, 0.0},
        {0, 0, 2, -2, 1, 177.8, 129, 0.1, -70, 0.0},
        {-1, 0, 2, 0, 2, 27.1, 123, 0.0, -53, 0.0},
        {1, 0, 0, 0, 1, 27.7, 63, 0.1, -33, 0.0},
        {0, 0, 0, 2, 0, 14.8, 63, 0.0, -2, 0.0},
        {-1, 0, 2, 2, 2, 9.6, -59, 0.0, 26, 0.0},
        {-1, 0, 0, 0, 1, -27.4, -58, -0.1, 32, 0.0},
        {1, 0, 2, 0, 1, 9.1, -51, 0.0, 27, 0.0},
        {-2, 0, 0, 2, 0, -205.9, -48, 0.0, 1, 0.0},
        {-2, 0, 2, 0, 1, 1305.5, 46, 0.0, -24, 0.0},
        {0, 0, 2, 2, 2, 7.1, -38, 0.0, 16, 0.0},
        {2, 0, 2, 0, 2, 6.9, -31, 0.0, 13, 0.0},
        {2, 0, 0, 0, 0, 13.8, 29, 0.0, -1, 0.0},
        {1, 0, 2, -2, 2, 23.9, 29, 0.0, -12, 0.0},
        {0, 0, 2, 0, 0, 13.6, 26, 0.0, -1, 0.0},
        {0, 0, 2, -2, 0, 173.3, -22, 0.0, 0, 0.0},
        {-1, 0, 2, 0, 1, 27.0, 21, 0.0, -10, 0.0},
        {0, 2, 0, 0, 0, 182.6, 17, -0.1, 0, 0.0},
        {0, 2, 2, -2, 2, 91.3, -16, 0.1, 7, 0.0},
        {-1, 0, 0, 2, 1, 32.0, 16, 0.0, -8, 0.0},
        {0, 1, 0, 0, 1, 386.0, -15, 0.0, 9, 0.0},
        {1, 0, 0, -2, 1, -31.7, -13, 0.0, 7, 0.0},
        {0, -1, 0, 0, 1, -346.6, -12, 0.0, 6, 0.0},
        {2, 0, -2, 0, 0, -1095.2, 11, 0.0, 0, 0.0},
        {-1, 0, 2, 2, 1, 9.5, -10, 0.0, 5, 0.0},
        {1, 0, 2, 2, 2, 5.6, -8, 0.0, 3, 0.0},
        {0, -1, 2, 0, 2, 14.2, -7, 0.0, 3, 0.0},
        {0, 0, 2, 2, 1, 7.1, -7, 0.0, 3, 0.0},
        {1, 1, 0, -2, 0, -34.8, -7, 0.0, 0, 0.0},
        {0, 1, 2, 0, 2, 13.2, 7, 0.0, -3, 0.0},
        {-2, 0, 0, 2, 1, -199.8, -6, 0.0, 3, 0.0},
        {0, 0, 0, 2, 1, 14.8, -6, 0.0, 3, 0.0},
        {2, 0, 2, -2, 2, 12.8, 6, 0.0, -3, 0.0},
        {1, 0, 0, 2, 0, 9.6, 6, 0.0, 0, 0.0},
        {1, 0, 2, -2, 1, 23.9, 6, 0.0, -3, 0.0},
        {0, 0, 0, -2, 1, -14.7, -5, 0.0, 3, 0.0},
        {0, -1, 2, -2, 1, 346.6, -5, 0.0, 3, 0.0},
        {2, 0, 2, 0, 1, 6.9, -5, 0.0, 3, 0.0},
        {1, -1, 0, 0, 0, 29.8, 5, 0.0, 0, 0.0},
        {1, 0, 0, -1, 0, 411.8, -4, 0.0, 0, 0.0},
        {0, 0, 0, 1, 0, 29.5, -4, 0.0, 0, 0.0},
        {0, 1, 0, -2, 0, -15.4, -4, 0.0, 0, 0.0},
        {1, 0, -2, 0, 0, -26.9, 4, 0.0, 0, 0.0},
        {2, 0, 0, -2, 1, 212.3, 4, 0.0, -2, 0.0},
        {0, 1, 2, -2, 1, 119.6, 4, 0.0, -2, 0.0},
        {1, 1, 0, 0, 0, 25.6, -3, 0.0, 0, 0.0},
        {1, -1, 0, -1, 0, -3232.9, -3, 0.0, 0, 0.0},
        {-1, -1, 2, 2, 2, 9.8, -3, 0.0, 1, 0.0},
        {0, -1, 2, 2, 2, 7.2, -3, 0.0, 1, 0.0},
        {1, -1, 2, 0, 2, 9.4, -3, 0.0, 1, 0.0},
        {3, 0, 2, 0, 2, 5.5, -3, 0.0, 1, 0.0},
        {-2, 0, 2, 0, 2, 1615.7, -3, 0.0, 1, 0.0},
        {1, 0, 2, 0, 0, 9.1, 3, 0.0, 0, 0.0},
        {-1, 0, 2, 4, 2, 5.8, -2, 0.0, 1, 0.0},
        {1, 0, 0, 0, 2, 27.8, -2, 0.0, 1, 0.0},
        {-1, 0, 2, -2, 1, -32.6, -2, 0.0, 1, 0.0},
        {0, -2, 2, -2, 1, 6786.3, -2, 0.0, 1, 0.0},
        {-2, 0, 0, 0, 1, -13.7, -2, 0.0, 1, 0.0},
        {2, 0, 0, 0, 1, 13.8, 2, 0.0, -1, 0.0},
        {3, 0, 0, 0, 0, 9.2, 2, 0.0, 0, 0.0},
        {1, 1, 2, 0, 2, 8.9, 2, 0.0, -1, 0.0},
        {0, 0, 2, 1, 2, 9.3, 2, 0.0, -1, 0.0},
        {1, 0, 0, 2, 1, 9.6, -1, 0.0, 0, 0.0},
        {1, 0, 2, 2, 1, 5.6, -1, 0.0, 1, 0.0},
        {1, 1, 0, -2, 1, -34.7, -1, 0.0, 0, 0.0},
        {0, 1, 0, 2, 0, 14.2, -1, 0.0, 0, 0.0},
        {0, 1, 2, -2, 0, 117.5, -1, 0.0, 0, 0.0},
        {0, 1, -2, 2, 0, -329.8, -1, 0.0, 0, 0.0},
        {1, 0, -2, 2, 0, 23.8, -1, 0.0, 0, 0.0},
        {1, 0, -2, -2, 0, -9.5, -1, 0.0, 0, 0.0},
        {1, 0, 2, -2, 0, 32.8, -1, 0.0, 0, 0.0},
        {1, 0, 0, -4, 0, -10.1, -1, 0.0, 0, 0.0},
        {2, 0, 0, -4, 0, -15.9, -1, 0.0, 0, 0.0},
        {0, 0, 2, 4, 2, 4.8, -1, 0.0, 0, 0.0},
        {0, 0, 2, -1, 2, 25.4, -1, 0.0, 0, 0.0},
        {-2, 0, 2, 4, 2, 7.3, -1, 0.0, 1, 0.0},
        {2, 0, 2, 2, 2, 4.7, -1, 0.0, 0, 0.0},
        {0, -1, 2, 0, 1, 14.2, -1, 0.0, 0, 0.0},
        {0, 0, -2, 0, 1, -13.6, -1, 0.0, 0, 0.0},
        {0, 0, 4, -2, 2, 12.7, 1, 0.0, 0, 0.0},
        {0, 1, 0, 0, 2, 409.2, 1, 0.0, 0, 0.0},
        {1, 1, 2, -2, 2, 22.5, 1, 0.0, -1, 0.0},
        {3, 0, 2, -2, 2, 8.7, 1, 0.0, 0, 0.0},
        {-2, 0, 2, 2, 2, 14.6, 1, 0.0, -1, 0.0},
        {-1, 0, 0, 0, 2, -27.3, 1, 0.0, -1, 0.0},
        {0, 0, -2, 2, 1, -169.0, 1, 0.0, 0, 0.0},
        {0, 1, 2, 0, 1, 13.1, 1, 0.0, 0, 0.0},
        {-1, 0, 4, 0, 2, 9.1, 1, 0.0, 0, 0.0},
        {2, 1, 0, -2, 0, 131.7, 1, 0.0, 0, 0.0},
        {2, 0, 0, 2, 0, 7.1, 1, 0.0, 0, 0.0},
        {2, 0, 2, -2, 1, 12.8, 1, 0.0, -1, 0.0},
        {2, 0, -2, 0, 1, -943.2, 1, 0.0, 0, 0.0},
        {1, -1, 0, -2, 0, -29.3, 1, 0.0, 0, 0.0},
        {-1, 0, 0, 1, 1, -388.3, 1, 0.0, 0, 0.0},
        {-1, -1, 0, 2, 1, 35.0, 1, 0.0, 0, 0.0},
        {0, 1, 0, 1, 0, 27.3, 1, 0.0, 0, 0.0}};
    double ang;
    int i, j;

    *dpsi = *deps = 0.0;

    for (i = 0; i < 106; i++)
        {
            ang = 0.0;
            for (j = 0; j < 5; j++)
                {
                    ang += nut[i][j] * f[j];
                }
            *dpsi += (nut[i][6] + nut[i][7] * t) * sin(ang);
            *deps += (nut[i][8] + nut[i][9] * t) * cos(ang);
        }
    *dpsi *= 1e-4 * AS2R; /* 0.1 mas -> rad */
    *deps *= 1e-4 * AS2R;
}


/* eci to ecef transformation matrix -------------------------------------------
 * compute eci to ecef transformation matrix
 * args   : gtime_t tutc     I   time in utc
 *          double *erpv     I   erp values {xp,yp,ut1_utc,lod} (rad,rad,s,s/d)
 *          double *U        O   eci to ecef transformation matrix (3 x 3)
 *          double *gmst     IO  greenwich mean sidereal time (rad)
 *                               (NULL: no output)
 * return : none
 * note   : see ref [3] chap 5
 *          not thread-safe
 *-----------------------------------------------------------------------------*/
void eci2ecef(gtime_t tutc, const double *erpv, double *U, double *gmst)
{
    const double ep2000[] = {2000, 1, 1, 12, 0, 0};
    static gtime_t tutc_;
    static double U_[9], gmst_;
    gtime_t tgps;
    double eps, ze, th, z, t, t2, t3, dpsi, deps, gast, f[5];
    double R1[9], R2[9], R3[9], R[9], W[9], N[9], P[9], NP[9];
    int i;

    trace(4, "eci2ecef: tutc=%s\n", time_str(tutc, 3));

    if (fabs(timediff(tutc, tutc_)) < 0.01)
        { /* read cache */
            for (i = 0; i < 9; i++)
                {
                    U[i] = U_[i];
                }
            if (gmst)
                {
                    *gmst = gmst_;
                }
            return;
        }
    tutc_ = tutc;

    /* terrestrial time */
    tgps = utc2gpst(tutc_);
    t = (timediff(tgps, epoch2time(ep2000)) + 19.0 + 32.184) / 86400.0 / 36525.0;
    t2 = t * t;
    t3 = t2 * t;

    /* astronomical arguments */
    ast_args(t, f);

    /* iau 1976 precession */
    ze = (2306.2181 * t + 0.30188 * t2 + 0.017998 * t3) * AS2R;
    th = (2004.3109 * t - 0.42665 * t2 - 0.041833 * t3) * AS2R;
    z = (2306.2181 * t + 1.09468 * t2 + 0.018203 * t3) * AS2R;
    eps = (84381.448 - 46.8150 * t - 0.00059 * t2 + 0.001813 * t3) * AS2R;
    Rz(-z, R1);
    Ry(th, R2);
    Rz(-ze, R3);
    matmul("NN", 3, 3, 3, 1.0, R1, R2, 0.0, R);
    matmul("NN", 3, 3, 3, 1.0, R, R3, 0.0, P); /* P=Rz(-z)*Ry(th)*Rz(-ze) */

    /* iau 1980 nutation */
    nut_iau1980(t, f, &dpsi, &deps);
    Rx(-eps - deps, R1);
    Rz(-dpsi, R2);
    Rx(eps, R3);
    matmul("NN", 3, 3, 3, 1.0, R1, R2, 0.0, R);
    matmul("NN", 3, 3, 3, 1.0, R, R3, 0.0, N); /* N=Rx(-eps)*Rz(-dspi)*Rx(eps) */

    /* greenwich apparent sidereal time (rad) */
    gmst_ = utc2gmst(tutc_, erpv[2]);
    gast = gmst_ + dpsi * cos(eps);
    gast += (0.00264 * sin(f[4]) + 0.000063 * sin(2.0 * f[4])) * AS2R;

    /* eci to ecef transformation matrix */
    Ry(-erpv[0], R1);
    Rx(-erpv[1], R2);
    Rz(gast, R3);
    matmul("NN", 3, 3, 3, 1.0, R1, R2, 0.0, W);
    matmul("NN", 3, 3, 3, 1.0, W, R3, 0.0, R); /* W=Ry(-xp)*Rx(-yp) */
    matmul("NN", 3, 3, 3, 1.0, N, P, 0.0, NP);
    matmul("NN", 3, 3, 3, 1.0, R, NP, 0.0, U_); /* U=W*Rz(gast)*N*P */

    for (i = 0; i < 9; i++)
        {
            U[i] = U_[i];
        }
    if (gmst)
        {
            *gmst = gmst_;
        }

    trace(5, "gmst=%.12f gast=%.12f\n", gmst_, gast);
    trace(5, "P=\n");
    tracemat(5, P, 3, 3, 15, 12);
    trace(5, "N=\n");
    tracemat(5, N, 3, 3, 15, 12);
    trace(5, "W=\n");
    tracemat(5, W, 3, 3, 15, 12);
    trace(5, "U=\n");
    tracemat(5, U, 3, 3, 15, 12);
}


/* decode antenna parameter field --------------------------------------------*/
int decodef(char *p, int n, double *v)
{
    int i;

    for (i = 0; i < n; i++)
        {
            v[i] = 0.0;
        }
    for (i = 0, p = strtok(p, " "); p && i < n; p = strtok(nullptr, " "))
        {
            v[i++] = atof(p) * 1e-3;
        }
    return i;
}


/* add antenna parameter -----------------------------------------------------*/
void addpcv(const pcv_t *pcv, pcvs_t *pcvs)
{
    pcv_t *pcvs_pcv;

    if (pcvs->nmax <= pcvs->n)
        {
            pcvs->nmax += 256;
            if (!(pcvs_pcv = static_cast<pcv_t *>(realloc(pcvs->pcv, sizeof(pcv_t) * pcvs->nmax))))
                {
                    trace(1, "addpcv: memory allocation error\n");
                    free(pcvs->pcv);
                    pcvs->pcv = nullptr;
                    pcvs->n = pcvs->nmax = 0;
                    return;
                }
            pcvs->pcv = pcvs_pcv;
        }
    pcvs->pcv[pcvs->n++] = *pcv;
}


/* read ngs antenna parameter file -------------------------------------------*/
int readngspcv(const char *file, pcvs_t *pcvs)
{
    FILE *fp;
    static const pcv_t pcv0 = {0, {}, {}, {0, 0}, {0, 0}, {{}, {}}, {{}, {}}};
    pcv_t pcv = {0, {}, {}, {0, 0}, {0, 0}, {{}, {}}, {{}, {}}};
    double neu[3];
    int n = 0;
    char buff[256];

    if (!(fp = fopen(file, "re")))
        {
            trace(2, "ngs pcv file open error: %s\n", file);
            return 0;
        }
    while (fgets(buff, sizeof(buff), fp))
        {
            if (strlen(buff) >= 62 && buff[61] == '|')
                {
                    continue;
                }

            if (buff[0] != ' ')
                {
                    n = 0; /* start line */
                }
            if (++n == 1)
                {
                    pcv = pcv0;
                    strncpy(pcv.type, buff, 61);
                    pcv.type[61] = '\0';
                }
            else if (n == 2)
                {
                    if (decodef(buff, 3, neu) < 3)
                        {
                            continue;
                        }
                    pcv.off[0][0] = neu[1];
                    pcv.off[0][1] = neu[0];
                    pcv.off[0][2] = neu[2];
                }
            else if (n == 3)
                {
                    decodef(buff, 10, pcv.var[0]);
                }
            else if (n == 4)
                {
                    decodef(buff, 9, pcv.var[0] + 10);
                }
            else if (n == 5)
                {
                    if (decodef(buff, 3, neu) < 3)
                        {
                            continue;
                        };
                    pcv.off[1][0] = neu[1];
                    pcv.off[1][1] = neu[0];
                    pcv.off[1][2] = neu[2];
                }
            else if (n == 6)
                {
                    decodef(buff, 10, pcv.var[1]);
                }
            else if (n == 7)
                {
                    decodef(buff, 9, pcv.var[1] + 10);
                    addpcv(&pcv, pcvs);
                }
        }
    fclose(fp);

    return 1;
}


/* read antex file ----------------------------------------------------------*/
int readantex(const char *file, pcvs_t *pcvs)
{
    FILE *fp;
    static const pcv_t pcv0 = {0, {}, {}, {0, 0}, {0, 0}, {{}, {}}, {{}, {}}};
    pcv_t pcv = {0, {}, {}, {0, 0}, {0, 0}, {{}, {}}, {{}, {}}};
    double neu[3];
    int i, f, freq = 0, state = 0, freqs[] = {1, 2, 5, 6, 7, 8, 0};
    char buff[256];

    trace(3, "readantex: file=%s\n", file);

    if (!(fp = fopen(file, "re")))
        {
            trace(2, "antex pcv file open error: %s\n", file);
            return 0;
        }
    while (fgets(buff, sizeof(buff), fp))
        {
            if (strlen(buff) < 60 || strstr(buff + 60, "COMMENT"))
                {
                    continue;
                }

            if (strstr(buff + 60, "START OF ANTENNA"))
                {
                    pcv = pcv0;
                    state = 1;
                }
            if (strstr(buff + 60, "END OF ANTENNA"))
                {
                    addpcv(&pcv, pcvs);
                    state = 0;
                }
            if (!state)
                {
                    continue;
                }

            if (strstr(buff + 60, "TYPE / SERIAL NO"))
                {
                    strncpy(pcv.type, buff, 20);
                    pcv.type[20] = '\0';
                    strncpy(pcv.code, buff + 20, 20);
                    pcv.code[20] = '\0';
                    if (!strncmp(pcv.code + 3, "        ", 8))
                        {
                            pcv.sat = satid2no(pcv.code);
                        }
                }
            else if (strstr(buff + 60, "VALID FROM"))
                {
                    if (!str2time(buff, 0, 43, &pcv.ts))
                        {
                            continue;
                        }
                }
            else if (strstr(buff + 60, "VALID UNTIL"))
                {
                    if (!str2time(buff, 0, 43, &pcv.te))
                        {
                            continue;
                        }
                }
            else if (strstr(buff + 60, "START OF FREQUENCY"))
                {
                    if (sscanf(buff + 4, "%d", &f) < 1)
                        {
                            continue;
                        }
                    for (i = 0; i < NFREQ; i++)
                        {
                            if (freqs[i] == f)
                                {
                                    break;
                                }
                        }
                    if (i < NFREQ)
                        {
                            freq = i + 1;
                        }
                }
            else if (strstr(buff + 60, "END OF FREQUENCY"))
                {
                    freq = 0;
                }
            else if (strstr(buff + 60, "NORTH / EAST / UP"))
                {
                    if (freq < 1 || NFREQ < freq)
                        {
                            continue;
                        }
                    if (decodef(buff, 3, neu) < 3)
                        {
                            continue;
                        }
                    pcv.off[freq - 1][0] = neu[pcv.sat ? 0 : 1]; /* x or e */
                    pcv.off[freq - 1][1] = neu[pcv.sat ? 1 : 0]; /* y or n */
                    pcv.off[freq - 1][2] = neu[2];               /* z or u */
                }
            else if (strstr(buff, "NOAZI"))
                {
                    if (freq < 1 || NFREQ < freq)
                        {
                            continue;
                        }
                    if ((i = decodef(buff + 8, 19, pcv.var[freq - 1])) <= 0)
                        {
                            continue;
                        }
                    for (; i < 19; i++)
                        {
                            pcv.var[freq - 1][i] = pcv.var[freq - 1][i - 1];
                        }
                }
        }
    fclose(fp);

    return 1;
}


/* read antenna parameters ------------------------------------------------------
 * read antenna parameters
 * args   : char   *file       I   antenna parameter file (antex)
 *          pcvs_t *pcvs       IO  antenna parameters
 * return : status (1:ok,0:file open error)
 * notes  : file with the externsion .atx or .ATX is recognized as antex
 *          file except for antex is recognized ngs antenna parameters
 *          see reference [3]
 *          only support non-azimuth-depedent parameters
 *-----------------------------------------------------------------------------*/
int readpcv(const char *file, pcvs_t *pcvs)
{
    pcv_t *pcv;
    const char *ext;
    int i, stat;

    trace(3, "readpcv: file=%s\n", file);

    if (!(ext = strrchr(file, '.')))
        {
            ext = "";
        }

    if (!strcmp(ext, ".atx") || !strcmp(ext, ".ATX"))
        {
            stat = readantex(file, pcvs);
        }
    else
        {
            stat = readngspcv(file, pcvs);
        }
    for (i = 0; i < pcvs->n; i++)
        {
            pcv = pcvs->pcv + i;
            trace(4, "sat=%2d type=%20s code=%s off=%8.4f %8.4f %8.4f  %8.4f %8.4f %8.4f\n",
                pcv->sat, pcv->type, pcv->code, pcv->off[0][0], pcv->off[0][1],
                pcv->off[0][2], pcv->off[1][0], pcv->off[1][1], pcv->off[1][2]);
        }
    return stat;
}


/* search antenna parameter ----------------------------------------------------
 * read satellite antenna phase center position
 * args   : int    sat         I   satellite number (0: receiver antenna)
 *          char   *type       I   antenna type for receiver antenna
 *          gtime_t time       I   time to search parameters
 *          pcvs_t *pcvs       IO  antenna parameters
 * return : antenna parameter (NULL: no antenna)
 *-----------------------------------------------------------------------------*/
pcv_t *searchpcv(int sat, const char *type, gtime_t time,
    const pcvs_t *pcvs)
{
    pcv_t *pcv;
    char buff[MAXANT] = "", *types[2], *p;
    int i, j, n = 0;

    trace(3, "searchpcv: sat=%2d type=%s\n", sat, type);

    if (sat)
        { /* search satellite antenna */
            for (i = 0; i < pcvs->n; i++)
                {
                    pcv = pcvs->pcv + i;
                    if (pcv->sat != sat)
                        {
                            continue;
                        }
                    if (pcv->ts.time != 0 && timediff(pcv->ts, time) > 0.0)
                        {
                            continue;
                        }
                    if (pcv->te.time != 0 && timediff(pcv->te, time) < 0.0)
                        {
                            continue;
                        }
                    return pcv;
                }
        }
    else
        {
            if (strlen(type) < MAXANT + 1)
                {
                    strcpy(buff, type);
                }
            else
                {
                    trace(1, "type array is too long");
                }
            for (p = strtok(buff, " "); p && n < 2; p = strtok(nullptr, " "))
                {
                    types[n++] = p;
                }
            if (n <= 0)
                {
                    return nullptr;
                }

            /* search receiver antenna with radome at first */
            for (i = 0; i < pcvs->n; i++)
                {
                    pcv = pcvs->pcv + i;
                    for (j = 0; j < n; j++)
                        {
                            if (!strstr(pcv->type, types[j]))
                                {
                                    break;
                                }
                        }
                    if (j >= n)
                        {
                            return pcv;
                        }
                }
            /* search receiver antenna without radome */
            for (i = 0; i < pcvs->n; i++)
                {
                    pcv = pcvs->pcv + i;
                    if (strstr(pcv->type, types[0]) != pcv->type)
                        {
                            continue;
                        }

                    trace(2, "pcv without radome is used type=%s\n", type);
                    return pcv;
                }
        }
    return nullptr;
}


/* read station positions ------------------------------------------------------
 * read positions from station position file
 * args   : char  *file      I   station position file containing
 *                               lat(deg) lon(deg) height(m) name in a line
 *          char  *rcvs      I   station name
 *          double *pos      O   station position {lat,lon,h} (rad/m)
 *                               (all 0 if search error)
 * return : none
 *-----------------------------------------------------------------------------*/
void readpos(const char *file, const char *rcv, double *pos)
{
    static double poss[2048][3];
    static char stas[2048][16];
    FILE *fp;
    int i, j, len, np = 0;
    char buff[256], str[256];

    trace(3, "readpos: file=%s\n", file);

    if (!(fp = fopen(file, "re")))
        {
            fprintf(stderr, "reference position file open error : %s\n", file);
            return;
        }
    while (np < 2048 && fgets(buff, sizeof(buff), fp))
        {
            if (buff[0] == '%' || buff[0] == '#')
                {
                    continue;
                }
            if (sscanf(buff, "%lf %lf %lf %s", &poss[np][0], &poss[np][1], &poss[np][2],
                    str) < 4)
                {
                    continue;
                }
            // strncpy(stas[np], str, 15); This line triggers a warning. Replaced by:
            memcpy(stas[np], str, 15 * sizeof(stas[np][0]));
            stas[np++][15] = '\0';
        }
    fclose(fp);
    len = static_cast<int>(strlen(rcv));
    for (i = 0; i < np; i++)
        {
            if (strncmp(stas[i], rcv, len) != 0)
                {
                    continue;
                }
            for (j = 0; j < 3; j++)
                {
                    pos[j] = poss[i][j];
                }
            pos[0] *= D2R;
            pos[1] *= D2R;
            return;
        }
    pos[0] = pos[1] = pos[2] = 0.0;
}


/* read blq record -----------------------------------------------------------*/
int readblqrecord(FILE *fp, double *odisp)
{
    double v[11];
    char buff[256];
    int i, n = 0;

    while (fgets(buff, sizeof(buff), fp))
        {
            if (!strncmp(buff, "$$", 2))
                {
                    continue;
                }
            if (sscanf(buff, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                    v, v + 1, v + 2, v + 3, v + 4, v + 5, v + 6, v + 7, v + 8, v + 9, v + 10) < 11)
                {
                    continue;
                }
            for (i = 0; i < 11; i++)
                {
                    odisp[n + i * 6] = v[i];
                }
            if (++n == 6)
                {
                    return 1;
                }
        }
    return 0;
}


/* read blq ocean tide loading parameters --------------------------------------
 * read blq ocean tide loading parameters
 * args   : char   *file       I   BLQ ocean tide loading parameter file
 *          char   *sta        I   station name
 *          double *odisp      O   ocean tide loading parameters
 * return : status (1:ok, 0:file open error)
 *-----------------------------------------------------------------------------*/
int readblq(const char *file, const char *sta, double *odisp)
{
    FILE *fp;
    char buff[256], staname[32] = "", name[32], *p;

    /* station name to upper case */
    sscanf(sta, "%16s", staname);
    for (p = staname; (*p = static_cast<char>(toupper(static_cast<int>(*p)))); p++)
        {
            ;
        }

    if (!(fp = fopen(file, "re")))
        {
            trace(2, "blq file open error: file=%s\n", file);
            return 0;
        }
    while (fgets(buff, sizeof(buff), fp))
        {
            if (!strncmp(buff, "$$", 2) || strlen(buff) < 2)
                {
                    continue;
                }

            if (sscanf(buff + 2, "%16s", name) < 1)
                {
                    continue;
                }
            for (p = name; (*p = static_cast<char>(toupper(static_cast<int>(*p)))); p++)
                {
                    ;
                }
            if (strcmp(name, staname) != 0)
                {
                    continue;
                }

            /* read blq record */
            if (readblqrecord(fp, odisp))
                {
                    fclose(fp);
                    return 1;
                }
        }
    fclose(fp);
    trace(2, "no otl parameters: sta=%s file=%s\n", sta, file);
    return 0;
}


/* read earth rotation parameters ----------------------------------------------
 * read earth rotation parameters
 * args   : char   *file       I   IGS ERP file (IGS ERP ver.2)
 *          erp_t  *erp        O   earth rotation parameters
 * return : status (1:ok,0:file open error)
 *-----------------------------------------------------------------------------*/
int readerp(const char *file, erp_t *erp)
{
    FILE *fp;
    erpd_t *erp_data;
    double v[14] = {};
    char buff[256];

    trace(3, "readerp: file=%s\n", file);

    if (!(fp = fopen(file, "re")))
        {
            trace(2, "erp file open error: file=%s\n", file);
            return 0;
        }
    while (fgets(buff, sizeof(buff), fp))
        {
            if (sscanf(buff, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                    v, v + 1, v + 2, v + 3, v + 4, v + 5, v + 6, v + 7, v + 8, v + 9, v + 10, v + 11, v + 12, v + 13) < 5)
                {
                    continue;
                }
            if (erp->n >= erp->nmax)
                {
                    erp->nmax = erp->nmax <= 0 ? 128 : erp->nmax * 2;
                    erp_data = static_cast<erpd_t *>(realloc(erp->data, sizeof(erpd_t) * erp->nmax));
                    if (!erp_data)
                        {
                            free(erp->data);
                            erp->data = nullptr;
                            erp->n = erp->nmax = 0;
                            fclose(fp);
                            return 0;
                        }
                    erp->data = erp_data;
                }
            erp->data[erp->n].mjd = v[0];
            erp->data[erp->n].xp = v[1] * 1e-6 * AS2R;
            erp->data[erp->n].yp = v[2] * 1e-6 * AS2R;
            erp->data[erp->n].ut1_utc = v[3] * 1e-7;
            erp->data[erp->n].lod = v[4] * 1e-7;
            erp->data[erp->n].xpr = v[12] * 1e-6 * AS2R;
            erp->data[erp->n++].ypr = v[13] * 1e-6 * AS2R;
        }
    fclose(fp);
    return 1;
}


/* get earth rotation parameter values -----------------------------------------
 * get earth rotation parameter values
 * args   : erp_t  *erp        I   earth rotation parameters
 *          gtime_t time       I   time (gpst)
 *          double *erpv       O   erp values {xp,yp,ut1_utc,lod} (rad,rad,s,s/d)
 * return : status (1:ok,0:error)
 *-----------------------------------------------------------------------------*/
int geterp(const erp_t *erp, gtime_t time, double *erpv)
{
    const double ep[] = {2000, 1, 1, 12, 0, 0};
    double mjd, day, a;
    int i, j, k;

    trace(4, "geterp:\n");

    if (erp->n <= 0)
        {
            return 0;
        }

    mjd = 51544.5 + (timediff(gpst2utc(time), epoch2time(ep))) / 86400.0;

    if (mjd <= erp->data[0].mjd)
        {
            day = mjd - erp->data[0].mjd;
            erpv[0] = erp->data[0].xp + erp->data[0].xpr * day;
            erpv[1] = erp->data[0].yp + erp->data[0].ypr * day;
            erpv[2] = erp->data[0].ut1_utc - erp->data[0].lod * day;
            erpv[3] = erp->data[0].lod;
            return 1;
        }
    if (mjd >= erp->data[erp->n - 1].mjd)
        {
            day = mjd - erp->data[erp->n - 1].mjd;
            erpv[0] = erp->data[erp->n - 1].xp + erp->data[erp->n - 1].xpr * day;
            erpv[1] = erp->data[erp->n - 1].yp + erp->data[erp->n - 1].ypr * day;
            erpv[2] = erp->data[erp->n - 1].ut1_utc - erp->data[erp->n - 1].lod * day;
            erpv[3] = erp->data[erp->n - 1].lod;
            return 1;
        }
    for (j = 0, k = erp->n - 1; j < k - 1;)
        {
            i = (j + k) / 2;
            if (mjd < erp->data[i].mjd)
                {
                    k = i;
                }
            else
                {
                    j = i;
                }
        }
    if (erp->data[j].mjd == erp->data[j + 1].mjd)
        {
            a = 0.5;
        }
    else
        {
            a = (mjd - erp->data[j].mjd) / (erp->data[j + 1].mjd - erp->data[j].mjd);
        }
    erpv[0] = (1.0 - a) * erp->data[j].xp + a * erp->data[j + 1].xp;
    erpv[1] = (1.0 - a) * erp->data[j].yp + a * erp->data[j + 1].yp;
    erpv[2] = (1.0 - a) * erp->data[j].ut1_utc + a * erp->data[j + 1].ut1_utc;
    erpv[3] = (1.0 - a) * erp->data[j].lod + a * erp->data[j + 1].lod;
    return 1;
}

/* compare ephemeris ---------------------------------------------------------*/
int cmpeph(const void *p1, const void *p2)
{
    auto *q1 = (eph_t *)p1, *q2 = (eph_t *)p2;
    return q1->ttr.time != q2->ttr.time ? static_cast<int>(q1->ttr.time - q2->ttr.time) : (q1->toe.time != q2->toe.time ? static_cast<int>(q1->toe.time - q2->toe.time) : q1->sat - q2->sat);
}


/* sort and unique ephemeris -------------------------------------------------*/
void uniqeph(nav_t *nav)
{
    eph_t *nav_eph;
    int i, j;

    trace(3, "uniqeph: n=%d\n", nav->n);

    if (nav->n <= 0)
        {
            return;
        }

    qsort(nav->eph, nav->n, sizeof(eph_t), cmpeph);

    for (i = 1, j = 0; i < nav->n; i++)
        {
            if (nav->eph[i].sat != nav->eph[j].sat ||
                nav->eph[i].iode != nav->eph[j].iode)
                {
                    nav->eph[++j] = nav->eph[i];
                }
        }
    nav->n = j + 1;

    if (!(nav_eph = static_cast<eph_t *>(realloc(nav->eph, sizeof(eph_t) * nav->n))))
        {
            trace(1, "uniqeph malloc error n=%d\n", nav->n);
            free(nav->eph);
            nav->eph = nullptr;
            nav->n = nav->nmax = 0;
            return;
        }
    nav->eph = nav_eph;
    nav->nmax = nav->n;

    trace(4, "uniqeph: n=%d\n", nav->n);
}


/* compare glonass ephemeris -------------------------------------------------*/
int cmpgeph(const void *p1, const void *p2)
{
    auto *q1 = (geph_t *)p1, *q2 = (geph_t *)p2;
    return q1->tof.time != q2->tof.time ? static_cast<int>(q1->tof.time - q2->tof.time) : (q1->toe.time != q2->toe.time ? static_cast<int>(q1->toe.time - q2->toe.time) : q1->sat - q2->sat);
}


/* sort and unique glonass ephemeris -----------------------------------------*/
void uniqgeph(nav_t *nav)
{
    geph_t *nav_geph;
    int i, j;

    trace(3, "uniqgeph: ng=%d\n", nav->ng);

    if (nav->ng <= 0)
        {
            return;
        }

    qsort(nav->geph, nav->ng, sizeof(geph_t), cmpgeph);

    for (i = j = 0; i < nav->ng; i++)
        {
            if (nav->geph[i].sat != nav->geph[j].sat ||
                nav->geph[i].toe.time != nav->geph[j].toe.time ||
                nav->geph[i].svh != nav->geph[j].svh)
                {
                    nav->geph[++j] = nav->geph[i];
                }
        }
    nav->ng = j + 1;

    if (!(nav_geph = static_cast<geph_t *>(realloc(nav->geph, sizeof(geph_t) * nav->ng))))
        {
            trace(1, "uniqgeph malloc error ng=%d\n", nav->ng);
            free(nav->geph);
            nav->geph = nullptr;
            nav->ng = nav->ngmax = 0;
            return;
        }
    nav->geph = nav_geph;
    nav->ngmax = nav->ng;

    trace(4, "uniqgeph: ng=%d\n", nav->ng);
}


/* compare sbas ephemeris ----------------------------------------------------*/
int cmpseph(const void *p1, const void *p2)
{
    auto *q1 = (seph_t *)p1, *q2 = (seph_t *)p2;
    return q1->tof.time != q2->tof.time ? static_cast<int>(q1->tof.time - q2->tof.time) : (q1->t0.time != q2->t0.time ? static_cast<int>(q1->t0.time - q2->t0.time) : q1->sat - q2->sat);
}


/* sort and unique sbas ephemeris --------------------------------------------*/
void uniqseph(nav_t *nav)
{
    seph_t *nav_seph;
    int i, j;

    trace(3, "uniqseph: ns=%d\n", nav->ns);

    if (nav->ns <= 0)
        {
            return;
        }

    qsort(nav->seph, nav->ns, sizeof(seph_t), cmpseph);

    for (i = j = 0; i < nav->ns; i++)
        {
            if (nav->seph[i].sat != nav->seph[j].sat ||
                nav->seph[i].t0.time != nav->seph[j].t0.time)
                {
                    nav->seph[++j] = nav->seph[i];
                }
        }
    nav->ns = j + 1;

    if (!(nav_seph = static_cast<seph_t *>(realloc(nav->seph, sizeof(seph_t) * nav->ns))))
        {
            trace(1, "uniqseph malloc error ns=%d\n", nav->ns);
            free(nav->seph);
            nav->seph = nullptr;
            nav->ns = nav->nsmax = 0;
            return;
        }
    nav->seph = nav_seph;
    nav->nsmax = nav->ns;

    trace(4, "uniqseph: ns=%d\n", nav->ns);
}


/* unique ephemerides ----------------------------------------------------------
 * unique ephemerides in navigation data and update carrier wave length
 * args   : nav_t *nav    IO     navigation data
 * return : number of epochs
 *-----------------------------------------------------------------------------*/
void uniqnav(nav_t *nav)
{
    int i, j;

    trace(3, "uniqnav: neph=%d ngeph=%d nseph=%d\n", nav->n, nav->ng, nav->ns);

    /* unique ephemeris */
    uniqeph(nav);
    uniqgeph(nav);
    uniqseph(nav);

    /* update carrier wave length */
    for (i = 0; i < MAXSAT; i++)
        {
            for (j = 0; j < NFREQ; j++)
                {
                    nav->lam[i][j] = satwavelen(i + 1, j, nav);
                }
        }
}


/* compare observation data -------------------------------------------------*/
int cmpobs(const void *p1, const void *p2)
{
    auto *q1 = (obsd_t *)p1, *q2 = (obsd_t *)p2;
    double tt = timediff(q1->time, q2->time);
    if (fabs(tt) > DTTOL)
        {
            return tt < 0 ? -1 : 1;
        }
    if (q1->rcv != q2->rcv)
        {
            return static_cast<int>(q1->rcv) - static_cast<int>(q2->rcv);
        }
    return static_cast<int>(q1->sat) - static_cast<int>(q2->sat);
}


/* sort and unique observation data --------------------------------------------
 * sort and unique observation data by time, rcv, sat
 * args   : obs_t *obs    IO     observation data
 * return : number of epochs
 *-----------------------------------------------------------------------------*/
int sortobs(obs_t *obs)
{
    int i, j, n;

    trace(3, "sortobs: nobs=%d\n", obs->n);

    if (obs->n <= 0)
        {
            return 0;
        }

    qsort(obs->data, obs->n, sizeof(obsd_t), cmpobs);

    /* delete duplicated data */
    for (i = j = 0; i < obs->n; i++)
        {
            if (obs->data[i].sat != obs->data[j].sat ||
                obs->data[i].rcv != obs->data[j].rcv ||
                timediff(obs->data[i].time, obs->data[j].time) != 0.0)
                {
                    obs->data[++j] = obs->data[i];
                }
        }
    obs->n = j + 1;

    for (i = n = 0; i < obs->n; i = j, n++)
        {
            for (j = i + 1; j < obs->n; j++)
                {
                    if (timediff(obs->data[j].time, obs->data[i].time) > DTTOL)
                        {
                            break;
                        }
                }
        }
    return n;
}


/* screen by time --------------------------------------------------------------
 * screening by time start, time end, and time interval
 * args   : gtime_t time  I      time
 *          gtime_t ts    I      time start (ts.time==0:no screening by ts)
 *          gtime_t te    I      time end   (te.time==0:no screening by te)
 *          double  tint  I      time interval (s) (0.0:no screen by tint)
 * return : 1:on condition, 0:not on condition
 *-----------------------------------------------------------------------------*/
int screent(gtime_t time, gtime_t ts, gtime_t te, double tint)
{
    return (tint <= 0.0 || fmod(time2gpst(time, nullptr) + DTTOL, tint) <= DTTOL * 2.0) &&
           (ts.time == 0 || timediff(time, ts) >= -DTTOL) &&
           (te.time == 0 || timediff(time, te) < DTTOL);
}


/* read/save navigation data ---------------------------------------------------
 * save or load navigation data
 * args   : char    file  I      file path
 *          nav_t   nav   O/I    navigation data
 * return : status (1:ok,0:no file)
 *-----------------------------------------------------------------------------*/
int readnav(const char *file, nav_t *nav)
{
    FILE *fp;
    eph_t eph0 = {0, 0, 0, 0, 0, 0, 0, 0, {0, 0}, {0, 0}, {0, 0}, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, {}, {}, 0.0, 0.0};
    geph_t geph0 = {0, 0, 0, 0, 0, 0, {0, 0}, {0, 0}, {}, {}, {}, 0.0, 0.0, 0.0};
    char buff[4096], *p;
    int32_t toe_time, tof_time, toc_time, ttr_time;
    int i, sat, prn;

    trace(3, "loadnav: file=%s\n", file);

    if (!(fp = fopen(file, "re")))
        {
            return 0;
        }

    while (fgets(buff, sizeof(buff), fp))
        {
            if (!strncmp(buff, "IONUTC", 6))
                {
                    for (i = 0; i < 8; i++)
                        {
                            nav->ion_gps[i] = 0.0;
                        }
                    for (i = 0; i < 4; i++)
                        {
                            nav->utc_gps[i] = 0.0;
                        }
                    nav->leaps = 0;
                    sscanf(buff, "IONUTC,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d",
                        &nav->ion_gps[0], &nav->ion_gps[1], &nav->ion_gps[2], &nav->ion_gps[3],
                        &nav->ion_gps[4], &nav->ion_gps[5], &nav->ion_gps[6], &nav->ion_gps[7],
                        &nav->utc_gps[0], &nav->utc_gps[1], &nav->utc_gps[2], &nav->utc_gps[3],
                        &nav->leaps);
                    continue;
                }
            if ((p = strchr(buff, ',')))
                {
                    *p = '\0';
                }
            else
                {
                    continue;
                }
            if (!(sat = satid2no(buff)))
                {
                    continue;
                }
            if (satsys(sat, &prn) == SYS_GLO)
                {
                    nav->geph[prn - 1] = geph0;
                    nav->geph[prn - 1].sat = sat;
                    toe_time = tof_time = 0;
                    sscanf(p + 1,
                        "%d,%d,%d,%d,%d,%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,"
                        "%lf,%lf,%lf,%lf",
                        &nav->geph[prn - 1].iode, &nav->geph[prn - 1].frq, &nav->geph[prn - 1].svh,
                        &nav->geph[prn - 1].sva, &nav->geph[prn - 1].age,
                        &toe_time, &tof_time,
                        &nav->geph[prn - 1].pos[0], &nav->geph[prn - 1].pos[1], &nav->geph[prn - 1].pos[2],
                        &nav->geph[prn - 1].vel[0], &nav->geph[prn - 1].vel[1], &nav->geph[prn - 1].vel[2],
                        &nav->geph[prn - 1].acc[0], &nav->geph[prn - 1].acc[1], &nav->geph[prn - 1].acc[2],
                        &nav->geph[prn - 1].taun, &nav->geph[prn - 1].gamn, &nav->geph[prn - 1].dtaun);
                    nav->geph[prn - 1].toe.time = toe_time;
                    nav->geph[prn - 1].tof.time = tof_time;
                }
            else
                {
                    nav->eph[sat - 1] = eph0;
                    nav->eph[sat - 1].sat = sat;
                    toe_time = toc_time = ttr_time = 0;
                    sscanf(p + 1,
                        "%d,%d,%d,%d,%d,%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,"
                        "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d",
                        &nav->eph[sat - 1].iode, &nav->eph[sat - 1].iodc, &nav->eph[sat - 1].sva,
                        &nav->eph[sat - 1].svh,
                        &toe_time, &toc_time, &ttr_time,
                        &nav->eph[sat - 1].A, &nav->eph[sat - 1].e, &nav->eph[sat - 1].i0,
                        &nav->eph[sat - 1].OMG0, &nav->eph[sat - 1].omg, &nav->eph[sat - 1].M0,
                        &nav->eph[sat - 1].deln, &nav->eph[sat - 1].OMGd, &nav->eph[sat - 1].idot,
                        &nav->eph[sat - 1].crc, &nav->eph[sat - 1].crs, &nav->eph[sat - 1].cuc,
                        &nav->eph[sat - 1].cus, &nav->eph[sat - 1].cic, &nav->eph[sat - 1].cis,
                        &nav->eph[sat - 1].toes, &nav->eph[sat - 1].fit, &nav->eph[sat - 1].f0,
                        &nav->eph[sat - 1].f1, &nav->eph[sat - 1].f2, &nav->eph[sat - 1].tgd[0],
                        &nav->eph[sat - 1].code, &nav->eph[sat - 1].flag);
                    nav->eph[sat - 1].toe.time = toe_time;
                    nav->eph[sat - 1].toc.time = toc_time;
                    nav->eph[sat - 1].ttr.time = ttr_time;
                }
        }
    fclose(fp);
    return 1;
}


int savenav(const char *file, const nav_t *nav)
{
    FILE *fp;
    int i;
    char id[32];

    trace(3, "savenav: file=%s\n", file);

    if (!(fp = fopen(file, "we")))
        {
            return 0;
        }

    for (i = 0; i < MAXSAT; i++)
        {
            if (nav->eph[i].ttr.time == 0)
                {
                    continue;
                }
            satno2id(nav->eph[i].sat, id);
            fprintf(fp,
                "%s,%d,%d,%d,%d,%d,%d,%d,%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,"
                "%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,"
                "%.14E,%.14E,%.14E,%.14E,%.14E,%d,%d\n",
                id, nav->eph[i].iode, nav->eph[i].iodc, nav->eph[i].sva,
                nav->eph[i].svh, static_cast<int>(nav->eph[i].toe.time),
                static_cast<int>(nav->eph[i].toc.time), static_cast<int>(nav->eph[i].ttr.time),
                nav->eph[i].A, nav->eph[i].e, nav->eph[i].i0, nav->eph[i].OMG0,
                nav->eph[i].omg, nav->eph[i].M0, nav->eph[i].deln, nav->eph[i].OMGd,
                nav->eph[i].idot, nav->eph[i].crc, nav->eph[i].crs, nav->eph[i].cuc,
                nav->eph[i].cus, nav->eph[i].cic, nav->eph[i].cis, nav->eph[i].toes,
                nav->eph[i].fit, nav->eph[i].f0, nav->eph[i].f1, nav->eph[i].f2,
                nav->eph[i].tgd[0], nav->eph[i].code, nav->eph[i].flag);
        }
    for (i = 0; i < MAXPRNGLO; i++)
        {
            if (nav->geph[i].tof.time == 0)
                {
                    continue;
                }
            satno2id(nav->geph[i].sat, id);
            fprintf(fp,
                "%s,%d,%d,%d,%d,%d,%d,%d,%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,"
                "%.14E,%.14E,%.14E,%.14E,%.14E,%.14E\n",
                id, nav->geph[i].iode, nav->geph[i].frq, nav->geph[i].svh,
                nav->geph[i].sva, nav->geph[i].age, static_cast<int>(nav->geph[i].toe.time),
                static_cast<int>(nav->geph[i].tof.time),
                nav->geph[i].pos[0], nav->geph[i].pos[1], nav->geph[i].pos[2],
                nav->geph[i].vel[0], nav->geph[i].vel[1], nav->geph[i].vel[2],
                nav->geph[i].acc[0], nav->geph[i].acc[1], nav->geph[i].acc[2],
                nav->geph[i].taun, nav->geph[i].gamn, nav->geph[i].dtaun);
        }
    fprintf(fp,
        "IONUTC,%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,%.14E,"
        "%.14E,%.14E,%.14E,%d",
        nav->ion_gps[0], nav->ion_gps[1], nav->ion_gps[2], nav->ion_gps[3],
        nav->ion_gps[4], nav->ion_gps[5], nav->ion_gps[6], nav->ion_gps[7],
        nav->utc_gps[0], nav->utc_gps[1], nav->utc_gps[2], nav->utc_gps[3],
        nav->leaps);

    fclose(fp);
    return 1;
}


/* free observation data -------------------------------------------------------
 * free memory for observation data
 * args   : obs_t *obs    IO     observation data
 * return : none
 *-----------------------------------------------------------------------------*/
void freeobs(obs_t *obs)
{
    free(obs->data);
    obs->data = nullptr;
    obs->n = obs->nmax = 0;
}


/* free navigation data ---------------------------------------------------------
 * free memory for navigation data
 * args   : nav_t *nav    IO     navigation data
 *          int   opt     I      option (or of followings)
 *                               (0x01: gps/qzs ephmeris, 0x02: glonass ephemeris,
 *                                0x04: sbas ephemeris,   0x08: precise ephemeris,
 *                                0x10: precise clock     0x20: almanac,
 *                                0x40: tec data)
 * return : none
 *-----------------------------------------------------------------------------*/
void freenav(nav_t *nav, int opt)
{
    if (opt & 0x01)
        {
            free(nav->eph);
            nav->eph = nullptr;
            nav->n = nav->nmax = 0;
        }
    if (opt & 0x02)
        {
            free(nav->geph);
            nav->geph = nullptr;
            nav->ng = nav->ngmax = 0;
        }
    if (opt & 0x04)
        {
            free(nav->seph);
            nav->seph = nullptr;
            nav->ns = nav->nsmax = 0;
        }
    if (opt & 0x08)
        {
            free(nav->peph);
            nav->peph = nullptr;
            nav->ne = nav->nemax = 0;
        }
    if (opt & 0x10)
        {
            free(nav->pclk);
            nav->pclk = nullptr;
            nav->nc = nav->ncmax = 0;
        }
    if (opt & 0x20)
        {
            free(nav->alm);
            nav->alm = nullptr;
            nav->na = nav->namax = 0;
        }
    if (opt & 0x40)
        {
            free(nav->tec);
            nav->tec = nullptr;
            nav->nt = nav->ntmax = 0;
        }
    if (opt & 0x80)
        {
            free(nav->fcb);
            nav->fcb = nullptr;
            nav->nf = nav->nfmax = 0;
        }
}


/* debug trace functions -----------------------------------------------------*/
//#ifdef TRACE
//
FILE *fp_trace = nullptr;      /* file pointer of trace */
char file_trace[1024];         /* trace file */
static int level_trace = 0;    /* level of trace */
unsigned int tick_trace = 0;   /* tick time at traceopen (ms) */
gtime_t time_trace = {0, 0.0}; /* time at traceopen */
pthread_mutex_t lock_trace;    /* lock for trace */

void traceswap(void)
{
    gtime_t time = utc2gpst(timeget());
    char path[1024];

    rtk_lock(&lock_trace);

    if (static_cast<int>(time2gpst(time, nullptr) / INT_SWAP_TRAC) ==
        static_cast<int>(time2gpst(time_trace, nullptr) / INT_SWAP_TRAC))
        {
            rtk_unlock(&lock_trace);
            return;
        }
    time_trace = time;

    if (!reppath(file_trace, path, time, "", ""))
        {
            rtk_unlock(&lock_trace);
            return;
        }
    if (fp_trace)
        {
            fclose(fp_trace);
        }

    if (!(fp_trace = fopen(path, "we")))
        {
            fp_trace = stderr;
        }
    rtk_unlock(&lock_trace);
}


void traceopen(const char *file)
{
    gtime_t time = utc2gpst(timeget());
    char path[1024];

    reppath(file, path, time, "", "");
    if (!*path || !(fp_trace = fopen(path, "we")))
        {
            fp_trace = stderr;
        }
    if (strlen(file) < 1025)
        {
            strcpy(file_trace, file);
        }
    else
        {
            trace(1, "file array is too long");
        }
    tick_trace = tickget();
    time_trace = time;
    initlock(&lock_trace);
}


void traceclose(void)
{
    if (fp_trace && fp_trace != stderr)
        {
            fclose(fp_trace);
        }
    fp_trace = nullptr;
    file_trace[0] = '\0';
}


void tracelevel(int level)
{
    level_trace = level;
}
//extern void trace(int level, const char *format, ...)
//{
//    va_list ap;
//
//    /* print error message to stderr */
//    if (level <= 1) {
//        va_start(ap,format); vfprintf(stderr,format,ap); va_end(ap);
//    }
//    if (!fp_trace||level>level_trace) return;
//    traceswap();
//    fprintf(fp_trace,"%d ",level);
//    va_start(ap,format); vfprintf(fp_trace,format,ap); va_end(ap);
//    fflush(fp_trace);
//}

void tracet(int level, const char *format, ...)
{
    va_list ap;
    if (!fp_trace || level > level_trace)
        {
            return;
        }
    traceswap();
    fprintf(fp_trace, "%d %9.3f: ", level, (tickget() - tick_trace) / 1000.0);
    va_start(ap, format);
    vfprintf(fp_trace, format, ap);
    va_end(ap);
    fflush(fp_trace);
}

void tracemat(int level, const double *A, int n, int m, int p, int q)
{
    std::string buffer_;
    matsprint(A, n, m, p, q, buffer_);
    VLOG(level) << buffer_;
}


void traceobs(int level __attribute__((unused)), const obsd_t *obs __attribute__((unused)), int n __attribute__((unused)))
{
    //    char str[64],id[16];
    //    int i;
    //
    //    if (!fp_trace||level>level_trace) return;
    //    for (i=0;i<n;i++) {
    //        time2str(obs[i].time,str,3);
    //        satno2id(obs[i].sat,id);
    //        fprintf(fp_trace," (%2d) %s %-3s rcv%d %13.3f %13.3f %13.3f %13.3f %d %d %d %d %3.1f %3.1f\n",
    //              i+1,str,id,obs[i].rcv,obs[i].L[0],obs[i].L[1],obs[i].P[0],
    //              obs[i].P[1],obs[i].LLI[0],obs[i].LLI[1],obs[i].code[0],
    //              obs[i].code[1],obs[i].SNR[0]*0.25,obs[i].SNR[1]*0.25);
    //    }
    //    fflush(fp_trace);
}
//extern void tracenav(int level, const nav_t *nav)
//{
//    char s1[64],s2[64],id[16];
//    int i;
//
//    if (!fp_trace||level>level_trace) return;
//    for (i=0;i<nav->n;i++) {
//        time2str(nav->eph[i].toe,s1,0);
//        time2str(nav->eph[i].ttr,s2,0);
//        satno2id(nav->eph[i].sat,id);
//        fprintf(fp_trace,"(%3d) %-3s : %s %s %3d %3d %02x\n",i+1,
//                id,s1,s2,nav->eph[i].iode,nav->eph[i].iodc,nav->eph[i].svh);
//    }
//    fprintf(fp_trace,"(ion) %9.4e %9.4e %9.4e %9.4e\n",nav->ion_gps[0],
//            nav->ion_gps[1],nav->ion_gps[2],nav->ion_gps[3]);
//    fprintf(fp_trace,"(ion) %9.4e %9.4e %9.4e %9.4e\n",nav->ion_gps[4],
//            nav->ion_gps[5],nav->ion_gps[6],nav->ion_gps[7]);
//    fprintf(fp_trace,"(ion) %9.4e %9.4e %9.4e %9.4e\n",nav->ion_gal[0],
//            nav->ion_gal[1],nav->ion_gal[2],nav->ion_gal[3]);
//}
//extern void tracegnav(int level, const nav_t *nav)
//{
//    char s1[64],s2[64],id[16];
//    int i;
//
//    if (!fp_trace||level>level_trace) return;
//    for (i=0;i<nav->ng;i++) {
//        time2str(nav->geph[i].toe,s1,0);
//        time2str(nav->geph[i].tof,s2,0);
//        satno2id(nav->geph[i].sat,id);
//        fprintf(fp_trace,"(%3d) %-3s : %s %s %2d %2d %8.3f\n",i+1,
//                id,s1,s2,nav->geph[i].frq,nav->geph[i].svh,nav->geph[i].taun*1e6);
//    }
//}
//extern void tracehnav(int level, const nav_t *nav)
//{
//    char s1[64],s2[64],id[16];
//    int i;
//
//    if (!fp_trace||level>level_trace) return;
//    for (i=0;i<nav->ns;i++) {
//        time2str(nav->seph[i].t0,s1,0);
//        time2str(nav->seph[i].tof,s2,0);
//        satno2id(nav->seph[i].sat,id);
//        fprintf(fp_trace,"(%3d) %-3s : %s %s %2d %2d\n",i+1,
//                id,s1,s2,nav->seph[i].svh,nav->seph[i].sva);
//    }
//}
//extern void tracepeph(int level, const nav_t *nav)
//{
//    char s[64],id[16];
//    int i,j;
//
//    if (!fp_trace||level>level_trace) return;
//
//    for (i=0;i<nav->ne;i++) {
//        time2str(nav->peph[i].time,s,0);
//        for (j=0;j<MAXSAT;j++) {
//            satno2id(j+1,id);
//            fprintf(fp_trace,"%-3s %d %-3s %13.3f %13.3f %13.3f %13.3f %6.3f %6.3f %6.3f %6.3f\n",
//                    s,nav->peph[i].index,id,
//                    nav->peph[i].pos[j][0],nav->peph[i].pos[j][1],
//                    nav->peph[i].pos[j][2],nav->peph[i].pos[j][3]*1e9,
//                    nav->peph[i].std[j][0],nav->peph[i].std[j][1],
//                    nav->peph[i].std[j][2],nav->peph[i].std[j][3]*1e9);
//        }
//    }
//}
//extern void tracepclk(int level, const nav_t *nav)
//{
//    char s[64],id[16];
//    int i,j;
//
//    if (!fp_trace||level>level_trace) return;
//
//    for (i=0;i<nav->nc;i++) {
//        time2str(nav->pclk[i].time,s,0);
//        for (j=0;j<MAXSAT;j++) {
//            satno2id(j+1,id);
//            fprintf(fp_trace,"%-3s %d %-3s %13.3f %6.3f\n",
//                    s,nav->pclk[i].index,id,
//                    nav->pclk[i].clk[j][0]*1e9,nav->pclk[i].std[j][0]*1e9);
//        }
//    }
//}
//extern void traceb(int level, const unsigned char *p, int n)
//{
//    int i;
//    if (!fp_trace||level>level_trace) return;
//    for (i=0;i<n;i++) fprintf(fp_trace,"%02X%s",*p++,i%8==7?" ":"");
//    fprintf(fp_trace,"\n");
//}
//#else

//void traceopen(const char *file) {}
//void traceclose(void) {}
//void tracelevel(int level) {}
void trace(int level, const char *format, ...)
{
    va_list ap;
    char buffer[256];
    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);
    std::string str(buffer);
    VLOG(level) << "RTKLIB TRACE[" << level << "]:" << str;
}
//void tracet  (int level, const char *format, ...) {}
//void tracemat(int level, const double *A, int n, int m, int p, int q) {}
//void traceobs(int level, const obsd_t *obs, int n) {}
//void tracenav(int level, const nav_t *nav) {}
//void tracegnav(int level, const nav_t *nav) {}
//void tracehnav(int level, const nav_t *nav) {}
//void tracepeph(int level, const nav_t *nav) {}
//void tracepclk(int level, const nav_t *nav) {}
//void traceb  (int level, const unsigned char *p, int n) {}

//#endif /* TRACE */


/* execute command -------------------------------------------------------------
 * execute command line by operating system shell
 * args   : char   *cmd      I   command line
 * return : execution status (0:ok,0>:error)
 *-----------------------------------------------------------------------------*/
int execcmd(const char *cmd)
{
    trace(3, "execcmd: cmd=%s\n", cmd);
    return system(cmd);
}


/* create directory ------------------------------------------------------------
 * create directory if not exist
 * args   : char   *path     I   file path to be saved
 * return : none
 * notes  : not recursive. only one level
 *-----------------------------------------------------------------------------*/
void createdir(const char *path)
{
    char buff[1024], *p;
    //tracet(3, "createdir: path=%s\n", path);

    if (strlen(path) < 1025)
        {
            strcpy(buff, path);
        }
    else
        {
            trace(1, "path is too long");
        }
    if (!(p = strrchr(buff, FILEPATHSEP)))
        {
            return;
        }
    *p = '\0';

    if (mkdir(buff, 0777) != 0)
        {
            trace(1, "Error creating folder");
        }
}


/* replace string ------------------------------------------------------------*/
int repstr(char *str, const char *pat, const char *rep)
{
    int len = static_cast<int>(strlen(pat));
    char buff[1024], *p, *q, *r;

    for (p = str, r = buff; *p; p = q + len)
        {
            if (!(q = strstr(p, pat)))
                {
                    break;
                }
            strncpy(r, p, q - p);
            r += q - p;
            r += sprintf(r, "%s", rep);
        }
    if (p <= str)
        {
            return 0;
        }

    if (strlen(p) < 1025)
        {
            strcpy(r, p);
        }
    else
        {
            trace(1, "pat array is too long");
        }
    strcpy(str, buff);
    return 1;
}


/* replace keywords in file path -----------------------------------------------
 * replace keywords in file path with date, time, rover and base station id
 * args   : char   *path     I   file path (see below)
 *          char   *rpath    O   file path in which keywords replaced (see below)
 *          gtime_t time     I   time (gpst)  (time.time==0: not replaced)
 *          char   *rov      I   rover id string        ("": not replaced)
 *          char   *base     I   base station id string ("": not replaced)
 * return : status (1:keywords replaced, 0:no valid keyword in the path,
 *                  -1:no valid time)
 * notes  : the following keywords in path are replaced by date, time and name
 *              %Y -> yyyy : year (4 digits) (1900-2099)
 *              %y -> yy   : year (2 digits) (00-99)
 *              %m -> mm   : month           (01-12)
 *              %d -> dd   : day of month    (01-31)
 *              %h -> hh   : hours           (00-23)
 *              %M -> mm   : minutes         (00-59)
 *              %S -> ss   : seconds         (00-59)
 *              %n -> ddd  : day of year     (001-366)
 *              %W -> wwww : gps week        (0001-9999)
 *              %D -> d    : day of gps week (0-6)
 *              %H -> h    : hour code       (a=0,b=1,c=2,...,x=23)
 *              %ha-> hh   : 3 hours         (00,03,06,...,21)
 *              %hb-> hh   : 6 hours         (00,06,12,18)
 *              %hc-> hh   : 12 hours        (00,12)
 *              %t -> mm   : 15 minutes      (00,15,30,45)
 *              %r -> rrrr : rover id
 *              %b -> bbbb : base station id
 *-----------------------------------------------------------------------------*/
int reppath(const char *path, char *rpath, gtime_t time, const char *rov,
    const char *base)
{
    double ep[6], ep0[6] = {2000, 1, 1, 0, 0, 0};
    int week, dow, doy, stat = 0;
    char rep[64];

    strcpy(rpath, path);

    if (!strstr(rpath, "%"))
        {
            return 0;
        }
    if (*rov)
        {
            stat |= repstr(rpath, "%r", rov);
        }
    if (*base)
        {
            stat |= repstr(rpath, "%b", base);
        }
    if (time.time != 0)
        {
            time2epoch(time, ep);
            ep0[0] = ep[0];
            dow = static_cast<int>(floor(time2gpst(time, &week) / 86400.0));
            doy = static_cast<int>(floor(timediff(time, epoch2time(ep0)) / 86400.0)) + 1;
            sprintf(rep, "%02d", (static_cast<int>(ep[3]) / 3) * 3);
            stat |= repstr(rpath, "%ha", rep);
            sprintf(rep, "%02d", (static_cast<int>(ep[3]) / 6) * 6);
            stat |= repstr(rpath, "%hb", rep);
            sprintf(rep, "%02d", (static_cast<int>(ep[3]) / 12) * 12);
            stat |= repstr(rpath, "%hc", rep);
            sprintf(rep, "%04.0f", ep[0]);
            stat |= repstr(rpath, "%Y", rep);
            sprintf(rep, "%02.0f", fmod(ep[0], 100.0));
            stat |= repstr(rpath, "%y", rep);
            sprintf(rep, "%02.0f", ep[1]);
            stat |= repstr(rpath, "%m", rep);
            sprintf(rep, "%02.0f", ep[2]);
            stat |= repstr(rpath, "%d", rep);
            sprintf(rep, "%02.0f", ep[3]);
            stat |= repstr(rpath, "%h", rep);
            sprintf(rep, "%02.0f", ep[4]);
            stat |= repstr(rpath, "%M", rep);
            sprintf(rep, "%02.0f", floor(ep[5]));
            stat |= repstr(rpath, "%S", rep);
            sprintf(rep, "%03d", doy);
            stat |= repstr(rpath, "%n", rep);
            sprintf(rep, "%04d", week);
            stat |= repstr(rpath, "%W", rep);
            sprintf(rep, "%d", dow);
            stat |= repstr(rpath, "%D", rep);
            sprintf(rep, "%c", 'a' + static_cast<int>(ep[3]));
            stat |= repstr(rpath, "%H", rep);
            sprintf(rep, "%02d", (static_cast<int>(ep[4]) / 15) * 15);
            stat |= repstr(rpath, "%t", rep);
        }
    else if (strstr(rpath, "%ha") || strstr(rpath, "%hb") || strstr(rpath, "%hc") ||
             strstr(rpath, "%Y") || strstr(rpath, "%y") || strstr(rpath, "%m") ||
             strstr(rpath, "%d") || strstr(rpath, "%h") || strstr(rpath, "%M") ||
             strstr(rpath, "%S") || strstr(rpath, "%n") || strstr(rpath, "%W") ||
             strstr(rpath, "%D") || strstr(rpath, "%H") || strstr(rpath, "%t"))
        {
            return -1; /* no valid time */
        }
    return stat;
}


/* replace keywords in file path and generate multiple paths -------------------
 * replace keywords in file path with date, time, rover and base station id
 * generate multiple keywords-replaced paths
 * args   : char   *path     I   file path (see below)
 *          char   *rpath[]  O   file paths in which keywords replaced
 *          int    nmax      I   max number of output file paths
 *          gtime_t ts       I   time start (gpst)
 *          gtime_t te       I   time end   (gpst)
 *          char   *rov      I   rover id string        ("": not replaced)
 *          char   *base     I   base station id string ("": not replaced)
 * return : number of replaced file paths
 * notes  : see reppath() for replacements of keywords.
 *          minimum interval of time replaced is 900s.
 *-----------------------------------------------------------------------------*/
int reppaths(const char *path, char *rpath[], int nmax, gtime_t ts,
    gtime_t te, const char *rov, const char *base)
{
    gtime_t time;
    double tow, tint = 86400.0;
    int i, n = 0, week;

    trace(3, "reppaths: path =%s nmax=%d rov=%s base=%s\n", path, nmax, rov, base);

    if (ts.time == 0 || te.time == 0 || timediff(ts, te) > 0.0)
        {
            return 0;
        }

    if (strstr(path, "%S") || strstr(path, "%M") || strstr(path, "%t"))
        {
            tint = 900.0;
        }
    else if (strstr(path, "%h") || strstr(path, "%H"))
        {
            tint = 3600.0;
        }

    tow = time2gpst(ts, &week);
    time = gpst2time(week, floor(tow / tint) * tint);

    while (timediff(time, te) <= 0.0 && n < nmax)
        {
            reppath(path, rpath[n], time, rov, base);
            if (n == 0 || strcmp(rpath[n], rpath[n - 1]) != 0)
                {
                    n++;
                }
            time = timeadd(time, tint);
        }
    for (i = 0; i < n; i++)
        {
            trace(3, "reppaths: rpath=%s\n", rpath[i]);
        }
    return n;
}


/* satellite carrier wave length -----------------------------------------------
 * get satellite carrier wave lengths
 * args   : int    sat       I   satellite number
 *          int    frq       I   frequency index (0:L1,1:L2,2:L5/3,...)
 *          nav_t  *nav      I   navigation messages
 * return : carrier wave length (m) (0.0: error)
 *-----------------------------------------------------------------------------*/
double satwavelen(int sat, int frq, const nav_t *nav)
{
    const double freq_glo[] = {FREQ1_GLO, FREQ2_GLO};
    const double dfrq_glo[] = {DFRQ1_GLO, DFRQ2_GLO};
    int i, sys = satsys(sat, nullptr);

    if (sys == SYS_GLO)
        {
            if (0 <= frq && frq <= 1)
                {
                    for (i = 0; i < nav->ng; i++)
                        {
                            if (nav->geph[i].sat != sat)
                                {
                                    continue;
                                }
                            return SPEED_OF_LIGHT / (freq_glo[frq] + dfrq_glo[frq] * nav->geph[i].frq);
                        }
                }
            else if (frq == 2)
                { /* L3 */
                    return SPEED_OF_LIGHT / FREQ3_GLO;
                }
        }
    else if (sys == SYS_BDS)
        {
            if (frq == 0)
                {
                    return SPEED_OF_LIGHT / FREQ1_BDS; /* B1 */
                }
            if (frq == 1)
                {
                    return SPEED_OF_LIGHT / FREQ2_BDS; /* B2 */
                }
            if (frq == 2)
                {
                    return SPEED_OF_LIGHT / FREQ3_BDS; /* B3 */
                }
        }
    else
        {
            if (frq == 0)
                {
                    return SPEED_OF_LIGHT / FREQ1; /* L1/E1 */
                }
            if (frq == 1)
                {
                    return SPEED_OF_LIGHT / FREQ2; /* L2 */
                }
            if (frq == 2)
                {
                    return SPEED_OF_LIGHT / FREQ5; /* L5/E5a */
                }
            if (frq == 3)
                {
                    return SPEED_OF_LIGHT / FREQ6; /* L6/LEX */
                }
            if (frq == 4)
                {
                    return SPEED_OF_LIGHT / FREQ7; /* E5b */
                }
            if (frq == 5)
                {
                    return SPEED_OF_LIGHT / FREQ8; /* E5a+b */
                }
            else if (frq == 6)
                {
                    return SPEED_OF_LIGHT / FREQ9; /* S */
                }
        }
    return 0.0;
}


/* geometric distance ----------------------------------------------------------
 * compute geometric distance and receiver-to-satellite unit vector
 * args   : double *rs       I   satellilte position (ecef at transmission) (m)
 *          double *rr       I   receiver position (ecef at reception) (m)
 *          double *e        O   line-of-sight vector (ecef)
 * return : geometric distance (m) (0>:error/no satellite position)
 * notes  : distance includes sagnac effect correction
 *-----------------------------------------------------------------------------*/
double geodist(const double *rs, const double *rr, double *e)
{
    double r;
    int i;

    if (norm_rtk(rs, 3) < RE_WGS84)
        {
            return -1.0;
        }
    for (i = 0; i < 3; i++)
        {
            e[i] = rs[i] - rr[i];
        }
    r = norm_rtk(e, 3);
    for (i = 0; i < 3; i++)
        {
            e[i] /= r;
        }
    return r + DEFAULT_OMEGA_EARTH_DOT * (rs[0] * rr[1] - rs[1] * rr[0]) / SPEED_OF_LIGHT;
}


/* satellite azimuth/elevation angle -------------------------------------------
 * compute satellite azimuth/elevation angle
 * args   : double *pos      I   geodetic position {lat,lon,h} (rad,m)
 *          double *e        I   receiver-to-satellilte unit vector (ecef)
 *          double *azel     IO  azimuth/elevation {az,el} (rad) (NULL: no output)
 *                               (0.0<=azel[0]<2*pi,-pi/2<=azel[1]<=pi/2)
 * return : elevation angle (rad)
 *-----------------------------------------------------------------------------*/
double satazel(const double *pos, const double *e, double *azel)
{
    double az = 0.0, el = PI / 2.0, enu[3];

    if (pos[2] > -RE_WGS84)
        {
            ecef2enu(pos, e, enu);
            az = dot(enu, enu, 2) < 1e-12 ? 0.0 : atan2(enu[0], enu[1]);
            if (az < 0.0)
                {
                    az += 2 * PI;
                }
            el = asin(enu[2]);
        }
    if (azel)
        {
            azel[0] = az;
            azel[1] = el;
        }
    return el;
}


/* compute dops ----------------------------------------------------------------
 * compute DOP (dilution of precision)
 * args   : int    ns        I   number of satellites
 *          double *azel     I   satellite azimuth/elevation angle (rad)
 *          double elmin     I   elevation cutoff angle (rad)
 *          double *dop      O   DOPs {GDOP,PDOP,HDOP,VDOP}
 * return : none
 * notes  : dop[0]-[3] return 0 in case of dop computation error
 *-----------------------------------------------------------------------------*/
void dops(int ns, const double *azel, double elmin, double *dop)
{
    double H[4 * MAXSAT], Q[16], cosel, sinel;
    int i, n;

    for (i = 0; i < 4; i++)
        {
            dop[i] = 0.0;
        }
    for (i = n = 0; i < ns && i < MAXSAT; i++)
        {
            if (azel[1 + i * 2] < elmin || azel[1 + i * 2] <= 0.0)
                {
                    continue;
                }
            cosel = cos(azel[1 + i * 2]);
            sinel = sin(azel[1 + i * 2]);
            H[4 * n] = cosel * sin(azel[i * 2]);
            H[1 + 4 * n] = cosel * cos(azel[i * 2]);
            H[2 + 4 * n] = sinel;
            H[3 + 4 * n++] = 1.0;
        }
    if (n < 4)
        {
            return;
        }

    matmul("NT", 4, 4, n, 1.0, H, H, 0.0, Q);
    if (!matinv(Q, 4))
        {
            dop[0] = std::sqrt(Q[0] + Q[5] + Q[10] + Q[15]); /* GDOP */
            dop[1] = std::sqrt(Q[0] + Q[5] + Q[10]);         /* PDOP */
            dop[2] = std::sqrt(Q[0] + Q[5]);                 /* HDOP */
            dop[3] = std::sqrt(Q[10]);                       /* VDOP */
        }
}


/* ionosphere model ------------------------------------------------------------
 * compute ionospheric delay by broadcast ionosphere model (klobuchar model)
 * args   : gtime_t t        I   time (gpst)
 *          double *ion      I   iono model parameters {a0,a1,a2,a3,b0,b1,b2,b3}
 *          double *pos      I   receiver position {lat,lon,h} (rad,m)
 *          double *azel     I   azimuth/elevation angle {az,el} (rad)
 * return : ionospheric delay (L1) (m)
 *-----------------------------------------------------------------------------*/
double ionmodel(gtime_t t, const double *ion, const double *pos,
    const double *azel)
{
    const double ion_default[] = {/* 2004/1/1 */
        0.1118E-07, -0.7451e-08, -0.5961e-07, 0.1192E-06,
        0.1167E+06, -0.2294E+06, -0.1311e+06, 0.1049E+07};
    double tt, f, psi, phi, lam, amp, per, x;
    int week;

    if (pos[2] < -1e3 || azel[1] <= 0)
        {
            return 0.0;
        }
    if (norm_rtk(ion, 8) <= 0.0)
        {
            ion = ion_default;
        }

    /* earth centered angle (semi-circle) */
    psi = 0.0137 / (azel[1] / PI + 0.11) - 0.022;

    /* subionospheric latitude/longitude (semi-circle) */
    phi = pos[0] / PI + psi * cos(azel[0]);
    if (phi > 0.416)
        {
            phi = 0.416;
        }
    else if (phi < -0.416)
        {
            phi = -0.416;
        }
    lam = pos[1] / PI + psi * sin(azel[0]) / cos(phi * PI);

    /* geomagnetic latitude (semi-circle) */
    phi += 0.064 * cos((lam - 1.617) * PI);

    /* local time (s) */
    tt = 43200.0 * lam + time2gpst(t, &week);
    tt -= floor(tt / 86400.0) * 86400.0; /* 0 <= tt<86400 */

    /* slant factor */
    f = 1.0 + 16.0 * pow(0.53 - azel[1] / PI, 3.0);

    /* ionospheric delay */
    amp = ion[0] + phi * (ion[1] + phi * (ion[2] + phi * ion[3]));
    per = ion[4] + phi * (ion[5] + phi * (ion[6] + phi * ion[7]));
    amp = amp < 0.0 ? 0.0 : amp;
    per = per < 72000.0 ? 72000.0 : per;
    x = 2.0 * PI * (tt - 50400.0) / per;

    return SPEED_OF_LIGHT * f * (fabs(x) < 1.57 ? 5E-9 + amp * (1.0 + x * x * (-0.5 + x * x / 24.0)) : 5E-9);
}


/* ionosphere mapping function -------------------------------------------------
 * compute ionospheric delay mapping function by single layer model
 * args   : double *pos      I   receiver position {lat,lon,h} (rad,m)
 *          double *azel     I   azimuth/elevation angle {az,el} (rad)
 * return : ionospheric mapping function
 *-----------------------------------------------------------------------------*/
double ionmapf(const double *pos, const double *azel)
{
    if (pos[2] >= HION)
        {
            return 1.0;
        }
    return 1.0 / cos(asin((RE_WGS84 + pos[2]) / (RE_WGS84 + HION) * sin(PI / 2.0 - azel[1])));
}


/* ionospheric pierce point position -------------------------------------------
 * compute ionospheric pierce point (ipp) position and slant factor
 * args   : double *pos      I   receiver position {lat,lon,h} (rad,m)
 *          double *azel     I   azimuth/elevation angle {az,el} (rad)
 *          double re        I   earth radius (km)
 *          double hion      I   altitude of ionosphere (km)
 *          double *posp     O   pierce point position {lat,lon,h} (rad,m)
 * return : slant factor
 * notes  : see ref [2], only valid on the earth surface
 *          fixing bug on ref [2] A.4.4.10.1 A-22,23
 *-----------------------------------------------------------------------------*/
double ionppp(const double *pos, const double *azel, double re,
    double hion, double *posp)
{
    double cosaz, rp, ap, sinap, tanap;

    rp = re / (re + hion) * cos(azel[1]);
    ap = PI / 2.0 - azel[1] - asin(rp);
    sinap = sin(ap);
    tanap = tan(ap);
    cosaz = cos(azel[0]);
    posp[0] = asin(sin(pos[0]) * cos(ap) + cos(pos[0]) * sinap * cosaz);

    if ((pos[0] > 70.0 * D2R && tanap * cosaz > tan(PI / 2.0 - pos[0])) ||
        (pos[0] < -70.0 * D2R && -tanap * cosaz > tan(PI / 2.0 + pos[0])))
        {
            posp[1] = pos[1] + PI - asin(sinap * sin(azel[0]) / cos(posp[0]));
        }
    else
        {
            posp[1] = pos[1] + asin(sinap * sin(azel[0]) / cos(posp[0]));
        }
    return 1.0 / sqrt(1.0 - rp * rp);
}


/* troposphere model -----------------------------------------------------------
 * compute tropospheric delay by standard atmosphere and saastamoinen model
 * args   : gtime_t time     I   time
 *          double *pos      I   receiver position {lat,lon,h} (rad,m)
 *          double *azel     I   azimuth/elevation angle {az,el} (rad)
 *          double humi      I   relative humidity
 * return : tropospheric delay (m)
 *-----------------------------------------------------------------------------*/
double tropmodel(gtime_t time __attribute__((unused)), const double *pos, const double *azel,
    double humi)
{
    const double temp0 = 15.0; /* temparature at sea level */
    double hgt, pres, temp, e, z, trph, trpw;

    if (pos[2] < -100.0 || 1e4 < pos[2] || azel[1] <= 0)
        {
            return 0.0;
        }

    /* standard atmosphere */
    hgt = pos[2] < 0.0 ? 0.0 : pos[2];

    pres = 1013.25 * pow(1.0 - 2.2557E-5 * hgt, 5.2568);
    temp = temp0 - 6.5E-3 * hgt + 273.16;
    e = 6.108 * humi * exp((17.15 * temp - 4684.0) / (temp - 38.45));

    /* saastamoninen model */
    z = PI / 2.0 - azel[1];
    trph = 0.0022768 * pres / (1.0 - 0.00266 * cos(2.0 * pos[0]) - 0.00028 * hgt / 1e3) / cos(z);
    trpw = 0.002277 * (1255.0 / temp + 0.05) * e / cos(z);
    return trph + trpw;
}
#ifndef IERS_MODEL


double interpc(const double coef[], double lat)
{
    int i = static_cast<int>(lat / 15.0);
    if (i < 1)
        {
            return coef[0];
        }
    if (i > 4)
        {
            return coef[4];
        }
    return coef[i - 1] * (1.0 - lat / 15.0 + i) + coef[i] * (lat / 15.0 - i);
}


double mapf(double el, double a, double b, double c)
{
    double sinel = sin(el);
    return (1.0 + a / (1.0 + b / (1.0 + c))) / (sinel + (a / (sinel + b / (sinel + c))));
}


double nmf(gtime_t time, const double pos[], const double azel[],
    double *mapfw)
{
    /* ref [5] table 3 */
    /* hydro-ave-a,b,c, hydro-amp-a,b,c, wet-a,b,c at latitude 15,30,45,60,75 */
    const double coef[][5] = {
        {1.2769934E-3, 1.2683230E-3, 1.2465397E-3, 1.2196049E-3, 1.2045996E-3},
        {2.9153695E-3, 2.9152299E-3, 2.9288445E-3, 2.9022565E-3, 2.9024912E-3},
        {62.610505E-3, 62.837393E-3, 63.721774E-3, 63.824265E-3, 64.258455E-3},

        {0.0000000E-0, 1.2709626E-5, 2.6523662E-5, 3.4000452E-5, 4.1202191e-5},
        {0.0000000E-0, 2.1414979E-5, 3.0160779E-5, 7.2562722E-5, 11.723375E-5},
        {0.0000000E-0, 9.0128400E-5, 4.3497037E-5, 84.795348E-5, 170.37206E-5},

        {5.8021897E-4, 5.6794847E-4, 5.8118019E-4, 5.9727542E-4, 6.1641693E-4},
        {1.4275268E-3, 1.5138625E-3, 1.4572752E-3, 1.5007428E-3, 1.7599082E-3},
        {4.3472961e-2, 4.6729510E-2, 4.3908931e-2, 4.4626982E-2, 5.4736038E-2}};
    const double aht[] = {2.53E-5, 5.49E-3, 1.14E-3}; /* height correction */

    double y, cosy, ah[3], aw[3], dm, el = azel[1], lat = pos[0] * R2D, hgt = pos[2];
    int i;

    if (el <= 0.0)
        {
            if (mapfw)
                {
                    *mapfw = 0.0;
                }
            return 0.0;
        }
    /* year from doy 28, added half a year for southern latitudes */
    y = (time2doy(time) - 28.0) / 365.25 + (lat < 0.0 ? 0.5 : 0.0);

    cosy = cos(2.0 * PI * y);
    lat = fabs(lat);

    for (i = 0; i < 3; i++)
        {
            ah[i] = interpc(coef[i], lat) - interpc(coef[i + 3], lat) * cosy;
            aw[i] = interpc(coef[i + 6], lat);
        }
    /* ellipsoidal height is used instead of height above sea level */
    dm = (1.0 / sin(el) - mapf(el, aht[0], aht[1], aht[2])) * hgt / 1e3;

    if (mapfw)
        {
            *mapfw = mapf(el, aw[0], aw[1], aw[2]);
        }

    return mapf(el, ah[0], ah[1], ah[2]) + dm;
}
#endif /* !IERS_MODEL */


/* troposphere mapping function ------------------------------------------------
 * compute tropospheric mapping function by NMF
 * args   : gtime_t t        I   time
 *          double *pos      I   receiver position {lat,lon,h} (rad,m)
 *          double *azel     I   azimuth/elevation angle {az,el} (rad)
 *          double *mapfw    IO  wet mapping function (NULL: not output)
 * return : dry mapping function
 * note   : see ref [5] (NMF) and [9] (GMF)
 *          original JGR paper of [5] has bugs in eq.(4) and (5). the corrected
 *          paper is obtained from:
 *          ftp://web.haystack.edu/pub/aen/nmf/NMF_JGR.pdf
 *-----------------------------------------------------------------------------*/
double tropmapf(gtime_t time, const double pos[], const double azel[],
    double *mapfw)
{
#ifdef IERS_MODEL
    const double ep[] = {2000, 1, 1, 12, 0, 0};
    double mjd, lat, lon, hgt, zd, gmfh, gmfw;
#endif
    trace(4, "tropmapf: pos=%10.6f %11.6f %6.1f azel=%5.1f %4.1f\n",
        pos[0] * R2D, pos[1] * R2D, pos[2], azel[0] * R2D, azel[1] * R2D);

    if (pos[2] < -1000.0 || pos[2] > 20000.0)
        {
            if (mapfw)
                {
                    *mapfw = 0.0;
                }
            return 0.0;
        }
#ifdef IERS_MODEL
    mjd = 51544.5 + (timediff(time, epoch2time(ep))) / 86400.0;
    lat = pos[0];
    lon = pos[1];
    hgt = pos[2] - geoidh(pos); /* height in m (mean sea level) */
    zd = PI / 2.0 - azel[1];

    /* call GMF */
    gmf_(&mjd, &lat, &lon, &hgt, &zd, &gmfh, &gmfw);

    if (mapfw) *mapfw = gmfw;
    return gmfh;
#else
    return nmf(time, pos, azel, mapfw); /* NMF */
#endif
}


/* interpolate antenna phase center variation --------------------------------*/
double interpvar(double ang, const double *var)
{
    double a = ang / 5.0; /* ang=0-90 */
    int i = static_cast<int>(a);
    if (i < 0)
        {
            return var[0];
        }
    if (i >= 18)
        {
            return var[18];
        }
    return var[i] * (1.0 - a + i) + var[i + 1] * (a - i);
}


/* receiver antenna model ------------------------------------------------------
 * compute antenna offset by antenna phase center parameters
 * args   : pcv_t *pcv       I   antenna phase center parameters
 *          double *azel     I   azimuth/elevation for receiver {az,el} (rad)
 *          int     opt      I   option (0:only offset,1:offset+pcv)
 *          double *dant     O   range offsets for each frequency (m)
 * return : none
 * notes  : current version does not support azimuth dependent terms
 *-----------------------------------------------------------------------------*/
void antmodel(const pcv_t *pcv, const double *del, const double *azel,
    int opt, double *dant)
{
    double e[3], off[3], cosel = cos(azel[1]);
    int i, j;

    trace(4, "antmodel: azel=%6.1f %4.1f opt=%d\n", azel[0] * R2D, azel[1] * R2D, opt);

    e[0] = sin(azel[0]) * cosel;
    e[1] = cos(azel[0]) * cosel;
    e[2] = sin(azel[1]);

    for (i = 0; i < NFREQ; i++)
        {
            for (j = 0; j < 3; j++)
                {
                    off[j] = pcv->off[i][j] + del[j];
                }

            dant[i] = -dot(off, e, 3) + (opt ? interpvar(90.0 - azel[1] * R2D, pcv->var[i]) : 0.0);
        }
    trace(5, "antmodel: dant=%6.3f %6.3f\n", dant[0], dant[1]);
}


/* satellite antenna model ------------------------------------------------------
 * compute satellite antenna phase center parameters
 * args   : pcv_t *pcv       I   antenna phase center parameters
 *          double nadir     I   nadir angle for satellite (rad)
 *          double *dant     O   range offsets for each frequency (m)
 * return : none
 *-----------------------------------------------------------------------------*/
void antmodel_s(const pcv_t *pcv, double nadir, double *dant)
{
    int i;

    trace(4, "antmodel_s: nadir=%6.1f\n", nadir * R2D);

    for (i = 0; i < NFREQ; i++)
        {
            dant[i] = interpvar(nadir * R2D * 5.0, pcv->var[i]);
        }
    trace(5, "antmodel_s: dant=%6.3f %6.3f\n", dant[0], dant[1]);
}


/* sun and moon position in eci (ref [4] 5.1.1, 5.2.1) -----------------------*/
void sunmoonpos_eci(gtime_t tut, double *rsun, double *rmoon)
{
    const double ep2000[] = {2000, 1, 1, 12, 0, 0};
    double t, f[5], eps, Ms, ls, rs, lm, pm, rm, sine, cose, sinp, cosp, sinl, cosl;

    trace(4, "sunmoonpos_eci: tut=%s\n", time_str(tut, 3));

    t = timediff(tut, epoch2time(ep2000)) / 86400.0 / 36525.0;

    /* astronomical arguments */
    ast_args(t, f);

    /* obliquity of the ecliptic */
    eps = 23.439291 - 0.0130042 * t;
    sine = sin(eps * D2R);
    cose = cos(eps * D2R);

    /* sun position in eci */
    if (rsun)
        {
            Ms = 357.5277233 + 35999.05034 * t;
            ls = 280.460 + 36000.770 * t + 1.914666471 * sin(Ms * D2R) + 0.019994643 * sin(2.0 * Ms * D2R);
            rs = AU * (1.000140612 - 0.016708617 * cos(Ms * D2R) - 0.000139589 * cos(2.0 * Ms * D2R));
            sinl = sin(ls * D2R);
            cosl = cos(ls * D2R);
            rsun[0] = rs * cosl;
            rsun[1] = rs * cose * sinl;
            rsun[2] = rs * sine * sinl;

            trace(5, "rsun =%.3f %.3f %.3f\n", rsun[0], rsun[1], rsun[2]);
        }
    /* moon position in eci */
    if (rmoon)
        {
            lm = 218.32 + 481267.883 * t + 6.29 * sin(f[0]) - 1.27 * sin(f[0] - 2.0 * f[3]) +
                 0.66 * sin(2.0 * f[3]) + 0.21 * sin(2.0 * f[0]) - 0.19 * sin(f[1]) - 0.11 * sin(2.0 * f[2]);
            pm = 5.13 * sin(f[2]) + 0.28 * sin(f[0] + f[2]) - 0.28 * sin(f[2] - f[0]) -
                 0.17 * sin(f[2] - 2.0 * f[3]);
            rm = RE_WGS84 / sin((0.9508 + 0.0518 * cos(f[0]) + 0.0095 * cos(f[0] - 2.0 * f[3]) +
                                    0.0078 * cos(2.0 * f[3]) + 0.0028 * cos(2.0 * f[0])) *
                                D2R);
            sinl = sin(lm * D2R);
            cosl = cos(lm * D2R);
            sinp = sin(pm * D2R);
            cosp = cos(pm * D2R);
            rmoon[0] = rm * cosp * cosl;
            rmoon[1] = rm * (cose * cosp * sinl - sine * sinp);
            rmoon[2] = rm * (sine * cosp * sinl + cose * sinp);

            trace(5, "rmoon=%.3f %.3f %.3f\n", rmoon[0], rmoon[1], rmoon[2]);
        }
}


/* sun and moon position -------------------------------------------------------
 * get sun and moon position in ecef
 * args   : gtime_t tut      I   time in ut1
 *          double *erpv     I   erp value {xp,yp,ut1_utc,lod} (rad,rad,s,s/d)
 *          double *rsun     IO  sun position in ecef  (m) (NULL: not output)
 *          double *rmoon    IO  moon position in ecef (m) (NULL: not output)
 *          double *gmst     O   gmst (rad)
 * return : none
 *-----------------------------------------------------------------------------*/
void sunmoonpos(gtime_t tutc, const double *erpv, double *rsun,
    double *rmoon, double *gmst)
{
    gtime_t tut;
    double rs[3], rm[3], U[9], gmst_;

    trace(4, "sunmoonpos: tutc=%s\n", time_str(tutc, 3));

    tut = timeadd(tutc, erpv[2]); /* utc -> ut1 */

    /* sun and moon position in eci */
    sunmoonpos_eci(tut, rsun ? rs : nullptr, rmoon ? rm : nullptr);

    /* eci to ecef transformation matrix */
    eci2ecef(tutc, erpv, U, &gmst_);

    /* sun and moon position in ecef */
    if (rsun)
        {
            matmul("NN", 3, 1, 3, 1.0, U, rs, 0.0, rsun);
        }
    if (rmoon)
        {
            matmul("NN", 3, 1, 3, 1.0, U, rm, 0.0, rmoon);
        }
    if (gmst)
        {
            *gmst = gmst_;
        }
}


/* carrier smoothing -----------------------------------------------------------
 * carrier smoothing by Hatch filter
 * args   : obs_t  *obs      IO  raw observation data/smoothed observation data
 *          int    ns        I   smoothing window size (epochs)
 * return : none
 *-----------------------------------------------------------------------------*/
void csmooth(obs_t *obs, int ns)
{
    double Ps[2][MAXSAT][NFREQ] = {}, Lp[2][MAXSAT][NFREQ] = {}, dcp;
    int i, j, s, r, n[2][MAXSAT][NFREQ] = {};
    obsd_t *p;

    trace(3, "csmooth: nobs=%d,ns=%d\n", obs->n, ns);

    for (i = 0; i < obs->n; i++)
        {
            p = &obs->data[i];
            s = p->sat;
            r = p->rcv;
            for (j = 0; j < NFREQ; j++)
                {
                    if (s <= 0 || MAXSAT < s || r <= 0 || 2 < r)
                        {
                            continue;
                        }
                    if (p->P[j] == 0.0 || p->L[j] == 0.0)
                        {
                            continue;
                        }
                    if (p->LLI[j])
                        {
                            n[r - 1][s - 1][j] = 0;
                        }
                    if (n[r - 1][s - 1][j] == 0)
                        {
                            Ps[r - 1][s - 1][j] = p->P[j];
                        }
                    else
                        {
                            dcp = LAM_CARR[j] * (p->L[j] - Lp[r - 1][s - 1][j]);
                            Ps[r - 1][s - 1][j] = p->P[j] / ns + (Ps[r - 1][s - 1][j] + dcp) * (ns - 1) / ns;
                        }
                    if (++n[r - 1][s - 1][j] < ns)
                        {
                            p->P[j] = 0.0;
                        }
                    else
                        {
                            p->P[j] = Ps[r - 1][s - 1][j];
                        }
                    Lp[r - 1][s - 1][j] = p->L[j];
                }
        }
}


/* uncompress file -------------------------------------------------------------
 * uncompress (uncompress/unzip/uncompact hatanaka-compression/tar) file
 * args   : char   *file     I   input file
 *          char   *uncfile  O   uncompressed file
 * return : status (-1:error,0:not compressed file,1:uncompress completed)
 * note   : creates uncompressed file in tempolary directory
 *          gzip and crx2rnx commands have to be installed in commands path
 *-----------------------------------------------------------------------------*/
int rtk_uncompress(const char *file, char *uncfile)
{
    int stat = 0;
    char *p, cmd[2048] = "", tmpfile[1024] = "", buff[1024], *fname, *dir = (char *)"";

    trace(3, "rtk_uncompress: file=%s\n", file);

    if (strlen(file) < 1025)
        {
            strcpy(tmpfile, file);
        }
    else
        {
            trace(1, "file array is too long");
        }
    if (!(p = strrchr(tmpfile, '.')))
        {
            return 0;
        }

    /* uncompress by gzip */
    if (!strcmp(p, ".z") || !strcmp(p, ".Z") ||
        !strcmp(p, ".gz") || !strcmp(p, ".GZ") ||
        !strcmp(p, ".zip") || !strcmp(p, ".ZIP"))
        {
            strcpy(uncfile, tmpfile);
            uncfile[p - tmpfile] = '\0';
            sprintf(cmd, R"(gzip -f -d -c "%s" > "%s")", tmpfile, uncfile);

            if (execcmd(cmd))
                {
                    if (remove(uncfile) != 0)
                        {
                            trace(1, "Error removing file");
                        }
                    return -1;
                }
            if (strlen(uncfile) < 1025)
                {
                    strcpy(tmpfile, uncfile);
                }
            stat = 1;
        }
    /* extract tar file */
    if ((p = strrchr(tmpfile, '.')) && !strcmp(p, ".tar"))
        {
            strcpy(uncfile, tmpfile);
            uncfile[p - tmpfile] = '\0';
            strcpy(buff, tmpfile);
            fname = buff;
            if ((p = strrchr(buff, '/')))
                {
                    *p = '\0';
                    dir = fname;
                    fname = p + 1;
                }
            // sprintf(cmd, "tar -C \"%s\" -xf \"%s\"", dir, tmpfile);
            // NOTE: This sprintf triggers a format overflow warning. Replaced by:
            std::ostringstream temp;
            std::string s_aux1(dir);
            std::string s_aux2(tmpfile);
            temp << "tar -C " << s_aux1 << " -xf " << s_aux2;
            std::string s_aux = temp.str();
            int n = s_aux.length();
            if (n < 2048)
                {
                    for (int i = 0; i < n; i++)
                        {
                            cmd[i] = s_aux[i];
                        }
                }

            if (execcmd(cmd))
                {
                    if (stat)
                        {
                            if (remove(tmpfile) != 0)
                                {
                                    trace(1, "Error removing file");
                                }
                        }
                    return -1;
                }
            if (stat)
                {
                    if (remove(tmpfile) != 0)
                        {
                            trace(1, "Error removing file");
                        }
                }
            stat = 1;
        }
    /* extract hatanaka-compressed file by cnx2rnx */
    else if ((p = strrchr(tmpfile, '.')) && strlen(p) > 3 && (*(p + 3) == 'd' || *(p + 3) == 'D'))
        {
            strcpy(uncfile, tmpfile);
            uncfile[p - tmpfile + 3] = *(p + 3) == 'D' ? 'O' : 'o';
            sprintf(cmd, R"(crx2rnx < "%s" > "%s")", tmpfile, uncfile);

            if (execcmd(cmd))
                {
                    if (remove(uncfile) != 0)
                        {
                            trace(1, "Error removing file");
                        }
                    if (stat)
                        {
                            if (remove(tmpfile) != 0)
                                {
                                    trace(1, "Error removing file");
                                }
                        }
                    return -1;
                }
            if (stat)
                {
                    if (remove(tmpfile) != 0)
                        {
                            trace(1, "Error removing file");
                        }
                }
            stat = 1;
        }
    trace(3, "rtk_uncompress: stat=%d\n", stat);
    return stat;
}


/* expand file path ------------------------------------------------------------
 * expand file path with wild-card (*) in file
 * args   : char   *path     I   file path to expand (captal insensitive)
 *          char   *paths    O   expanded file paths
 *          int    nmax      I   max number of expanded file paths
 * return : number of expanded file paths
 * notes  : the order of expanded files is alphabetical order
 *-----------------------------------------------------------------------------*/
int expath(const char *path, char *paths[], int nmax)
{
    int i, j, n = 0;
    char tmp[1024] = "";
    struct dirent *d;
    DIR *dp;
    const char *file = path;
    char dir[1024] = "", s1[1024], s2[1024], *p, *q, *r;

    trace(3, "expath  : path=%s nmax=%d\n", path, nmax);

    //TODO: Fix  invalid conversion from ‘const char*’ to ‘char*’
    //if ((p=strrchr(path,'/')) || (p=strrchr(path,'\\'))) {
    //    file=p+1; strncpy(dir,path,p-path+1); dir[p-path+1]='\0';
    //}
    if (!(dp = opendir(*dir ? dir : ".")))
        {
            return 0;
        }
    while ((d = readdir(dp)))
        {
            if (*(d->d_name) == '.')
                {
                    continue;
                }
            sprintf(s1, "^%s$", d->d_name);
            sprintf(s2, "^%s$", file);
            for (p = s1; *p; p++)
                {
                    *p = static_cast<char>(tolower(static_cast<int>(*p)));
                }
            for (p = s2; *p; p++)
                {
                    *p = static_cast<char>(tolower(static_cast<int>(*p)));
                }

            for (p = s1, q = strtok_r(s2, "*", &r); q; q = strtok_r(nullptr, "*", &r))
                {
                    if ((p = strstr(p, q)))
                        {
                            p += strlen(q);
                        }
                    else
                        {
                            break;
                        }
                }
            if (p && n < nmax)
                {
                    sprintf(paths[n++], "%s%s", dir, d->d_name);
                }
        }
    closedir(dp);
    /* sort paths in alphabetical order */
    for (i = 0; i < n - 1; i++)
        {
            for (j = i + 1; j < n; j++)
                {
                    if (strcmp(paths[i], paths[j]) > 0)
                        {
                            if (strlen(paths[i]) < 1025)
                                {
                                    strcpy(tmp, paths[i]);
                                }
                            else
                                {
                                    trace(1, "Path is too long");
                                }
                            strcpy(paths[i], paths[j]);
                            strcpy(paths[j], tmp);
                        }
                }
        }
    for (i = 0; i < n; i++)
        {
            trace(3, "expath  : file=%s\n", paths[i]);
        }

    return n;
}

/* From RTKLIB 2.4.2 */
void windupcorr(gtime_t time, const double *rs, const double *rr, double *phw)
{
    double ek[3], exs[3], eys[3], ezs[3], ess[3], exr[3], eyr[3], eks[3], ekr[3], E[9];
    double dr[3], ds[3], drs[3], r[3], pos[3], rsun[3], cosp, ph, erpv[5] = {0};
    int i;

    trace(4, "windupcorr: time=%s\n", time_str(time, 0));

    /* sun position in ecef */
    sunmoonpos(gpst2utc(time), erpv, rsun, nullptr, nullptr);

    /* unit vector satellite to receiver */
    for (i = 0; i < 3; i++)
        {
            r[i] = rr[i] - rs[i];
        }
    if (!normv3(r, ek))
        {
            return;
        }

    /* unit vectors of satellite antenna */
    for (i = 0; i < 3; i++)
        {
            r[i] = -rs[i];
        }
    if (!normv3(r, ezs))
        {
            return;
        }
    for (i = 0; i < 3; i++)
        {
            r[i] = rsun[i] - rs[i];
        }
    if (!normv3(r, ess))
        {
            return;
        }
    cross3(ezs, ess, r);
    if (!normv3(r, eys))
        {
            return;
        }
    cross3(eys, ezs, exs);

    /* unit vectors of receiver antenna */
    ecef2pos(rr, pos);
    xyz2enu(pos, E);
    exr[0] = E[1];
    exr[1] = E[4];
    exr[2] = E[7]; /* x = north */
    eyr[0] = -E[0];
    eyr[1] = -E[3];
    eyr[2] = -E[6]; /* y = west  */

    /* phase windup effect */
    cross3(ek, eys, eks);
    cross3(ek, eyr, ekr);
    for (i = 0; i < 3; i++)
        {
            ds[i] = exs[i] - ek[i] * dot(ek, exs, 3) - eks[i];
            dr[i] = exr[i] - ek[i] * dot(ek, exr, 3) + ekr[i];
        }
    cosp = dot(ds, dr, 3) / norm_rtk(ds, 3) / norm_rtk(dr, 3);
    if (cosp < -1.0)
        {
            cosp = -1.0;
        }
    else if (cosp > 1.0)
        {
            cosp = 1.0;
        }
    ph = acos(cosp) / 2.0 / PI;
    cross3(ds, dr, drs);
    if (dot(ek, drs, 3) < 0.0)
        {
            ph = -ph;
        }

    *phw = ph + floor(*phw - ph + 0.5); /* in cycle */
}
