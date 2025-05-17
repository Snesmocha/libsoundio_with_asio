#include "asio.h"
// I DO NOT CARE THE FACT I AM USING CHAT GPT TO HELP ME READ THE DOCUMENTATION, FUCK ASIO FUCK ASIO FUCK ASIO FUCK ASIO

#include "soundio_private.h"




#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define COBJMACROS
#include <windows.h>
#include <objbase.h>
#include <stdio.h>

#include <string.h>
#include <stdint.h>

#include "casio.h"



#define LOWER_UPPERCASE_LETTERS(str)                       \
    do {                                                   \
        for (int i = 0; (str)[i] != '\0'; i++) {           \
            if ((str)[i] >= 'A' && (str)[i] <= 'Z')        \
                (str)[i] += 33;                            \
        }                                                  \
    } while (0)


#define ASIODRV_DESC		"description"
#define INPROC_SERVER		"InprocServer32"
#define ASIO_PATH			"software\\asio"
#define COM_CLSID			"clsid"




// unused
#if 0
int find_driver_path(char* clsid_str, char* dll_path, int dll_path_size)
{
	HKEY hkEnum,hksub,hkpath;
	char databuf[512];
	LONG cr,rc = -1;
	DWORD datatype,datasize;
	DWORD index;
	OFSTRUCT ofs;
	HFILE hfile;

	LOWER_UPPERCASE_LETTERS(clsid_str);
	
	if ((cr = RegOpenKey(HKEY_CLASSES_ROOT,COM_CLSID,&hkEnum)) != ERROR_SUCCESS) 
		return -1;
	index = 0;

	int loop = 1;
	while(loop)
	{
		cr = RegEnumKey(hkEnum,index++,(LPTSTR)databuf,512);
		if (cr != ERROR_SUCCESS) continue;
		
		LOWER_UPPERCASE_LETTERS(databuf);
		
		if (!strcmp(databuf, clsid_str)) loop = 0;
		else continue;
		
		if ((cr = RegOpenKeyEx(hkEnum,(LPCTSTR)databuf,0,KEY_READ,&hksub)) != ERROR_SUCCESS )
		{
			RegCloseKey(hksub);
			break;
		}

		if ((cr = RegOpenKeyEx(hksub,(LPCTSTR)INPROC_SERVER,0,KEY_READ,&hkpath)) != ERROR_SUCCESS) 
		{
			RegCloseKey(hksub);
			RegCloseKey(hkpath);
			break;	
		}
		datatype = REG_SZ; datasize = (DWORD)dll_path_size;
		cr = RegQueryValueEx(hkpath,0,0,&datatype,(LPBYTE)dll_path,&datasize);
		if (cr == ERROR_SUCCESS) 
		{
			memset(&ofs,0,sizeof(OFSTRUCT));
			ofs.cBytes = sizeof(OFSTRUCT); 
			hfile = OpenFile(dll_path,&ofs,OF_EXIST);
			if (hfile) rc = 0; 
		}
		RegCloseKey(hkpath);
		RegCloseKey(hksub);
		
		
	}
	
	RegCloseKey(hkEnum);


	return rc;
}
#endif 


