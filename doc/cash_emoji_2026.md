# Cash Emoji for the local 2026 MAIN client

Implemented separately from the storage baseline. The effective client resources
register three permanent packs, each priced at 10 Nyangvine_Fruit (Catnip,
item 6909). Pack 1 contains custom emojis 93–97 and five variants; packs 2 and 3
each contain ten variants. Variant entries reuse basic emotion IDs, with the
pack ID selecting their artwork. Their registration is in
`c_InsertEmotionVarientListTable`, not just `c_InsertEmotionListTable`.

| Pack | Allowed emotion IDs |
| --- | --- |
| 1 | 93, 94, 95, 96, 97, 29, 27, 38, 52, 1 |
| 2 | 15, 5, 33, 37, 0, 1, 2, 10, 11, 12 |
| 3 | 43, 39, 40, 44, 37, 42, 47, 14, 4, 56 |

Client metadata has no expiry for these packs. Pack 2 and 3 sale start dates
are 2023-08-29 and 2023-11-01; both precede this local 2026 client deployment.

## Behavior

Catalog entries are routed by `db/cash_emotion_db.yml` to
`db/re/cash_emotion_db.yml` or `db/pre-re/cash_emotion_db.yml`, followed by
the optional `db/import/cash_emotion_db.yml`. Edit `Id`, `Currency`, `Price`, `Registry`
and `Emotions`, then run `@reloadcashemotiondb`; catalog edits need no rebuild.
The default Pre-Renewal catalog is empty because `Nyangvine_Fruit` is Renewal-only.
`Currency` accepts an item AegisName such as `Nyangvine_Fruit`; numeric IDs
remain compatible with existing overrides. When omitted on a new pack, the
default is `ITEMID_NYANGVINE_FRUIT` from `itemdb.hpp` (6909). Unknown names
and items outside the protocol's 16-bit item-ID range reject the reload.
`Emotions` uses exported source constants such as `ET_SURPRISE`, `ET_KEK`,
and `ET_CUSTOM_1`; existing integer values are also accepted. Unknown names
are rejected. Native IDs 88..107 are explicitly added to the source enum,
preserving IDs 0..87. Incoming basic/legacy packets cannot use paid custom IDs.
Prices/currency and artwork IDs must also match the client tables. YAML does
not distribute new artwork or change the price displayed by the client.
Keep existing Registry keys stable: changing a key changes the entitlement
looked up for every account. Packs are account-wide and permanent.
Invalid rows or failed imports reject the entire reload and preserve the
active catalog. Correct the file before trying the reload command again.

- Map entry sends 0x0bf6 with server time, UTC offset and permanent owned packs.
- Purchase request 0x0bec is seven bytes: pack ID, payment item ID and quoted
  price. Native code confirms the final byte is price 10, not a sale-type flag.
- The server validates pack, currency, quote, Basic Skill, loaded registry,
  connection to char-server, player interaction state and available inventory.
- Success consumes exactly 10 Catnip across stacks, sets account variable
  `#CashEmotionPack1`, `#CashEmotionPack2` or `#CashEmotionPack3`, requests
  registry/inventory persistence, and sends 0x0bed for the purchased pack.
- Duplicate purchase returns 0x0bee result 2 without consuming another item.
- Unknown packs and mismatched currency/price return generic failure 255.
  Result 3 means an existing purchase under another sale type, so it is not
  appropriate for catalog validation failures.
- Paid 0x0be9 requests require ownership and a valid pack/emotion pair, preserve
  action/skill/flood restrictions, and broadcast 0x0bea with full 16-bit fields.
- Existing basic/legacy emotion handling remains available. Modern basic
  requests cannot wrap a 16-bit invalid ID into a valid byte.

Ownership is account-wide and permanent. No schema migration or player data
conversion is required. Save durability follows the existing rAthena inventory
and account-registry save paths; this does not add a cross-table SQL transaction.

## Verification

- Release x64 map-server build succeeded.
- `cash_emotion_test.cpp` includes the actual implementation and tests invalid
  quotes/items/packs, insufficient funds, registry refusal, interaction state,
  split stacks, duplicate purchase, ownership serialization and emotion rules.
- `test_cash_emoji_live.py` creates its own temporary account and character,
  connects through login/char/map servers, purchases, retries purchase, emits a
  paid emoji, checks SQL saved balance/ownership and verifies relogin restores
  ownership. The temporary character/account is removed after it is offline.
- Native diagnostic client confirms payment item 6909, quoted price 10 and
  permanent pack 1 appearing in the native ownership vector after initialization.
- User runtime evidence confirms pack 1 purchase and emotion 95 use.
- Three-pack hardcoded catalog passed the expanded harness and live purchase,
  duplicate, variant broadcast, SQL balance and relogin tests. The YAML build,
  database-backed handler harness and live `@reloadcashemotiondb` test pass
  with the ET_* catalog. Live invalid-import retention testing is pending.
  Visual purchase/use for packs 2 and 3 is pending.

## Files and rollback

Build `src/common/common.vcxproj` before `src/map/map-server.vcxproj` after
changing the common database interface. The local map project links
`.vs/build/common.lib`; a stale library is ABI-incompatible with the new header.

Implementation: `src/map/cash_emotion.inc`, included by `clif.cpp`, and packet
registration in `clif_packetdb.hpp`; the YAML database is in
`cash_emotion.hpp` / `cash_emotion.cpp`.

The Cash Emoji guard is `PACKETVER_MAIN_NUM >= 20230920`: the BF6 initialization
layout includes timezone from that date in the
[reference implementation](https://github.com/AoShinRO/brHades/blob/294bb5b7aab20c2b2f7ba53129ad4adb85841b66/src/map/packets.hpp#L1659).
Earlier implementations use BEF without timezone. This is source-backed layout
compatibility, not runtime verification of a 2023 executable. Runtime checks
here use the 2026 MAIN client. BE9 alone was registered in rAthena from
20230705; that date does not establish compatibility for the whole module.

Prior map binary is backed up at
`Z:\New folder\UnPACK\diagnostics\20260906\server_gameplay_backup\map-server.before-cash-emoji.exe`.
To roll back, stop map-server after players log out, restore that binary and
restart it from the repository root. Preserve account variables and inventory;
do not delete entitlements or restore an old database snapshot over purchases.
