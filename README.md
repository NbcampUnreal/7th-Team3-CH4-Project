# Planty Race

> 멀티플레이 파티 게임 | Unreal Engine 5 / C++

##  프로젝트 개요
 장르 : 멀티플레이 파티 게임 
 플랫폼 : PC 
 팀 규모 : 6명
 기간 : 4주
 최대 인원 10인 온라인 
 담당 : 서버 아키텍처 

##  팀 구성
- 총 6명 참여
- 본 README는 서버 아키텍처 담당 파트 기준으로 작성

## 🎯 담당 역할 — Multiplayer Server Architecture

### Server Authority 구조 설계
- 모든 게임 판정은 Server에서만 처리
- 클라이언트는 입력 전송 + 결과 표현만 담당
- 치트 방지 + 일관성 보장

### GameFramework 활용
- `GameMode` → 라운드 진행 · 탈락 판정 (Server only)
- `GameState` → 전체 게임 상태 동기화 (Replicated)
- `PlayerState` → 개인 점수 · 순위 관리

### RPC 구현
- `ServerRPC` 클라이언트 입력 → 서버 판정
- `ClientRPC` 개인 UI 갱신
- `NetMulticast` 전체 이벤트 전파

## 🛠 기술 스택
`Unreal Engine 5`, `C++`, `Multiplayer`, `Server Authority`, `RPC`, `GameFramework`
