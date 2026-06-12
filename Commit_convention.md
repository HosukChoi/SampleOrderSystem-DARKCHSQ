# Commit Convention

## 커밋 메시지 구조

```
<type>(<scope>): <subject>

<body>

<footer>

Author: <이름> <이메일>
Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
```

---

## Type

| 타입 | 설명 |
|------|------|
| `feat` | 새로운 기능 추가 |
| `fix` | 버그 수정 |
| `docs` | 문서 수정 |
| `style` | 코드 포맷 변경 (기능 변경 없음) |
| `refactor` | 코드 리팩토링 |
| `test` | 테스트 추가 또는 수정 |
| `chore` | 빌드 설정, 패키지 수정 등 기타 작업 |

---

## 규칙

- type은 앞뒤로 대괄호('[', ']')을 이용하여 감싸야한다.
- `subject`는 50자 이내로 작성
- 제목 끝에 마침표(`.`) 사용 금지
- 제목은 명령형으로 작성 (예: "변경한다" → "변경")
- `body`는 72자마다 줄바꿈
- `body`에는 **무엇을**, **왜** 변경했는지 작성
- `footer`에 `Author: <이름> <이메일>` 형식으로 작성자 정보를 포함
- Claude가 작성에 참여한 경우 `Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>` 추가

---

## 예시

```
[feat]: 로그인 기능 추가

이메일과 비밀번호를 이용한 로그인 기능을 구현함.
JWT 토큰 방식으로 인증 처리.

Closes #12
Author: hsw.choi darkchsq@naver.com
Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
```

```
[fix]: 사용자 조회 시 null 반환 오류 수정

Author: hsw.choi darkchsq@naver.com
Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
```

```
[docs]: README에 설치 방법 추가

Author: hsw.choi darkchsq@naver.com
Co-Authored-By: Claude Sonnet 4.6 <noreply@anthropic.com>
```
