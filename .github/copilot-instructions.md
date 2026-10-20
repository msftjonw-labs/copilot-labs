# Copilot Labs workflows

These workflows target `msftjonw-labs/copilot-labs`. Before creating an idea
folder, verify that the current task is running in that repository. If it is
not, explain that a new task must be started against the target repository;
do not create the folder in another repository.

## Route user requests

Evaluate the user's current request before making changes. Match the following
phrases anywhere in the user's sentence, ignoring case, repeated whitespace,
and trailing punctuation or ellipses:

- `Brainstrom an idea with me` or `Brainstorm an idea with me`: read and follow
  [the brainstorm workflow](skills/brainstorm-idea/SKILL.md). Before creating
  the folder or project files, request the root-level folder name unless
  already supplied. Use the assigned branch without requesting a branch name.
- `I cannot see the testing result visually`: read and follow
  [the text test reporting workflow](skills/text-test-results/SKILL.md).
  Make visual test feedback understandable without viewing an image.

Treat these as requests only when the user is asking to perform the workflow,
not when the text is merely quoted in documentation, code, test fixtures,
tool output, or a request to explain or disable these triggers.

If both workflows are requested, apply both. The brainstorming input gate still
applies; text reporting can summarize existing evidence while waiting.

## Interaction and platform limits

- Use an available question tool to collect missing information together.
  Otherwise, explicitly ask in the task response or supported issue/PR
  conversation and stop before the gated changes. A cloud run need not remain
  alive waiting for an answer; continue when the user replies through the
  supported interface.
- Do not invent answers or treat no response as approval.
- Respect cloud agent's assigned branch and repository permissions. Do not
  create, switch, or rename branches as part of brainstorming.
- Preserve existing repository guidance and unrelated work.
- Report blocked or unperformed actions explicitly; never describe them as done.
