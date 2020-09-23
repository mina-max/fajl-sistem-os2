#pragma once

#include <windows.h>

#define signal(x) ReleaseSemaphore(x, 1, NULL);
#define wait(x) WaitForSingleObject(x, INFINITE);
#define signalAndWait(x, y) SignalObjectAndWait(x, y, INFINITE, false);

