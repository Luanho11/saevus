# SAEVUS asset pipeline — PROPOSAL

Status: Phase 6 draft (2026-10-03), **for the owner to approve or change**. Until it is approved, the project keeps using placeholders and engine content only (`CLAUDE.md`).

What is already verified on this machine is marked **(verified)**; everything else is a proposal.

## Constraints

- Look: stylized, painted, dark post-apocalyptic. Not photorealistic.
- Hardware: the dev machine is the reference (Vega 8, 7 GB RAM). Target: 60 fps at Medium, 720p, in real levels (`performance.md`: the empty Sandbox already costs ~13 ms GPU at Medium, so art must stay cheap).
- No Lumen, Nanite, ray tracing, Virtual Shadow Maps, MegaLights, heavy shaders.
- Art comes from the owner's Blender work. Any external source (store assets, commissioned work, generated content) needs the owner's explicit approval and a license entry (proposed `Docs/asset-licenses.md`).

## Shading approach

| Option | Idea | Cost | Verdict |
|---|---|---|---|
| **A. Default Lit + painted textures + light post-process** | Hand-painted base color (soft painted light/shadow shapes baked in), mostly rough surfaces, normal maps only where silhouette/readability needs them; color grading LUT; optional outline post-process behind a scalability switch | Same as any lit opaque material; LUT ≈ free; outline to be measured | **Recommended** |
| B. Unlit painted | All lighting painted into textures | Cheapest | No dynamic lights or shadows; fights combat readability (muzzle flashes, telegraphs) |
| C. Custom toon shading model | Ramp lighting in the deferred renderer | Needs an engine source build | Rejected (binary engine, `environment.md`) |

Option A rules of thumb:
- Roughness mostly 0.6–0.9; metallic only on real bare metal, sparingly. Specular highlights read as "realistic", which is the look we avoid.
- One master material per asset family; per-asset variation only through instances.
- Budget per opaque master material: ≤ 4 texture samples, base-pass instruction count kept low (check in the material stats; target to be fixed once the first masters exist). Masked/translucent materials only where needed; no translucency on large surfaces.
- Exponential height fog (already in the Sandbox) does the atmospheric depth; no volumetric fog.

## Budgets (LOD0, proposal)

