# Agent guidance

Before changes, read the [shared BrainboxEmb agent guidance](https://github.com/brainboxemb/brainboxemb.meta/blob/main/AGENTS.md), including its cross-repository Git/CI conventions. This file adds experiment-local navigation and boundaries only.

Read `README.md`, `docs/01-plan.md`, `docs/02-design.md` and the current implementation issue before starting work.

Experiment 009 is coordinated in `brainboxemb.meta` issue #188; implementation and evidence belong here. Use issue/PR review boundaries. Do not turn a prototype result into a production decision without retained verification.

The C parser, font and renderer are the authority for display output. JavaFX must render received pixels, not duplicate C drawing logic. Do not add libuv, LVGL or Arduino merely in anticipation of future requirements. Keep the baseline compatible with Java 21 / JavaFX 21 / NetBeans and portable C11.
