# 2026 kRO skill evidence

This directory records the client-side evidence used for skill IDs 6606 through
6643. It deliberately separates values present in the primary client data from
server behavior that cannot be recovered from those files.

## Scope

- The covered range contains 38 records. `AT_NATURE_AID` (6606) and
  `AT_NATURE_HARMONY` (6607) came from rAthena PR 9765; this change adds the
  remaining 36 database records.
- Six records are internal triggers: 6616 through 6620 and 6636.
- Four Primed Trap attacks (6626 through 6629) are visible in the Windhawk
  client tree, have `WH_PRIMED_TRAP` level 1 as their client prerequisite, and
  are usable only while the Primed Trap status is active.
- IDs 6644 and 6645 are absent from the primary `skillid` table.

## Sources and priority

Primary evidence is the supplied `Z:\KR_RO1_Live_20260401_155632\data.grf`.
Its filesystem modification time is 2026-09-16; that timestamp is not an
independently verified client release date. The archive is read-only and was
decoded with the local GPakEx 0x80 capable GRF tooling. Exact
archive metadata, extracted member sizes, SHA-256 hashes, client costs, ranges,
prerequisites, delays, and EFST IDs are in [manifest.json](manifest.json).

The public design note is supporting evidence:

- <https://ro.gnjoy.com/news/devnote/View.asp?category=1&seq=4200579&curpage=1>

When the design note and the supplied client differ, the supplied client wins
for IDs, SP/AP, ranges, cast delays, cooldowns, and prerequisites. For example,
the primary client has Elemental Integration duration 240 seconds and cooldown
3 seconds, and Mirage Swarm consumes 30 AP.

## Confirmed structure

- `MT_OVERDRIVE_PROTOCAL` is the exact client spelling.
- Elemental Integration invokes five hidden elemental attacks (6616-6620).
- Seventh Kick invokes hidden all-satellites variant 6636.
- Broken Heaven reuses the existing Second Brand/Judgement mark for 5 seconds;
  the primary client does not define a new judgement-mark EFST.
- Mirage Swarm reuses the `SHINKIROU` unit with three active instances.
- Nature Harmony requires Truth of Ice, Truth of Wind, and Truth of Earth at
  level 1.
- Field of Kirin is a range-9 single-target cast whose damage is centered on
  the caster; the Korean description contains both facts explicitly.
- Breaking Limit raises Wind Cutter Turbo damage by 120% and High Magnum Break
  damage by 100%; the existing status lasts 300 seconds.
- The current rebalance changes Servant Weapon Sign into a critical weapon hit
  and reduces each Acidified Zone bottle cost from two to one.

## Seventh Kick external evidence

The [official demonstration](https://imgc.gnjoy.com/Bbs/Editor/2026/06/30/7f4a74eb1a5a41eebe7b75f875ef4cbf104239.gif)
in the design note shows five lit satellites, then six and seven after successive
normal Seventh Kick hits. The following cast displays the enhanced skill name
and clears the orbit. Frames 0, 8, 80, and 144 show those states respectively.
The GIF SHA-256 is
`fee66193d283666fefee5d8cef20ffcd5d5a6fcf7446639aee5a6b331ce8b86f`.

A [first-hand player demonstration at 23:10](https://www.youtube.com/watch?v=ZPWKxHcLaqA&t=1390s)
describes the satellites as lasting 10 seconds with refresh. This supports the
GRF's 10-second duration and a refreshed timer on each successful normal hit.
[Divine Pride 6635](https://www.divine-pride.net/database/skill/6635) and
[6636](https://www.divine-pride.net/database/skill/6636) corroborate the normal
and enhanced descriptions and ratios, but do not document further state rules.
Their incomplete metadata does not override the primary client arrays.

The implementation counts successful normal hits up to seven, including
killing blows but excluding Cicada/Soul Shadow blocks, refreshes the 10-second
timer, and invokes skill 6636 on the next
cast after reaching seven. Expiry clears the ready state. Consuming the orbit
at cast time even on a miss, using the same duration for the ready state, and
sending the count in status `val1` remain implementation choices; no packet
capture or dedicated miss test establishes those details. Dispel behavior and
direct scripted casts of the hidden skill have not been verified against kRO.

## Unknown or server-derived behavior

The client descriptions name Base Level and trait-stat scaling but do not encode
their numeric coefficients or Base Level normalization. The main-class handlers
retain independently supported ratio/mastery terms and omit an unsupported
trait coefficient. The expanded-class handlers currently use the rAthena-style
inference `ratio += 5 * POW/CON/SPL` followed by `BaseLevel / 100`; that choice
is implementation-derived and was not recovered from this GRF.

The GRF also does not define:

- the satellite count or transition rule between `EFST_SEVENTH_KICK_SKILLORB`
  and `EFST_SEVENTH_KICK_MAX`; the external observations above supply these;
- the exact Mirage Swarm placement offsets. Its description does explicitly
  require failure when a placement cell is blocked. The implementation checks
  three fixed positions relative to the caster before replacing existing mirages;
- a mapping from the four Primed attack levels to the four older trap levels;
- a duration for the Thundering charge added by Nature Rage under Truth of Wind;
- numeric POW, CON, or SPL coefficients mentioned only qualitatively.

Broken Heaven says to heal `3 * skill level` percent of final damage (3-15%),
capped at 50,000, but does not
state whether an area hit shares one cap or applies it per damaged target. The
current per-target callback applies the cap to each damaged target; this remains
a server-behavior ambiguity rather than a client-proven aggregation rule.

`skilltreeview` and `skillinfolist` show the four Primed attacks as level-5
learnable skills gated by Primed Trap level 1. Granting temporary levels copied
from the older traps would be an inference and is not supported by the primary
files.

## Reproduction

`tools/skill_2026/ExtractClientEvidence.cs` extracts only the listed client
members. `DecompileLub.cs` converts those LUB members to bounded Lua text.
`build_manifest.py` reads the decoded files and writes the JSON document to
standard output. The archive itself is never modified.

Example manifest rebuild after extraction:

```powershell
python tools/skill_2026/build_manifest.py `
  "Z:\New folder\skill2026-primary-out"
```

After source integration, validate DB uniqueness, all 38 enum mappings,
`MAX_SKILL` capacity, the eight SC/EFST exports, every new skill status
reference, client arrays, and tree prerequisites:

```powershell
python -X utf8 -B tools/skill_2026/validate_manifest.py --source-root .
python -X utf8 -B -m unittest discover -s tools/skill_2026 -p test_validate_manifest.py
```

## Validation

- The manifest validator passed for all 38 records: 32 tree-visible and six
  internal triggers. The three negative validator tests also passed.
- MSBuild `Release|x64` builds of `map-server.vcxproj` and
  `map-server-generator.vcxproj` passed with zero warnings and zero errors.
  Outputs and logs are kept in the ignored `.vs` directory.
- A focused lifecycle review covered Seven Kick hit/miss handling, killing
  blows, defensive blocks, refresh, enhanced dispatch, and expiry. This is
  source review, not a live gameplay test.
- No live SQL startup or in-game verification was performed. Damage
  coefficients and state details identified above still need comparison with
  measured kRO behavior.
