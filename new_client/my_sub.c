#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

#include "mosquitto.h"

// 토큰 종류 열거형
typedef enum _TOKEN_TYPE
{
    TOKEN_STRING,       // 문자열 토큰
    TOKEN_NUMBER,       // 숫자 토큰
} TOKEN_TYPE;

// 토큰 구조체
typedef struct _TOKEN
{
    TOKEN_TYPE type;        // 토큰 종류
    union {
		char *string;       // 문자열 포인터
		double number;      // 실수형 숫자
	};
    bool isArray;           // 배열인지 아닌지
} TOKEN;

#define TOKEN_COUNT 20      // ??

// JSON 구조체
typedef struct _JSON
{
    TOKEN tokens[TOKEN_COUNT];  // 토큰 배열

    int token_count;
} JSON;

// file read
char* readFile(char* filename, int* readSize)
{
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL)
        return NULL;

    int size;
    char* buffer;

    // 파일 크기 구하기
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 파일 크기 + NULL 공간만큼 메모리 할당하고 0으로 초기화
    buffer = malloc(size + 1);
    memset(buffer, 0, size + 1);

    // 파일 읽기
    if (fread(buffer, size, 1, fp) < 1)
    {
        *readSize = 0;
        free(buffer);
        fclose(fp);
        return NULL;
    }

    *readSize = size;
    fclose(fp);
    return buffer;
}

int parseJSON(char* doc, int size, JSON* json)
{
    int tokenIndex = 0;
    int pos = 0;

    if (doc[pos] != '{') // 시작 문자 체크
		return -1;

    ++pos;

    while (pos < size)
    {
        switch (doc[pos])
        {
            case '"':
            {
                // 문자열의 시작 위치
                char* begin = doc + pos + 1;
                // 문자열 끝위치
                char* end = strchr(begin, '"');
                if (end == NULL) // 잘못된 문법
                {
                    return -1;
                }
                int stringLength = end - begin;

                json->tokens[tokenIndex].type = TOKEN_STRING;
                json->tokens[tokenIndex].string = malloc(stringLength + 1);
                memset(json->tokens[tokenIndex].string, 0, stringLength + 1);
                memcpy(json->tokens[tokenIndex].string, begin, stringLength);
                json->tokens[tokenIndex].string[stringLength] = '\0';

                ++tokenIndex;
                pos = pos + stringLength + 1;
            }
            break;

            /*
            case ':':
            {
                // 문자열의 시작 위치
                char* begin = doc + pos + 1;
                // 문자열 끝위치
                char* end = strchr(begin, ',');
                if (end == NULL) // 잘못된 문법
                    break;

                //if ()
            }
            break;
            */
        }

        ++pos;
    }

    json->token_count = tokenIndex;
    return tokenIndex;
}

void freeJSON(JSON* json)
{
    for (int i = 0; i < TOKEN_COUNT; ++i)
    {
        if (json->tokens[i].type == TOKEN_STRING)
            free(json->tokens[i].string);
    }
}

//#define mqtt_host "localhost"
//#define mqtt_port 1883

#define MQTT_HOSTNAME "mqtt.serviceport.co.kr"//"localhost"
#define MQTT_PORT 1883

#define MQTT_TOPIC "oksystem/sw"


static int run = 1;