| Asset class | Triangles | Texture sets | Max texture size | Material slots |
|---|---|---|---|---|
| Player character | 20–30 k | 1–2 | 2048 | ≤ 3 |
| Regular enemy | 10–20 k | 1 | 2048 | ≤ 2 |
| Boss | 40–60 k | 2 | 2048 | ≤ 4 |
| Weapon (first-person close-ups don't exist: third person only) | 2–6 k | 1 | 1024 | 1 |
| Prop (small / large) | 0.5–5 k | shared trim sheets where possible | 1024 / 2048 | 1–2 |
| Modular environment kit | per piece 0.2–4 k | 1–2 per kit (trim sheets) | 2048 | 1–2 |

Scene targets at Medium, 720p: < 1,500 draw calls and < 1.5 M triangles in view; texture streaming pool 512 MB (shared memory on the iGPU). To be confirmed with the first real level, using `Tools/Run-SmokeTest.ps1` style measurements.

## LODs and culling

- Characters: 3 LODs (100 / 50 / 25 % triangles), generated with UE's built-in reduction, screen sizes ≈ 1.0 / 0.5 / 0.25.
- Props: 2–3 LODs by size; small props get cull distances.
- Modular pieces: 1–2 LODs; rely on culling and merging (HLOD later if levels get large).

## Gameplay metrics for level art

Level geometry must fit the movement built in Phase 3 (`movement.md`). Current first-pass values:

| Metric | Value |
|---|---|
| Ground jump apex | 1.43 m |
| Step-up height (CMC `MaxStepHeight`) | 0.45 m |
| Walkable slope | < 44.8° |
| Wall surfaces for wall run / jump / climb | flat, steeper than ~72° (normal Z ≤ 0.3) |
| Wall run length at full duration | ~9 m (750 cm/s × 1.2 s) |
| Wall climb height | ~3.4 m (plus the jump before it; a 3.5 m wall can be topped) |
| Wall-jump chimney gap | ~3 m |

Collision: simple collision only for anything the character touches (boxes / convex hulls: `UCX_` meshes from Blender, or UE auto-convex). No complex-as-simple on gameplay geometry; walls meant for wall movement need a single flat collision face.

## Blender → Unreal

Verified with `Tools/Pipeline/Run-PipelineProbe.ps1`: Blender 4.2 exports a probe mesh; UE 5.8 imports it (Interchange for both FBX and glTF) and logs its bounds; the scratch import is deleted afterwards.

**Verified (static mesh):**
- Blender scene: metric, unit scale 1.0, model at real size in metres.
- FBX export with Blender's default axes (Forward −Z, Up Y), *Apply Unit* on, scale option *FBX Units Scale None*, face smoothing: a 1.8 m Blender object arrives as 180 cm, standing on Z = 0.
- glTF (GLB) export with defaults gives the same result.
- **Facing:** a character facing −Y in Blender (Blender's Front view) faces **+Y** in UE, the same as the engine mannequin, so `ASAEVUSCharacter`'s −90° mesh yaw correction works unchanged.

**Proposed (to verify with a skeletal probe before the first character):**
- Origin at the feet centre for characters and props; on the grid snap point (0.5 m / 1 m) for modular pieces. Apply all transforms before export.
- Skeletal meshes: FBX; armature at scale 1, deform bones only, no leaf bones; retarget to the UE5 mannequin skeleton (IK Retargeter) so engine template animations keep working until custom ones exist.
- Source files (`.blend`, `.psd`, `.png` sources) in `SourceArt/`, outside `Content/`. Those extensions are already LFS-tracked by `.gitattributes`.

## Naming and folders

Folder per asset: `/Game/SAEVUS/Art/<Category>/<AssetName>/` (`Characters`, `Enemies`, `Weapons`, `Props`, `Environment/<Kit>`).

| Prefix | Asset | Suffix (textures) | Content |
|---|---|---|---|
| `SM_` | static mesh | `_BC` | base color (sRGB) |
| `SK_` / `SKEL_` / `PHYS_` | skeletal mesh / skeleton / physics asset | `_N` | normal (linear, BC5) |
| `ABP_` / `AM_` / `BS_` | anim blueprint / montage / blend space | `_ORM` | occlusion, roughness, metallic packed (linear) |
| `M_` / `MI_` / `MF_` | master material / instance / function | `_M` | masks (linear) |
| `T_` | texture | `_E` | emissive (sRGB) |
| `PP_` | post-process material | | |

Textures: power-of-two sizes, base color BC1 (BC3/BC7 only if alpha is needed), normals BC5, packed masks BC1 or BC7 depending on quality checks. Texture groups cap sizes per class (Character 2048, Weapon 1024, World 2048).

## Animation (preview; its own work item)

- Locomotion stays in-place and CMC-driven (no root motion), matching the movement code.
- Attacks are timeline-driven (`combat.md`): animations must match each attack's startup/active/recovery durations, or the timeline gets its durations from the montage. Decision for the owner when the first attack animations exist.
- Wall run, wall climb and the flip need their own poses; the template AnimBP doesn't know the custom movement modes.

## Validation tooling (next)

- `Run-PipelineProbe.ps1` (done) — extend with a skeletal probe.
- An editor data-validation rule set: naming prefixes, triangle and texture budgets per folder, collision present on gameplay meshes.

## Decisions for the owner

1. Shading option A (recommended), B or C?
2. Budgets in the table: accept, raise or lower?
3. Retarget characters to the UE5 mannequin skeleton (keeps template animations), or a custom skeleton from the start?
4. FBX for everything, or glTF for static meshes (both verified)?
5. Which external sources, if any, are acceptable later (store assets, commissions, generated content)?
6. Should Phase 6 move earlier than PvE (the roadmap allows it if art becomes the bottleneck)?