// probably pretty slow tbh... 
int find_all_driver_paths(char dll_paths[MAX_DRIVERS][MAX_PATH], int max_paths)
{
    HKEY hkEnum, hksub, hkpath, hkAsio, hkDriver;
    char databuf[512], clsid_match[512], dll_path[MAX_PATH];
    LONG cr;
    DWORD datatype, datasize, index = 0, found = 0;

    if ((cr = RegOpenKey(HKEY_LOCAL_MACHINE, ASIO_PATH, &hkAsio)) != ERROR_SUCCESS)
        return -1;

    while (found < (DWORD)max_paths) {
        DWORD name_len = sizeof(databuf);
        cr = RegEnumKeyA(hkAsio, index++, databuf, name_len);
        if (cr != ERROR_SUCCESS) break;

        if (RegOpenKeyExA(hkAsio, databuf, 0, KEY_READ, &hkDriver) != ERROR_SUCCESS)
            continue;

        datasize = sizeof(clsid_match);
        datatype = REG_SZ;
        if (RegQueryValueExA(hkDriver, "CLSID", 0, &datatype, (LPBYTE)clsid_match, &datasize) != ERROR_SUCCESS) {
            RegCloseKey(hkDriver);
            continue;
        }

        LOWER_UPPERCASE_LETTERS(clsid_match);

        if ((cr = RegOpenKey(HKEY_CLASSES_ROOT, "CLSID", &hkEnum)) != ERROR_SUCCESS) {
            RegCloseKey(hkDriver);
            break;
        }

        DWORD clsid_index = 0;
        while (1) {
            cr = RegEnumKeyA(hkEnum, clsid_index++, databuf, sizeof(databuf));
            if (cr != ERROR_SUCCESS) break;

            LOWER_UPPERCASE_LETTERS(databuf);
            if (strcmp(databuf, clsid_match) != 0)
                continue;

            if ((cr = RegOpenKeyExA(hkEnum, databuf, 0, KEY_READ, &hksub)) != ERROR_SUCCESS)
                break;

            if ((cr = RegOpenKeyExA(hksub, "InprocServer32", 0, KEY_READ, &hkpath)) != ERROR_SUCCESS) 
            {
                RegCloseKey(hksub);
                break;
            }

            datasize = sizeof(dll_path);
            datatype = REG_SZ;
            cr = RegQueryValueExA(hkpath, 0, 0, &datatype, (LPBYTE)dll_path, &datasize);
            if (cr == ERROR_SUCCESS) 
            {
                OFSTRUCT ofs = { 0 };
                ofs.cBytes = sizeof(OFSTRUCT);
                HFILE hfile = OpenFile(dll_path, &ofs, OF_EXIST);
                if (hfile) strncpy_s(dll_paths[found++], MAX_PATH, dll_path, MAX_PATH);
                
            }

            RegCloseKey(hkpath);
            RegCloseKey(hksub);
            break;
        }

        RegCloseKey(hkEnum);
        RegCloseKey(hkDriver);
    }

    RegCloseKey(hkAsio);
    return found;
}

int get_asio_drivers(struct SoundIoDeviceAsioDriver* driver_array)
{
	struct SoundIoDeviceAsioDriver drivers[MAX_DRIVERS] = {};
	char paths[MAX_DRIVERS][260];
	int count = find_all_driver_paths(paths, MAX_DRIVERS);
	if (count > MAX_DRIVERS) return -1;
	for(int i = 0; i < count; i++)
		strncpy_s(drivers[i].dll_path, MAX_KEY_NAME, paths[i], MAX_PATH);
	
	driver_array = drivers;
	
	return count;
}



int load_asio_driver(struct SoundIoDeviceAsioDriver* driver, int index)
{
	if (index < 0 || index > MAX_DRIVERS) return -1;
	driver[index].module = LoadLibrary(driver[index].dll_path);
	if (!driver[index].module) return -1;
	
	ASIOCreateInstanceFn create_fn = (ASIOCreateInstanceFn)GetProcAddress(driver[index].module, "CreateASIO"); // shut the fuck up, no one cares you cunt - pink guy
	if (!create_fn) return -1;

	char* name;
	driver[index].asio_driver = create_fn();
	driver[index].asio_driver->lpVtbl->getDriverName(driver[index].asio_driver, name);
	strncpy_s(driver[index].driver_name, MAX_DRIVER_NAME, name, MAX_DRIVER_NAME);
	
	return 0;
}

int set_active_asio_driver(struct SoundIoDeviceAsio* driver_arr, int index)
{
	driver_arr->current_device = index;
	int error;
	if(!driver_arr->drivers[driver_arr->current_device].asio_driver)
		error = load_asio_driver(driver_arr->drivers, driver_arr->current_device);
	return error;
}

void destroy_asio_driver(struct SoundIoDeviceAsioDriver* driver, int index)
{
	if (driver[index].module != NULL) FreeLibrary(driver[index].module);
}

// callbacks


void soundio_asio_destroy(struct SoundIoPrivate *dp) 
{
	
}

void soundio_asio_flush_events(struct SoundIoPrivate *siop) {
    // TODO: Implement ASIO event flushing
}

void soundio_asio_wait_events(struct SoundIoPrivate *siop) {
    // TODO: Wait for ASIO events
}

void soundio_asio_wakeup(struct SoundIoPrivate *siop) {
    // TODO: Wake up ASIO thread
}

void soundio_asio_force_device_scan(struct SoundIoPrivate *siop) {
    
}
// im still trying to figure out where asio is initialized
int soundio_asio_outstream_open(struct SoundIoPrivate *si, struct SoundIoOutStreamPrivate *os) {
    // TODO: Start ASIO output stream
	struct SoundIoAsio 		  *sid = &si->backend_data.asio;
    struct SoundIoOutStreamAsio  *osd = &os->backend_data.asio;
    struct SoundIoOutStream 	 *outstream = &os->pub;
    struct SoundIoDevice 		*device = outstream->device;
    struct SoundIoDevicePrivate  *dev = (struct SoundIoDevicePrivate *)device;
    struct SoundIoDeviceAsio 	*dd = &dev->backend_data.asio;

    if (sid->is_shutdown) return SoundIoErrorBackendDisconnected;
    if (!outstream->name) outstream->name = "SoundIoOutStream";
    
    outstream->software_latency = device->software_latency_current;
    osj->period_size = sij->period_size;

    if (dj->drivers != NULL)
    {
		
    }

    return 0;
}

