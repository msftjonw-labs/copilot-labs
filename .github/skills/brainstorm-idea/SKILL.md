---
name: brainstorm-idea
description: 'Brainstorm an idea in msftjonw-labs/copilot-labs. Use when the user says Brainstrom an idea with me or Brainstorm an idea with me anywhere in a request. Collect a root-level folder name before creating idea files.'
---

# Brainstorm an idea

## 1. Collect inputs before changes

Confirm that the task targets `msftjonw-labs/copilot-labs`. If not, stop and
direct the user to start a task there.

Read the current request and relevant conversation for an explicitly supplied
folder name. Ask for the missing input:

- Root-level folder name.

Do not choose the folder name on the user's behalf. If interactive questions
are not available, ask in the supported response/comment surface and end the
run without creating the project folder. Resume after the user supplies the
name.

## 2. Validate the folder

- The folder must be one new, non-hidden directory directly under the repository
  root, not an absolute path or nested path. Reject `.`, `..`, path separators,
  traversal, drive prefixes, and names that are invalid on Windows or Linux.
  For portability, request a name using lowercase letters, numbers, and single
  hyphens, beginning and ending with a letter or number.
- If the name is invalid or the target already exists (including a
  case-insensitive collision), explain the issue and request a different name.
  Do not overwrite, merge into, or delete an existing directory.

Do not create the idea folder until a valid name is supplied.

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

Use the platform's normal change-persistence flow.
Do not claim that a commit, push, or PR exists unless the platform confirms it.
Do not create a PR solely because brainstorming was requested.

## 4. Continue the conversation

Report the created folder and persistence status.

Ask a small number of focused questions about the idea. As the user answers,
update the idea document with confirmed decisions and keep unresolved
questions visible. If text-based test feedback is also requested, follow
the sibling `text-test-results` skill for any testing work.
