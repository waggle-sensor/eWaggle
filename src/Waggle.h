#ifndef __H_WAGGLE__
#define __H_WAGGLE__

#include "waggle/waggle2.h"

#if defined(ARDUINO_ARCH_AVR)
#include "waggle/waggle_arduino.h"
#elif defined(ARDUINO_ARCH_SAM)
#include "waggle/waggle_arduino.h"
#else
#include "waggle/waggle_native.h"
#endif

#endif
