#include <portmidi.h>
#include <stdio.h>
#include <stdlib.h>
#include <porttime.h>

PmTimestamp current_timestamp = 0;

static PmTimestamp portmidi_timeproc(void *time_info)
{
	(void)time_info;
	return (current_timestamp);
}

void process_midi(PtTimestamp timestamp, void *userData)
{
	current_timestamp++;
}

void wait_ms()
{
	PmTimestamp last_time = current_timestamp;
	while (current_timestamp < last_time + 10)
		;
}

void test_single_note(PortMidiStream *stream)
{
	PmTimestamp last_time = current_timestamp;
	PmTimestamp last_timestamp = current_timestamp;
	uint8_t is_note_alive = TRUE;

	while (current_timestamp < last_timestamp + 60000)
	{
		last_time = last_time + 500;
		while (last_time > current_timestamp)
			;
		if (is_note_alive)
		{
			PmError pme = Pm_WriteShort(stream, current_timestamp,
										// Pm_Message(type << 4 | chan, byte1, byte2)
										Pm_Message(0x9 << 4, 70, 100)); // status << 4 | channel, note, velocity;
			(void)pme;
		}
		else
		{
			PmError pme = Pm_WriteShort(stream, current_timestamp,
										// Pm_Message(type << 4 | chan, byte1, byte2)
										Pm_Message(0x8 << 4, 70, 0)); // status << 4 | channel, note, velocity;
			(void)pme;
		}
		is_note_alive = !is_note_alive;
	}
	wait_ms();
	PmError pme = Pm_WriteShort(stream, current_timestamp,
								// Pm_Message(type << 4 | chan, byte1, byte2)
								Pm_Message(0x8 << 4, 70, 0)); // status << 4 | channel, note, velocity;
	(void)pme;
}

void test_all_notes(PortMidiStream *stream)
{
	PmTimestamp last_time = current_timestamp;
	PmTimestamp last_timestamp = current_timestamp;
	uint8_t current_note = 0;

	while (current_timestamp < last_timestamp + 60000)
	{
		last_time = last_time + 300;
		while (last_time > current_timestamp)
			;
		printf("current_note : 0x%02X (%03d)\n", current_note, current_note);
		PmError pme = Pm_WriteShort(stream, current_timestamp,
									// Pm_Message(type << 4 | chan, byte1, byte2)
									Pm_Message(0x9 << 4, current_note, 100)); // status | channel, note, velocity;
		(void)pme;
		if (current_note > 0)
		{
			//Wait ms make sure that the last signal is correctly received by the Midi output device
			wait_ms();
			PmError pme = Pm_WriteShort(stream, current_timestamp,
										// Pm_Message(type << 4 | chan, byte1, byte2)
										Pm_Message(0x8 << 4, current_note - 1, 0));
			(void)pme;
		}
		current_note = current_note < 0xFF ? current_note + 1 : 0;
	}
}

int main(void)
{
	PmTimestamp last_time = 0;
	PortMidiStream *stream;
	Pt_Start(1, &process_midi, 0);
	int midi_count = Pm_CountDevices();
	printf("Device number : %d\n", midi_count);
	for (int i = 0; i < midi_count; i++)
	{
		PmDeviceInfo const *device_info = Pm_GetDeviceInfo(i);
		if (device_info && device_info->output)
			printf("id = %d, name : %s\n", i, device_info->name);
	}
	//Stream ptr, device id, *** ,buffer size, current time, ***, latency
	Pm_OpenOutput(&stream, 2, NULL, 128, portmidi_timeproc, NULL, 0);

	test_single_note(stream);
	test_all_notes(stream);
	Pm_Close(&stream);
	Pm_Terminate();
	return (0);
}