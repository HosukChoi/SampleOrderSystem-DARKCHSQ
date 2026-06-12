# AI 개발 행동 지침

## Karpathy Guidelines

→ [karpathyGuidelines.md](./karpathyGuidelines.md)

모든 코드 작성·리뷰·리팩토링 시 아래 4원칙을 준수한다.

1. **코딩 전 사고**: 가정을 명시적으로 밝히고, 불명확한 사항은 구현 전에 질문한다.
2. **단순함 우선**: 요청한 것만 구현. 불필요한 추상화·유연성·에러 핸들링 추가 금지.
3. **외과적 변경**: 요청과 직접 관련된 코드만 수정. 인접 코드 임의 개선 금지.
4. **목표 기반 실행**: 성공 기준을 먼저 정의하고, 검증 가능한 단계로 분해하여 진행.

---

## Superpowers Skills

개발 흐름의 각 단계에서 아래 skill을 의무적으로 사용한다.

| 단계 | Skill | 사용 시점 |
|---|---|---|
| 기능 추가 / 설계 | `superpowers:brainstorming` | 새 기능 구현 전, 요구사항·설계 탐색 |
| 구현 계획 수립 | `superpowers:writing-plans` | 스펙/요구사항이 있는 멀티스텝 작업 전 |
| 계획 실행 | `superpowers:executing-plans` | 작성된 구현 계획을 별도 세션에서 실행할 때 |
| 병렬 작업 분배 | `superpowers:dispatching-parallel-agents` | 독립적인 2개 이상 작업이 있을 때 |
| 서브에이전트 개발 | `superpowers:subagent-driven-development` | 현재 세션에서 독립 태스크를 병렬 실행할 때 |
| 버그 / 테스트 실패 | `superpowers:systematic-debugging` | 버그·예상치 못한 동작 발생 시, 수정 전 |
| TDD | `superpowers:test-driven-development` | 기능·버그픽스 구현 전, 코드 작성 전 |
| 코드 리뷰 요청 | `superpowers:requesting-code-review` | 작업 완료 또는 major feature 구현 후 |
| 코드 리뷰 수신 | `superpowers:receiving-code-review` | 리뷰 피드백을 반영하기 전 |
| 완료 검증 | `superpowers:verification-before-completion` | "완료"·"수정됨"을 주장하기 전, 커밋/PR 전 |
| 브랜치 완료 | `superpowers:finishing-a-development-branch` | 구현 완료 후 merge/PR 방식 결정 시 |
| Git Worktree | `superpowers:using-git-worktrees` | 격리된 작업 공간이 필요한 기능 작업 시작 전 |
