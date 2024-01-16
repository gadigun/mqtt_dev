#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mosquitto.h>

#define MQTT_HOSTNAME "mqtt.serviceport.co.kr"//"localhost"
#define MQTT_PORT 1883

#define MQTT_TOPIC "myTopic"

int main(int argc, char *argv[])
{
	int rc;
	struct mosquitto_message *mosq;

	// 초기화
	mosquitto_lib_init();

	// 모스키토 런타임 객체와 클라이언트 랜덤 ID 생성
	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL)
	{
		printf("Cant initiallize mosquitto library\n");
		exit(-1);
	}

	//mosquitto_username_pw_set(mosq, MQTT_USERNAME, MQTT_PASSWORD);

	// MQTT 서버 연결 설립, keep-alive 메시지 사용 안함
	int ret = mosquitto_connect(mosq, MQTT_HOSTNAME, MQTT_PORT, 0);
   if (ret) {
      printf("Cant connect to mosquitto server\n");
      exit(-1);
   }

   char text[20] = "Nice to meet u!\n";

   ret = mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(text), text, 0, false);
   if (ret) {
      printf("Cant connect to mosquitto server\n");
      exit(-1);
   }

   // 네트워크 동작이 끝나기 전에 모스키토 동작을 막기위해 잠깐의 딜레이가 필요함
   //sleep(1);

   mosquitto_disconnect(mosq);
   mosquitto_destroy(mosq);
   mosquitto_lib_cleanup();

   return 0;
	/*
	rc = mosquitto_subscribe_simple(
			&msg, 1, true,
			"irc/#", 0,
			"test.mosquitto.org", 1883,
			NULL, 60, true,
			NULL, NULL,
			NULL, NULL);

	if(rc){
		printf("Error: %s\n", mosquitto_strerror(rc));
		mosquitto_lib_cleanup();
		return rc;
	}

	printf("%s %s\n", msg->topic, (char *)msg->payload);
	mosquitto_message_free(&msg);
	mosquitto_lib_cleanup();

	return 0;
	*/
}