void soundio_asio_outstream_destroy(struct SoundIoPrivate *siop, struct SoundIoOutStreamPrivate *os) {
    // TODO: Destroy output stream resources
}

int soundio_asio_outstream_start(struct SoundIoPrivate *siop, struct SoundIoOutStreamPrivate *os) {

    
    return 0;
}

int soundio_asio_outstream_begin_write(struct SoundIoPrivate *siop, struct SoundIoOutStreamPrivate *os,
                                       struct SoundIoChannelArea **out_areas, int *out_frame_count) {
    // TODO: Prepare output buffer for writing
    return 0;
}


int soundio_asio_outstream_end_write(struct SoundIoPrivate *siop, struct SoundIoOutStreamPrivate *os) {
    // TODO: Commit written audio frames
    return 0;
}

int soundio_asio_outstream_clear_buffer(struct SoundIoPrivate *siop, struct SoundIoOutStreamPrivate *os) {
    // TODO: Clear playback buffer
    return 0;
}

int soundio_asio_outstream_pause(struct SoundIoPrivate *siop, struct SoundIoOutStreamPrivate *os, bool pause) {
    // TODO: Pause or resume output stream
    return 0;
}

int soundio_asio_outstream_get_latency(struct SoundIoPrivate *siop, struct SoundIoOutStreamPrivate *os, double *out_latency) {
    // TODO: Query ASIO output latency
    *out_latency = 0.0;
    return 0;
}

int soundio_asio_outstream_set_volume(struct SoundIoPrivate *siop, struct SoundIoOutStreamPrivate *os, float volume) {
    // Optional: If ASIO supports volume control
    return 0;
}

int soundio_asio_instream_open(struct SoundIoPrivate *siop, struct SoundIoInStreamPrivate *is) {
    // TODO: Open ASIO input stream
    return 0;
}

void soundio_asio_instream_destroy(struct SoundIoPrivate *siop, struct SoundIoInStreamPrivate *is) {
    // TODO: Destroy input stream resources
}

int soundio_asio_instream_start(struct SoundIoPrivate *siop, struct SoundIoInStreamPrivate *is) {
    // TODO: Start ASIO input stream
    return 0;
}

int soundio_asio_instream_begin_read(struct SoundIoPrivate *siop, struct SoundIoInStreamPrivate *is,
                                     struct SoundIoChannelArea **out_areas, int *out_frame_count) {
    // TODO: Begin read from input buffer
    return 0;
}

int soundio_asio_instream_end_read(struct SoundIoPrivate *siop, struct SoundIoInStreamPrivate *is) {
    // TODO: Finalize read
    return 0;
}

int soundio_asio_instream_pause(struct SoundIoPrivate *siop, struct SoundIoInStreamPrivate *is, bool pause) {
    // TODO: Pause or resume input stream
    return 0;
}

int soundio_asio_instream_get_latency(struct SoundIoPrivate *siop, struct SoundIoInStreamPrivate *is, double *out_latency) {
    // TODO: Get input latency
    *out_latency = 0.0;
    return 0;
}




int soundio_asio_init(struct SoundIoPrivate *si)
{
	struct SoundIoAsio *sij = &si->backend_data.asio;
	struct SoundIo *soundio = &si->pub;
    

    if (!SOUNDIO_ATOMIC_FLAG_TEST_AND_SET(global_msg_callback_flag)) 
    {
    //    if (soundio->asio_error_callback)
    //        jack_set_error_function(soundio->asio_error_callback);
    //    if (soundio->jack_info_callback)
    //        jack_set_info_function(soundio->asio_info_callback);
    //    SOUNDIO_ATOMIC_FLAG_CLEAR(global_msg_callback_flag);
    }

    sij->mutex = soundio_os_mutex_create();
    if (!sij->mutex) {
        //destroy_jack(si);
        return SoundIoErrorNoMem;
    }

    sij->cond = soundio_os_cond_create();
    if (!sij->cond) {
        //destroy_jack(si);
        return SoundIoErrorNoMem;
	}
	


}

