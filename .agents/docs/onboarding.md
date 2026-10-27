# Agent setup

How coding agents are wired into this repo. Read when adding a doc, skill or rule.

## Layout

| Path | Holds |
| --- | --- |
| `AGENTS.md` | Lean entry file every agent reads: build, hard constraints, docs index |
| `.agents/docs/` | Agent-facing docs, each linked from `AGENTS.md` with a "read when…" hook |
| `.agents/plans/` | Plans and investigations (gitignored, local per developer) |
| `.claude/skills/`, `.claude/rules/` | Claude Code skills and always-on rules |

Keep `AGENTS.md` lean: a new topic gets its own doc in `.agents/docs/` plus one index line, linked
with a plain markdown link. Never an `@path` import: Claude Code loads imports at launch, which
defeats loading docs on demand.

## Hooking up an agent

- **Claude Code** reads `AGENTS.md` itself; don't add a `CLAUDE.md`. A `CLAUDE.md`,
  `.claude/CLAUDE.md` or `CLAUDE.local.md` here or in any parent folder makes Claude read it
  **instead of** `AGENTS.md`. If you keep a personal `CLAUDE.local.md`, start it with `@AGENTS.md`,
  or set **Project instructions** to `claude-md-and-agents-md`.
- **Any other agent**: point its entry-file mechanism at `AGENTS.md` (many read it natively), or
  re-run `/agentify-project` to research and wire it.

## Skills

Skills live in `.claude/skills/`, so only Claude Code sees them. Sharing them with other agents
means moving them to `.agents/skills/` with a relative symlink per skill in `.claude/skills/`. On
Windows, symlinks need Developer Mode and `git config core.symlinks true`; without both, git checks
them out as plain text files.

## Checking a new doc

Give a fresh session a task that needs the doc and check that it opens it. If it doesn't, the
"read when…" hook is too weak: fix it via `/self-improve`.
