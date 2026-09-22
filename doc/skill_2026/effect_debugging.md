# Skill visual-effect follow-up

The reported client executable date is `20260901`. A specific failing skill,
whether its damage/buff works, and the server executable used for the report
have not yet been supplied. The findings below distinguish source defects from
client behavior that still requires reproduction.

## Missing cast notifications

Several new damage handlers only called `skill_attack`, which sends
`ZC_NOTIFY_SKILL` through `clif_skill_damage`. Comparable existing handlers also
send `ZC_USE_SKILL` through `clif_skill_nodamage` to notify clients of skill use.
Examples include Frenzy Shot, Effligo, Servant Weapon Phantom/Demolition,
Elemental Buster, Gale Storm, Rhythm Shooting, and Shield Chain Rush.

The affected new handlers are 6608, 6611, 6613, 6616-6622, 6624, 6626-6630,
6633, 6635-6638, and 6642, plus the rebalance of Servant Weapon Sign. Their
missing cast notification is the targeted correction. No global change to
`WeaponSkillImpl`, `skill_attack`, or packet layouts is needed.

For splash attacks the notification belongs to the outer cast, before target
traversal. Sending it inside the per-target damage callback would duplicate
animation notifications. Seventh Kick chooses the normal or enhanced ID
before notification; Field of Kirin deliberately uses the caster as the
animation target because its damage area is centered there.

Shield Slam, Overdrive Protocol, Venom Ignition, Wraith Dash, Elemental
Integration's buff, and Broken Heaven already send a cast notification. If
these also have no visual effect, the missing notifications alone do not
explain that behavior. Whether Rampant Vine and Lex Expiatrix specifically
require the additional packet is inferred from related handlers, rather than
verified from a client capture.

## Regression cases

Before the correction, tracing the affected handlers reaches a damage packet
without a cast notification. After the correction, check the following on the
reported client. These are manual packet/gameplay cases, not completed tests.

| Case | Expected result |
| --- | --- |
| Phantom Dagger against one enemy | One successful cast notification with ID 6613 and the requested level, followed by the existing damage. |
| Elemental Integration proc or a splash skill against three enemies | One cast notification per receiving client; the original damage/hit count per enemy is unchanged. |
| Seventh Kick without a completed orbit | One cast notification for 6635. |
| Seventh Kick with a completed orbit | One cast notification for 6636, with no extra 6635 notification. |
| Field of Kirin | Cast notification targets the caster; the existing damage search remains centered on the caster. |
| Failed prerequisite or rejected Servant Weapon Sign target | No successful cast notification and no additional damage. |
| A successful cast that misses | A cast notification still occurs; damage and satellite gain follow the existing hit rules. |
| Shield Slam as an existing-notification control | No extra notification is introduced. |

Count notifications per receiving client, rather than counting calls to the
underlying send function: area broadcasts and disguise handling can route one
logical notification through multiple sends.

## Build and deployment observations

The supplied GRF contains dedicated effect resources for the new skill groups.
Primed Trap attacks and Mirage Swarm reuse family resources. No `HideEffect`
or equivalent selector was found in their primary skill metadata. The extracted
`skilleffectinfolist.lub` lists only 66 older skill IDs and none of 6608-6643,
so that table cannot establish how the reported executable dispatches them.
The local reference `Ragexe.exe` has PE timestamp 2026-09-16 08:45:34 UTC,
different from the reported client date. This difference does not prove whether
the user's executable supports the new effects; its actual path/data set and
runtime behavior remain unconfirmed.

The local source defaults to `PACKETVER 20211103` and the inspected MSBuild
configuration has no override. For the two packets involved here, that version
and `20260901` select the same layouts: `0x01de ZC_NOTIFY_SKILL` and
`0x09cb ZC_USE_SKILL`. This does not establish compatibility for every other
packet, but the version difference alone does not explain this visual path.

At the start of this investigation, `map-server.exe` in the repository root
was dated 2026-09-09. The previous skill implementation was built to
`.vs/build/map-server.exe`, dated 2026-09-22. Using the old executable would
omit the new handlers entirely. The executable used in the user's test remains
unconfirmed; no running `map-server.exe` process was found on this Windows host.

## Completed verification

- Main and expanded notification fixes passed an independent source review:
  no per-target AoE duplicate, correct normal/enhanced Seventh Kick ID, and
  existing prerequisite rejection before notification.
- The updated `map-server.vcxproj` built successfully as `Release|x64` with
  zero warnings and zero errors. The build log is
  `.vs/skills-2026-effects-build.log`; the executable is
  `.vs/build/map-server.exe`.
  This local build has SHA-256
  `d7375cfbab9fa45387231a6aab97c43c7251d915427919a141a9ebf89e7eabd4`.
- MSBuild dependency tracking includes the directly included handler `.cpp`
  files in their owning job factories. The generator intentionally excludes
  concrete handlers and does not need rebuilding for these FX changes.
- All 49 task-added Author labels in 47 files now read
  `kreanlnwza Ai assistant Code [Astra].`; the previous label is absent.
- Actual effect rendering and damage on client `20260901` have not been
  verified. The manual regression cases above remain open.
