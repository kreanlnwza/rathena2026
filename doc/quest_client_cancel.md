# Client quest cancellation

Author: V!be Coding [kreanlnwza] AI Assistant (Codex)

The optional `ClientCancel` boolean in `QUEST_DB` allows the client quest UI to
cancel a specific quest. New entries default to `false`. An import that omits
the field preserves the existing value; an explicit `false` revokes permission.
Invalid values reject the row, and permission is updated only after successful
row parsing. No existing quest is enabled automatically.

`quest_client_cancel(sd, quest_id)` returns `0` only when an owned quest in
`Q_ACTIVE` or `Q_INACTIVE` state is removed successfully. It returns `1` for
missing definitions, missing ownership, disabled permission, completed quests,
invalid states, or deletion failure. The existing `quest_delete` and NPC
`erasequest` behavior remain unchanged.

Cancellation uses the existing quest log update and save paths. The packet
handler sends the native `0x0C3E` cancellation acknowledgement; this path suppresses
the legacy `0x02B4` delete notification because the native acknowledgement needs
to look up the quest name before removing the client entry. Other callers retain
the default `quest_delete(..., notify_client = true)` behavior.

Cancellation does **not** reset NPC variables, linked quests, inventory items,
reward history, or cooldown records held elsewhere. Enabling a story or cooldown
quest without reviewing its scripts can leave the character stuck or allow
repeated rewards. Completed quests cannot be cancelled through this interface.

For a custom quest whose scripts have been reviewed and whose cancellation
requires only deleting its quest-log record, add an override to
`db/import/quest_db.yml`:

```yaml
Header:
  Type: QUEST_DB
  Version: 3

Body:
  - Id: 90000 # Replace with the ID of your reviewed, existing custom quest.
    ClientCancel: true
```

Use a dedicated NPC cancellation workflow when cancellation must update script
variables, consume or return items, check event times, or change other quests.
The presence of a delete button or cancellation message in client resources does
not grant server permission.

## Protocol evidence

MAIN builds use `PACKETVER_MAIN_NUM >= 20260514`, the build boundary reported in
https://github.com/rathena/rathena/issues/10057. This is a reported boundary, not
an independently verified earliest binary. Gravity documents live deployment on
May 20, 2026: https://ro.gnjoy.com/news/notice/View.asp?BBSMode=10001&seq=8241.
The global PACKETVER setting is not changed by this feature.

Static inspection of the local September 1, 2026 client, SHA-256
`4d5fe5a25c76c33147d56a001308c0ca3a34b97f97f257279364e9c9f4eec6fd`, confirms:

| Packet | Size | Fields after the uint16 opcode |
| --- | --- | --- |
| 0x0C3D request | 6 | int32 quest ID |
| 0x0C3E response | 8 | int32 quest ID, int16 result |

Request construction is at VA 0x5586B1; length registrations are at 0xC8E227
and 0xC8E239. Receive dispatch at 0x570877 reads quest ID at offset 2 and signed
result at offset 6. The result table at 0x5A3030 maps 0 to success (message 4396),
1 and 3 to failure (4397), 2 to a time requirement failure (4714), and 4 to a
generic message. This implementation sends 0 only after successful deletion,
and 1 otherwise. Requests during NPC interactions or blocked action states fail.

The client also needs the quest ID in `System/QuestEraseList.lub`; its list and
the server permission must agree. The preview-selection clear button is a
separate feature and does not delete quest records.

## Local verification setup

The local import enables only Hornet 11114, Condor 11115 and Worm Tail 11117.
Their Eden 11-25 script grants the hunt record without persistent NPC variables;
redemption replaces it with a separate cooldown record. Cooldown IDs 11124,
11125 and 11127 remain disabled. The client list retains all 575 original IDs
and adds these three IDs. Both local data files have diagnostic backups.

The local `db/import/quest_db.yml` and client binary data are not tracked in this
server commit. To reproduce the test permissions, merge these entries into the
`Body` of a `QUEST_DB` version 3 import, and add the same IDs to the client list:

```yaml
Body:
  - Id: 11114
    ClientCancel: true
  - Id: 11115
    ClientCancel: true
  - Id: 11117
    ClientCancel: true
```

An optional GM fixture avoids changing a character's level to accept the board:
`@loadnpc npc/custom/quest_cancel_test.txt`, then talk to Quest Cancel Test at
Prontera (156,180). It requires group ID 99 or higher, grants only one of these
three quests, and never replaces existing progress. The local
`npc/scripts_custom.conf` now auto-loads this GM fixture.
Close the NPC dialog before testing the client delete button. Reconnect after
deletion and confirm the quest stays absent; completing a quest or requesting
an unapproved quest must not remove it.

Build verification: Release x64 map-server links successfully. The executable
policy harness tests the current source function and YAML permission parsing;
it does not replace an end-to-end client click and persistence test.
