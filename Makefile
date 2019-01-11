all:
	arm-linux-androideabi-gcc hype.c -o hype -Wall libandroid.so
send:
	adb push hype /data/local/tmp/
