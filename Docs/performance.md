# SAEVUS performance

Status: Phase 1 baseline. Measured on the dev machine, which is also the reference "modest" target.

## How to measure

```
powershell -ExecutionPolicy Bypass -File Tools\Run-SmokeTest.ps1 [-Scalability 0..3] [-ExtraCmds "sg.ShadowQuality 1"] [-ProfileGPU]
```

`USAEVUSSmokeTestSubsystem` (`-SAEVUSSmokeTest`) runs, sprints and jumps on its own in `L_Sandbox`, then logs one summary line. Frame timing is sampled only during the run/sprint/jump stages (spawn and settle hitches excluded):

- **average / worst** — wall-clock frame time;
- **game / render** — `GGameThreadTime` / `GRenderThreadTime`;
- **GPU** — `RHIGetGPUFrameCycles()`.

`-ProfileGPU` also captures one `ProfileGPU` frame mid-sprint (per-pass GPU timings in `Saved\Logs\Verify\smoke.log`).

The first run after a build or a cleared DDC is not a valid sample: shader and asset loading give 400 ms hitches and inflate game time (seen: game 27.8 ms cold vs 8.9 ms warm). Run twice and keep the second.

## Baseline (2026-10-03)

Machine: Ryzen 5 2500U, Vega 8 (driver 27.20.22002.57), 7 GB RAM. 1280 × 720 windowed, DX12 SM6, `L_Sandbox`, TAA, no Lumen / Nanite / RT / VSM. Warm cache.

| Scalability | Frame (ms) | fps | Game (ms) | GPU (ms) | Runs |
|---|---|---|---|---|---|
| 0 Low | 8.5 | 118 | 7.5 | 6.4 | 1 |
| 1 Medium | 14.7 – 18.1 | 55 – 68 | 7.4 – 8.9 | 12.9 – 13.8 | 3 |
| 2 High | 20.4 – 22.4 (one outlier 27.5) | 45 – 49 | 7.1 – 8.4 | 18.5 – 19.6 (24.2) | 3 |
| 3 Epic | 27.2 | 37 | 6.7 | 24.5 | 1 |
| none (saved user settings = Epic) | 29.1 | 34 | 7.1 | 25.4 | 1 |

High with one group dropped to Medium (single runs, noise is about ±1 ms):

| Group lowered | Frame (ms) | GPU (ms) |
|---|---|---|
| EffectsQuality | 19.0 | 17.3 |
| PostProcessQuality | 20.1 | 18.2 |
| ReflectionQuality | 20.3 | 18.5 |
| ShadingQuality | 20.4 | 18.4 |
| ShadowQuality | 20.6 | 17.9 |
| GlobalIlluminationQuality | 21.3 | 18.8 |

Forcing native resolution (`sg.ResolutionQuality 100`): High 22.3 ms (no change), Medium 20.2 – 23.0 ms (up from ~15).

## Reading

- **GPU-bound.** Game thread is a flat 7–9 ms at every level. The render-thread number tracks GPU time + ~2 ms, consistent with it waiting on the GPU.
- **Resolution is the big lever.** No single scalability group explains the High → Medium gap (each saves ≤ 1.5 ms). Medium's gain comes from rendering below native resolution; at native, Medium costs about the same as High.
- **Medium is the 60 fps tier on this machine** at 720p, with little headroom (55–68 fps across runs; the laptop's frame time varies between runs, likely thermal / shared-memory). High holds ~45 fps.
- The game thread already costs ~8 ms with one character and an empty map. Watch it as gameplay systems are added; it becomes the limit once the GPU side is cut further.
- The Sandbox is trivial geometry, so these numbers are a floor, not a budget for real levels.

## Not decided yet

- Default scalability / resolution for shipped settings, and whether to run the engine hardware benchmark on first launch. That is a settings-menu feature, not Phase 1.
- Per-pass GPU breakdown (`-ProfileGPU`) is available but not recorded; worth doing when the art direction (Phase 6) defines the shading cost.