void handle_signal(int s)
{
   run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int reason_code)
{
   printf("connect callback, rc=%d\n", reason_code);

   int rc;
   /* Print out the connection result. mosquitto_connack_string() produces an
    * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
    * clients is mosquitto_reason_string().
    */
   printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
   if (reason_code != 0) {
       /* If the connection fails for any reason, we don't want to keep on
        * retrying in this example, so disconnect. Without this, the client
        * will attempt to reconnect. */
       mosquitto_disconnect(mosq);
   }

   rc = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);//subscribe
   if (rc != MOSQ_ERR_SUCCESS)
   {
       fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
       // We might as well disconnect if we were unable to subscribe
       mosquitto_disconnect(mosq);
   }
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    bool match = 0;
// printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);
    printf("receive message(%s) : %s\n",message->topic, message->payload);
   
    // json parse
    JSON json = { 0, };

    if (parseJSON(message->payload, message->payloadlen, &json) < 0)
    {
        freeJSON(&json);
		printf("json parse error\n");
		return;
    }

    // json token이 홀수이면 에러
    if (json.token_count % 2 != 0)
    {
        freeJSON(&json);
        printf("json parse error\n");
        return;
    }

    int buffer_size = message->payloadlen + 10;
    char *buffer = malloc(buffer_size);
    memset(buffer, 0, buffer_size);

    char json_buffer[64];
    memset(json_buffer, 0, 64);

    strcat(buffer, "{");

    int cur_buffer_index = 1;

    for (int i = 0; i < json.token_count; ++i)
    {
        if (json.tokens[i].type == TOKEN_STRING)
        {
            memset(json_buffer, 0, 64);

            //printf("json token : %s : %s\n", json.tokens[i].string, json.tokens[i + 1].string);
            if (strcmp(json.tokens[i].string, "T") == 0)
            {
                sprintf(json_buffer, "\"%s\":\"%s\"", json.tokens[i].string, strcmp(json.tokens[i + 1].string, "0") == 0 ? "1" : "0");
            }
            else if (strcmp(json.tokens[i].string, "SV") == 0)
            {
                sprintf(json_buffer, "\"%s\":\"%s\"", json.tokens[i].string, strcmp(json.tokens[i + 1].string, "0") == 0 ? "1" : "0");
            }
            else
            {
                sprintf(json_buffer, "\"%s\":\"%s\"", json.tokens[i].string, json.tokens[i + 1].string);
            }
            ++i;

            strcat(buffer, json_buffer);
        }

        if (i < json.token_count - 1)
        {
            strcat(buffer, ",");
            ++cur_buffer_index;
        }
    }
    strcat(buffer, "}\n");

    printf("send data : %s", buffer);
   
    mosquitto_publish(mosq, NULL, message->topic, strlen(buffer), buffer, 0, false);

    freeJSON(&json);
    free(buffer);

    /*
    mosquitto_topic_matches_sub("/devices/wb-adc/controls/+", message->topic, &match);  
    if (match)
    {
        printf("got message for ADC topic\n");
    }
    */
}

/* Callback called when the broker sends a SUBACK in response to a SUBSCRIBE. */
void on_subscribe(struct mosquitto* mosq, void* obj, int mid, int qos_count, const int* granted_qos)
{
    int i;
    bool have_subscription = false;

    /* In this example we only subscribe to a single topic at once, but a
     * SUBSCRIBE can contain many topics at once, so this is one way to check
     * them all. */
    for (i = 0; i < qos_count; i++) {
        printf("on_subscribe: %d:granted qos = %d\n", i, granted_qos[i]);
        if (granted_qos[i] <= 2) {
            have_subscription = true;
        }
    }
    if (have_subscription == false) {
        /* The broker rejected all of our subscriptions, we know we only sent
         * the one SUBSCRIBE, so there is no point remaining connected. */
        fprintf(stderr, "Error: All subscriptions rejected.\n");
        mosquitto_disconnect(mosq);
    }
}


// read text file
// server addr
// servcer port
// id
// pw

// mqtt.serviceport.co.kr:1883

int main(int argc, char* argv[])
{
    uint8_t reconnect = true;
    //char clientid[24];//id를 사용하는 경우
    struct mosquitto* mosq;
    int rc = 0;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    mosquitto_lib_init();

    //메모리 초기화
    //memset(clientid, 0, 24);//맨 앞부터 0을 24개 삽입 (초기화)
    //snprintf(clientid, 23, "mysql_log_%d", getpid());//23길이의 clientid에 pid를 가진 문자열 삽입
    // mosq = mosquitto_new(clientid, true, 0);//mosquitto 구조체 생성 <-
    mosq = mosquitto_new(NULL, true, 0);//mosquitto 구조체 생성
    if (mosq == NULL)
    {
        fprintf(stderr, "Error: Out of memory.\n");
        return 1;
    }

    mosquitto_connect_callback_set(mosq, connect_callback);
    mosquitto_message_callback_set(mosq, message_callback);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);

    mosquitto_username_pw_set(mosq, "", "");

    rc = mosquitto_connect(mosq, MQTT_HOSTNAME, MQTT_PORT, 60);//mosqutiio 서버와 연결
    if (rc != MOSQ_ERR_SUCCESS)
    {
        mosquitto_destroy(mosq);
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        return 1;
    }

    /*
    while(run)
    {
       rc = mosquitto_loop(mosq, -1, 1);
       if (run && rc)
       {
          printf("connection error!\n");
          sleep(10);
          mosquitto_reconnect(mosq);
       }
    }
    */
    mosquitto_loop_forever(mosq, -1, 1);

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
