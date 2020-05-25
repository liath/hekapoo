#ifndef _HEKAPOO_H
#define _HEKAPOO_H

#include "si4010_types.h"

// defines GPIO pins used for button input (see button vector mapping in AN370)
#define bButtonMask_c 0x1F; // GPIO0, GPIO1, GPIO2, GPIO3, GPIO4
#define bModOOK_c 0

#define f_RkeFreqOOK_c 3E+8 // for RKEdemo OOK
#define b_PaLevel_c 37       // PA level
#define b_PaMaxDrv_c 0       // PA level
#define b_PaNominalCap_c 10  // PA level

#define iLowBatMv_c 2500

#define bButton1_c 0x01
#define bButton2_c 0x02
#define bButton3_c 0x04
#define bButton4_c 0x08
#define bButton5_c 0x10
// defines position of button bits in status byte of the transmitted packets
#define M_ButtonBits_c 0x1F

// Amount of time, the application will wait after boot without
// getting a qualified button push before giving up and shutting the chip down
#define bMaxWaitForPush_c 50
#define wRepeatInterval_c 100 // ms
#define bRepeatCount_c 1
#define bBatteryWait_c 100
// This specifies the number of times that the RTC ISR is called between
// each call to the button service routine.  The actual time between
// calls is dependent on the setting of RTC_CTRL.RTC_DIV which dictates how
// often the RTC ISR is called (5ms in this demo).It means that the interval
// between calls to the vBsr_Service() function is 5*2=10ms.
#define bDebounceInterval_c 2
// Size of FIFO of captured buttons .. max number of unserviced button changes
#define bPtsSize_c 3
//---------------------------------------------------------------------------
//    PROTOTYPES OF STATIC FUNCTIONS:
//-------------------------------------------------------------------------
// Assemble packet for Rke demo receiver
void vPacketAssemble(void);
// Buttoons checking
void vButtonCheck(void);

void vRepeatTxLoop(void);

BYTE bEnc_CustomEncode(BYTE xdata *pboEncodedBytes, BYTE biByteToEncode);

//---------------------------------------------------------------------------
//    VARIABLES:
BYTE bIsr_DebounceCount;
LWORD xdata lLEDOnTime;
BYTE xdata bRepeatCount;
LWORD lTimestamp;
BYTE bdata bPrevBtn = 0;
// Pointer to the head of the frame to be sent out.
BYTE xdata *pbFrameHead;
BYTE xdata bFrameSize;
BYTE bBatStatus;

// de bruijn, precomputed for 10bit sweeping using utilities.js
code BYTE abFrame[128] = {0x00, 0x20, 0x18, 0x0a, 0x03, 0x81, 0x20, 0x58, 0x1a,
  0x07, 0x82, 0x20, 0x98, 0x2a, 0x0b, 0x83, 0x20, 0xd8, 0x3a, 0x0f, 0x84, 0x23,
  0x09, 0x42, 0x70, 0xa4, 0x2b, 0x0b, 0x42, 0xf0, 0xc4, 0x33, 0x0d, 0x43, 0x70,
  0xe4, 0x3b, 0x0f, 0x43, 0xf1, 0x14, 0x47, 0x12, 0x44, 0xb1, 0x34, 0x4f, 0x14,
  0xc5, 0x51, 0x5c, 0x59, 0x16, 0xc5, 0xd1, 0x7c, 0x63, 0x28, 0xce, 0x34, 0x8d,
  0x63, 0x68, 0xde, 0x39, 0x8e, 0xa3, 0xb8, 0xf2, 0x3d, 0x8f, 0xa3, 0xf9, 0x26,
  0x4a, 0x92, 0xe4, 0xd9, 0x3a, 0x4f, 0x94, 0xa7, 0x2a, 0xca, 0xd2, 0xbc, 0xb3,
  0x2d, 0x4b, 0x72, 0xec, 0xbd, 0x2f, 0xcc, 0xd3, 0x3c, 0xd5, 0x35, 0xcd, 0xb3,
  0x74, 0xdf, 0x39, 0xd6, 0x76, 0x9d, 0xe7, 0xa9, 0xee, 0x7d, 0x9f, 0xa7, 0xfa,
  0xab, 0xab, 0x6a, 0xfa, 0xd6, 0xf5, 0xdd, 0x7b, 0x5f, 0xdb, 0x76, 0xfd, 0xdf,
  0x7b, 0xff};

#define bFrameSize_c sizeof(abFrame)

int iBatteryMv;

BYTE xdata bButtonState;

float xdata fDesiredFreq;
/* Structure for setting up the ODS .. must be in XDATA */
tOds_Setup xdata rOdsSetup;

/* Structure for setting up the XO .. must be in XDATA */
tFCast_XoSetup xdata rXoSetup;

/* Structure for setting up the PA .. must be in XDATA */
tPa_Setup xdata rPaSetup;

/* BSR control structure */
tBsr_Setup xdata rBsrSetup;
BYTE xdata abBsrPtsPlaceHolder[bPtsSize_c * 2] = {0};
WORD wPacketCount;
#endif /* _HEKAPOO_H */
