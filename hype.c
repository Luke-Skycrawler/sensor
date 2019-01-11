//linux header
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

//android header
#include <android/sensor.h>
#include <android/looper.h>
//#include

//networking
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "acceleration.h"

int sock = -1;
int bytes= 0;
float proximity = 0;
struct accelerator_data acc_data;


void die(char *err){
	printf("%s\n", err);
	exit(1);
}



void init_sock(char *ip ){
	sock = socket(AF_INET, SOCK_STREAM, 0 );
	struct sockaddr_in addr;
	if( sock == -1 )
		die("socket");

	memset( &addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons( 9000 );

	if( connect( sock, (struct sockaddr *)&addr, sizeof(addr) )<0){
		perror("");
		die("connect");
	}
	memset( &acc_data, 0, sizeof(acc_data) );
}


void displaySensorData(int sensor_type, ASensorEvent *data){
	switch( sensor_type ){
		case ASENSOR_TYPE_PROXIMITY:
//			printf("proximity.distance :%f\n", data->distance);//
			proximity = data->distance;
			send( sock, &proximity, sizeof(proximity), 0 );
			break;
		case ASENSOR_TYPE_ACCELEROMETER:
//			printf("acceleration : X=%f Y=%f Z=%f\n", data->acceleration.x, data->acceleration.y, data->acceleration.z );
			acc_data.x = data->acceleration.x;
			acc_data.y = data->acceleration.y;
			acc_data.z = data->acceleration.z;

			bytes = send( sock , &acc_data , sizeof(acc_data), 0 );
			if( bytes < sizeof(acc_data) || (bytes == -1) )
				exit(1);
	}
}

int main(int argv, char *argc[]){
	if( argv < 2 ){
		printf("./hype <ip address>\n");
		exit(1);
	}

	/*initialize socket*/
	init_sock( argc[1] );

	ASensorManager *sensor_manager = ASensorManager_getInstance();
	ASensorList sensor_list = NULL;
	int sensor_count = 0;
	int sensor_type  = 0;
	int count ;




	if( sensor_manager == NULL )
		die("error, sensor_manager");

	sensor_count = ASensorManager_getSensorList(sensor_manager, &sensor_list);
	for( count = 0 ; count < sensor_count ; count++){
		printf("*********************************\n");
		printf("sensor_name :%s\n", ASensor_getName(sensor_list[count]));
		printf("sensor_vendor :%s\n", ASensor_getVendor(sensor_list[count]));
		sensor_type = ASensor_getType(sensor_list[count]);
		switch (sensor_type){
			case ASENSOR_TYPE_ACCELEROMETER:
				printf("ASENSOR_TYPE_ACCELEROMETER\n");
				break;
			case ASENSOR_TYPE_MAGNETIC_FIELD:
				printf("ASENSOR_TYPE_MAGNETIC_FIELD\n");
				break;
			case ASENSOR_TYPE_GYROSCOPE:
				printf("ASENSOR_TYPE_GYROSCOPE\n");
				break;
			case ASENSOR_TYPE_LIGHT:
				printf("ASENSOR_TYPE_LIGHT\n");
				break;
			case ASENSOR_TYPE_PROXIMITY:
				printf("ASENSOR_TYPE_PROXIMITY\n");
				break;
			default:
				printf("ASENSOR_TYPE_UNKNOWN\n");
		}
	}
	
//////////////////////////

	int looperID = 1;

	//sensor queue
	ASensorEventQueue *sensor_queue = ASensorManager_createEventQueue(sensor_manager, ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS), looperID, NULL, NULL );

	if( !sensor_queue )
		die("sensor_queue");

	//sensor
	ASensor const *sensor_acc = ASensorManager_getDefaultSensor( sensor_manager, ASENSOR_TYPE_ACCELEROMETER );
	if( sensor_acc == NULL )
		die("sensor_acc");

	//enable
	if( ASensorEventQueue_enableSensor(sensor_queue, sensor_acc) < 0)
		die("sensor enable");
	printf("%s enabled\n", ASensor_getName(sensor_acc));

	//read data
	const int kNumEvents = 1;
	const int kTimeoutMilliSecs = 10000;

	int i ;

	for (;;) {
 		i = ASensorEventQueue_hasEvents(sensor_queue);
		if( i == 0 )
			continue;
		ASensorEvent data[kNumEvents];
		memset(data, 0, sizeof(data));


		int ident = ALooper_pollAll( kTimeoutMilliSecs, NULL, NULL, NULL );
		if (ident != looperID) {
			goto sec;
		}

		if (ASensorEventQueue_getEvents(sensor_queue, data, kNumEvents) <= 0) {
			goto sec;
		}

		displaySensorData(ASENSOR_TYPE_ACCELEROMETER, data );

		sec:
		sleep( 5 );
	}
	
	//disable
	ASensorEventQueue_disableSensor(sensor_queue, sensor_acc );
	printf("%s disabled\n", ASensor_getName(sensor_acc));

	//destroy
	ASensorManager_destroyEventQueue(sensor_manager, sensor_queue);
	return 0;
}




