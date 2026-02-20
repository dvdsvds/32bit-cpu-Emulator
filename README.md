# 32비트 CPU 에뮬레이터 상세 문서

## 목차
1. [프로젝트 개요](#프로젝트-개요)
2. [전체 아키텍처](#전체-아키텍처)
3. [주요 파일 상세 설명](#주요-파일-상세-설명)
4. [CPU 내부 동작 원리](#cpu-내부-동작-원리)
5. [명령어 세트 아키텍처(ISA)](#명령어-세트-아키텍처isa)
6. [파이프라인 구조](#파이프라인-구조)
7. [메모리 시스템](#메모리-시스템)
8. [인터럽트 및 예외 처리](#인터럽트-및-예외-처리)

---

## 프로젝트 개요

이 프로젝트는 **32비트 CPU 에뮬레이터**입니다. 컴파일러가 생성한 어셈블리 코드를 실행할 수 있는 가상 CPU를 구현합니다.

### 주요 특징
- **32비트 RISC 아키텍처**
- **3단계 파이프라인** (IF, ID, EX, WB)
- **분기 예측기** (2-bit saturating counter)
- **데이터 포워딩** (Data Forwarding)
- **인터럽트 및 예외 처리**
- **CSR (Control and Status Register) 지원**
- **메모리 매핑 I/O**

### 빌드 가능한 실행 파일
- `cpu`: CPU 에뮬레이터 실행 파일
- `assembler`: 어셈블리 코드를 바이너리로 변환하는 어셈블러

---

## 전체 아키텍처

### CPU 에뮬레이터 구조

**파이프라인 단계:**
- IF Stage (Instruction Fetch) → ID Stage (Instruction Decode) → EX Stage (Execute) → WB Stage (Write Back)
- 각 단계는 파이프라인 레지스터를 통해 연결됨
- 파이프라인 레지스터: IF_ID, ID_EX, EX_WB

**레지스터 파일:**
- 32개 범용 레지스터 (r0 ~ r31)
- 플래그 레지스터 (ZF, CF, SF, OF)
- CSR 레지스터 (15개)

**분기 예측기:**
- 256 엔트리
- 2-bit saturating counter 방식

**메모리 시스템:**
- 총 16MB 메모리 공간
- 프로그램 메모리 영역
- 스택 영역 (0x00FFFFFC부터 시작)
- MMIO 영역:
  - 0x100: 정수 출력
  - 0x104: 불린 출력
  - 0x108: 문자열 출력
  - 0x110: 키보드 입력 읽기
  - 0x114: 키보드 상태 확인

---

## 주요 파일 상세 설명

### 1. **include/types.hpp** - 기본 타입 정의
- **역할**: 프로젝트 전반에서 사용되는 기본 타입 정의
- **주요 타입**:
  - `u8`, `u16`, `u32`: 부호 없는 정수 타입
  - `reg_index`: 레지스터 인덱스 (0~31)
  - `addr_t`: 메모리 주소 (32비트)
  - `flags_t`: 플래그 레지스터
  - `inst_t`: 명령어 (32비트)
- **상수 정의**:
  - `REGISTER_COUNT = 32`: 레지스터 개수
  - `MEMORY_SIZE = 16MB`: 메모리 크기
  - `STACK_START = MEMORY_SIZE - 4`: 스택 시작 주소
- **플래그 비트 정의**:
  - `ZF` (Zero Flag): 결과가 0인지
  - `CF` (Carry Flag): 캐리 발생 여부
  - `SF` (Sign Flag): 부호 비트
  - `OF` (Overflow Flag): 오버플로우 발생 여부

### 2. **include/isa/opcode.hpp** - 명령어 코드 정의
- **역할**: 모든 명령어의 opcode 정의
- **명령어 그룹**:
  - **ARITH**: 산술 연산 (ADD, SUB, MUL, DIV)
  - **COMPARE**: 비교 연산 (CMP)
  - **MOVE**: 데이터 이동 (MOV, LUI, AUI, LOADI)
  - **MEMORY**: 메모리 접근 (LOADW, LOADB, STOREW, STOREB, PUSH, POP)
  - **BRANCH**: 분기 (BJMP)
  - **JUMP**: 점프 (JMP, CALL, RET, CALLR)
  - **SYSTEM**: 시스템 명령 (HLT, NOP, SYSCALL, TRAP, IRET)
  - **CSR**: 제어/상태 레지스터 (CSRR, CSRW, CSRWI)
- **총 34개의 명령어** 지원

### 3. **include/isa/format.hpp** - 명령어 포맷
- **역할**: 명령어 인코딩 포맷 정의
- **포맷 종류**:
  - `R`: 레지스터-레지스터 (rd, rs1, rs2)
  - `I`: 즉시값 포함 (rd, rs1, imm)
  - `M`: 메모리 접근 (rd, rs1, offset)
  - `S`: 시스템/점프 (주소 또는 즉시값)
  - `C`: CSR 접근 (rd/rs1, csr)

### 4. **include/isa/mode.hpp** - 주소 지정 모드
- **역할**: 피연산자 주소 지정 방식
- **모드 종류**:
  - `REGISTER`: 레지스터 모드
  - `IMMEDIATE`: 즉시값 모드
  - `FLAG`: 플래그 모드
  - `RESERVED`: 예약됨

### 5. **include/isa/cond.hpp** - 분기 조건
- **역할**: 조건부 분기 명령어의 조건 정의
- **조건 종류**:
  - `EQ`: 같음 (ZF=1)
  - `NE`: 다름 (ZF=0)
  - `LT`: 작음 (SF≠OF)
  - `LE`: 작거나 같음 (ZF=1 OR SF≠OF)
  - `GT`: 큼 (ZF=0 AND SF=OF)
  - `GE`: 크거나 같음 (SF=OF)
  - `CS`: 캐리 설정 (CF=1)
  - `CC`: 캐리 클리어 (CF=0)

### 6. **include/decoder/decoder.hpp** - 명령어 디코더
- **역할**: 32비트 명령어를 디코딩하여 구성 요소 추출
- **주요 기능**:
  - `decode_inst()`: 명령어 전체 디코딩
  - `extract_opcode()`: opcode 추출
  - `extract_rd()`, `extract_rs1()`, `extract_rs2()`: 레지스터 번호 추출
  - `extract_imm19()`, `extract_imm21()`, `extract_imm16()` 등: 즉시값 추출
  - `extract_addr23()`, `extract_addr26()`: 주소 추출
  - `extract_cond()`: 분기 조건 추출
- **디코딩 결과 구조체**:
  ```cpp
  struct decode {
      Opcode opcode;
      Mode mode;
      Format format;
      reg_index rd, rs1, rs2, csr;
      int imm19, imm21, imm16, imm26, imm11;
      int addr23, addr26;
      Cond cond;
  };
  ```

### 7. **include/decoder/encoding.hpp** - 인코딩 유틸리티
- **역할**: 명령어 인코딩을 위한 비트 추출 및 확장 함수
- **주요 함수**:
  - `extract_bits()`: 비트 필드 추출
  - `sign_extend()`: 부호 확장
- **비트 위치 정의**:
  - `MODE_LSB = 0`: 모드 비트 시작 위치
  - `OPCODE_LSB = 26`: opcode 비트 시작 위치
  - `RD_LSB = 21`, `RS1_LSB = 16`, `RS2_LSB = 11`: 레지스터 비트 위치
  - 각종 즉시값 및 주소 비트 위치 정의

### 8. **include/cpu/memory.hpp & src/memory.cpp** - 메모리 시스템
- **역할**: 16MB 메모리 공간 관리 및 MMIO 처리
- **주요 기능**:
  - `read_u8()`, `read_u16()`, `read_u32()`: 메모리 읽기
  - `write_u8()`, `write_u16()`, `write_u32()`: 메모리 쓰기
  - `write_u32_direct()`: 직접 메모리 쓰기 (MMIO 우회)
- **메모리 매핑 I/O (MMIO)**:
  - `0x100`: 정수 출력 (PRINT:INT)
  - `0x104`: 불린 출력 (PRINT:BOOL)
  - `0x108`: 문자열 출력 (PRINT:STR) - 주소 전달
  - `0x110`: 키보드 입력 읽기
  - `0x114`: 키보드 입력 가능 여부 확인
- **키보드 버퍼**: 큐를 사용한 비동기 입력 처리

### 9. **include/cpu/csr.hpp** - 제어/상태 레지스터
- **역할**: CPU 제어 및 상태 정보 저장
- **CSR 레지스터 (15개)**:
  - `EPC`: 예외 발생 시 PC 저장
  - `CAUSE`: 예외/인터럽트 원인
  - `STATUS`: CPU 상태 (IE, PIE, MODE)
  - `IVTBR`: 인터럽트 벡터 테이블 베이스 주소
  - `IMASK`: 인터럽트 마스크
  - `IPENDING`: 대기 중인 인터럽트
  - `SEPC`, `SSTATUS`, `SCRATCH`: 시스템 레지스터
  - `CYCLE`: 사이클 카운터
  - `TPERIOD`, `TCONTROL`, `TCOUNTER`: 타이머 관련
  - `EFLAGS`: 플래그 레지스터 백업
  - `SP`: 스택 포인터
- **인터럽트 타입**:
  - `TIMER`: 타이머 인터럽트
  - `EXT0~EXT3`: 외부 인터럽트 0~3
  - `SWI`: 소프트웨어 인터럽트
- **예외 타입**:
  - `INVALID_OPCODE`: 잘못된 opcode
  - `DIVISION_BY_ZERO`: 0으로 나누기
  - `ALIGNMENT_ERROR`: 정렬 오류
  - `ACCESS_VIOLATION`: 접근 위반
  - `BREAKPOINT`: 브레이크포인트

### 10. **include/cpu/pipeline.hpp & src/pipeline.cpp** - 파이프라인
- **역할**: 3단계 파이프라인 레지스터 관리
- **파이프라인 레지스터**:
  - `IF_ID`: Instruction Fetch - Instruction Decode
    - `inst`: 명령어
    - `curr_pc`: 현재 PC
    - `predicted_pc`: 예측된 PC
    - `is_valid`: 유효성 플래그
  - `ID_EX`: Instruction Decode - Execute
    - opcode, format, mode, cond
    - 레지스터 값 (v_rs1, v_rs2, curr_v_rd)
    - 레지스터 번호 (n_rs1, n_rs2, n_rd, n_csr)
    - 즉시값들 (imm19, imm21, imm16, imm26, imm11)
    - 주소들 (addr23, addr26)
    - 제어 신호 (reg_write, mem_read, mem_write, is_branch)
  - `EX_WB`: Execute - Write Back
    - `alu_result`: ALU 연산 결과
    - `mem_read_data`: 메모리에서 읽은 데이터
    - `mem_addr`: 메모리 주소
    - `w_data`: 쓸 데이터
    - `branch_taken`: 분기 취해짐 여부
    - `flags_write`: 플래그 쓰기 여부
    - `f`: 플래그 값
- **더블 버퍼링**: 두 개의 파이프라인 레지스터 세트를 번갈아 사용

### 11. **include/cpu/predictor.hpp & src/predictor.cpp** - 분기 예측기
- **역할**: 분기 명령어의 방향 예측
- **구현 방식**: 2-bit saturating counter
- **예측 테이블**: 256 엔트리
- **상태**:
  - `0`: Strongly Not Taken
  - `1`: Weakly Not Taken
  - `2`: Weakly Taken (초기값)
  - `3`: Strongly Taken
- **동작**:
  - `predict()`: PC 기반으로 분기 예측
  - `update()`: 실제 분기 결과로 테이블 업데이트
  - 인덱스 계산: `(pc >> 2) & 0xFF`

### 12. **include/cpu/cpu.hpp & src/cpu.cpp** - CPU 코어
- **역할**: CPU의 핵심 로직 구현
- **주요 구성 요소**:
  - 32개 범용 레지스터 (r0는 항상 0)
  - 15개 CSR 레지스터
  - 플래그 레지스터 (ZF, CF, SF, OF)
  - 프로그램 카운터 (PC)
  - 메모리 시스템
  - 파이프라인
  - 분기 예측기
- **주요 메서드**:
  - `step()`: 한 사이클 실행
  - `run()`: 프로그램 실행 (HLT까지)
  - `if_stage()`: Instruction Fetch 단계
  - `id_stage()`: Instruction Decode 단계
  - `ex_stage()`: Execute 단계
  - `wb_stage()`: Write Back 단계
  - `handle_interrupt()`: 인터럽트 처리
  - `update_timer()`: 타이머 업데이트

### 13. **include/encoder/encoder.hpp & src/encoder.cpp** - 인코더
- **역할**: 어셈블리 명령어를 바이너리로 인코딩
- **주요 함수**: 각 명령어별 인코딩 함수 제공
  - `ADD()`, `SUB()`, `AND()`, `OR()`, `XOR()` 등
  - `LOADW()`, `STOREW()`, `LOADB()`, `STOREB()`
  - `JMP()`, `BJMP()`, `CALL()`, `RET()`
  - `CSRR()`, `CSRW()`, `CSRWI()`
  - 등등...

### 14. **src/assembler.cpp** - 어셈블러
- **역할**: 어셈블리 소스 코드를 바이너리로 변환
- **2-Pass 어셈블러**:
  - **1차 패스**: 레이블 주소 수집
  - **2차 패스**: 명령어 인코딩
- **지원 기능**:
  - 레이블 정의 및 참조
  - 주석 처리 (`//`)
  - 레지스터 파싱 (`r0` ~ `r31`, `r_temp`)
  - 즉시값 파싱 (10진수, 16진수 `0x`)
  - 레이블 주소 계산

---

## CPU 내부 동작 원리

### 1. 명령어 실행 사이클

**한 사이클 (step) 실행 순서:**

1. **키보드 입력 처리**
2. **타이머 업데이트**
3. **인터럽트 처리**

4. **WB Stage (Write Back)**
   - 레지스터에 결과 쓰기
   - 메모리에서 데이터 읽기
   - 메모리에 데이터 쓰기
   - 플래그 업데이트

5. **EX Stage (Execute)**
   - ALU 연산 수행
   - 메모리 주소 계산
   - 분기 조건 평가
   - 데이터 포워딩 처리

6. **ID Stage (Instruction Decode)**
   - 명령어 디코딩
   - 레지스터 값 읽기
   - 제어 신호 생성

7. **IF Stage (Instruction Fetch)**
   - PC에서 명령어 읽기
   - 분기 예측
   - PC 업데이트

8. **파이프라인 플러시 처리**
9. **파이프라인 클럭 (더블 버퍼 전환)**

### 2. 데이터 포워딩 (Data Forwarding)

**목적**: 파이프라인 해저드 해결

**동작 원리**:
```cpp
bool CPU::need_forwarding(reg_index num) {
    EX_WB& exwb = pipe.getEXWB();
    return ((exwb.is_valid == true) && 
            (exwb.reg_write == true || exwb.mem_read == true) &&
            (num != 0) && 
            (exwb.n_rd == num));
}
```

**포워딩 시나리오**:
```
명령어 1: ADD r1, r2, r3    (EX 단계)
명령어 2: SUB r4, r1, r5    (ID 단계)  ← r1이 아직 쓰이지 않음!

해결: EX 단계의 결과를 ID 단계로 포워딩
```

**포워딩 로직**:
```cpp
auto get_forwarded_value = [&](reg_index reg, u32 original_val) -> u32 {
    if(need_forwarding(reg)) {
        return exwb.mem_read ? exwb.mem_read_data : exwb.alu_result;
    }
    return original_val;
};
```

### 3. 분기 예측 및 플러시

**분기 예측 과정**:
1. IF 단계에서 BJMP 명령어 감지
2. 분기 예측기로 방향 예측
3. 예측된 주소로 PC 업데이트
4. EX 단계에서 실제 조건 평가
5. 예측이 틀리면 파이프라인 플러시

**플러시 처리**:
```cpp
if(take) {
    if(idex.predicted_pc != target) {
        pc = target;
        need_flush_ifid = true;  
        need_flush_idex = true;  
    }
} else {
    if(idex.predicted_pc != fallthrough) {
        pc = fallthrough;
        need_flush_ifid = true;  
        need_flush_idex = true;  
    }
}
```

### 4. 메모리 쓰기 히스토리

**목적**: 메모리-레지스터 의존성 해결

**동작**:
- 최근 8개의 메모리 쓰기 기록 유지
- 메모리 읽기 시 히스토리에서 검색
- 히스토리에 있으면 포워딩, 없으면 실제 메모리 읽기

```cpp
struct MemWrite {
    bool valid;
    addr_t addr;
    u32 data;
};
MemWrite mem_history[8];
```

### 5. 플래그 업데이트

**산술 연산 플래그**:
- `ZF`: 결과가 0인지
- `SF`: 부호 비트 (MSB)
- `CF`: 캐리 발생 (덧셈: result < op1, 뺄셈: op1 < op2)
- `OF`: 오버플로우 (부호가 같은 두 수의 연산 결과 부호가 다름)

**논리 연산 플래그**:
- `ZF`, `SF`만 업데이트
- `CF`, `OF`는 false

**시프트 연산 플래그**:
- `ZF`, `SF`만 업데이트

---

## 명령어 세트 아키텍처(ISA)

### 명령어 인코딩 포맷

#### R 포맷 (Register)

| 비트 위치 | 31-26 | 25-24 | 23-21 | 20-16 | 15-11 | 10-2 | 1-0 |
|---------|-------|-------|-------|-------|-------|------|-----|
| 필드 | OPCODE | MODE | RD | RS1 | RS2 | (unused) | MODE |
| 크기 | 6 bits | 2 bits | 5 bits | 5 bits | 5 bits | 9 bits | 2 bits |

#### I 포맷 (Immediate)

| 비트 위치 | 31-26 | 25-24 | 23-21 | 20-16 | 15-2 | 1-0 |
|---------|-------|-------|-------|-------|------|-----|
| 필드 | OPCODE | MODE | RD | RS1 | IMM19 | MODE |
| 크기 | 6 bits | 2 bits | 5 bits | 5 bits | 19 bits | 2 bits |

#### M 포맷 (Memory)

| 비트 위치 | 31-26 | 25-24 | 23-21 | 20-16 | 15-2 | 1-0 |
|---------|-------|-------|-------|-------|------|-----|
| 필드 | OPCODE | MODE | RD | RS1 | IMM16 | MODE |
| 크기 | 6 bits | 2 bits | 5 bits | 5 bits | 16 bits | 2 bits |

#### S 포맷 (System/Jump)

| 비트 위치 | 31-26 | 25-24 | 23-0 |
|---------|-------|-------|------|
| 필드 | OPCODE | MODE | ADDR26/IMM26 |
| 크기 | 6 bits | 2 bits | 26 bits |

### 주요 명령어 설명

#### 산술 연산
- **ADD rd, rs1, rs2/imm**: 덧셈
- **SUB rd, rs1, rs2/imm**: 뺄셈
- **MUL rd, rs1, rs2/imm**: 곱셈
- **DIV rd, rs1, rs2/imm**: 나눗셈

#### 논리 연산
- **AND rd, rs1, rs2/imm**: 비트 AND
- **OR rd, rs1, rs2/imm**: 비트 OR
- **XOR rd, rs1, rs2/imm**: 비트 XOR

#### 시프트 연산
- **SHL rd, rs1, rs2/imm**: 논리 왼쪽 시프트
- **SHR rd, rs1, rs2/imm**: 논리 오른쪽 시프트
- **SAR rd, rs1, rs2/imm**: 산술 오른쪽 시프트

#### 비교 연산
- **CMP rs1, rs2/imm**: 비교 (플래그만 설정, 결과는 저장 안 함)

#### 데이터 이동
- **MOV rd, rs1**: 레지스터 간 이동
- **LUI rd, imm21**: 상위 21비트 로드
- **AUI rd, imm21**: 상위 21비트 더하기
- **LOADI rd, imm21**: 즉시값 로드

#### 메모리 접근
- **LOADW rd, rs1, imm16**: 워드(32비트) 로드
- **LOADB rd, rs1, imm16**: 바이트(8비트) 로드
- **STOREW rd, rs1, imm16**: 워드 저장
- **STOREB rd, rs1, imm16**: 바이트 저장

#### 스택 연산
- **PUSH rs**: 스택에 푸시
- **POP rd**: 스택에서 팝

#### 제어 흐름
- **JMP addr26**: 무조건 점프
- **BJMP cond, addr23**: 조건부 분기
- **CALL addr26**: 함수 호출 (주소)
- **CALLR rs**: 함수 호출 (레지스터)
- **RET**: 함수 반환

#### 시스템 명령
- **HLT**: 정지
- **NOP**: 무연산
- **SYSCALL**: 시스템 호출
- **TRAP imm26**: 트랩 발생
- **IRET**: 인터럽트 복귀

#### CSR 명령
- **CSRR rd, csr**: CSR 읽기
- **CSRW rs1, csr**: CSR 쓰기
- **CSRWI csr, imm11**: CSR 즉시값 쓰기

---

## 파이프라인 구조

### 파이프라인 단계

#### 1. IF (Instruction Fetch) 단계
- **역할**: 명령어 인출
- **동작**:
  1. PC에서 32비트 명령어 읽기
  2. BJMP 명령어인 경우 분기 예측
  3. 예측된 주소로 PC 업데이트
  4. IF_ID 레지스터에 저장

#### 2. ID (Instruction Decode) 단계
- **역할**: 명령어 디코딩 및 레지스터 읽기
- **동작**:
  1. 명령어 디코딩 (opcode, format, mode 등)
  2. 레지스터 파일에서 값 읽기
  3. 제어 신호 생성 (reg_write, mem_read, mem_write, is_branch)
  4. ID_EX 레지스터에 저장

#### 3. EX (Execute) 단계
- **역할**: 연산 수행 및 메모리 주소 계산
- **동작**:
  1. 데이터 포워딩 처리
  2. ALU 연산 수행
  3. 메모리 주소 계산
  4. 분기 조건 평가
  5. 분기 예측 검증 및 플러시 결정
  6. EX_WB 레지스터에 저장

#### 4. WB (Write Back) 단계
- **역할**: 결과를 레지스터/메모리에 쓰기
- **동작**:
  1. 레지스터에 ALU 결과 쓰기
  2. 메모리에서 데이터 읽기 (LOAD 명령)
  3. 메모리에 데이터 쓰기 (STORE 명령)
  4. 플래그 레지스터 업데이트
  5. 메모리 쓰기 히스토리 업데이트

### 파이프라인 해저드 및 해결

#### 1. 데이터 해저드 (Data Hazard)
- **문제**: 이전 명령어의 결과를 다음 명령어가 사용
- **해결**: 데이터 포워딩

#### 2. 제어 해저드 (Control Hazard)
- **문제**: 분기 명령어로 인한 파이프라인 플러시
- **해결**: 분기 예측 + 플러시

#### 3. 구조 해저드 (Structural Hazard)
- **문제**: 리소스 충돌
- **해결**: 더블 버퍼링된 파이프라인 레지스터

---

## 메모리 시스템

### 메모리 레이아웃

| 주소 범위 | 영역 | 설명 |
|---------|------|------|
| 0x00000000 ~ | 프로그램 영역 | 실행 코드 저장 |
| ~ | 데이터 영역 | 전역 변수 및 정적 데이터 |
| ~ | 힙 영역 | 동적 할당 메모리 |
| 0x00FFFFFC ~ 0x01000000 | 스택 영역 | 스택 포인터(SP) 시작 위치, 아래로 확장 |

**참고**: 스택은 높은 주소에서 낮은 주소로 확장됩니다.

### 메모리 매핑 I/O (MMIO)

| 주소 | 기능 | 설명 |
|------|------|------|
| 0x100 | PRINT:INT | 정수 출력 (32비트 부호 있는 정수) |
| 0x104 | PRINT:BOOL | 불린 출력 (0=false, 1=true) |
| 0x108 | PRINT:STR | 문자열 출력 (주소 전달) |
| 0x110 | KEYBOARD_IN | 키보드 입력 읽기 (1바이트) |
| 0x114 | KEYBOARD_STATUS | 키보드 입력 가능 여부 (0=없음, 1=있음) |

### 메모리 접근 동작

**읽기**:
- `read_u32(0x110)`: 키보드 버퍼에서 문자 읽기
- `read_u32(0x114)`: 키보드 상태 확인
- 그 외: 실제 메모리에서 읽기

**쓰기**:
- `write_u32(0x100)`: 정수 출력
- `write_u32(0x104)`: 불린 출력
- `write_u32(0x108)`: 문자열 출력 (주소 전달)
- 그 외: 실제 메모리에 쓰기

---

## 인터럽트 및 예외 처리

### 인터럽트 처리 흐름

**인터럽트 처리 단계:**

1. **인터럽트 발생**
2. **handle_interrupt() 호출**
3. **STATUS 레지스터의 IE 확인**
4. **IPENDING & IMASK 확인**
5. **인터럽트가 있으면:**
   - EPC에 현재 PC 저장
   - CAUSE에 인터럽트 코드 저장
   - EFLAGS에 플래그 저장
   - STATUS 업데이트 (IE=0, MODE=1, PIE 저장)
   - IVTBR에서 핸들러 주소 읽기
   - PC를 핸들러 주소로 설정
   - 파이프라인 플러시
6. **핸들러 실행**
7. **IRET 명령으로 복귀:**
   - EPC에서 PC 복원
   - EFLAGS에서 플래그 복원
   - STATUS 복원 (IE=PIE, MODE=0)

### 인터럽트 타입

1. **TIMER**: 타이머 인터럽트
   - TCOUNTER가 TPERIOD에 도달하면 발생
   - TCONTROL의 ENABLE 비트로 제어

2. **EXT0~EXT3**: 외부 인터럽트
   - 외부 장치에서 발생

3. **SWI**: 소프트웨어 인터럽트
   - SYSCALL 명령으로 발생

### 예외 처리

예외는 인터럽트와 유사하게 처리되지만:
- `INTERRUPT_LSB`가 0으로 설정됨
- 예외 코드가 CAUSE에 저장됨

### 타이머 동작

```cpp
void CPU::update_timer() {
    u32 enable = (tcontrol >> ENABLE_LSB) & 1;
    if(enable == 0) return;
    
    tcounter++;
    if(tcounter >= tperiod) {
        tcounter = 0;
        trigger_interrupt(Interrupt::TIMER);
        
        u32 tmode = (tcontrol >> TMODE_LSB) & 1;
        if(tmode == 0) {  // One-shot 모드
            tcontrol &= ~(1u << ENABLE_LSB);
        }
    }
}
```

---

## 레지스터 파일

### 범용 레지스터 (32개)

- **r0**: 항상 0 (읽기 전용)
- **r1 ~ r30**: 일반 목적 레지스터
- **r31 (r_temp)**: 임시 레지스터

### 플래그 레지스터

- **ZF (Zero Flag)**: 결과가 0인지
- **CF (Carry Flag)**: 캐리 발생
- **SF (Sign Flag)**: 부호 비트
- **OF (Overflow Flag)**: 오버플로우

### CSR 레지스터 (15개)

| 인덱스 | 이름 | 설명 |
|--------|------|------|
| 0 | EPC | 예외 발생 시 PC |
| 1 | CAUSE | 예외/인터럽트 원인 |
| 2 | STATUS | CPU 상태 (IE, PIE, MODE) |
| 3 | IVTBR | 인터럽트 벡터 테이블 베이스 |
| 4 | IMASK | 인터럽트 마스크 |
| 5 | IPENDING | 대기 중인 인터럽트 |
| 6 | SEPC | 시스템 EPC |
| 7 | SSTATUS | 시스템 STATUS |
| 8 | SCRATCH | 시스템 스크래치 |
| 9 | CYCLE | 사이클 카운터 |
| 10 | TPERIOD | 타이머 주기 |
| 11 | TCONTROL | 타이머 제어 |
| 12 | TCOUNTER | 타이머 카운터 |
| 13 | EFLAGS | 플래그 백업 |
| 14 | SP | 스택 포인터 |

---

## 어셈블러 사용법

### 어셈블리 문법

```assembly
// 레이블 정의
label_name:
    add r1, r2, r3
    sub r4, r1, r5

// 레지스터 사용
mov r1, r2
add r3, r1, r4

// 즉시값 사용
addi r1, r2, 10      // r1 = r2 + 10
addi r1, r2, 0x10    // 16진수

// 메모리 접근
loadw r1, r2, 0      // r1 = mem[r2 + 0]
storew r1, r2, 4     // mem[r2 + 4] = r1

// 분기
bjmpeq label_name    // 같으면 분기
jmp label_name       // 무조건 점프

// 함수 호출
call function_name
ret
```

### 어셈블러 실행

```bash
assembler input.asm output.bin
```