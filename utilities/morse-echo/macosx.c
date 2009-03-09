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

void audio_init (int sample_rate)
{
	AudioStreamBasicDescription outinfo;
	OSStatus status;
	unsigned long size;
	unsigned int buflen;

	pthread_mutex_init (&buffer_mutex, 0);
	pthread_cond_init (&buffer_mailbox, 0);

	size = sizeof (outdev);
	status = AudioHardwareGetProperty (kAudioHardwarePropertyDefaultOutputDevice,
		&size, &outdev);
	if (status || outdev == kAudioDeviceUnknown) {
		fprintf (stderr, "audio: cannot get audio device\n");
		exit (-1);
	}

	/* get default output format */
	size = sizeof (outinfo);
	status = AudioDeviceGetProperty (outdev, 0, false,
		kAudioDevicePropertyStreamFormat, &size, &outinfo);
	if (status) {
		fprintf (stderr, "audio: cannot get audio stream format\n");
		exit (-1);
	}
	if (outinfo.mFormatID != kAudioFormatLinearPCM ||
	    ! (outinfo.mFormatFlags & kAudioFormatFlagIsFloat) ||
	    outinfo.mFramesPerPacket != 1 ||
	    outinfo.mSampleRate != sample_rate ||
	    outinfo.mBytesPerFrame / outinfo.mChannelsPerFrame != sizeof (float)) {
		fprintf (stderr, "audio: unsupported audio format\n");
		exit (-1);
	}

	/* get requested buffer length */
	size = sizeof (buflen);
	status = AudioDeviceGetProperty (outdev, 0, false,
		kAudioDevicePropertyBufferSize, &size, &buflen);
	if (status) {
		fprintf (stderr, "audio: cannot get audio buffer length\n");
		exit (-1);
	}

	audio_channels = outinfo.mChannelsPerFrame;
	buffer_size = buflen;

	/* Set the IO proc that CoreAudio will call when it needs data */
	status = AudioDeviceAddIOProc (outdev, audio_callback, 0);
	if (status) {
		fprintf (stderr, "audio: cannot add audio i/o procedure\n");
		exit (-1);
	}
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
