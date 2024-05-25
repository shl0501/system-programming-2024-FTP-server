* Assignment3-2
과제 목표 : 두 개의 소켓을 사용하여 control connection과 port connection을 생성하여 data를 송수신한다.
  * control connection
    * ACK 메시지 등을 주고 받음
    * client에서 port 명령어와 FTP 명령어를 전달
  * data connection
    * client로부터 받은 FTP 명령어에 대한 결과를 전달
    * 한번 사용한 후 close
  * 주요 동작
    1. client에서 random port number를 만들고 해당 port number와 ip address를 PORT command 형태로 변환
       (ex) PORT 127,0,0,2,48,57
    2. server에서 PORT command를 변환하여 data connection 연결
    3. data connection을 통해 ftp 명령어 결과 송수신

  * flow chart
    ![flow3 drawio (8)](https://github.com/shl0501/system-programming-2024-FTP-server/assets/114389927/cf95d096-6048-4504-b199-791e10599e01)

  * 결과화면
    ![image](https://github.com/shl0501/system-programming-2024-FTP-server/assets/114389927/7ca87249-bee7-4817-846f-358eefbea26b)

 

    
   
