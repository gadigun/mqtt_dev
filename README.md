# mqtt_dev

### 1. Linux 상에서 Mosquitto 소스코드를 Build 하기 위한 라이브러리를 설치한다.

```
$ sudo apt-get install build-essential libc-ares-dev uuid-dev libssl-dev libmysqlclient-dev

  build-essential     // 빌드 가능할 라이브러리 설치 ( 여기서 gcc, make 등등 설치 합니다. )
```

### 2. 만약 /var/lib/dpkg/lock 잠금 파일을 얻을 수 없다고 나온다면 다음과 같은 명령어를 통해 lock 파일 삭제를 진행한 후 다시 진행합니다.

```
$ rm /var/lib/dpkg/lock
```

### 3. Install Mosquitto Source

```
$ cd ~/
$ mkdir mosquitto_sources
$ cd mosquitto_sources/
$ wget http://mosquitto.org/files/source/mosquitto-2.0.18.tar.gz
$ tar xvf mosquitto-2.0.18.tar.gz
```

### 4. make Mosquitto source

```
$ cd ~/mosquitto_sources/mosquitto-2.0.18/
$ make
$ sudo make install
```

### 6. 여기까지 설치 후 

```
new_client 파일을 받아서 mosquitto-2.0.18 폴더 아래에 추가 합니다.
해당 폴더로 들어가서
$sudo make
$./my_sub 실행시키면 됩니다.
```

참조 https://wnsgml972.github.io/mqtt/2018/02/13/mqtt_ubuntu-install/


https://github.com/jmattsson/freescale-imx-toolchain.git

toolchain path setting

// ubuntu 64 에서 32 bit 실행 하기 위한 작업
sudo dpkg --add-architecture i386

sudo apt-get update
sudo apt-get install libc6:i386 libncurses5:i386 libstdc++6:i386

freescale-imx-toolchain 압축 풀고 /usr/bin/ 에 옮겨 넣는다. ( arm-none-linux-gnueabi 폴더를 )

패스 등록 하고
sudo vi /etc/profile
export PATH=$PATH:/usr/arm-none-linux-gnueabi/bin
추가


tool chain old version
https://launchpad.net/gcc-arm-embedded/+download?direction=backwards&start=10


makefile 관련
https://blog.naver.com/damhiya/220437112567

CMake 관련
https://www.tuwlab.com/ece/27234
