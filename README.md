# pcr
PCR Thermocycler

Code and other documentation for a really cheap thermocycler built from an Arduino (Ardweeny, actually), peltier junction, and a slice of an aluminum rod with holes drilled into it.   Includes in-tube and in-block thermosensors for calibration.

A python program establishes contact with the 'dumb' thermocycler and sends it temperature values according to the schedule in "settings.txt".  There is an opportunity for a better interactive program to produce cycle schedules and write the 'settings' file.  Once a few good schedules are worked out, they could be downloaded into the Arduino so that it would be a true stand-alone thermocycler with a button to choose one of a few PCR profiles.


