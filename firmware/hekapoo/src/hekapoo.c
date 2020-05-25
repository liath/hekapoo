// normal stuffs
#include "stdlib.h"
// si4010 stuffs
#include "si4010.h"
#include "si4010_api_rom.h"
// our stuffs
#include "hekapoo.h"

void main(void) {
  PDMD = 1;
  // Disable the Matrix and Roff modes on GPIO[3:1]
  PORT_CTRL &= ~(M_PORT_MATRIX | M_PORT_ROFF | M_PORT_STROBE);
  PORT_CTRL |= M_PORT_STROBE;
  PORT_CTRL &= (~M_PORT_STROBE);
  // Turn LED control off
  GPIO_LED = 0;
  vSys_Setup(10); // 80ms battery insert time?
  vSys_SetMasterTime(0);
  // Setup the bandgap
  vSys_BandGapLdo(1);

  // if part is burned to user or run mode.
  if ((PROT0_CTRL & M_NVM_BLOWN) > 1) {
    // Check the first power up bit meaning keyfob is powered on by battery
    // insertion
    if (0 != (SYSGEN & M_POWER_1ST_TIME)) {
      vSys_FirstPowerUp(); // Function will shutdown.
    }
  }
  // Set LED intensity. Valid values are 0 (off), 1, 2 and 3
  vSys_LedIntensity(3);
  lLEDOnTime = 20;
  // Initialize isr call counter
  bIsr_DebounceCount = 0;

  // BSR parameters initialization
  rBsrSetup.bButtonMask = bButtonMask_c;
  rBsrSetup.pbPtsReserveHead = abBsrPtsPlaceHolder;
  rBsrSetup.bPtsSize = 3;
  rBsrSetup.bPushQualThresh = 3;
  // Setup the BSR
  vBsr_Setup(&rBsrSetup);

  // Setup the RTC to tick every 5ms and clear it. Keep it disabled.
  RTC_CTRL = (0x07 << B_RTC_DIV) | M_RTC_CLR;
  // Enable the RTC
  RTC_CTRL |= M_RTC_ENA;
  // Enable the RTC interrupt and global interrupt enable
  ERTC = 1;
  EA = 1;

  // Setup the PA.
  rPaSetup.bLevel = b_PaLevel_c;
  rPaSetup.wNominalCap = b_PaNominalCap_c;
  rPaSetup.bMaxDrv = b_PaMaxDrv_c;
  rPaSetup.fAlpha = 0.0;
  rPaSetup.fBeta = 0.3462;
  vPa_Setup(&rPaSetup);

  rOdsSetup.bModulationType = bModOOK_c;
  // Setup the STL encoding for Manchester. No user encoding function therefore
  // the pointer is NULL.
  vStl_EncodeSetup(3, &bEnc_CustomEncode);
  fDesiredFreq = f_RkeFreqOOK_c;

  // Setup the ODS
  // Set group width to 7, which means 8 bits/encoded byte to be transmitted.
  // The value must match the output width of the data encoding function
  // set by vStl_EncodeSetup()!
  rOdsSetup.bGroupWidth = 7;
  rOdsSetup.bClkDiv = 5;
  rOdsSetup.bEdgeRate = 0;
  // Configure the warm up intervals
  rOdsSetup.bLcWarmInt = 8;
  rOdsSetup.bDivWarmInt = 5;
  rOdsSetup.bPaWarmInt = 4;
  // Should be 2000 but this yieled the closet signal to the original
  rOdsSetup.wBitRate = 1959;
  vOds_Setup(&rOdsSetup);

  // Setup frequency casting .. needed to be called once per boot
  vFCast_Setup();

  // Measure the battery voltage in mV, only once per boot to save power
  // Make loaded measurement .. bBatteryWait_c * 17ms wait after loading
  iBatteryMv = iMVdd_Measure(bBatteryWait_c);
  if (iBatteryMv < iLowBatMv_c) {
    bBatStatus = 0;
  } else {
    bBatStatus = 1;
  }

  pbFrameHead = abFrame;
  bFrameSize = bFrameSize_c;

  // Setup the DMD temperature sensor for temperature mode
  vDmdTs_RunForTemp(3); // Skip first 3 samples
  // Wait until there is a valid DMD temp sensor sample
  while (0 == bDmdTs_GetSamplesTaken());

  /*------------------------------------------------------------------------------
   *    TRANSMISSION LOOP PHASE
   *------------------------------------------------------------------------------
   */

  // Application loop, including push button state analyzation and transmission.
  while (1) {
    // Buttons analyzation
    vButtonCheck();
    if (bButtonState) {
      // Packet transmit repeat counter
      bRepeatCount = bRepeatCount_c;
      // Transmit.
      vRepeatTxLoop();
      // Sync counter increment
    } else if ((lSys_GetMasterTime() >> 5) > bMaxWaitForPush_c) {
      // if part is burned to user or run mode.
      if ((PROT0_CTRL & M_NVM_BLOWN) > 1) {
        // Disable all interrupts
        EA = 0;
        // Shutdown
        vSys_Shutdown();
      }
    }
  }
}

