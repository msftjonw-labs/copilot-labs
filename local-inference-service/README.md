# Local inference service

## Idea

“Using Windows containers to host local inference service” with GPU-accelerated workloads.

## Problem and intended audience

Proposal: offer local inference behind a reusable service endpoint for applications
running on Windows, potentially reducing reliance on remote inference services.
The intended audience and primary use case remain open.

## Confirmed goals and constraints

- Explore GPU-accelerated inference hosted in Windows containers.
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

Proposal: start with document retrieval, then add a small chat model for private
document Q&A. No workload or model has been selected.

### Container and GPU feasibility

- Native Windows containers and Linux containers on a Windows host through WSL2
  are distinct deployment options; the latter is only an alternative if accepted.
- Validate host OS, GPU, drivers, container base image, isolation mode, and GPU
  API support together before selecting an inference stack.
- Investigate ONNX Runtime with DirectML as a possible backend, without assuming
  that host support implies support inside the chosen Windows container.
- Do not assume CUDA-oriented serving stacks support native Windows containers.
- Compare native Windows containers against a host-native service if container
  GPU support prevents the desired workload; this would require a scope decision.

## Open questions and next steps

1. Which Windows version, GPU model, and VRAM capacity are available?
2. Are native Windows containers mandatory, or are Linux containers on Windows acceptable?
3. Which workload and intended users should be prioritized?
4. What latency, concurrency, context length, and quality targets matter?
5. Must operation be offline, and what authentication and data-handling boundaries are needed?
6. After those decisions, validate one supported GPU/runtime/container combination,
   then propose a small feasibility experiment with measured latency, memory use,
   and workload-specific quality. No experiment has been run.
