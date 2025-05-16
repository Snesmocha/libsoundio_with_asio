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

struct SoundIoPrivate;
int soundio_asio_init(struct SoundIoPrivate *si);

struct SoundIoAsio {

};

struct SoundIoDeviceAsio {

};

struct SoundIoInStreamAsio {

};

struct SoundIoOutStreamAsio {

};

#endif
