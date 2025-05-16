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


#include <asio/aiso.h>
#include <asio/asiodrivers.h>
#include <combaseapi.h>

#include "soundio_internal.h"
#include "os.h"
#include "list.h"
#include "atomics.h"

// asios fundimental design is quite different to other vsts

struct SoundIoPrivate;
int soundio_asio_init(struct SoundIoPrivate *si);

// additional asio helper structs


struct SoundIoDeviceAsioDriver {
	int driver_id;
	int dll_path_size;
	char* dll_path;
	
	int driver_path_size;
	char* driver_name;
	void* asio_driver;
};


// the core soundio structs
struct SoundIoDeviceAsio {
	int driver_size;
	struct SoundIoDeviceAsioDriver* drivers;
};

// the actual data used for asio, since asio doesn't handle thread management this must be initialized
struct SoundIoAsio {
	
    struct SoundIoOsMutex *mutex;
    struct SoundIoOsCond *cond;	

    int sample_rate;
    int period_size;
    bool is_shutdown;
    bool emitted_shutdown_cb;
        
};


// input and output streams
struct SoundIoInStreamAsio {
	
};

struct SoundIoOutStreamAsio {

};

#endif
