## 구현기능
### 구현 명령어
* NLST(ls) (+옵션 -a, -l, -al) : directory에 존재하는 file/directory 출력
* LIST : NLST -al 동작과 동일
* PWD(pwd) : 현재 동작하는 directory 출력
* CWD(cd) : Directory 이동
* CDUP(cd ..) : 이전 directory로 이동
* MKD(mkdir) : Directory 생성
* DELE(delete) : File 삭제
* RMD(rmdir) : Directory 삭제
* RNFR&RNTO(rename) : 이름 변경
* QUIT(quit) : Child process 종료
* Type
* get(RETR) : 다른 경로에서 파일을 현재 경로로 받아오기
* put(STOR) : 현재 경로상에 있는 파일을 다른 경로로 전달하기
  
### 기타 구현 사항
* port 번호 인자로 받아서 처리
* IP Access control, User authentication 구현
* Control connection, data connection
* 로그 파일 생성 및 기록
