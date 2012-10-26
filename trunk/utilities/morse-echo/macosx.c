/*
 * Copyright (C) 2005 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can redistribute this file and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software Foundation;
 * either version 2 of the License, or (at your discretion) any later version.
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <CoreAudio/AudioHardware.h>
#include "audio.h"

static int audio_channels;
static int buffer_size;
static int device_started;
static AudioDeviceID outdev;
static AudioDeviceIOProcID audio_callback_id;

/*
 * Ring buffer
 */
#define MAXSAMPLES 10000

static float buffer [MAXSAMPLES];
static unsigned int buf_read_pos;
static unsigned int buf_write_pos;

static pthread_mutex_t buffer_mutex; /* mutex covering buffer variables */
static pthread_cond_t buffer_mailbox;

/*
 * The function that the CoreAudio thread calls when it wants more data
 */
static OSStatus audio_callback (AudioDeviceID inDevice, const AudioTimeStamp *inNow,
	const AudioBufferList *inInputData, const AudioTimeStamp *inInputTime,
	AudioBufferList *outOutputData, const AudioTimeStamp *inOutputTime,
	void *inClientData)
{
	char *buf, *limit;
	float sample;

	/* Fill buffer with audio data. */
	buf = (char*) outOutputData->mBuffers[0].mData;
	limit = buf + buffer_size;

	/* accessing common variables, locking mutex */
	pthread_mutex_lock (&buffer_mutex);
	for (;;) {
		if (buf + audio_channels * sizeof (float) > limit)
			break;

		if (buf_read_pos == buf_write_pos) {
			/* buffer empty */
			sample = 0;
		} else {
			sample = buffer [buf_read_pos];

			++buf_read_pos;
			if (buf_read_pos >= MAXSAMPLES)
				buf_read_pos = 0;
		}

		*(float*) buf = sample;
		buf += sizeof (float);
		if (audio_channels > 1) {
			*(float*) buf = sample;
			buf += sizeof (float);
		}
	}
	pthread_mutex_unlock (&buffer_mutex);
	pthread_cond_signal (&buffer_mailbox);

	/* Return the number of prepared bytes. */
	outOutputData->mBuffers[0].mDataByteSize = buf -
		(char*) outOutputData->mBuffers[0].mData;
	return 0;
}

/*
 * Return sample rate.
 */
int audio_init ()
{
	AudioStreamBasicDescription outinfo;
	OSStatus status;
	unsigned size;
	unsigned int buflen;
        AudioObjectPropertyAddress property_address = {
                0,                                  // mSelector
                kAudioObjectPropertyScopeGlobal,    // mScope
                kAudioObjectPropertyElementMaster   // mElement
        };

	pthread_mutex_init (&buffer_mutex, 0);
	pthread_cond_init (&buffer_mailbox, 0);

	/* Get default output device */
        property_address.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
        outdev = kAudioObjectUnknown;
        size = sizeof(outdev);
        status = AudioObjectGetPropertyData (kAudioObjectSystemObject,
                &property_address, 0, 0, &size, &outdev);
	if (status || outdev == kAudioObjectUnknown) {
		fprintf (stderr, "audio: cannot get audio device\n");
		exit (-1);
	}

	/* Get default output format */
        property_address.mSelector = kAudioDevicePropertyStreamFormat;
	size = sizeof (outinfo);
        status = AudioObjectGetPropertyData (outdev, &property_address,
            0, 0, &size, &outinfo);
	if (status) {
		fprintf (stderr, "audio: cannot get audio stream format\n");
		exit (-1);
	}
	if (outinfo.mFormatID != kAudioFormatLinearPCM) {
		fprintf (stderr, "audio: unsupported audio format = %u\n",
                        outinfo.mFormatID);
		exit (-1);
	}
	if (! (outinfo.mFormatFlags & kAudioFormatFlagIsFloat)) {
		fprintf (stderr, "audio: no support for float audio format, flags = %#x\n",
                        outinfo.mFormatFlags);
		exit (-1);
	}
	if (outinfo.mFramesPerPacket != 1) {
		fprintf (stderr, "audio: too many frames per packet = %u\n",
                        outinfo.mFramesPerPacket);
		exit (-1);
	}
	if (outinfo.mBytesPerFrame / outinfo.mChannelsPerFrame != sizeof (float)) {
		fprintf (stderr, "audio: bad sample size = %u, expected %lu\n",
                        outinfo.mBytesPerFrame / outinfo.mChannelsPerFrame, sizeof (float));
		exit (-1);
	}

	/* get requested buffer length */
        property_address.mSelector = kAudioDevicePropertyBufferSize;
	size = sizeof (buflen);
        status = AudioObjectGetPropertyData (outdev, &property_address,
            0, 0, &size, &buflen);
	if (status) {
		fprintf (stderr, "audio: cannot get audio buffer length\n");
		exit (-1);
	}

	audio_channels = outinfo.mChannelsPerFrame;
	buffer_size = buflen;

	/* Set the IO proc that CoreAudio will call when it needs data */
        audio_callback_id = 0;
        status = AudioDeviceCreateIOProcID (outdev, audio_callback,
            0, &audio_callback_id);
	if (status) {
		fprintf (stderr, "audio: cannot add audio i/o procedure\n");
		exit (-1);
	}

	return (unsigned) outinfo.mSampleRate;
}

static void audio_start ()
{
	OSStatus status;

	/* Start callback */
	status = AudioDeviceStart (outdev, audio_callback);
	if (status) {
		fprintf (stderr, "audio: cannot start audio device\n");
		exit (-1);
	}
	device_started = 1;
}

void audio_flush ()
{
	if (! device_started)
		audio_start ();

	pthread_mutex_lock (&buffer_mutex);
	while (buf_write_pos != buf_read_pos) {
		pthread_mutex_unlock (&buffer_mutex);
		pthread_cond_wait (&buffer_mailbox, &buffer_mutex);
	}
	pthread_mutex_unlock (&buffer_mutex);
}

void audio_output (float sample)
{
	int next_write_pos;

	pthread_mutex_lock (&buffer_mutex);
again:
	next_write_pos = buf_write_pos + 1;
	if (next_write_pos >= MAXSAMPLES)
		next_write_pos = 0;

	if (next_write_pos == buf_read_pos) {
		/* buffer is full */
		pthread_mutex_unlock (&buffer_mutex);
		if (! device_started)
			audio_start ();
		pthread_cond_wait (&buffer_mailbox, &buffer_mutex);
		goto again;
	}

	buffer [buf_write_pos] = sample;
	buf_write_pos = next_write_pos;

	pthread_mutex_unlock (&buffer_mutex);
}
