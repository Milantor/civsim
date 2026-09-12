# AGENT RULES — civsim

Durable rules for anyone (agent or human) working in this repo: tooling, workflow,
code style, and the shape of the project. Transient/one-off task notes do NOT belong
here — this is the standing contract.

---

## Repo & build

- C++26. CMake + FetchContent: **raylib 5.5**, **raylib-cpp v5.5.0**, **EnTT v3.16.0**.
- Sources are auto-globbed from `src/**` (`GLOB_RECURSE` + `CONFIGURE_DEPENDS`).
  **Do NOT edit `CMakeLists.txt` to add/remove files** — just drop them under `src/`.
- Deps are already vendored in `build/_deps`; no re-fetch needed.
- Build (game only, recommended):
  ```sh
  make -C build civsim 2>&1 | tail -5
  ```
  Plain `make -C build` also builds the EnTT doxygen `docs` target — that spews
  doxygen warnings from `_deps/entt-src`, **not** from our code. Ignore those.
- If you ever delete `build/CMakeFiles/civsim.dir/*.make` (e.g. during a forced clean),
  reconfigure with `cmake -B build` before building again.

## Workflow (this environment)

- **Small increments.** ONE edit, then re-read the file, then build. Do not batch.
- **Verify after every write** — `nl -ba <file>` or `sed -n '1,10p' <file>`.
  Recover a bad edit with `git checkout -- <file>`.
- **One tool call at a time.** Large/batched tool calls have a habit of hanging.
- **Tool caveat:**
  - `edit_existing_file` has been observed **injecting assistant reasoning text into
    files** (it corrupted `src/main.hpp` before) — avoid it.
  - `single_find_and_replace` is usually reliable, but it has **silently mis-applied an
    edit and dropped lines** while reporting success. **Always re-read the file after
    editing** to confirm the change landed correctly.
- Prefer short commands, e.g. `make -C build civsim 2>&1 | tail -15`.

## Code style

- Indent with **4 spaces**.
- Keep the existing comment style: inline `//` notes inside functions, and section
  banners like `// --------- LEVEL MODEL ---------`. **Keep all comments and
  commented-out code** — they are history, don't delete them.
- Doxygen `/** ... @param ... @return ... */` on every **public** header entity
  (function, struct, enum). Internal helpers go in an `anonymous namespace` in the
  `.cpp`, not in the header.
- Naming: functions & constants `PascalCase`, variables `camelCase`, types `PascalCase`.
- Everything lives under `civsim::...`: `<module>::<domain>`. Never put project code in
  the global namespace. **No `using namespace` in headers.**
- **Includes live in HEADERS, not `.cpp`.** A `.cpp` should include only its own header
  (plus `<...>` std headers).
  - **Only exception:** `src/level/cluster.cpp` keeps `#include "level/level.hpp"`,
    because `cluster.hpp` only forward-declares `Level`.
- **raylib vs raylib-cpp:**
  - Use **`raylib::` types** for anything we **store, pass, or own** (`raylib::Window`,
    `raylib::Camera2D`, `raylib::Vector2`, `raylib::Texture2D`, `raylib::Image`, ...).
  - Use **raw C calls** for **one-shot** work only — poking the framebuffer or querying
    input/draw (`DrawTextureRec`, `ImageDraw`, `DrawText`, `IsKeyDown`, ...).
  - **Never** feed a raw C handle (`Texture2D`, `Image`, ...) into a `raylib::` parameter:
    raylib-cpp's implicit converting ctor silently builds an **owning temporary** that
    `Unload()`s at the end of the call expression (this once freed the atlas mid-frame →
    black screen). Own the resource as a `raylib::` object and pass it **by reference**.
  - Qualify one-shot calls whose raylib-cpp wrapper collides by ADL once an argument is a
    `raylib::` type (e.g. `raylib::ExportImage(...)` vs the C `ExportImage`).
- **Move-only / don't restructure.** Type and move discipline stays as is.

## Don't do (unless explicitly asked)

- Don't change logic, algorithms, or performance while "refactoring" — refactors are
  **move-only**.
- Don't touch `CMakeLists.txt`.
- Don't change `settings.hpp` tune values, and don't change the
  `namespace GameSettings = civsim::settings;` alias.
- Don't add features, caches, `unordered_map`s, etc. as part of a refactor.
- Don't rename things without a reason.

## Architecture map

```
src/
  main.cpp/.hpp      app boot; BuildAtlas; stubs BuildTileVBO/DrawTileVBO; UpdateDrawFrame
  camera.*           civsim::camera     — movement + stepped zoom
  settings.hpp       civsim::settings   — all tunables (inline constexpr)
  tile.hpp           civsim::tile       — TileType, TileInfo/SpriteInfo, TileInfos, GetTileName
  seed.*             civsim::seed       — MakeRandomSeed, MixSeed
  level/
    level.*          civsim::level::Level      — core model + At()
    cluster.*        civsim::level::cluster   — RockCluster, FindClusterByTile, MakeRockClusters
    cursor.*         civsim::level::cursor    — GetTileUnderCursor
    gen/{river,rocks,factory}.*   civsim::level::gen[:river|:rocks]
    io/{render,save}.*            civsim::level::io
    debug/tools.*                 civsim::level::debug
  balls.cpp/.hpp     fully commented-out dead ECS demo — ignore
```

Namespaces:
`civsim`, `civsim::camera`, `civsim::settings`, `civsim::tile`, `civsim::seed`,
`civsim::level`, `civsim::level::cluster`, `civsim::level::cursor`,
`civsim::level::gen`, `civsim::level::gen::river`, `civsim::level::gen::rocks`,
`civsim::level::io`, `civsim::level::debug`.

## Domain facts

- Map is **250×250**, row-major, one byte per tile.
  Tile bytes: **`Dirt 0, Water 1, Sand 2, Rock 3, Ore 4`**.
- Default recipe (`gen::GenerateDefaultLevel`): dirt base → river + sand banks → rock
  clusters. Same seed ⇒ same map (`seed::MixSeed` domains: `0x52495652` river,
  `0x524f434b` rocks).
- Controls: WASD move, mouse wheel / numpad `+`/`-` zoom, `space` = regenerate.
- Right-click a **Rock** tile → ~40% of that rock cluster becomes **Ore**.
- On window close: writes `build/bin/generatedlevel.dat` + `.png` (and `atlas.png`).
- `scripts/` and `balls.*` are dead — ignore.

## Known TODOs / backlog

- Texture **ATLAS + VBO** rendering to replace per-tile `io::DrawLevel` blitting.
  `main.cpp` has a half-working `BuildAtlas` and stubbed `BuildTileVBO` / `DrawTileVBO`.
- `tile::TileInfo` carries a nested `sprite{u0,v0,u1,v1,spriteFile}` **plus** a
  deprecated duplicate `spriteFile` on `TileInfo` itself — needs reworking. Open
  question: should `TileInfo` carry a `TileType` id field?
- `tile::TileInfos` is hardcoded — should be loaded from json.
- `io/save.hpp`: both `SaveLevelAsPng` overloads are flagged for rework; the
  `Texture2D atlas` overload has no definition (unused; latent link error if called).
- `main.cpp`: the `ANNIHILATE!` comment is a placeholder for a future GUI/text system.
