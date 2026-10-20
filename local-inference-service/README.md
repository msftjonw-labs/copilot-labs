# Local inference service

## Idea

“Using Windows containers to host local inference service” with GPU-accelerated workloads.

## Problem and intended audience

The user wants a test of mandatory Windows containers in the context of
Linux-based inference workloads. Proposal: evaluate whether an equivalent
inference workload can run in a native Windows container and compare it with a
Linux-container baseline. Confirm whether compatibility, performance, or both
are the primary objective. The intended audience remains open.

## Confirmed goals and constraints

- Explore GPU-accelerated inference hosted in Windows containers.
- Native Windows containers are mandatory; Linux containers on a Windows host
  are not a substitute for the test subject.
- GPU model, VRAM capacity, and Windows version have not been established.
- Use the root-level folder `local-inference-service`.
- The preferred branch is `copilot/local-inference-service`. Continuing on the
  assigned branch `copilot/brainstorm-inference-service-using-windows-contain`
  was explicitly approved; no branch rename is planned.
- This is a brainstorming space, not an implemented service.

## Options to explore

These are candidate models, not validated Windows-container configurations.
Model licensing, runtime support, quantization, GPU memory needs, and quality
must be checked before selection.

| Workload | Candidate models | Trade-offs |
| --- | --- | --- |
| Private chat and summarization | Phi-3.5 Mini; Qwen2.5 3B/7B Instruct | Smaller models reduce memory needs; larger models may improve quality but require more resources. |
| Coding assistance | Qwen2.5-Coder 7B | Useful for code explanation and generation; long contexts increase memory requirements. |
| Document retrieval | BGE-small; E5-small embeddings | A focused initial workload; document Q&A also needs retrieval infrastructure and a generation model. |
| Audio transcription | Whisper base or small | A measurable, focused service; model size trades resource usage against transcription quality. |

Proposal: start with a small embedding model and a fixed set of input texts to
isolate container/runtime feasibility before adding a chat model. No workload
or model has been selected.

### Hardware basics

- A GPU is the processor used to accelerate inference.
- VRAM is the GPU's dedicated memory. Model weights, intermediate data, and
  context or batches consume memory; model file size alone is not sufficient
  to determine capacity.
- Inventory the available machine before recommending hardware or model sizes.
  Do not assume that a GPU supported by one runtime works in Windows containers.

### Container and GPU feasibility

- Windows containers cannot directly run Linux container images or Linux
  binaries. The workload needs a Windows-compatible runtime and dependencies;
  reusing model artifacts also depends on runtime and format compatibility.
- Linux containers, including those hosted through WSL2, may be a comparison
  baseline but cannot satisfy the Windows-container requirement.
- Validate host OS, GPU, drivers, container base image, isolation mode, and GPU
  API support together before selecting an inference stack.
- Investigate ONNX Runtime with DirectML as a possible backend, without assuming
  that host support implies support inside the chosen Windows container.
- Do not assume CUDA-oriented serving stacks support native Windows containers.
- If GPU execution in the Windows container is unsupported, record it as a
  feasibility blocker rather than silently substituting host-native inference
  or CPU execution and calling the GPU test successful.

### Proposed comparison

First demonstrate actual GPU execution inside a Windows container. Then compare
against a Linux-container baseline using the same model revision, inputs,
precision, batch size, and equivalent hardware where feasible. Record differences
in runtime, backend, drivers, and host environment rather than attributing all
differences to the container OS.

Measure setup/support blockers, cold start, warmed latency, throughput, GPU
memory use, and output quality or numerical agreement. Report CPU fallbacks
explicitly. No hardware configuration or performance result is claimed.

## Open questions and next steps

1. Is an existing Windows machine available, or should this experiment target
   newly provisioned hardware? Inventory its OS and GPU before choosing a stack.
2. Does "Linux-based workloads" mean porting an existing service, benchmarking
   an equivalent model, or both? Is there an existing Linux baseline?
3. Which workload and intended users should be prioritized?
4. What latency, concurrency, context length, and quality targets matter?
5. Must operation be offline, and what authentication and data-handling boundaries are needed?
6. After those decisions, validate one supported GPU/runtime/container combination,
   then propose a small feasibility experiment with measured latency, memory use,
   and workload-specific quality. No experiment has been run.
