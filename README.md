* Assignment3-1
  * access.txt
    * client가 접속 가능한 IP address 저장
    * wildcard 사용
  * passwd
    * 사용자의 username, password 등 저장
  * client, server 소켓
    * client 소켓 IP address, Port number 지정 (예시) ./cli 127.0.01 20001
    * server 소켓 port number 지정 (예시) ./srv 20001
  1. server에서 client 소켓의 ip address 확인 (access.txt)
  2. client에서 user로부터 username, password 입력받기 (3회 이상 틀릴 시, log-in 실패)
  3. server에서 입력받은 username, password로 로그인 가능 여부 확인 (passwd)
