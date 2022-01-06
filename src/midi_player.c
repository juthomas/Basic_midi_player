#include <portmidi.h>
#include <stdio.h>
#include <stdlib.h>
#include "porttime.h"

PmTimestamp current_timestamp = 0;


static PmTimestamp portmidi_timeproc(void *time_info) {
  (void)time_info;
  return (current_timestamp);
}

void process_midi(PtTimestamp timestamp, void *userData)
{
	current_timestamp++;
}

int		main(void)
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
	Pm_OpenOutput(&stream, 2, NULL, 128, portmidi_timeproc, NULL, 0);


	//WIP
	while (current_timestamp < 60000)
	{
		last_time = last_time + 500;
		while (last_time > current_timestamp) ;
		printf("Tour de boucle\n");
		int type = 0x8, chan = 0, byte1 = 3, byte2 = 0xc;
		PmError pme = Pm_WriteShort(stream, current_timestamp,
                                Pm_Message(type << 4 | chan, byte1, byte2));
    (void)pme;
	}
	return (0);
}