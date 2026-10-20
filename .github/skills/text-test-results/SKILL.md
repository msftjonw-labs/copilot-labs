---
name: text-test-results
description: 'Make visual testing feedback understandable in text. Use when the user says I cannot see the testing result visually anywhere in a request, cannot view screenshots or videos, or requests accessible test evidence and measured results.'
---

# Text-based test results

## 1. Establish the evidence

Apply this reporting format to the requested test results and subsequent
testing in the current task. It supplements testing; it does not replace
running tests or turn screenshots into proof of functional correctness.

Locate relevant runner output, test reports, screenshots, traces, browser
observations, and available CI artifacts. Identify the tested revision and
environment when known.

If results already exist, summarize them first. Re-run the smallest relevant
test selection only when needed, safe, and supported by the repository's
existing tooling. State if results are stale or from a different revision.
Do not install a new test or image-analysis framework merely to fill a report.

If the requested evidence is not accessible, say exactly what is missing and
what access, artifact, or command would unblock the report. Never invent test
counts, image contents, coverage, timings, or successful checks.

## 2. Translate visual feedback

When an image or video can actually be inspected, describe:

- The page or component and the tested state.
- The action performed.
- The expected outcome and the observed outcome.
- Visible labels, messages, layout differences, clipping, overlap, and errors
  relevant to the test.

Use readable labels instead of "the red area" or "see screenshot." Include
text and DOM evidence when available. A DOM assertion alone does not prove
visual appearance, and a screenshot alone does not prove interaction behavior.

If image inspection is unavailable, do not pretend to see the image. Use
available logs, DOM assertions, accessibility snapshots, or numerical
comparison results; clearly state which visual properties remain unverified.
If none are available, report the visual check as blocked or not verified.

## 3. Report only defensible metrics

- From the runner: total, passed, failed, skipped, and duration, when available.
  Preserve the runner's categories and distinguish retries, flaky outcomes,
  incomplete execution, and collection errors.
- Pass rate, only when passed and failed counts are known and completed:
  `passed / (passed + failed) * 100`. Exclude skipped tests and state the
  denominator. If no tests completed, report `N/A`, not 100%.
- Coverage: value and scope only if coverage was actually collected.
- Visual diff: changed pixels and percentage, baseline, comparison method,
  and configured threshold only if supplied by a real comparison tool.
  Do not estimate pixel differences from an image by eye.
- Performance: measured values, units, sample conditions, and an existing
  budget if available. Do not invent a performance target.
- Accessibility: automated findings only if an accessibility check ran;
  do not equate a clean automated scan with complete accessibility compliance.

Use `not collected`, `not available`, or `not applicable` with a reason instead
of zero for missing measurements. If there is no baseline or acceptance
threshold, describe observations without inventing a visual pass/fail.

## 4. Use this response structure

Include the report inline in the task response or PR comment, not only as a
link or image attachment. Adapt rows to the evidence actually available.

```text
Overall: PASS / FAIL / PARTIAL / BLOCKED / NOT RUN
Scope: <test selection and revision, if known>
Environment: <browser, viewport, OS, or other relevant conditions>
Command or source: <exact command, CI run, or existing artifact>

Tests: <runner counts, including failed and skipped>
Pass rate: <value and denominator, or N/A with reason>
Duration: <measured value and units, or not available>
Other metrics: <collected coverage, visual diff, performance, or accessibility>

Scenario | Expected | Observed | Status | Evidence
<scenario> | <outcome> | <text description> | <status> | <test/log/artifact>

Failures or regressions: <specific behavior, assertion, and relevant error>
Limitations: <missing evidence, stale results, or unverified visual properties>
Next action: <fix, rerun, or needed user input>
```

Use FAIL for an established required-check failure; PARTIAL when some required
checks ran but others remain unverified; BLOCKED when required verification
cannot proceed; NOT RUN when it was not attempted. PASS requires evidence that
all requested checks completed successfully. A high pass rate alone is not PASS.

Keep evidence links as supplementary details. The text must be understandable
without opening images. Do not expose secrets or personal data from logs or
screenshots. Save a report file only if the user requests it or the repository
already has an appropriate reporting convention.
