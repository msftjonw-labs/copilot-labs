---
name: brainstorm-idea
description: 'Brainstorm an idea in msftjonw-labs/copilot-labs. Use when the user says Brainstrom an idea with me or Brainstorm an idea with me anywhere in a request. Collect a root-level folder name and preferred branch name before creating idea files, respecting cloud agent branch restrictions.'
---

# Brainstorm an idea

## 1. Collect inputs before changes

Confirm that the task targets `msftjonw-labs/copilot-labs`. If not, stop and
direct the user to start a task there.

Read the current request and relevant conversation for explicitly supplied
names. Ask for all missing inputs in one interaction:

- Root-level folder name.
- Preferred branch name.

Do not choose either name on the user's behalf. State that the cloud agent
may be restricted to its assigned branch. If interactive questions are not
available, ask in the supported response/comment surface and end the run
without creating the project folder. Resume after the user supplies inputs.

## 2. Validate the folder and branch constraints

- The folder must be one new, non-hidden directory directly under the repository
  root, not an absolute path or nested path. Reject `.`, `..`, path separators,
  traversal, drive prefixes, and names that are invalid on Windows or Linux.
  For portability, request a name using lowercase letters, numbers, and single
  hyphens, beginning and ending with a letter or number.
- If the name is invalid or the target already exists (including a
  case-insensitive collision), explain the issue and request a different name.
  Do not overwrite, merge into, or delete an existing directory.
- Identify the actual assigned branch using available session metadata or
  supported read-only repository tools. If it cannot be established, report
  that limitation before making changes.
- Validate a preferred branch name using existing platform/repository facilities
  when available. Do not interpolate user-supplied names into shell commands.
  A name that is syntactically valid is not necessarily writable by the agent.
- If the preferred branch differs from the assigned branch, explain both names
  and ask whether to continue on the assigned branch. Alternatively, the user
  can arrange a task on a PR branch that cloud agent is permitted to update.
  Do not change branches, rename the assigned branch, push elsewhere, or
  bypass restrictions. Do not silently substitute a branch name.

Do not create the idea folder until valid names are supplied and any branch
mismatch has been explicitly resolved.

## 3. Persist the brainstorming space

Create the approved directory directly under the repository root and add
`README.md` inside it. Git does not preserve an empty directory.

Keep this initial document concise:

- Idea title and the user's idea in their own terms.
- Problem and intended audience.
- Confirmed goals and constraints.
- Options to explore, including trade-offs.
- Open questions and next steps.

Label proposals and unknowns rather than inventing decisions. If no idea
details were supplied, leave those details explicitly open and invite the
user to describe the idea. Do not scaffold an application, add dependencies,
or implement a solution unless separately requested.

Use the platform's normal change-persistence flow on the authorized branch.
Do not claim that a commit, push, or PR exists unless the platform confirms it.
Do not create a PR solely because brainstorming was requested.

## 4. Continue the conversation

Report the created folder, actual branch, and persistence status. Distinguish
the preferred branch from the actual branch if they differ.

Ask a small number of focused questions about the idea. As the user answers,
update the idea document with confirmed decisions and keep unresolved
questions visible. If text-based test feedback is also requested, follow
the sibling `text-test-results` skill for any testing work.