/*------------------------------------------------------------------------------
 *
 *    FUNCTION DESCRIPTION:
 *      This function contains the loop which consists of three procedures,
 *      tx setup, tx loop and tx shutdown in the application.
 *      During waiting for repeat transmission, check button state.
 *      Once any new push button is detected, then transmit the new packet
 *      instead of the current packet.
 *
 *------------------------------------------------------------------------------
 */
void vRepeatTxLoop(void) {
  vFCast_Tune(fDesiredFreq);

  // Wait until there is a temperature sample available
  while (0 == bDmdTs_GetSamplesTaken());
  //  Tune the PA with the temperature as an argument
  vPa_Tune(iDmdTs_GetLatestTemp());

  // Convert whole frame before transmission
  vStl_PreLoop();
  do {
    // get current timestamp
    lTimestamp = lSys_GetMasterTime();
    // if part is burned to user or run mode
    if ((PROT0_CTRL & M_NVM_BLOWN) > 1) {
      // turn LED on
      GPIO_LED = 1;
    }
    // wait for LED ON time to expire
    while ((lSys_GetMasterTime() - lTimestamp) < lLEDOnTime);
    GPIO_LED = 0; // turn LED off
    // Transmit packet
    vStl_SingleTxLoop(pbFrameHead, bFrameSize);
    // Wait repeat interval.
    while ((lSys_GetMasterTime() - lTimestamp) < wRepeatInterval_c);
  } while (--bRepeatCount);

  vStl_PostLoop();

  // Clear time value for next button push detecting.
  vSys_SetMasterTime(0);

  return;
}

void isr_rtc(void) interrupt INTERRUPT_RTC using 1 {
  // Update the master time by 5 every time this isr is run.
  // clear the RTC_INT
  RTC_CTRL &= ~M_RTC_INT;
  vSys_IncMasterTime(5);
  bIsr_DebounceCount++;
  if ((bIsr_DebounceCount % bDebounceInterval_c) == 0) {
    vBsr_Service();
  }

  return;
}

/*------------------------------------------------------------------------------
 * Update bAp_ButtonState which indicates what to be transmitted.
 * Check the elements on PTS (push tracking strcture) to see if any GPIO
 * has been pressed or released.
 * If any new pressed button has detected, the corresponding flag will be
 * set and the associated information will be transmitted in application loop
 * procedure.
 *----------------------------------------------------------------------------*/
void vButtonCheck(void) {
  // Disable RTC interrupt, or button state might be updated.
  ERTC = 0;
  bButtonState = 0; // comment this line out if autorepeat needed
  // Some buttons were pressed or released
  if (bBsr_GetPtsItemCnt()) {
    bButtonState = wBsr_Pop() & 0xFF;
    if (bPrevBtn) {
      bPrevBtn = bButtonState;
      bButtonState = 0;
    } else {
      bPrevBtn = bButtonState;
    }
  }
  // Enable RTC interrupt
  ERTC = 1;

  return;
}

BYTE bEnc_CustomEncode(BYTE xdata *pboEncodedBytes, BYTE biByteToEncode) {
  BYTE bitPosition, out;

  bitPosition = 8;
  do {
    bitPosition--;
    out = 0;

    if ((biByteToEncode >> bitPosition) & 1U) {
      out |= 0x07; // xxxx0111
    } else {
      out |= 0x01; // xxxx0001
    }
    bitPosition--;

    if ((biByteToEncode >> bitPosition) & 1U) {
      out |= 0x70; // 0111xxxx
    } else {
      out |= 0x10; // 0001xxxx
    }

    *pboEncodedBytes = out;
    pboEncodedBytes++;
  } while (bitPosition > 0);

  return 4;
}

void vIsr_Dmd(void) interrupt INTERRUPT_DMD using 2 {
  vDmdTs_ClearDmdIntFlag();
  vDmdTs_IsrCall();
}
