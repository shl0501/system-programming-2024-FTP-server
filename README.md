## 프로젝트 개요
프로젝트 이름: FTP 기반 클라이언트-서버 프로그래밍

## 프로젝트 목표
- FTP 방식의 Control Connection과 Data Connection 구현
- 파일 기반 사용자 인증 및 보안 강화
- 멀티프로세스를 통한 다중 클라이언트 동시 처리
- 데이터 무결성 보장 (ASCII/BINARY 전송 모드 지원)

## 핵심 기능
- 제어 연결(Control Connection): ls, get, put, pwd, quit 명령어 처리
- 데이터 연결(Data Connection): PORT 명령어를 통한 임시 데이터 소켓 생성 및 전송
- 보안 및 사용자 인증: access.txt, passwd 파일을 이용한 IP 및 사용자 검증
- 프로세스 관리: fork()를 통한 각 클라이언트 별 프로세스 생성 및 관리
- 시그널 처리: SIGINT, SIGALRM 활용하여 서버 종료 및 클라이언트 해제

