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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <asio/aiso.h>
#include <asio/asiodrivers.h>
#include <combaseapi.h>

#include "soundio_internal.h"
#include "os.h"
#include "list.h"
#include "atomics.h"

// asios fundimental design is quite different to other vsts


#define DRVERR			-5000
#define DRVERR_INVALID_PARAM		DRVERR-1
#define DRVERR_DEVICE_ALREADY_OPEN	DRVERR-2
#define DRVERR_DEVICE_NOT_FOUND		DRVERR-3

#define MAX_PATH_LEN			512
#define MAX_DRIVERNAME_LEN		128

struct SoundIoPrivate;
int soundio_asio_init(struct SoundIoPrivate *si);


// shit is fucked


// to translate to other meanings
// each asio driver here is considered a "port" 
struct SoundIoDeviceAsioPort {
	char* driver_name;
	int driver_name_len;

	char* driver_path;
	int driver_path_len;

	int driver_id;
	CLSID class_id;

	void* asio_driver;
};


struct SoundIoDeviceAsio {
	int device_count;
	struct SoundIoDeviceAsioPort* driver;
};

struct SoundIoAsio {
    struct SoundIoOsMutex *mutex;
    struct SoundIoOsCond *cond;
	struct SoundIoOsThread *thread;
	bool abort;
    
	double sample_rate;

	unsigned int max_input_channels;
	unsigned int max_output_channels;

	bool is_shutdown;
	int shutdown_error;
	bool emitted_shutdown_cb;

	
	
};

struct SoundIoOutStreamAsio {
	long input_channels;
	long input_latency;
	long input_buffer;
	ASIOBufferInfo buffer_info[];
	
};

struct SoundIoInStreamAsio {
	
};


#endif
