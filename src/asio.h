/*
 * Copyright (c) 2015 Andrew Kelley
 *
 * This file is part of libsoundio, which is MIT licensed.
 * See http://opensource.org/licenses/MIT
 *
 * additional modification by ricky wang 
 */

#ifndef SOUNDIO_ASIO_H
#define SOUNDIO_ASIO_H

#include <windows.h>
#include <combaseapi.h>

#include "casio.h"

#include "soundio_internal.h"
#include "os.h"
#include "list.h"
#include "atomics.h"

#ifndef MAX_DRIVERS
	#define MAX_DRIVERS   32
#endif

#define MAX_KEY_NAME  512
#define MAX_DRIVER_NAME 128

// asios fundimental design is quite different to other backends


// NOTES:
/*

	asio decides input output data... 
	therefore input and output are considered the same data

	devices in asio are NOT the outputs from your audio, they are considered the driver in this instance

*/

struct SoundIoPrivate;
int soundio_asio_init(struct SoundIoPrivate *si);

// additional asio helper structs


struct SoundIoDeviceAsioDriver {	
	char dll_path[MAX_PATH];
	char driver_name[MAX_DRIVER_NAME];

	IASIO* asio_driver; // just check if null. the type is IASIO defined in asio.c but here is treated as a void* 
	HMODULE module;
};


// the core soundio structs
struct SoundIoDeviceAsio {
	int current_device;
	int size;
	struct SoundIoDeviceAsioDriver* drivers;
};

// the actual data used for asio, since asio doesn't handle thread management this must be initialized
struct SoundIoAsio {
    struct SoundIoDeviceAsioDriver* current_driver;
		
    struct SoundIoOsMutex *mutex;
    struct SoundIoOsCond *cond;
    
    int sample_rate;
    int period_size;
    bool is_shutdown;
    bool emitted_shutdown_cb;
};


// input and output streams
struct SoundIoInStreamAsio {
	struct SoundIoDeviceAsioDriver* current_driver;
};

struct SoundIoOutStreamAsio {
	struct SoundIoDeviceAsioDriver* current_driver;
	
};

#endif
